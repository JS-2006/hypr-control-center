#include "config.h"

#include <gdesktop-enums.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include "cc-night-light-page.h"
#include "cc-hyprsunset.h"

#include "cc-display-config-manager.h"
#include "cc-hostname.h"
#include "shell/cc-config.h"

struct _CcNightLightPage {
    AdwBin parent;

    AdwViewStack *main_stack;
    GtkWidget *night_light_settings;
    GtkWidget *box_manual;
    GtkButton *button_from_am;
    GtkButton *button_from_pm;
    GtkButton *button_to_am;
    GtkButton *button_to_pm;
    AdwStatusPage *night_light_unsupported_page;
    GtkWidget *infobar_disabled;
    AdwSwitchRow *night_light_toggle_row;
    AdwComboRow *schedule_type_row;
    GtkWidget *from_spinbuttons_box;
    GtkSpinButton *spinbutton_from_hours;
    GtkSpinButton *spinbutton_from_minutes;
    GtkWidget *to_spinbuttons_box;
    GtkSpinButton *spinbutton_to_hours;
    GtkSpinButton *spinbutton_to_minutes;
    GtkStack *stack_from;
    GtkStack *stack_to;

    GtkAdjustment *adjustment_from_hours;
    GtkAdjustment *adjustment_from_minutes;
    GtkAdjustment *adjustment_to_hours;
    GtkAdjustment *adjustment_to_minutes;
    GtkAdjustment *adjustment_color_temperature;

    gboolean ignore_value_changed;
    GDesktopClockFormat clock_format;

    GSettings *settings_clock;
    CcDisplayConfigManager *config_manager;
};

G_DEFINE_FINAL_TYPE (CcNightLightPage, cc_night_light_page, ADW_TYPE_BIN);

#define CLOCK_SCHEMA "org.gnome.desktop.interface"
#define CLOCK_FORMAT_KEY "clock-format"

static void
dialog_update_state (CcNightLightPage *self);

static void
apply_night_light (CcNightLightPage *self)
{
  gboolean enabled;
  guint temperature;

  enabled = cc_config_get_boolean ( "night-light-enabled");
  temperature = cc_config_get_int ( "night-light-temperature");

  if (enabled)
    cc_hyprsunset_set_temperature (temperature, NULL);
  else
    cc_hyprsunset_disable ();
}

static void
dialog_adjustments_set_frac_hours (CcNightLightPage *self, gdouble value, GtkAdjustment *adj_hours,
                                   GtkAdjustment *adj_mins, GtkStack *stack, GtkButton *button_am, GtkButton *button_pm)
{
    gdouble hours;
    gdouble mins = 0.f;
    gboolean is_pm = FALSE;
    gboolean is_24h;

    is_24h = self->clock_format == G_DESKTOP_CLOCK_FORMAT_24H;
    mins = modf (value, &hours) * 60.f;
    if (!is_24h) {
        if (hours > 12) {
            hours -= 12;
            is_pm = TRUE;
        } else if (hours < 1.0) {
            hours += 12;
            is_pm = FALSE;
        } else if (hours == 12.f) {
            is_pm = TRUE;
        }
    }

    g_debug ("setting adjustment %.3f to %.0f:%02.0f", value, hours, mins);

    self->ignore_value_changed = TRUE;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (adj_hours), hours);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (adj_mins), mins);
    self->ignore_value_changed = FALSE;

    gtk_widget_set_visible (GTK_WIDGET (stack), !is_24h);
    gtk_stack_set_visible_child (stack, is_pm ? GTK_WIDGET (button_pm) : GTK_WIDGET (button_am));
}

static void
dialog_update_state (CcNightLightPage *self)
{
    if (cc_display_config_manager_get_night_light_supported (self->config_manager)) {
        gboolean automatic;
        gboolean enabled;

        enabled = cc_config_get_boolean ( "night-light-enabled");
        automatic = cc_config_get_boolean ( "night-light-schedule-automatic");

        gtk_widget_set_sensitive (self->box_manual, enabled && !automatic);
        gtk_widget_set_sensitive (GTK_WIDGET (self->schedule_type_row), enabled);
        adw_combo_row_set_selected (self->schedule_type_row, automatic ? 0 : 1);

        {
          gdouble value;
          value = cc_config_get_double ( "night-light-schedule-from");
          dialog_adjustments_set_frac_hours (self, value, self->adjustment_from_hours, self->adjustment_from_minutes,
                                             self->stack_from, self->button_from_am, self->button_from_pm);
        }

        {
          gdouble value;
          value = cc_config_get_double ( "night-light-schedule-to");
          dialog_adjustments_set_frac_hours (self, value, self->adjustment_to_hours, self->adjustment_to_minutes,
                                             self->stack_to, self->button_to_am, self->button_to_pm);
        }

        self->ignore_value_changed = TRUE;
        gtk_adjustment_set_value (self->adjustment_color_temperature,
                                  (gdouble) cc_config_get_int ( "night-light-temperature"));
        self->ignore_value_changed = FALSE;

        adw_view_stack_set_visible_child_name (self->main_stack, "night-light-page");
    } else {
        adw_status_page_set_description (
            self->night_light_unsupported_page, cc_hostname_is_vm_chassis (cc_hostname_get_default ())
            ? _("Night Light cannot be used from a virtual machine")
            : _("This could be the result of the graphics driver being used, or the desktop being used remotely."));
        adw_view_stack_set_visible_child_name (self->main_stack, "night-light-unsupported-page");
    }
}

