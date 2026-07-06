#include "config.h"

#include "cc-common-language.h"
#include "stub-languages.h"

typedef struct {
    const char *locale;
    const char *lang_code;
    const char *country_code;
    const char *language_name;
    const char *layout_id;
} LocaleEntry;

static const LocaleEntry locale_table[] = {
    { "en_US.UTF-8",     "en", "US", "English",               "us" },
    { "en_GB.UTF-8",     "en", "GB", "English (UK)",          "gb" },
    { "de_DE.UTF-8",     "de", "DE", "Deutsch",               "de" },
    { "fr_FR.UTF-8",     "fr", "FR", "Fran\u00e7ais",         "fr" },
    { "es_ES.UTF-8",     "es", "ES", "Espa\u00f1ol",          "es" },
    { "it_IT.UTF-8",     "it", "IT", "Italiano",              "it" },
    { "pt_PT.UTF-8",     "pt", "PT", "Portugu\u00eas",        "pt" },
    { "pt_BR.UTF-8",     "pt", "BR", "Portugu\u00eas (Brasil)", "br" },
    { "ru_RU.UTF-8",     "ru", "RU", "\u0420\u0443\u0441\u0441\u043a\u0438\u0439", "ru" },
    { "ja_JP.UTF-8",     "ja", "JP", "\u65e5\u672c\u8a9e",    "jp" },
    { "ko_KR.UTF-8",     "ko", "KR", "\ud55c\uad6d\uc5b4",   "kr" },
    { "zh_CN.UTF-8",     "zh", "CN", "\u4e2d\u6587",          "cn" },
    { "sv_SE.UTF-8",     "sv", "SE", "Svenska",               "se" },
    { "nb_NO.UTF-8",     "nb", "NO", "Norsk bokm\u00e5l",     "no" },
    { "fi_FI.UTF-8",     "fi", "FI", "Suomi",                 "fi" },
    { "da_DK.UTF-8",     "da", "DK", "Dansk",                 "dk" },
    { "nl_NL.UTF-8",     "nl", "NL", "Nederlands",            "nl" },
    { "pl_PL.UTF-8",     "pl", "PL", "Polski",                "pl" },
    { "cs_CZ.UTF-8",     "cs", "CZ", "\u010ce\u0161tina",     "cz" },
    { "sk_SK.UTF-8",     "sk", "SK", "Sloven\u010dina",       "sk" },
    { "hu_HU.UTF-8",     "hu", "HU", "Magyar",                "hu" },
    { "ro_RO.UTF-8",     "ro", "RO", "Rom\u00e2n\u0103",     "ro" },
    { "bg_BG.UTF-8",     "bg", "BG", "\u0411\u044a\u043b\u0433\u0430\u0440\u0441\u043a\u0438", "bg" },
    { "el_GR.UTF-8",     "el", "GR", "\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac", "gr" },
    { "he_IL.UTF-8",     "he", "IL", "\u05e2\u05d1\u05e8\u05d9\u05ea", "il" },
    { "ar_SA.UTF-8",     "ar", "SA", "\u0627\u0644\u0639\u0631\u0628\u064a\u0629", "ara" },
    { "fa_IR.UTF-8",     "fa", "IR", "\u0641\u0627\u0631\u0633\u06cc", "ir" },
    { "th_TH.UTF-8",     "th", "TH", "\u0e44\u0e17\u0e22",    "th" },
    { "vi_VN.UTF-8",     "vi", "VN", "Ti\u1ebfng Vi\u1ec7t",  "vn" },
    { "tr_TR.UTF-8",     "tr", "TR", "T\u00fcrk\u00e7e",      "tr" },
    { "uk_UA.UTF-8",     "uk", "UA", "\u0423\u043a\u0440\u0430\u0457\u043d\u0441\u044c\u043a\u0430", "ua" },
    { "et_EE.UTF-8",     "et", "EE", "Eesti",                 "ee" },
    { "lv_LV.UTF-8",     "lv", "LV", "Latvie\u0161u",         "lv" },
    { "lt_LT.UTF-8",     "lt", "LT", "Lietuvi\u0173",         "lt" },
    { "hr_HR.UTF-8",     "hr", "HR", "Hrvatski",              "hr" },
    { "sr_RS.UTF-8",     "sr", "RS", "\u0421\u0440\u043f\u0441\u043a\u0438", "rs" },
    { "sl_SI.UTF-8",     "sl", "SI", "Sloven\u0161\u010dina", "si" },
    { "mk_MK.UTF-8",     "mk", "MK", "\u041c\u0430\u043a\u0435\u0434\u043e\u043d\u0441\u043a\u0438", "mk" },
    {  NULL, NULL, NULL, NULL, NULL }
};

static const LocaleEntry *
find_locale_entry (const char *locale)
{
    if (!locale) return NULL;
    for (int i = 0; locale_table[i].locale; i++) {
        if (g_strcmp0 (locale_table[i].locale, locale) == 0)
            return &locale_table[i];
    }
    return NULL;
}

gchar **
gnome_get_all_locales (void)
{
    GPtrArray *arr = g_ptr_array_new ();
    for (int i = 0; locale_table[i].locale; i++)
        g_ptr_array_add (arr, g_strdup (locale_table[i].locale));
    g_ptr_array_add (arr, NULL);
    return (gchar **) g_ptr_array_free (arr, FALSE);
}

gboolean
gnome_parse_locale (const gchar  *locale,
                     gchar       **language_code,
                     gchar       **country_code,
                     gchar       **codeset,
                     gchar       **modifier)
{
    if (!locale || !*locale)
        return FALSE;

    g_auto(GStrv) parts = g_strsplit (locale, ".", 2);
    g_auto(GStrv) lang_country = g_strsplit (parts[0], "_", 2);

    if (!lang_country[0] || !*lang_country[0])
        return FALSE;

    if (language_code)
        *language_code = g_strdup (lang_country[0]);
    if (country_code)
        *country_code = lang_country[1] ? g_strdup (lang_country[1]) : NULL;
    if (codeset)
        *codeset = parts[1] ? g_strdup (parts[1]) : NULL;
    if (modifier)
        *modifier = NULL;

    return TRUE;
}

gchar *
gnome_get_language_from_locale (const gchar *locale,
                                 const gchar *locale_to_use)
{
    const LocaleEntry *e = find_locale_entry (locale);
    if (e)
        return g_strdup (e->language_name);
    return g_strdup (locale);
}

gchar *
gnome_get_language_from_code (const gchar *code,
                               const gchar *locale_to_use)
{
    for (int i = 0; locale_table[i].locale; i++) {
        if (g_strcmp0 (locale_table[i].lang_code, code) == 0)
            return g_strdup (locale_table[i].language_name);
    }
    return g_strdup (code);
}

gboolean
gnome_get_input_source_from_locale (const gchar  *locale,
                                     const gchar **type,
                                     const gchar **id)
{
    const LocaleEntry *e = find_locale_entry (locale);
    if (!e)
        return FALSE;
    if (type) *type = "xkb";
    if (id)   *id   = e->layout_id;
    return TRUE;
}

GHashTable *
cc_common_language_get_initial_languages (void)
{
    return g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}
