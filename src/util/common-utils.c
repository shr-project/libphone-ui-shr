#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>
#include <ctype.h>

#include "common-utils.h"

static GHashTable *ref_counter = NULL;

GValue *
common_utils_new_gvalue_string(const char *value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_STRING);
	g_value_set_string(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_int(int value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_INT);
	g_value_set_int(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_boolean(int value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_BOOLEAN);
	g_value_set_boolean(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_pointer(gpointer value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_POINTER);
	g_value_set_pointer(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_boxed(GType type, gpointer value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, type);
	g_value_set_boxed_take_ownership(val, value);

	return val;
}

void
common_utils_gvalue_free(gpointer val)
{
	GValue *value = (GValue *)val;
	g_value_unset(value);
	g_free(value);
}

void *
common_utils_object_ref(void *object)
{
	void *ret;
	int count;
	if (!ref_counter) {
		ref_counter = g_hash_table_new_full(g_direct_hash, g_direct_equal,
					NULL, NULL);
	}
	ret = g_hash_table_lookup(ref_counter, object);
	if (ret) {
		count = GPOINTER_TO_INT(ret);
		count++;
	}
	else {
		count = 1;
	}
	g_hash_table_replace(ref_counter, object, GINT_TO_POINTER(count));
	return object;
}

int
common_utils_object_unref(void *object)
{
	void *ret;
	int count;

	ret = g_hash_table_lookup(ref_counter, object);
	if (!ret) {
		return -1;
	}

	count = GPOINTER_TO_INT(ret);
	if (count <= 1) {
		g_hash_table_remove(ref_counter, object);
		return 0;
	}
	else {
		g_hash_table_replace(ref_counter, object, GINT_TO_POINTER(count - 1));
	}
	return count;
}

void
common_utils_object_unref_free(void *object)
{
	if (common_utils_object_unref(object) <= 1) {
		free(object);
	}
}

int
common_utils_object_get_ref(void *object)
{
	void *ret;
	int count;
	ret = g_hash_table_lookup(ref_counter, object);
	if (!ret) {
		return 0;
	}

	count = GPOINTER_TO_INT(ret);
	return count;
}

int
common_utils_is_pin(const char *string)
{
	if (strlen(string) < 4 || strlen(string) > 8)
		return 0;

	const char *p;
	for (p = string ; *p ; p++) {
		if (!isdigit(*p)) {
			return 0;
		}
	}

	return 1;
}


int
common_utils_is_puk(const char *string)
{
	if (strlen(string) != 8)
		return 0;

	const char *p;
	for (p = string ; *p ; p++) {
		if (!isdigit(*p)) {
			return 0;
		}
	}

	return 1;
}

char *
common_utils_string_strip_newline(char *string)
{
	char *p;

	for (p = string; *p; p++) {
		if (*p == '\n' || *p == '\t') {
			*p = ' ';
		}
	}
	return string;
}

char *
common_utils_timestamp_to_date(long timestamp)
{
	char *ret = malloc(35);
	strftime(ret, 31, "%d.%m.%Y %H:%M" LTR_STRING, localtime(&timestamp));
	return ret;
}

void
common_utils_debug_dump_hashtable(GHashTable* hasht)
{
	GHashTableIter iter;
	gpointer _key, _val;

	g_debug("Debug Dump of HashTable");
	if (!hasht) {
		g_debug("--| hashtable is NULL");
		return;
	}
	g_hash_table_iter_init(&iter, hasht);
	while (g_hash_table_iter_next(&iter, &_key, &_val)) {
		const char *key = (const char *)_key;
		g_debug("--| key = '%s'", key);
		if (G_IS_VALUE(_val)) {
			const GValue *val = (const GValue *) _val;
			if (G_VALUE_HOLDS_BOXED(val)) {
				char **vl = (char **)g_value_get_boxed(val);
				int i = 0;
				while (vl[i]) {
					g_debug("----| '%s'", vl[i]);
					i++;
				}
			}
			else if (G_VALUE_HOLDS_STRING(val)) {
				g_debug("----| '%s'", g_value_get_string(val));
			}
			else if (G_VALUE_HOLDS_INT(val)) {
				g_debug("----| %d", g_value_get_int(val));
			}
			else if (G_VALUE_HOLDS_BOOLEAN(val)) {
				g_debug("----| %s", g_value_get_boolean(val) ?
							"TRUE" : "FALSE");
			}
			else {
				g_debug("----| value is of unhandled type!");
			}
		}
		else {
			g_debug("----| not a gvalue");
		}
	}
}
