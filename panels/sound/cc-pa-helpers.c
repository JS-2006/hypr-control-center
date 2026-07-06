#include "config.h"

#include <gio/gio.h>
#include <glib.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include "cc-pa-helpers.h"

struct _CcPAContext {
  pa_glib_mainloop *glib_mainloop;
  pa_context *context;
  gboolean connected;
  GMainLoop *wait_loop;
  gboolean wait_result;
};

static gboolean
timeout_cb (gpointer user_data)
{
  GMainLoop *loop = user_data;

  g_main_loop_quit (loop);
  return G_SOURCE_REMOVE;
}

static void
context_state_cb (pa_context *c, void *userdata)
{
  CcPAContext *ctx = userdata;

  ctx->wait_result = (pa_context_get_state (c) == PA_CONTEXT_READY);
  if (pa_context_get_state (c) == PA_CONTEXT_READY ||
      pa_context_get_state (c) == PA_CONTEXT_FAILED) {
    g_main_loop_quit (ctx->wait_loop);
  }
}

static gboolean
wait_for_result (CcPAContext *ctx, GError **error)
{
  guint timeout;

  timeout = g_timeout_add (10000, timeout_cb, ctx->wait_loop);
  g_main_loop_run (ctx->wait_loop);
  g_source_remove (timeout);

  if (!ctx->wait_result) {
    g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                 "PulseAudio operation failed");
    return FALSE;
  }

  return TRUE;
}

CcPAContext *
cc_pa_context_new (void)
{
  CcPAContext *ctx;

  ctx = g_new0 (CcPAContext, 1);
  ctx->glib_mainloop = pa_glib_mainloop_new (g_main_context_default ());
  ctx->context = pa_context_new (pa_glib_mainloop_get_api (ctx->glib_mainloop),
                                 "hypr-control-center");
  ctx->wait_loop = g_main_loop_new (g_main_context_default (), FALSE);
  ctx->connected = FALSE;

  return ctx;
}

void
cc_pa_context_free (CcPAContext *ctx)
{
  if (!ctx)
    return;

  if (ctx->context) {
    pa_context_disconnect (ctx->context);
    pa_context_unref (ctx->context);
  }
  if (ctx->glib_mainloop)
    pa_glib_mainloop_free (ctx->glib_mainloop);

  if (ctx->wait_loop)
    g_main_loop_unref (ctx->wait_loop);

  g_free (ctx);
}

void
cc_pa_device_info_free (CcPADeviceInfo *info)
{
  if (!info)
    return;

  g_free (info->name);
  g_free (info->description);
  g_free (info);
}

gboolean
cc_pa_context_connect (CcPAContext *ctx, GError **error)
{
  pa_context_set_state_callback (ctx->context, context_state_cb, ctx);

  if (pa_context_connect (ctx->context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
    g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Failed to connect: %s",
                 pa_strerror (pa_context_errno (ctx->context)));
    return FALSE;
  }

  return wait_for_result (ctx, error);
}

/* --- Device enumeration callbacks --- */

typedef struct {
  GPtrArray *devices;
  guint32 default_index;
  CcPAContext *ctx;
} EnumState;

static void
sink_info_cb (pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
  EnumState *state = userdata;
  CcPADeviceInfo *info;

  if (eol > 0 || eol < 0) {
    g_main_loop_quit (state->ctx->wait_loop);
    return;
  }

  info = g_new0 (CcPADeviceInfo, 1);
  info->index = i->index;
  info->name = g_strdup (i->name);
  info->description = g_strdup (i->description);
  info->is_default = (i->index == state->default_index);
  info->volume = pa_cvolume_avg (&i->volume);
  info->muted = i->mute;

  g_ptr_array_add (state->devices, info);
}

static void
server_info_sink_cb (pa_context *c, const pa_server_info *info, void *userdata)
{
  EnumState *state = userdata;

  if (!info) {
    g_main_loop_quit (state->ctx->wait_loop);
    return;
  }

  state->default_index = PA_INVALID_INDEX;
  pa_context_get_sink_info_by_name (c, info->default_sink_name,
                                    sink_info_cb, state);
}

