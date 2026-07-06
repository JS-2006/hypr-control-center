#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define GNOME_TYPE_XKB_INFO (gnome_xkb_info_get_type ())
G_DECLARE_FINAL_TYPE (GnomeXkbInfo, gnome_xkb_info, GNOME, XKB_INFO, GObject)

GnomeXkbInfo *gnome_xkb_info_new (void);

gboolean gnome_xkb_info_get_layout_info (GnomeXkbInfo *info,
                                          const gchar  *id,
                                          const gchar **display_name,
                                          const gchar **short_name,
                                          const gchar **xkb_layout,
                                          const gchar **xkb_variant);

GList *gnome_xkb_info_get_layouts_for_language (GnomeXkbInfo *info,
                                                 const gchar  *language_code);

GList *gnome_xkb_info_get_layouts_for_country (GnomeXkbInfo *info,
                                                const gchar  *country_code);

GList *gnome_xkb_info_get_all_layouts (GnomeXkbInfo *info);

G_END_DECLS
