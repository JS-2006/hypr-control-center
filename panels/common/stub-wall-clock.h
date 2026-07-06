#pragma once

#include <glib-object.h>
#include <time.h>

G_BEGIN_DECLS

#define GNOME_TYPE_WALL_CLOCK (gnome_wall_clock_get_type ())
G_DECLARE_FINAL_TYPE (GnomeWallClock, gnome_wall_clock, GNOME, WALL_CLOCK, GObject)

GnomeWallClock *gnome_wall_clock_new (void);

gchar *gnome_wall_clock_string_for_datetime (GnomeWallClock *clock,
                                              struct tm      *now,
                                              const gchar    *format,
                                              gboolean        show_seconds,
                                              gboolean        twelve_hour,
                                              gboolean        force_update);

G_END_DECLS