static void
on_schedule_type_row_selected_changed_cb (CcNightLightPage *self)
{
    guint selected;
    gboolean automatic;

    if (self->ignore_value_changed)
        return;

    selected = adw_combo_row_get_selected (self->schedule_type_row);
    automatic = selected == 0;

    cc_config_set_boolean ( "night-light-schedule-automatic", automatic);
}

static void
dialog_enabled_notify_cb (GtkSwitch *sw, GParamSpec *pspec, CcNightLightPage *self)
{
    cc_config_set_boolean ( "night-light-enabled", gtk_switch_get_active (sw));
    apply_night_light (self);
}

static void
dialog_undisable_clicked_cb (CcNightLightPage *self)
{
}

static gdouble
dialog_adjustments_get_frac_hours (CcNightLightPage *self, GtkAdjustment *adj_hours, GtkAdjustment *adj_mins,
                                   GtkStack *stack)
{
    gdouble value;

    value = gtk_adjustment_get_value (adj_hours);
    value += gtk_adjustment_get_value (adj_mins) / 60.0f;

    if (g_strcmp0 (gtk_stack_get_visible_child_name (stack), "pm") == 0)
        value += 12.f;

    return value;
}

static void
dialog_time_from_value_changed_cb (CcNightLightPage *self)
{
    gdouble value;

    if (self->ignore_value_changed)
        return;

    value = dialog_adjustments_get_frac_hours (self, self->adjustment_from_hours, self->adjustment_from_minutes,
                                               self->stack_from);

    if (value >= 24.f)
        value = fmod (value, 24);

    g_debug ("new value = %.3f", value);

    cc_config_set_double ( "night-light-schedule-from", value);
}

static void
dialog_time_to_value_changed_cb (CcNightLightPage *self)
{
    gdouble value;

    if (self->ignore_value_changed)
        return;

    value = dialog_adjustments_get_frac_hours (self, self->adjustment_to_hours, self->adjustment_to_minutes,
                                               self->stack_to);
    if (value >= 24.f)
        value = fmod (value, 24);

    g_debug ("new value = %.3f", value);

    cc_config_set_double ( "night-light-schedule-to", value);
}

static void
dialog_color_temperature_value_changed_cb (CcNightLightPage *self)
{
    gdouble value;

    if (self->ignore_value_changed)
        return;

    value = gtk_adjustment_get_value (self->adjustment_color_temperature);

    g_debug ("new value = %.0f", value);

    cc_config_set_int ( "night-light-temperature", (guint) value);
    apply_night_light (self);
}

static void
dialog_update_adjustments (CcNightLightPage *self)
{
    if (self->clock_format == G_DESKTOP_CLOCK_FORMAT_24H) {
        gtk_adjustment_set_lower (self->adjustment_from_hours, 0);
        gtk_adjustment_set_upper (self->adjustment_from_hours, 23);
    } else {
        if (gtk_adjustment_get_value (self->adjustment_from_hours) > 12)
            gtk_stack_set_visible_child (self->stack_from, GTK_WIDGET (self->button_from_pm));

        gtk_adjustment_set_lower (self->adjustment_from_hours, 1);
        gtk_adjustment_set_upper (self->adjustment_from_hours, 12);
    }

    if (self->clock_format == G_DESKTOP_CLOCK_FORMAT_24H) {
        gtk_adjustment_set_lower (self->adjustment_to_hours, 0);
        gtk_adjustment_set_upper (self->adjustment_to_hours, 23);
    } else {
        if (gtk_adjustment_get_value (self->adjustment_to_hours) > 12)
            gtk_stack_set_visible_child (self->stack_to, GTK_WIDGET (self->button_to_pm));

        gtk_adjustment_set_lower (self->adjustment_to_hours, 1);
        gtk_adjustment_set_upper (self->adjustment_to_hours, 12);
    }
}

