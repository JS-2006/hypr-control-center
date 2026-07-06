#include "config.h"
#include "stub-xkb-info.h"

typedef struct {
    const char *id;
    const char *display_name;
    const char *short_name;
    const char *xkb_layout;
    const char *xkb_variant;
    const char *language_codes;
    const char *country_codes;
} LayoutEntry;

static const LayoutEntry layouts[] = {
    { "us",           "English (US)",             "en", "us",        "",      "en",    "US" },
    { "us+alt-intl",  "English (US, alt. intl.)", "en", "us",        "alt-intl", "en", "US" },
    { "us+dvorak",    "English (US, Dvorak)",     "en", "us",        "dvorak", "en",  "US" },
    { "us+colemak",   "English (US, Colemak)",    "en", "us",        "colemak", "en", "US" },
    { "gb",           "English (UK)",             "en", "gb",        "",      "en",    "GB" },
    { "de",           "German",                   "de", "de",        "",      "de",    "DE" },
    { "de+neo",       "German (Neo 2)",           "de", "de",        "neo",   "de",    "DE" },
    { "fr",           "French",                   "fr", "fr",        "",      "fr",    "FR" },
    { "fr+azerty",    "French (AZERTY)",          "fr", "fr",        "azerty", "fr",   "FR" },
    { "fr+bepo",      "French (BEPO)",            "fr", "fr",        "bepo",  "fr",    "FR" },
    { "es",           "Spanish",                  "es", "es",        "",      "es",    "ES" },
    { "it",           "Italian",                  "it", "it",        "",      "it",    "IT" },
    { "pt",           "Portuguese",               "pt", "pt",        "",      "pt",    "PT" },
    { "br",           "Portuguese (Brazil)",      "br", "br",        "",      "pt",    "BR" },
    { "ru",           "Russian",                  "ru", "ru",        "",      "ru",    "RU" },
    { "jp",           "Japanese",                 "jp", "jp",        "",      "ja",    "JP" },
    { "kr",           "Korean",                   "kr", "kr",        "",      "ko",    "KR" },
    { "cn",           "Chinese",                  "cn", "cn",        "",      "zh",    "CN" },
    { "se",           "Swedish",                  "se", "se",        "",      "sv",    "SE" },
    { "no",           "Norwegian",                "no", "no",        "",      "no",    "NO" },
    { "fi",           "Finnish",                  "fi", "fi",        "",      "fi",    "FI" },
    { "dk",           "Danish",                   "dk", "dk",        "",      "da",    "DK" },
    { "nl",           "Dutch",                    "nl", "nl",        "",      "nl",    "NL" },
    { "pl",           "Polish",                   "pl", "pl",        "",      "pl",    "PL" },
    { "cz",           "Czech",                    "cz", "cz",        "",      "cs",    "CZ" },
    { "sk",           "Slovak",                   "sk", "sk",        "",      "sk",    "SK" },
    { "hu",           "Hungarian",                "hu", "hu",        "",      "hu",    "HU" },
    { "ro",           "Romanian",                 "ro", "ro",        "",      "ro",    "RO" },
    { "bg",           "Bulgarian",                "bg", "bg",        "",      "bg",    "BG" },
    { "gr",           "Greek",                    "gr", "gr",        "",      "el",    "GR" },
    { "il",           "Hebrew",                   "il", "il",        "",      "he",    "IL" },
    { "ara",          "Arabic",                   "ara","ara",       "",      "ar",    "SA" },
    { "ir",           "Persian",                  "ir", "ir",        "",      "fa",    "IR" },
    { "th",           "Thai",                     "th", "th",        "",      "th",    "TH" },
    { "vn",           "Vietnamese",               "vn", "vn",        "",      "vi",    "VN" },
    { "tr",           "Turkish",                  "tr", "tr",        "",      "tr",    "TR" },
    { "ua",           "Ukrainian",                "ua", "ua",        "",      "uk",    "UA" },
    { "ee",           "Estonian",                 "ee", "ee",        "",      "et",    "EE" },
    { "lv",           "Latvian",                  "lv", "lv",        "",      "lv",    "LV" },
    { "lt",           "Lithuanian",               "lt", "lt",        "",      "lt",    "LT" },
    { "hr",           "Croatian",                 "hr", "hr",        "",      "hr",    "HR" },
    { "rs",           "Serbian",                  "rs", "rs",        "",      "sr",    "RS" },
    { "si",           "Slovenian",                "si", "si",        "",      "sl",    "SI" },
    { "mk",           "Macedonian",               "mk", "mk",        "",      "mk",    "MK" },
    {  NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

struct _GnomeXkbInfo {
    GObject parent_instance;
};

G_DEFINE_FINAL_TYPE (GnomeXkbInfo, gnome_xkb_info, G_TYPE_OBJECT)

static void
gnome_xkb_info_init (GnomeXkbInfo *self)
{
}

static void
gnome_xkb_info_class_init (GnomeXkbInfoClass *klass)
{
}

GnomeXkbInfo *
gnome_xkb_info_new (void)
{
    return g_object_new (GNOME_TYPE_XKB_INFO, NULL);
}

static const LayoutEntry *
find_layout (const char *id)
{
    for (int i = 0; layouts[i].id; i++)
        if (g_strcmp0 (layouts[i].id, id) == 0)
            return &layouts[i];
    return NULL;
}

gboolean
gnome_xkb_info_get_layout_info (GnomeXkbInfo *info,
                                 const gchar  *id,
                                 const gchar **display_name,
                                 const gchar **short_name,
                                 const gchar **xkb_layout,
                                 const gchar **xkb_variant)
{
    const LayoutEntry *e = find_layout (id);
    if (!e)
        return FALSE;
    if (display_name) *display_name = e->display_name;
    if (short_name)   *short_name   = e->short_name;
    if (xkb_layout)   *xkb_layout   = e->xkb_layout;
    if (xkb_variant)  *xkb_variant  = e->xkb_variant;
    return TRUE;
}

static GList *
get_layouts_for_predicate (GnomeXkbInfo *info, gboolean (*match)(const LayoutEntry *, const char *), const char *value)
{
    GList *result = NULL;
    for (int i = 0; layouts[i].id; i++) {
        if (match (&layouts[i], value))
            result = g_list_prepend (result, g_strdup (layouts[i].id));
    }
    return result;
}

static gboolean
match_language (const LayoutEntry *e, const char *code)
{
    return g_strcmp0 (e->language_codes, code) == 0;
}

static gboolean
match_country (const LayoutEntry *e, const char *code)
{
    return g_strcmp0 (e->country_codes, code) == 0;
}

static gboolean
match_all (const LayoutEntry *e, const char *unused)
{
    return TRUE;
}

GList *
gnome_xkb_info_get_layouts_for_language (GnomeXkbInfo *info,
                                          const gchar  *language_code)
{
    return get_layouts_for_predicate (info, match_language, language_code);
}

GList *
gnome_xkb_info_get_layouts_for_country (GnomeXkbInfo *info,
                                         const gchar  *country_code)
{
    return get_layouts_for_predicate (info, match_country, country_code);
}

GList *
gnome_xkb_info_get_all_layouts (GnomeXkbInfo *info)
{
    return get_layouts_for_predicate (info, match_all, NULL);
}
