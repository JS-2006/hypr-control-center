#include "config.h"

#include <adwaita.h>
#include <glib/gi18n.h>

#include "cc-pa-helpers.h"
#include "cc-sound-panel.h"
#include "cc-sound-resources.h"

struct _CcSoundPanel {
  CcPanel parent_instance;

  CcPAContext *pa;

  AdwComboRow *output_device_row;
  GtkScale *output_scale;
  AdwComboRow *input_device_row;
  GtkScale *input_scale;

  GPtrArray *sinks;
  GPtrArray *sources;

  gboolean updating;
};

CC_PANEL_REGISTER (CcSoundPanel, cc_sound_panel)

static void
free_device_array (GPtrArray **arr)
{
  if (*arr) {
    g_ptr_array_unref (*arr);
    *arr = NULL;
  }
}

static void
refresh_devices (CcSoundPanel *self)
{
  GtkStringList *sink_list, *source_list;
  guint active_sink, active_source;
  guint i;

  free_device_array (&self->sinks);
  free_device_array (&self->sources);

  self->sinks = cc_pa_context_get_sinks (self->pa);
  self->sources = cc_pa_context_get_sources (self->pa);

  sink_list = gtk_string_list_new (NULL);
  active_sink = 0;
  for (i = 0; i < self->sinks->len; i++) {
    CcPADeviceInfo *info = g_ptr_array_index (self->sinks, i);
    gtk_string_list_append (sink_list, info->description);
    if (info->is_default)
      active_sink = i;
  }

  source_list = gtk_string_list_new (NULL);
  active_source = 0;
  for (i = 0; i < self->sources->len; i++) {
    CcPADeviceInfo *info = g_ptr_array_index (self->sources, i);
    gtk_string_list_append (source_list, info->description);
    if (info->is_default)
      active_source = i;
  }

  self->updating = TRUE;

  adw_combo_row_set_model (self->output_device_row, G_LIST_MODEL (sink_list));
  if (active_sink < self->sinks->len)
    adw_combo_row_set_selected (self->output_device_row, active_sink);

  adw_combo_row_set_model (self->input_device_row, G_LIST_MODEL (source_list));
  if (active_source < self->sources->len)
    adw_combo_row_set_selected (self->input_device_row, active_source);

  if (active_sink < self->sinks->len) {
    CcPADeviceInfo *info = g_ptr_array_index (self->sinks, active_sink);
    gtk_range_set_range (GTK_RANGE (self->output_scale), 0.0, 1.0);
    gtk_range_set_value (GTK_RANGE (self->output_scale), (gdouble) info->volume / PA_VOLUME_NORM);
  }

  if (active_source < self->sources->len) {
    CcPADeviceInfo *info = g_ptr_array_index (self->sources, active_source);
    gtk_range_set_range (GTK_RANGE (self->input_scale), 0.0, 1.0);
    gtk_range_set_value (GTK_RANGE (self->input_scale), (gdouble) info->volume / PA_VOLUME_NORM);
  }

  self->updating = FALSE;
}

static void
on_output_volume_changed (GtkScale *scale, gpointer user_data)
{
  CcSoundPanel *self = CC_SOUND_PANEL (user_data);
  guint idx;
  CcPADeviceInfo *info;

  if (self->updating || !self->sinks)
    return;

  idx = adw_combo_row_get_selected (self->output_device_row);
  if (idx >= self->sinks->len)
    return;

  info = g_ptr_array_index (self->sinks, idx);
  cc_pa_context_set_sink_volume (self->pa, info->index,
                                  (pa_volume_t) (gtk_range_get_value (GTK_RANGE (scale)) * PA_VOLUME_NORM));
}

