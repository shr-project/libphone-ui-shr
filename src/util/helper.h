#ifndef _HELPER_H
#define _HELPER_H

#include <time.h>
#include <glib.h>

char *string_replace_with_tags(char *string);
gboolean string_is_pin(const char *string);
gboolean string_is_puk(const char *string);
const char *string_skip_tel_prefix(const char *string);

#endif
