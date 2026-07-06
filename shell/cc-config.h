#pragma once

#include <glib.h>
#include <json-glib/json-glib.h>

#define CC_CONFIG_FILE "hypr-control-center.json"

gboolean    cc_config_init        (GError     **error);
void        cc_config_shutdown    (void);

gchar      *cc_config_get_string  (const gchar *key);
void        cc_config_set_string  (const gchar *key,
                                   const gchar *value);

gboolean    cc_config_get_boolean (const gchar *key);
void        cc_config_set_boolean (const gchar *key,
                                   gboolean     value);

gint        cc_config_get_int     (const gchar *key);
void        cc_config_set_int     (const gchar *key,
                                    gint         value);

gdouble     cc_config_get_double  (const gchar *key);
void        cc_config_set_double  (const gchar *key,
                                    gdouble      value);

gboolean    cc_config_get_window_state (gint  *width,
                                        gint  *height,
                                        gboolean *maximized);
void        cc_config_set_window_state (gint       width,
                                        gint       height,
                                        gboolean   maximized);
