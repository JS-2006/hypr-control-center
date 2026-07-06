#include "config.h"
#include "stub-wall-clock.h"

struct _GnomeWallClock {
    GObject parent_instance;
};

G_DEFINE_FINAL_TYPE (GnomeWallClock, gnome_wall_clock, G_TYPE_OBJECT)

static void
gnome_wall_clock_init (GnomeWallClock *self)
{
}

static void
gnome_wall_clock_class_init (GnomeWallClockClass *klass)
{
}

GnomeWallClock *
gnome_wall_clock_new (void)
{
    return g_object_new (GNOME_TYPE_WALL_CLOCK, NULL);
}

gchar *
gnome_wall_clock_string_for_datetime (GnomeWallClock *clock,
                                      struct tm      *now,
                                      const gchar    *format,
                                      gboolean        show_seconds,
                                      gboolean        twelve_hour,
                                      gboolean        force_update)
{
    gchar buf[256];
    strftime (buf, sizeof (buf), format, now);
    return g_strdup (buf);
}
