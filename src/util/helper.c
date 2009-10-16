#include "time.h"
#include <time.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>


gboolean
string_is_pin(const char *string)
{
	if (strlen(string) < 4 || strlen(string) > 8)
		return FALSE;

	const char *p = string;
	while (*p) {
		if (*p < '0' || *p > '9')
			return FALSE;
		*p++;
	}

	return TRUE;
}


gboolean
string_is_puk(const char *string)
{
	if (strlen(string) != 8)
		return FALSE;

	const char *p = string;
	while (*p) {
		if (*p < '0' || *p > '9')
			return FALSE;
		*p++;
	}

	return TRUE;
}



const char *
string_skip_tel_prefix(const char *string)
{
	if (string[0] == 't' && string[1] == 'e' && string[2] == 'l'
	    && string[3] == ':')
		return (string + 4);
	return (string);
}
