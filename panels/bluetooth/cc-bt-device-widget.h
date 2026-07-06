#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define CC_TYPE_BT_DEVICE_WIDGET (cc_bt_device_widget_get_type ())
G_DECLARE_FINAL_TYPE (CcBtDeviceWidget, cc_bt_device_widget, CC, BT_DEVICE_WIDGET, AdwBin);

gboolean cc_bt_device_widget_get_default_adapter_powered (CcBtDeviceWidget *self);
void     cc_bt_device_widget_set_default_adapter_powered (CcBtDeviceWidget *self, gboolean powered);

G_END_DECLS
