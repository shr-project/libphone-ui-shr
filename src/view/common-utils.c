#include "common-utils.h"

char *
common_utils_skip_tel_prefix(const char *string)
{
	if (!strncmp(string, "tel:", 4)) {
		string += 4;
	}
	return string;
}