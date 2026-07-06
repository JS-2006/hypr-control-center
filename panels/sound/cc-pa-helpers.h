#pragma once

#include <glib.h>
#include <pulse/pulseaudio.h>

G_BEGIN_DECLS

typedef struct _CcPAContext CcPAContext;

typedef struct {
  guint32 index;
  char *name;
  char *description;
  gboolean is_default;
  pa_volume_t volume;
  gboolean muted;
} CcPADeviceInfo;

CcPAContext *cc_pa_context_new        (void);
void         cc_pa_context_free       (CcPAContext *ctx);
gboolean     cc_pa_context_connect    (CcPAContext *ctx, GError **error);

GPtrArray   *cc_pa_context_get_sinks   (CcPAContext *ctx);
GPtrArray   *cc_pa_context_get_sources (CcPAContext *ctx);

void cc_pa_context_set_sink_volume   (CcPAContext *ctx, guint32 index, pa_volume_t volume);
void cc_pa_context_set_sink_mute     (CcPAContext *ctx, guint32 index, gboolean mute);

void cc_pa_context_set_source_volume (CcPAContext *ctx, guint32 index, pa_volume_t volume);
void cc_pa_context_set_source_mute   (CcPAContext *ctx, guint32 index, gboolean mute);

void         cc_pa_device_info_free  (CcPADeviceInfo *info);

G_END_DECLS
