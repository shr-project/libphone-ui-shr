#ifndef _COMMON_UTILS_H
#define _COMMON_UTILS_H
#include <glib.h>

char *
common_utils_skip_prefix(const char *string, const char *prefix);
char *
common_utils_add_prefix(const char *_number, const char *prefix);

GValue *
common_utils_new_gvalue_string(const char *value);

GValue *
common_utils_new_gvalue_int(int value);

GValue *
common_utils_new_gvalue_boolean(int value);

#endif