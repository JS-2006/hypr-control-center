#include "config.h"

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include "cc-bt-device-widget.h"

#define BLUEZ_NAME "org.bluez"
#define ADAPTER1_IFACE "org.bluez.Adapter1"
#define DEVICE1_IFACE "org.bluez.Device1"

struct _CcBtDeviceWidget {
  AdwBin parent_instance;

  GDBusProxy *adapter;
  GDBusProxy *adapter_props;

  GtkListBox *listbox;

  gboolean has_adapter;
  gboolean powered;
};

enum {
  ADAPTER_STATUS_CHANGED,
  SIGNAL_LAST
};

static guint signals[SIGNAL_LAST] = { 0 };

G_DEFINE_TYPE (CcBtDeviceWidget, cc_bt_device_widget, ADW_TYPE_BIN)

static void
emit_adapter_status_changed (CcBtDeviceWidget *self)
{
  g_signal_emit (self, signals[ADAPTER_STATUS_CHANGED], 0);
}

static void
refresh_device_list (CcBtDeviceWidget *self)
{
  g_autoptr(GDBusProxy) om = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) result = NULL;
  GVariantIter *obj_iter = NULL;
  GtkWidget *child;

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->listbox))) != NULL)
    gtk_list_box_remove (self->listbox, child);

  if (!self->adapter)
    return;

  om = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                       G_DBUS_PROXY_FLAGS_NONE, NULL,
                                       BLUEZ_NAME, "/",
                                       "org.freedesktop.DBus.ObjectManager",
                                       NULL, &error);
  if (!om) {
    g_debug ("Cannot get ObjectManager: %s", error->message);
    return;
  }

  result = g_dbus_proxy_call_sync (om, "GetManagedObjects", NULL,
                                    G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (!result) {
    g_debug ("GetManagedObjects failed: %s", error->message);
    return;
  }

  g_variant_get (result, "(a{oa{sa{sv}}})", &obj_iter);
  if (!obj_iter)
    return;

  {
    g_autoptr(GVariantIter) obj_iter_cleanup = obj_iter;
    char *object_path;
    GVariant *ifaces_v;

    while (g_variant_iter_loop (obj_iter, "{oa{sa{sv}}}", &object_path, &ifaces_v)) {
      if (!g_str_has_prefix (object_path, g_dbus_proxy_get_object_path (self->adapter)))
        continue;

      GVariantIter *iface_iter = NULL;
      g_variant_get (ifaces_v, "a{sa{sv}}", &iface_iter);
      if (!iface_iter)
        continue;

      {
        g_autoptr(GVariantIter) iface_iter_cleanup = iface_iter;
        char *iface_name;

        while (g_variant_iter_loop (iface_iter, "{sa{sv}}", &iface_name, NULL)) {
          if (g_strcmp0 (iface_name, DEVICE1_IFACE) == 0) {
            g_autoptr(GDBusProxy) dev = NULL;
            g_autoptr(GVariant) name_v = NULL, addr_v = NULL;

            dev = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                  G_DBUS_PROXY_FLAGS_NONE, NULL,
                                                  BLUEZ_NAME, object_path,
                                                  DEVICE1_IFACE, NULL, &error);
            if (!dev) continue;

            name_v = g_dbus_proxy_get_cached_property (dev, "Name");
            addr_v = g_dbus_proxy_get_cached_property (dev, "Address");

            {
              AdwActionRow *row = ADW_ACTION_ROW (adw_action_row_new ());
              gtk_widget_set_visible (GTK_WIDGET (row), TRUE);

              if (name_v)
                adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row),
                                                g_variant_get_string (name_v, NULL));
              if (addr_v)
                adw_action_row_set_subtitle (row,
                                              g_variant_get_string (addr_v, NULL));

              gtk_list_box_append (self->listbox, GTK_WIDGET (row));
            }
          }
        }
      }
    }
  }
}

static void
on_adapter_properties (GDBusProxy *proxy,
                       GVariant   *changed,
                       char      **invalidated,
                       gpointer    user_data)
{
  CcBtDeviceWidget *self = CC_BT_DEVICE_WIDGET (user_data);
  GVariant *v;

  v = g_variant_lookup_value (changed, "Powered", G_VARIANT_TYPE_BOOLEAN);
  if (v) {
    GtkWidget *child;

    self->powered = g_variant_get_boolean (v);
    gtk_widget_set_visible (GTK_WIDGET (self->listbox), self->powered);
    if (self->powered)
      refresh_device_list (self);
    else {
      while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->listbox))) != NULL)
        gtk_list_box_remove (self->listbox, child);
    }
    emit_adapter_status_changed (self);
  }
}

