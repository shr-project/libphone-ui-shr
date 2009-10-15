#include "time.h"
#include <time.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>


char *
string_replace_with_tags(char *string)
{
	int newlen = 0;
	char *in_p = string, *out_p;

	/* scan the string to see how much longer we have to get */
	while (*in_p) {
		switch (*in_p++) {
		case '\n':
			newlen += 3;
			break;
		case '\t':
			newlen += 4;
			break;
		}
	}

	if (!newlen)
		return (string);

	newlen += strlen(string);
	char *newstring = malloc(newlen + 1);
	in_p = string;
	out_p = newstring;
	while (*in_p) {
		switch (*in_p) {
		case '\n':
			*out_p++ = '<';
			*out_p++ = 'b';
			*out_p++ = 'r';
			*out_p++ = '>';
			break;
		case '\t':
			*out_p++ = '<';
			*out_p++ = 't';
			*out_p++ = 'a';
			*out_p++ = 'b';
			*out_p++ = '>';
			break;
		default:
			*out_p++ = *in_p;
			break;
		}
		in_p++;
	}
	*out_p = '\0';

	return (newstring);
}


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
