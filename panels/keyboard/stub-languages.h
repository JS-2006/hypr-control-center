#pragma once

#include <glib.h>

G_BEGIN_DECLS

gchar **gnome_get_all_locales (void);

gboolean gnome_parse_locale (const gchar  *locale,
                              gchar       **language_code,
                              gchar       **country_code,
                              gchar       **codeset,
                              gchar       **modifier);

gchar *gnome_get_language_from_locale (const gchar *locale,
                                        const gchar *locale_to_use);

gchar *gnome_get_language_from_code (const gchar *code,
                                      const gchar *locale_to_use);

gboolean gnome_get_input_source_from_locale (const gchar  *locale,
                                              const gchar **type,
                                              const gchar **id);

G_END_DECLS
