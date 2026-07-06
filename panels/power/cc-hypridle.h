#pragma once

#include <glib.h>

typedef struct {
  gchar *timeout;
  gchar *action;
} CcHypridleRule;

typedef struct {
  CcHypridleRule *rules;
  gsize n_rules;
} CcHypridleConfig;

CcHypridleConfig *cc_hypridle_config_new     (void);
void              cc_hypridle_config_free     (CcHypridleConfig *config);
gboolean          cc_hypridle_config_load     (CcHypridleConfig *config,
                                               GError          **error);
gboolean          cc_hypridle_config_save     (CcHypridleConfig *config,
                                               GError          **error);
gboolean          cc_hypridle_config_set_idle (CcHypridleConfig *config,
                                               guint             timeout_secs,
                                               const gchar      *action);
void              cc_hypridle_restart         (void);
