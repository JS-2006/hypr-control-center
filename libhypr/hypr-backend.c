#include "config.h"

#include <gio/gio.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <string.h>

#include "hypr-backend.h"

static gchar *
get_socket_path (void)
{
  const gchar *runtime_dir;
  const gchar *instance_sig;

  instance_sig = g_getenv ("HYPRLAND_INSTANCE_SIGNATURE");
  if (!instance_sig)
    return NULL;

  runtime_dir = g_getenv ("XDG_RUNTIME_DIR");
  if (!runtime_dir)
    return NULL;

  return g_build_filename (runtime_dir, "hypr", instance_sig, ".socket.sock", NULL);
}

static gchar *
send_command (const gchar *command, GError **error)
{
  g_autofree gchar *socket_path = get_socket_path ();
  GSocket *socket;
  GSocketAddress *address;
  gchar *response = NULL;
  gsize response_len = 0;

  if (!socket_path)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                   "HYPRLAND_INSTANCE_SIGNATURE not set");
      return NULL;
    }

  socket = g_socket_new (G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,
                         G_SOCKET_PROTOCOL_DEFAULT, error);
  if (!socket)
    return NULL;

  address = g_unix_socket_address_new (socket_path);
  if (!g_socket_connect (socket, address, NULL, error))
    {
      g_object_unref (socket);
      return NULL;
    }
  g_object_unref (address);

  if (!g_socket_send (socket, command, strlen (command), NULL, error))
    {
      g_object_unref (socket);
      return NULL;
    }

  if (!g_socket_receive (socket, (gchar *) &response_len, sizeof (gsize), NULL, error))
    {
      g_object_unref (socket);
      return NULL;
    }

  response = g_malloc (response_len + 1);
  if (!g_socket_receive (socket, response, response_len, NULL, error))
    {
      g_free (response);
      g_object_unref (socket);
      return NULL;
    }
  response[response_len] = '\0';

  g_object_unref (socket);
  return response;
}

JsonNode *
hypr_backend_call (const gchar *command, GError **error)
{
  g_autofree gchar *response = send_command (command, error);

  if (!response)
    return NULL;

  JsonParser *parser = json_parser_new ();
  if (!json_parser_load_from_data (parser, response, -1, error))
    {
      g_object_unref (parser);
      return NULL;
    }

  JsonNode *root = json_node_copy (json_parser_get_root (parser));
  g_object_unref (parser);

  return root;
}

static gboolean
hyprctl_call (const gchar *arg, gchar **out, GError **error)
{
  g_autofree gchar *cmd = g_strdup_printf ("/%s", arg);
  g_autofree gchar *response = send_command (cmd, error);

  if (!response)
    return FALSE;

  if (out)
    *out = g_steal_pointer (&response);

  return TRUE;
}

gboolean
hypr_backend_get_monitors (GPtrArray *monitors, GError **error)
{
  g_autoptr(JsonNode) root = hypr_backend_call ("j/monitors", error);

  if (!root || !JSON_NODE_HOLDS_ARRAY (root))
    return FALSE;

  JsonArray *arr = json_node_get_array (root);
  for (guint i = 0; i < json_array_get_length (arr); i++)
    {
      JsonObject *obj = json_array_get_object_element (arr, i);
      HyprMonitor *mon = g_new0 (HyprMonitor, 1);

      mon->id = json_object_get_int_member (obj, "id");
      mon->name = g_strdup (json_object_get_string_member (obj, "name"));
      mon->description = g_strdup (json_object_get_string_member (obj, "description"));
      mon->width = json_object_get_int_member (obj, "width");
      mon->height = json_object_get_int_member (obj, "height");
      mon->refresh_rate = json_object_get_int_member (obj, "refreshRate");
      mon->x = json_object_get_double_member (obj, "x");
      mon->y = json_object_get_double_member (obj, "y");

      mon->enabled = TRUE;

      g_ptr_array_add (monitors, mon);
    }

  return TRUE;
}

