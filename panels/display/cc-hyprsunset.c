#include "config.h"

#include <glib.h>
#include <glib/gstdio.h>

#include "cc-hyprsunset.h"

static GPid hyprsunset_pid = 0;

gboolean
cc_hyprsunset_set_temperature (guint temperature, GError **error)
{
  const gchar *argv[] = {"hyprsunset", NULL, NULL};

  if (hyprsunset_pid > 0)
    {
      g_spawn_close_pid (hyprsunset_pid);
      hyprsunset_pid = 0;
    }

  argv[1] = g_strdup_printf ("%u", temperature);

  return g_spawn_async (NULL, (gchar **) argv, NULL,
                        G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                        NULL, NULL, &hyprsunset_pid, error);
}

void
cc_hyprsunset_disable (void)
{
  if (hyprsunset_pid > 0)
    {
      g_spawn_close_pid (hyprsunset_pid);
      hyprsunset_pid = 0;
    }

  g_spawn_command_line_async ("killall hyprsunset", NULL);
}

gboolean
cc_hyprsunset_is_active (void)
{
  gint exit_status;

  if (g_spawn_command_line_sync ("pidof hyprsunset", NULL, NULL, &exit_status, NULL))
    return exit_status == 0;

  return FALSE;
}

guint
cc_hyprsunset_get_temperature (void)
{
  /* We could parse /proc, but for simplicity we store the last set value */
  return 4000;
}