static void
find_adapter (CcBtDeviceWidget *self)
{
  g_autoptr(GDBusProxy) om = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) result = NULL;
  GVariantIter *obj_iter = NULL;

  om = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                       G_DBUS_PROXY_FLAGS_NONE, NULL,
                                       BLUEZ_NAME, "/",
                                       "org.freedesktop.DBus.ObjectManager",
                                       NULL, &error);
  if (!om) {
    g_debug ("Cannot create ObjectManager: %s", error->message);
    self->has_adapter = FALSE;
    emit_adapter_status_changed (self);
    return;
  }

  result = g_dbus_proxy_call_sync (om, "GetManagedObjects", NULL,
                                    G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (!result) {
    g_debug ("GetManagedObjects failed: %s", error->message);
    self->has_adapter = FALSE;
    emit_adapter_status_changed (self);
    return;
  }

  g_variant_get (result, "(a{oa{sa{sv}}})", &obj_iter);
  if (!obj_iter) {
    self->has_adapter = FALSE;
    emit_adapter_status_changed (self);
    return;
  }

  {
    g_autoptr(GVariantIter) obj_iter_cleanup = obj_iter;
    char *object_path;
    GVariant *ifaces_v;

    while (g_variant_iter_loop (obj_iter, "{oa{sa{sv}}}", &object_path, &ifaces_v)) {
      GVariantIter *iface_iter = NULL;
      g_variant_get (ifaces_v, "a{sa{sv}}", &iface_iter);
      if (!iface_iter)
        continue;

      {
        g_autoptr(GVariantIter) iface_iter_cleanup = iface_iter;
        char *iface_name;

        while (g_variant_iter_loop (iface_iter, "{sa{sv}}", &iface_name, NULL)) {
          if (g_strcmp0 (iface_name, ADAPTER1_IFACE) == 0) {
            g_autoptr(GVariant) powered_v = NULL;

            if (self->adapter) {
              g_signal_handlers_disconnect_by_data (self->adapter, self);
              g_clear_object (&self->adapter);
              g_clear_object (&self->adapter_props);
            }

            self->adapter = g_dbus_proxy_new_for_bus_sync (
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
                BLUEZ_NAME, object_path, ADAPTER1_IFACE, NULL, &error);
            if (!self->adapter) {
              g_debug ("Cannot create adapter proxy: %s", error->message);
              continue;
            }

            self->adapter_props = g_dbus_proxy_new_for_bus_sync (
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
                BLUEZ_NAME, object_path, "org.freedesktop.DBus.Properties",
                NULL, &error);

            g_signal_connect (self->adapter, "g-properties-changed",
                              G_CALLBACK (on_adapter_properties), self);

            powered_v = g_dbus_proxy_get_cached_property (self->adapter, "Powered");
            self->powered = powered_v ? g_variant_get_boolean (powered_v) : FALSE;
            self->has_adapter = TRUE;

            gtk_widget_set_visible (GTK_WIDGET (self->listbox), self->powered);
            if (self->powered)
              refresh_device_list (self);
            emit_adapter_status_changed (self);
            return;
          }
        }
      }
    }
  }

  self->has_adapter = FALSE;
  emit_adapter_status_changed (self);
}

static void
on_interfaces_changed (GDBusConnection *connection,
                       const char      *sender_name,
                       const char      *object_path,
                       const char      *interface_name,
                       const char      *signal_name,
                       GVariant        *parameters,
                       gpointer         user_data)
{
  CcBtDeviceWidget *self = CC_BT_DEVICE_WIDGET (user_data);
  if (self->powered)
    refresh_device_list (self);
}

static void
cc_bt_device_widget_dispose (GObject *object)
{
  CcBtDeviceWidget *self = CC_BT_DEVICE_WIDGET (object);

  g_clear_object (&self->adapter);
  g_clear_object (&self->adapter_props);

  G_OBJECT_CLASS (cc_bt_device_widget_parent_class)->dispose (object);
}

static void
cc_bt_device_widget_class_init (CcBtDeviceWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = cc_bt_device_widget_dispose;

  signals[ADAPTER_STATUS_CHANGED] = g_signal_new ("adapter-status-changed",
                                                   CC_TYPE_BT_DEVICE_WIDGET,
                                                   G_SIGNAL_RUN_LAST, 0,
                                                   NULL, NULL,
                                                   g_cclosure_marshal_VOID__VOID,
                                                   G_TYPE_NONE, 0);
}

static void
cc_bt_device_widget_init (CcBtDeviceWidget *self)
{
  GtkWidget *sw;

  self->listbox = GTK_LIST_BOX (gtk_list_box_new ());
  gtk_list_box_set_selection_mode (self->listbox, GTK_SELECTION_NONE);
  gtk_widget_set_visible (GTK_WIDGET (self->listbox), FALSE);
  gtk_widget_set_margin_top (GTK_WIDGET (self->listbox), 12);
  gtk_widget_set_margin_bottom (GTK_WIDGET (self->listbox), 12);

  sw = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), GTK_WIDGET (self->listbox));
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  adw_bin_set_child (ADW_BIN (self), sw);

  {
    GDBusConnection *conn = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, NULL);
    if (conn) {
      g_dbus_connection_signal_subscribe (conn,
                                          BLUEZ_NAME,
                                          "org.freedesktop.DBus.ObjectManager",
                                          "InterfacesAdded",
                                          "/",
                                          NULL,
                                          G_DBUS_SIGNAL_FLAGS_NONE,
                                          on_interfaces_changed,
                                          self, NULL);
      g_dbus_connection_signal_subscribe (conn,
                                          BLUEZ_NAME,
                                          "org.freedesktop.DBus.ObjectManager",
                                          "InterfacesRemoved",
                                          "/",
                                          NULL,
                                          G_DBUS_SIGNAL_FLAGS_NONE,
                                          on_interfaces_changed,
                                          self, NULL);
      g_object_unref (conn);
    }
  }

  find_adapter (self);
}

gboolean
cc_bt_device_widget_get_default_adapter_powered (CcBtDeviceWidget *self)
{
  return self->powered;
}

void
cc_bt_device_widget_set_default_adapter_powered (CcBtDeviceWidget *self, gboolean powered)
{
  if (!self->adapter_props)
    return;

  g_dbus_proxy_call (self->adapter_props, "Set",
                     g_variant_new_parsed ("('org.bluez.Adapter1', 'Powered', %v)",
                                           g_variant_new_boolean (powered)),
                     G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}
