#ifndef _COMMON_UTILS_H
#define _COMMON_UTILS_H
#include <glib.h>
#include <glib-object.h>

char *
common_utils_skip_prefix(char *string, const char *prefix);

char *
common_utils_new_with_prefix(const char *_number, const char *prefix);

GValue *
common_utils_new_gvalue_string(const char *value);

GValue *
common_utils_new_gvalue_int(int value);

GValue *
common_utils_new_gvalue_boolean(int value);

void *
common_utils_object_ref(void *object);

int
common_utils_object_unref(void *object);
void
common_utils_object_unref_free(void *object);
int
common_utils_object_get_ref(void *object);

int
common_utils_is_pin(const char *string);
int
common_utils_is_puk(const char *string);

#endif