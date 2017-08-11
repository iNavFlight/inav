#include <string.h>

#include "common/string_light.h"

#include "fc/settings_generated.h"
#include "fc/settings.h"
#include "fc/settings_generated.c"

void clivalue_get_name(const clivalue_t *val, char *buf)
{
#ifdef CLIVALUE_COMPACT_NAMES
	uint8_t bpos = 0;
	for (uint8_t ii = 0; val->compact_name[ii] && ii <= CLIVALUE_MAX_NAME_BYTES; ii++) {
		if (ii > 0) {
			buf[bpos++] = '_';
		}
		const char *word = words[val->compact_name[ii]];
		strcpy(&buf[bpos], word);
		bpos += strlen(word);
	}
#else
	strcpy(buf, val->name);
#endif
}

uint8_t clivalue_name_contains(const clivalue_t *val, const char *cmdline)
{
#ifdef CLIVALUE_COMPACT_NAMES
	char name[CLIVALUE_MAX_NAME_LENGTH];
	clivalue_get_name(val, name);
#else
	const char *name = val->name;
#endif
	return strstr(name, cmdline) != NULL;
}

uint8_t clivalue_name_exact_match(const clivalue_t *val, const char *cmdline, uint8_t var_name_length)
{
#ifdef CLIVALUE_COMPACT_NAMES
	char name[CLIVALUE_MAX_NAME_LENGTH];
	clivalue_get_name(val, name);
#else
	const char *name = val->name;
#endif
	return sl_strncasecmp(cmdline, name, strlen(name)) == 0 && var_name_length == strlen(name);
}