static GPtrArray *
get_sinks (CcPAContext *ctx)
{
  EnumState state;

  state.devices = g_ptr_array_new_with_free_func ((GDestroyNotify) cc_pa_device_info_free);
  state.default_index = PA_INVALID_INDEX;
  state.ctx = ctx;
  ctx->wait_result = FALSE;

  pa_context_get_server_info (ctx->context, server_info_sink_cb, &state);

  {
    guint timeout = g_timeout_add (10000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }

  return state.devices;
}

static void
source_info_cb (pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
  EnumState *state = userdata;
  CcPADeviceInfo *info;

  if (eol > 0 || eol < 0) {
    g_main_loop_quit (state->ctx->wait_loop);
    return;
  }

  if (i->monitor_of_sink != PA_INVALID_INDEX)
    return;

  info = g_new0 (CcPADeviceInfo, 1);
  info->index = i->index;
  info->name = g_strdup (i->name);
  info->description = g_strdup (i->description);
  info->is_default = (i->index == state->default_index);
  info->volume = pa_cvolume_avg (&i->volume);
  info->muted = i->mute;

  g_ptr_array_add (state->devices, info);
}

static void
server_info_source_cb (pa_context *c, const pa_server_info *info, void *userdata)
{
  EnumState *state = userdata;

  if (!info) {
    g_main_loop_quit (state->ctx->wait_loop);
    return;
  }

  state->default_index = PA_INVALID_INDEX;
  pa_context_get_source_info_list (c, source_info_cb, state);
}

static GPtrArray *
get_sources (CcPAContext *ctx)
{
  EnumState state;

  state.devices = g_ptr_array_new_with_free_func ((GDestroyNotify) cc_pa_device_info_free);
  state.default_index = PA_INVALID_INDEX;
  state.ctx = ctx;
  ctx->wait_result = FALSE;

  pa_context_get_server_info (ctx->context, server_info_source_cb, &state);

  {
    guint timeout = g_timeout_add (10000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }

  return state.devices;
}

GPtrArray *
cc_pa_context_get_sinks (CcPAContext *ctx)
{
  return get_sinks (ctx);
}

GPtrArray *
cc_pa_context_get_sources (CcPAContext *ctx)
{
  return get_sources (ctx);
}

static void
noop_success_cb (pa_context *c, int success, void *userdata)
{
  CcPAContext *ctx = userdata;

  ctx->wait_result = success;
  g_main_loop_quit (ctx->wait_loop);
}

void
cc_pa_context_set_sink_volume (CcPAContext *ctx, guint32 index, pa_volume_t volume)
{
  pa_cvolume cv;

  ctx->wait_result = FALSE;
  pa_cvolume_set (&cv, 1, volume);
  pa_context_set_sink_volume_by_index (ctx->context, index, &cv,
                                       noop_success_cb, ctx);
  {
    guint timeout = g_timeout_add (5000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }
}

void
cc_pa_context_set_sink_mute (CcPAContext *ctx, guint32 index, gboolean mute)
{
  ctx->wait_result = FALSE;
  pa_context_set_sink_mute_by_index (ctx->context, index, mute,
                                     noop_success_cb, ctx);
  {
    guint timeout = g_timeout_add (5000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }
}

void
cc_pa_context_set_source_volume (CcPAContext *ctx, guint32 index, pa_volume_t volume)
{
  pa_cvolume cv;

  ctx->wait_result = FALSE;
  pa_cvolume_set (&cv, 1, volume);
  pa_context_set_source_volume_by_index (ctx->context, index, &cv,
                                         noop_success_cb, ctx);
  {
    guint timeout = g_timeout_add (5000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }
}

void
cc_pa_context_set_source_mute (CcPAContext *ctx, guint32 index, gboolean mute)
{
  ctx->wait_result = FALSE;
  pa_context_set_source_mute_by_index (ctx->context, index, mute,
                                       noop_success_cb, ctx);
  {
    guint timeout = g_timeout_add (5000, timeout_cb, ctx->wait_loop);
    g_main_loop_run (ctx->wait_loop);
    g_source_remove (timeout);
  }
}
