#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>

#include "common-utils.h"

char *
common_utils_skip_prefix(char *string, const char *prefix)
{
	int prefix_len = strlen(prefix);
	if (!strncmp(string, prefix, prefix_len)) {
		string += prefix_len;
	}
	return string;
}

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

char *
common_utils_new_with_prefix(const char *_number, const char *prefix)
{
	char *number = NULL;
	int prefix_len = strlen(prefix);
	if (strncmp(_number, prefix, prefix_len)) {
		number = malloc(strlen(_number) + prefix_len + 1); /* 5 is for "tel:" and the null */
		if (!number) {
			return NULL;
		}
		strcpy(number, prefix);
		strcat(number, _number);
	}


	return number;
}