void
hypr_backend_monitor_free (HyprMonitor *mon)
{
  if (!mon)
    return;
  g_free (mon->name);
  g_free (mon->description);
  g_free (mon);
}

gboolean
hypr_backend_get_inputs (GPtrArray *devices, GError **error)
{
  g_autoptr(JsonNode) root = hypr_backend_call ("j/devices", error);

  if (!root || !JSON_NODE_HOLDS_OBJECT (root))
    return FALSE;

  JsonObject *obj = json_node_get_object (root);
  JsonObject *input_obj = json_object_get_object_member (obj, "input");
  if (!input_obj)
    return FALSE;

  /* Extract keyboards */
  if (json_object_has_member (obj, "keyboards"))
    {
      JsonArray *kbs = json_object_get_array_member (obj, "keyboards");
      for (guint i = 0; i < json_array_get_length (kbs); i++)
        {
          JsonObject *kb = json_array_get_object_element (kbs, i);
          HyprInputDevice *dev = g_new0 (HyprInputDevice, 1);
          dev->name = g_strdup (json_object_get_string_member (kb, "name"));
          dev->type = g_strdup ("keyboard");
          dev->identifier = g_strdup (json_object_get_string_member (kb, "identifier"));
          g_ptr_array_add (devices, dev);
        }
    }

  /* Extract touchpads */
  if (json_object_has_member (obj, "touchpads"))
    {
      JsonArray *arr = json_object_get_array_member (obj, "touchpads");
      for (guint i = 0; i < json_array_get_length (arr); i++)
        {
          JsonObject *d = json_array_get_object_element (arr, i);
          HyprInputDevice *dev = g_new0 (HyprInputDevice, 1);
          dev->name = g_strdup (json_object_get_string_member (d, "name"));
          dev->type = g_strdup ("touchpad");
          dev->identifier = g_strdup (json_object_get_string_member (d, "identifier"));
          g_ptr_array_add (devices, dev);
        }
    }

  /* Extract mice */
    {
      JsonArray *mice = json_object_get_array_member (obj, "mice");
      for (guint i = 0; i < json_array_get_length (mice); i++)
        {
          JsonObject *m = json_array_get_object_element (mice, i);
          HyprInputDevice *dev = g_new0 (HyprInputDevice, 1);
          dev->name = g_strdup (json_object_get_string_member (m, "name"));
          dev->type = g_strdup ("mouse");
          dev->identifier = g_strdup (json_object_get_string_member (m, "identifier"));
          g_ptr_array_add (devices, dev);
        }
    }

  return TRUE;
}

void
hypr_backend_input_free (HyprInputDevice *dev)
{
  if (!dev)
    return;
  g_free (dev->name);
  g_free (dev->type);
  g_free (dev->identifier);
  g_free (dev);
}

gboolean
hypr_backend_set_keyword (const gchar *keyword, const gchar *value, GError **error)
{
  g_autofree gchar *cmd = g_strdup_printf ("keyword %s %s", keyword, value);
  g_autofree gchar *response = NULL;
  return hyprctl_call (cmd, &response, error);
}

gchar *
hypr_backend_get_option (const gchar *option, GError **error)
{
  g_autofree gchar *cmd = g_strdup_printf ("getoption %s", option);
  g_autofree gchar *response = NULL;

  if (!hyprctl_call (cmd, &response, error))
    return NULL;

  JsonParser *parser = json_parser_new ();
  if (!json_parser_load_from_data (parser, response, -1, error))
    {
      g_object_unref (parser);
      return NULL;
    }

  JsonObject *root = json_node_get_object (json_parser_get_root (parser));
  if (!root || !json_object_has_member (root, "str"))
    {
      g_object_unref (parser);
      return NULL;
    }

  gchar *val = g_strdup (json_object_get_string_member (root, "str"));
  g_object_unref (parser);
  return val;
}

gboolean
hypr_backend_is_available (void)
{
  return g_getenv ("HYPRLAND_INSTANCE_SIGNATURE") != NULL;
}