static void
dialog_clock_settings_changed_cb (CcNightLightPage *self)
{
    self->clock_format = g_settings_get_enum (self->settings_clock, CLOCK_FORMAT_KEY);

    gtk_adjustment_set_lower (self->adjustment_from_hours, 0);
    gtk_adjustment_set_upper (self->adjustment_from_hours, 23);
    gtk_adjustment_set_lower (self->adjustment_to_hours, 0);
    gtk_adjustment_set_upper (self->adjustment_to_hours, 23);

    gtk_spin_button_update (self->spinbutton_from_hours);
    gtk_spin_button_update (self->spinbutton_to_hours);

    dialog_update_state (self);
    dialog_update_adjustments (self);
}

static void
dialog_am_pm_from_button_clicked_cb (CcNightLightPage *self)
{
    gdouble value;
    value = cc_config_get_double ( "night-light-schedule-from");
    if (value > 12.f)
        value -= 12.f;
    else
        value += 12.f;
    if (value >= 24.f)
        value = fmod (value, 24);
    cc_config_set_double ( "night-light-schedule-from", value);
    g_debug ("new value = %.3f", value);
}

static void
dialog_am_pm_to_button_clicked_cb (CcNightLightPage *self)
{
    gdouble value;
    value = cc_config_get_double ( "night-light-schedule-to");
    if (value > 12.f)
        value -= 12.f;
    else
        value += 12.f;
    if (value >= 24.f)
        value = fmod (value, 24);
    cc_config_set_double ( "night-light-schedule-to", value);
    g_debug ("new value = %.3f", value);
}

static void
config_manager_changed_cb (CcNightLightPage *self)
{
    dialog_update_state (self);
}

static void
cc_night_light_page_finalize (GObject *object)
{
    CcNightLightPage *self = CC_NIGHT_LIGHT_PAGE (object);

    g_clear_object (&self->config_manager);
    g_clear_object (&self->settings_clock);

    G_OBJECT_CLASS (cc_night_light_page_parent_class)->finalize (object);
}

static void
cc_night_light_page_class_init (CcNightLightPageClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->finalize = cc_night_light_page_finalize;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/hypr/Settings/display/cc-night-light-page.ui");

    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, main_stack);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, adjustment_from_hours);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, adjustment_from_minutes);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, adjustment_to_hours);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, adjustment_to_minutes);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, adjustment_color_temperature);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, night_light_settings);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, box_manual);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, button_from_am);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, button_from_pm);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, button_to_am);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, button_to_pm);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, night_light_unsupported_page);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, infobar_disabled);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, night_light_toggle_row);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, schedule_type_row);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, from_spinbuttons_box);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, spinbutton_from_hours);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, spinbutton_from_minutes);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, to_spinbuttons_box);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, spinbutton_to_hours);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, spinbutton_to_minutes);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, stack_from);
    gtk_widget_class_bind_template_child (widget_class, CcNightLightPage, stack_to);

    gtk_widget_class_bind_template_callback (widget_class, dialog_am_pm_from_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_am_pm_to_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_enabled_notify_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_time_from_value_changed_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_time_to_value_changed_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_color_temperature_value_changed_cb);
    gtk_widget_class_bind_template_callback (widget_class, dialog_undisable_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, on_schedule_type_row_selected_changed_cb);
}

static void
cc_night_light_page_init (CcNightLightPage *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    apply_night_light (self);

    dialog_update_state (self);

    self->settings_clock = g_settings_new (CLOCK_SCHEMA);
    self->clock_format = g_settings_get_enum (self->settings_clock, CLOCK_FORMAT_KEY);
    dialog_update_adjustments (self);
    g_signal_connect_object (self->settings_clock, "changed::" CLOCK_FORMAT_KEY,
                             G_CALLBACK (dialog_clock_settings_changed_cb), self, G_CONNECT_SWAPPED);

    if (gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL) {
        gtk_widget_set_direction (self->to_spinbuttons_box, GTK_TEXT_DIR_LTR);
        gtk_widget_set_direction (self->from_spinbuttons_box, GTK_TEXT_DIR_LTR);
    }

    self->config_manager = cc_display_config_manager_new ();
    g_signal_connect_object (self->config_manager, "changed", G_CALLBACK (config_manager_changed_cb), self,
                             G_CONNECT_SWAPPED);
}

CcNightLightPage *
cc_night_light_page_new (void)
{
    return g_object_new (CC_TYPE_NIGHT_LIGHT_PAGE, NULL);
}
