#include <string.h>

#include "common/string_light.h"

#include "fc/settings_generated.h"
#include "fc/settings.h"

#include "fc/settings_generated.c"

extern const char *cliValueWords[];

void clivalue_get_name(const clivalue_t *val, char *buf)
{
	uint8_t bpos = 0;
	uint16_t n = 0;
#ifndef CLIVALUE_ENCODED_NAME_USES_DIRECT_INDEXING
	uint8_t shift = 0;
#endif
	for (uint8_t ii = 0; ii <= CLIVALUE_ENCODED_NAME_MAX_BYTES; ii++) {
#ifdef CLIVALUE_ENCODED_NAME_USES_DIRECT_INDEXING
		n = val->encoded_name[ii];
#else
		// Decode a variable size uint
		uint16_t b = val->encoded_name[ii];
		if (b >= 0x80) {
			// More bytes follow
			n |= (b&0x7f) << shift;
			shift += 7;
			continue;
		}
		// Final byte
		n |= b << shift;
#endif
		const char *word = cliValueWords[n];
		if (!word) {
			// No more words
			break;
		}
		if (bpos > 0) {
			// Word separator
			buf[bpos++] = '_';
		}
		strcpy(&buf[bpos], word);
		bpos += strlen(word);
#ifndef CLIVALUE_ENCODED_NAME_USES_DIRECT_INDEXING
		// Reset shift and n
		shift = 0;
		n = 0;
#endif
	}
	buf[bpos] = '\0';
}

bool clivalue_name_contains(const clivalue_t *val, const char *cmdline)
{
	char name[CLIVALUE_MAX_NAME_LENGTH];
	clivalue_get_name(val, name);
	return strstr(name, cmdline) != NULL;
}

bool clivalue_name_exact_match(const clivalue_t *val, const char *cmdline, uint8_t var_name_length)
{
	char name[CLIVALUE_MAX_NAME_LENGTH];
	clivalue_get_name(val, name);
	return sl_strncasecmp(cmdline, name, strlen(name)) == 0 && var_name_length == strlen(name);
}
