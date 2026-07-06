#pragma once

#include <glib-object.h>
#include "gsd-device-manager.h"

G_BEGIN_DECLS

GType gsd_device_type_get_type (void);
#define GSD_TYPE_DEVICE_TYPE (gsd_device_type_get_type ())

G_END_DECLS
