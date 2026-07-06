#pragma once

#include <glib.h>

gboolean cc_hyprsunset_set_temperature (guint temperature, GError **error);
void     cc_hyprsunset_disable        (void);
gboolean cc_hyprsunset_is_active      (void);
guint    cc_hyprsunset_get_temperature (void);