static void
on_input_volume_changed (GtkScale *scale, gpointer user_data)
{
  CcSoundPanel *self = CC_SOUND_PANEL (user_data);
  guint idx;
  CcPADeviceInfo *info;

  if (self->updating || !self->sources)
    return;

  idx = adw_combo_row_get_selected (self->input_device_row);
  if (idx >= self->sources->len)
    return;

  info = g_ptr_array_index (self->sources, idx);
  cc_pa_context_set_source_volume (self->pa, info->index,
                                    (pa_volume_t) (gtk_range_get_value (GTK_RANGE (scale)) * PA_VOLUME_NORM));
}

static void
on_output_device_changed (AdwComboRow *row, GParamSpec *pspec, gpointer user_data)
{
  CcSoundPanel *self = CC_SOUND_PANEL (user_data);
  guint idx;
  CcPADeviceInfo *info;

  if (self->updating || !self->sinks)
    return;

  idx = adw_combo_row_get_selected (row);
  if (idx >= self->sinks->len)
    return;

  info = g_ptr_array_index (self->sinks, idx);

  self->updating = TRUE;
  gtk_range_set_value (GTK_RANGE (self->output_scale), (gdouble) info->volume / PA_VOLUME_NORM);
  self->updating = FALSE;
}

static void
on_input_device_changed (AdwComboRow *row, GParamSpec *pspec, gpointer user_data)
{
  CcSoundPanel *self = CC_SOUND_PANEL (user_data);
  guint idx;
  CcPADeviceInfo *info;

  if (self->updating || !self->sources)
    return;

  idx = adw_combo_row_get_selected (row);
  if (idx >= self->sources->len)
    return;

  info = g_ptr_array_index (self->sources, idx);

  self->updating = TRUE;
  gtk_range_set_value (GTK_RANGE (self->input_scale), (gdouble) info->volume / PA_VOLUME_NORM);
  self->updating = FALSE;
}

static void
cc_sound_panel_dispose (GObject *object)
{
  CcSoundPanel *self = CC_SOUND_PANEL (object);

  free_device_array (&self->sinks);
  free_device_array (&self->sources);
  cc_pa_context_free (self->pa);

  G_OBJECT_CLASS (cc_sound_panel_parent_class)->dispose (object);
}

static const char *
cc_sound_panel_get_help_uri (CcPanel *panel)
{
  return "help:gnome-help/sound";
}

static void
cc_sound_panel_class_init (CcSoundPanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  CcPanelClass *panel_class = CC_PANEL_CLASS (klass);

  object_class->dispose = cc_sound_panel_dispose;
  panel_class->get_help_uri = cc_sound_panel_get_help_uri;

  gtk_widget_class_set_template_from_resource (widget_class,
                                                "/org/hypr/Settings/sound/cc-sound-panel.ui");

  gtk_widget_class_bind_template_child (widget_class, CcSoundPanel, output_device_row);
  gtk_widget_class_bind_template_child (widget_class, CcSoundPanel, output_scale);
  gtk_widget_class_bind_template_child (widget_class, CcSoundPanel, input_device_row);
  gtk_widget_class_bind_template_child (widget_class, CcSoundPanel, input_scale);
}

static void
cc_sound_panel_init (CcSoundPanel *self)
{
  g_autoptr(GError) error = NULL;

  g_resources_register (cc_sound_get_resource ());

  gtk_widget_init_template (GTK_WIDGET (self));

  self->pa = cc_pa_context_new ();

  if (!cc_pa_context_connect (self->pa, &error)) {
    g_warning ("Failed to connect to PulseAudio: %s", error->message);
    return;
  }

  refresh_devices (self);

  g_signal_connect (self->output_scale, "value-changed",
                    G_CALLBACK (on_output_volume_changed), self);
  g_signal_connect (self->input_scale, "value-changed",
                    G_CALLBACK (on_input_volume_changed), self);
  g_signal_connect (self->output_device_row, "notify::selected",
                    G_CALLBACK (on_output_device_changed), self);
  g_signal_connect (self->input_device_row, "notify::selected",
                    G_CALLBACK (on_input_device_changed), self);
}
