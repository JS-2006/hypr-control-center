#include "config.h"

#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#include "libhypr/hypr-backend.h"
#include "hypr-input-helper.h"

gboolean
mouse_is_present (void)
{
  GPtrArray *devices = g_ptr_array_new_with_free_func ((GDestroyNotify) hypr_backend_input_free);
  gboolean found = FALSE;

  if (hypr_backend_get_inputs (devices, NULL))
    {
      for (guint i = 0; i < devices->len; i++)
        {
          HyprInputDevice *dev = g_ptr_array_index (devices, i);
          if (g_strcmp0 (dev->type, "mouse") == 0)
            {
              found = TRUE;
              break;
            }
        }
    }

  g_ptr_array_unref (devices);
  return found;
}

gboolean
touchpad_is_present (void)
{
  GPtrArray *devices = g_ptr_array_new_with_free_func ((GDestroyNotify) hypr_backend_input_free);
  gboolean found = FALSE;

  if (hypr_backend_get_inputs (devices, NULL))
    {
      for (guint i = 0; i < devices->len; i++)
        {
          HyprInputDevice *dev = g_ptr_array_index (devices, i);
          if (g_strcmp0 (dev->type, "touchpad") == 0)
            {
              found = TRUE;
              break;
            }
        }
    }

  g_ptr_array_unref (devices);
  return found;
}

gboolean
touchscreen_is_present (void)
{
  GPtrArray *devices = g_ptr_array_new_with_free_func ((GDestroyNotify) hypr_backend_input_free);
  gboolean found = FALSE;

  if (hypr_backend_get_inputs (devices, NULL))
    {
      for (guint i = 0; i < devices->len; i++)
        {
          HyprInputDevice *dev = g_ptr_array_index (devices, i);
          if (g_strcmp0 (dev->type, "touch") == 0)
            {
              found = TRUE;
              break;
            }
        }
    }

  g_ptr_array_unref (devices);
  return found;
}

gboolean
pointingstick_is_present (void)
{
  GPtrArray *devices = g_ptr_array_new_with_free_func ((GDestroyNotify) hypr_backend_input_free);
  gboolean found = FALSE;

  if (hypr_backend_get_inputs (devices, NULL))
    {
      for (guint i = 0; i < devices->len; i++)
        {
          HyprInputDevice *dev = g_ptr_array_index (devices, i);
          if (g_strcmp0 (dev->type, "pointer") == 0)
            {
              found = TRUE;
              break;
            }
        }
    }

  g_ptr_array_unref (devices);
  return found;
}
