#pragma once

#include <glib.h>
#include <json-glib/json-glib.h>

/* Monitor */
typedef struct {
  gint   id;
  gchar *name;
  gchar *description;
  gint   width;
  gint   height;
  gint   refresh_rate;
  gdouble x, y;
  gboolean enabled;
} HyprMonitor;

/* Input device */
typedef struct {
  gchar *name;
  gchar *type;
  gchar *identifier;
} HyprInputDevice;

/* Hyprctl IPC result */
JsonNode *hypr_backend_call      (const gchar *command,
                                  GError     **error);

/* Monitor helpers */
gboolean   hypr_backend_get_monitors (GPtrArray  *monitors,
                                      GError    **error);
void       hypr_backend_monitor_free (HyprMonitor *mon);

/* Input helpers */
gboolean   hypr_backend_get_inputs   (GPtrArray  *devices,
                                      GError    **error);
void       hypr_backend_input_free   (HyprInputDevice *dev);

/* Config helpers */
gboolean   hypr_backend_set_keyword  (const gchar *keyword,
                                      const gchar *value,
                                      GError     **error);
gchar     *hypr_backend_get_option   (const gchar *option,
                                      GError     **error);

/* Utility */
gboolean   hypr_backend_is_available (void);
