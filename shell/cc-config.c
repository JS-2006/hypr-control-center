#include "cc-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static JsonNode *root = NULL;
static gchar *config_path = NULL;

static gchar *
get_config_path (void)
{
    const gchar *config_dir;
    gchar *path;

    config_dir = g_get_user_config_dir ();
    path = g_build_filename (config_dir, "hypr", CC_CONFIG_FILE, NULL);

    return path;
}

gboolean
cc_config_init (GError **error)
{
    JsonParser *parser;
    gboolean ret = TRUE;

    config_path = get_config_path ();

    if (g_file_test (config_path, G_FILE_TEST_EXISTS))
    {
        gchar *contents;
        gsize len;

        if (!g_file_get_contents (config_path, &contents, &len, error))
            return FALSE;

        parser = json_parser_new ();
        if (!json_parser_load_from_data (parser, contents, len, error))
        {
            g_object_unref (parser);
            g_free (contents);
            return FALSE;
        }

        root = json_node_copy (json_parser_get_root (parser));
        g_object_unref (parser);
        g_free (contents);
    }
    else
    {
        JsonBuilder *builder;

        builder = json_builder_new ();
        json_builder_begin_object (builder);
        json_builder_end_object (builder);
        root = json_builder_get_root (builder);
        g_object_unref (builder);
    }

    return TRUE;
}

void
cc_config_shutdown (void)
{
    if (root)
        json_node_free (root);
    g_free (config_path);
    root = NULL;
    config_path = NULL;
}

static JsonObject *
get_or_create_object (void)
{
    if (!root || !JSON_NODE_HOLDS_OBJECT (root))
        return NULL;
    return json_node_get_object (root);
}

gchar *
cc_config_get_string (const gchar *key)
{
    JsonObject *obj;

    obj = get_or_create_object ();
    if (!obj || !json_object_has_member (obj, key))
        return NULL;

    return g_strdup (json_object_get_string_member (obj, key));
}

void
cc_config_set_string (const gchar *key, const gchar *value)
{
    JsonObject *obj;
    gchar *contents;
    JsonGenerator *gen;

    obj = get_or_create_object ();
    if (!obj)
        return;

    if (value)
        json_object_set_string_member (obj, key, value);
    else
        json_object_remove_member (obj, key);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    contents = json_generator_to_data (gen, NULL);
    g_file_set_contents (config_path, contents, -1, NULL);
    g_free (contents);
    g_object_unref (gen);
}

gboolean
cc_config_get_boolean (const gchar *key)
{
    JsonObject *obj;

    obj = get_or_create_object ();
    if (!obj || !json_object_has_member (obj, key))
        return FALSE;

    return json_object_get_boolean_member (obj, key);
}

void
cc_config_set_boolean (const gchar *key, gboolean value)
{
    JsonObject *obj;
    gchar *contents;
    JsonGenerator *gen;

    obj = get_or_create_object ();
    if (!obj)
        return;

    json_object_set_boolean_member (obj, key, value);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    contents = json_generator_to_data (gen, NULL);
    g_file_set_contents (config_path, contents, -1, NULL);
    g_free (contents);
    g_object_unref (gen);
}

gint
cc_config_get_int (const gchar *key)
{
    JsonObject *obj;

    obj = get_or_create_object ();
    if (!obj || !json_object_has_member (obj, key))
        return -1;

    return (gint) json_object_get_int_member (obj, key);
}

void
cc_config_set_int (const gchar *key, gint value)
{
    JsonObject *obj;
    gchar *contents;
    JsonGenerator *gen;

    obj = get_or_create_object ();
    if (!obj)
        return;

    json_object_set_int_member (obj, key, value);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    contents = json_generator_to_data (gen, NULL);
    g_file_set_contents (config_path, contents, -1, NULL);
    g_free (contents);
    g_object_unref (gen);
}

gdouble
cc_config_get_double (const gchar *key)
{
    JsonObject *obj;

    obj = get_or_create_object ();
    if (!obj || !json_object_has_member (obj, key))
        return 0.0;
    return json_object_get_double_member (obj, key);
}

void
cc_config_set_double (const gchar *key, gdouble value)
{
    JsonObject *obj;
    gchar *contents;
    JsonGenerator *gen;

    obj = get_or_create_object ();
    if (!obj)
        return;

    json_object_set_double_member (obj, key, value);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    contents = json_generator_to_data (gen, NULL);
    g_file_set_contents (config_path, contents, -1, NULL);
    g_free (contents);
    g_object_unref (gen);
}

gboolean
cc_config_get_window_state (gint *width, gint *height, gboolean *maximized)
{
    JsonObject *obj;

    obj = get_or_create_object ();
    if (!obj)
        return FALSE;

    if (!json_object_has_member (obj, "window_state"))
        return FALSE;

    {
        JsonObject *state = json_object_get_object_member (obj, "window_state");

        if (width)
            *width = (gint) json_object_get_int_member (state, "width");
        if (height)
            *height = (gint) json_object_get_int_member (state, "height");
        if (maximized)
            *maximized = json_object_get_boolean_member (state, "maximized");

        return TRUE;
    }
}

void
cc_config_set_window_state (gint width, gint height, gboolean maximized)
{
    JsonObject *obj;
    JsonObject *state;
    gchar *contents;
    JsonGenerator *gen;

    obj = get_or_create_object ();
    if (!obj)
        return;

    if (json_object_has_member (obj, "window_state"))
        state = json_object_get_object_member (obj, "window_state");
    else
    {
        state = json_object_new ();
        json_object_set_object_member (obj, "window_state", state);
    }

    json_object_set_int_member (state, "width", width);
    json_object_set_int_member (state, "height", height);
    json_object_set_boolean_member (state, "maximized", maximized);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    contents = json_generator_to_data (gen, NULL);
    g_file_set_contents (config_path, contents, -1, NULL);
    g_free (contents);
    g_object_unref (gen);
}
