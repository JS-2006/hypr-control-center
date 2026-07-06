#include "config.h"

#include <glib.h>
#include <gio/gio.h>

#include "bg-apply.h"

gboolean
bg_apply_wallpaper (const char *uri, GError **error)
{
  g_autoptr(GSubprocess) proc = NULL;
  const char *argv[4];
  g_autofree char *path = NULL;

  if (g_str_has_prefix (uri, "file://"))
    path = g_filename_from_uri (uri, NULL, NULL);
  else
    path = g_strdup (uri);

  if (!path) {
    g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Invalid wallpaper URI: %s", uri);
    return FALSE;
  }

  argv[0] = "hyprctl";
  argv[1] = "hyprpaper";
  argv[2] = "wallpaper";
  argv[3] = path;

  proc = g_subprocess_newv ((const char * const *) argv,
                            G_SUBPROCESS_FLAGS_NONE, error);
  if (!proc)
    return FALSE;

  return g_subprocess_wait_check (proc, NULL, error);
}
