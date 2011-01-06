/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 * 		Tom "TAsn" Hacohen <tom@stosb.com>
 * 		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */


#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>
#include <ctype.h>

#include "common-utils.h"

static GHashTable *ref_counter = NULL;


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

void
common_utils_variant_unref(void* value)
{
	g_variant_unref(value);
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
		if (isspace(*p)) {
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
		GVariant *val = _val;
		g_debug("--| %s: %s", key, g_variant_print(val, TRUE));
	}
}
