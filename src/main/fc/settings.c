#include <string.h>
#include <stdint.h>

#include "common/string_light.h"
#include "common/utils.h"

#include "settings_generated.h"

#include "fc/settings.h"

#include "config/general_settings.h"
#include "flight/rpm_filter.h"
#include "settings_generated.c"

static bool settingGetWord(char *buf, int idx)
{
	if (idx == 0) {
		return false;
	}
	const uint8_t *ptr = settingNamesWords;
	char *bufPtr = buf;
	int used_bits = 0;
	int word = 1;
	for(;;) {
		int shift = 8 - SETTINGS_WORDS_BITS_PER_CHAR - used_bits;
		char chr;
		if (shift > 0) {
			chr = (*ptr >> shift) & (0xff >> (8 - SETTINGS_WORDS_BITS_PER_CHAR));
		} else {
			chr = (*ptr & (0xff >> (8 - (SETTINGS_WORDS_BITS_PER_CHAR + shift)))) << -shift;
			ptr++;
			chr |= (*ptr) >> (8 + shift);
		}
		if (word == idx) {
			if (chr == 0) {
				// Finished copying the word
				*bufPtr++ = '\0';
				break;
			}
			char c;
			if (chr < 27) {
				c = 'a' + (chr - 1);
			} else {
				c = wordSymbols[chr - 27];
			}
			*bufPtr++ = c;
		} else {
			if (chr == 0) {
				// Word end
				word++;
			}
		}
		used_bits = (used_bits + SETTINGS_WORDS_BITS_PER_CHAR) % 8;
	}
	return true;
}

void settingGetName(const setting_t *val, char *buf)
{
	uint8_t bpos = 0;
	uint16_t n = 0;
	char word[SETTING_MAX_WORD_LENGTH];
#ifndef SETTING_ENCODED_NAME_USES_BYTE_INDEXING
	uint8_t shift = 0;
#endif
	for (uint8_t ii = 0; ii < SETTING_ENCODED_NAME_MAX_BYTES; ii++) {
#ifdef SETTING_ENCODED_NAME_USES_BYTE_INDEXING
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
		if (!settingGetWord(word, n)) {
			// No more words
			break;
		}
		if (bpos > 0) {
			// Word separator
			buf[bpos++] = '_';
		}
		strcpy(&buf[bpos], word);
		bpos += strlen(word);
#ifndef SETTING_ENCODED_NAME_USES_BYTE_INDEXING
		// Reset shift and n
		shift = 0;
		n = 0;
#endif
	}
	buf[bpos] = '\0';
}

bool settingNameContains(const setting_t *val, char *buf, const char *cmdline)
{
	settingGetName(val, buf);
	return strstr(buf, cmdline) != NULL;
}

bool settingNameIsExactMatch(const setting_t *val, char *buf, const char *cmdline, uint8_t var_name_length)
{
	settingGetName(val, buf);
	return sl_strncasecmp(cmdline, buf, strlen(buf)) == 0 && var_name_length == strlen(buf);
}

const setting_t *settingFind(const char *name)
{
	char buf[SETTING_MAX_NAME_LENGTH];
	for (int ii = 0; ii < SETTINGS_TABLE_COUNT; ii++) {
		const setting_t *setting = &settingsTable[ii];
		settingGetName(setting, buf);
		if (strcmp(buf, name) == 0) {
			return setting;
		}
	}
	return NULL;
}

const setting_t *settingGet(unsigned index)
{
	return index < SETTINGS_TABLE_COUNT ? &settingsTable[index] : NULL;
}

unsigned settingGetIndex(const setting_t *val)
{
	return val - settingsTable;
}

bool settingsValidate(unsigned *invalidIndex)
{
	for (unsigned ii = 0; ii < SETTINGS_TABLE_COUNT; ii++) {
		const setting_t *setting = settingGet(ii);
		setting_min_t min = settingGetMin(setting);
		setting_max_t max = settingGetMax(setting);
		void *ptr = settingGetValuePointer(setting);
		bool isValid = false;
		switch (SETTING_TYPE(setting)) {
		case VAR_UINT8:
		{
			uint8_t *value = ptr;
			isValid = *value >= min && *value <= max;
			break;
		}
		case VAR_INT8:
		{
			int8_t *value = ptr;
			isValid = *value >= min && *value <= (int8_t)max;
			break;
		}
		case VAR_UINT16:
		{
			uint16_t *value = ptr;
			isValid = *value >= min && *value <= max;
			break;
		}
		case VAR_INT16:
		{
			int16_t *value = ptr;
			isValid = *value >= min && *value <= (int16_t)max;
			break;
		}
		case VAR_UINT32:
		{
			uint32_t *value = ptr;
			isValid = *value >= (uint32_t)min && *value <= max;
			break;
		}
		case VAR_FLOAT:
		{
			float *value = ptr;
			isValid = *value >= min && *value <= max;
			break;
		}
		case VAR_STRING:
			// We assume all strings are valid
			isValid = true;
			break;
		}
		if (!isValid) {
			if (invalidIndex) {
				*invalidIndex = ii;
			}
			return false;
		}
	}
	return true;
}

size_t settingGetValueSize(const setting_t *val)
{
	switch (SETTING_TYPE(val)) {
		case VAR_UINT8:
			FALLTHROUGH;
		case VAR_INT8:
			return 1;
		case VAR_UINT16:
			FALLTHROUGH;
		case VAR_INT16:
			return 2;
		case VAR_UINT32:
			FALLTHROUGH;
		case VAR_FLOAT:
			return 4;
		case VAR_STRING:
			return settingGetMax(val);
	}
	return 0; // Unreachable
}

pgn_t settingGetPgn(const setting_t *val)
{
	uint16_t pos = val - (const setting_t *)settingsTable;
	uint16_t acc = 0;
	for (uint8_t ii = 0; ii < SETTINGS_PGN_COUNT; ii++) {
		acc += settingsPgnCounts[ii];
		if (acc > pos) {
			return settingsPgn[ii];
		}
	}
	return -1;
}

static uint16_t getValueOffset(const setting_t *value)
{
    switch (SETTING_SECTION(value)) {
    case MASTER_VALUE:
        return value->offset;
    case PROFILE_VALUE:
        return value->offset + sizeof(pidProfile_t) * getConfigProfile();
    case CONTROL_RATE_VALUE:
        return value->offset + sizeof(controlRateConfig_t) * getConfigProfile();
    case BATTERY_CONFIG_VALUE:
        return value->offset + sizeof(batteryProfile_t) * getConfigBatteryProfile();
    }
    return 0;
}

void *settingGetValuePointer(const setting_t *val)
{
    const pgRegistry_t *pg = pgFind(settingGetPgn(val));
    return pg->address + getValueOffset(val);
}

const void * settingGetCopyValuePointer(const setting_t *val)
{
    const pgRegistry_t *pg = pgFind(settingGetPgn(val));
    return pg->copy + getValueOffset(val);
}

setting_min_t settingGetMin(const setting_t *val)
{
	if (SETTING_MODE(val) == MODE_LOOKUP) {
		return 0;
	}
	return settingMinMaxTable[SETTING_INDEXES_GET_MIN(val)];
}

setting_max_t settingGetMax(const setting_t *val)
{
	if (SETTING_MODE(val) == MODE_LOOKUP) {
		return settingLookupTables[val->config.lookup.tableIndex].valueCount - 1;
	}
	return settingMinMaxTable[SETTING_INDEXES_GET_MAX(val)];
}

const lookupTableEntry_t * settingLookupTable(const setting_t *val)
{
	if (SETTING_MODE(val) == MODE_LOOKUP && val->config.lookup.tableIndex < LOOKUP_TABLE_COUNT) {
		return &settingLookupTables[val->config.lookup.tableIndex];
	}
	return NULL;
}

const char * settingLookupValueName(const setting_t *val, unsigned v)
{
	const lookupTableEntry_t *table = settingLookupTable(val);
	if (table && v < table->valueCount) {
		return table->values[v];
	}
	return NULL;
}

size_t settingGetValueNameMaxSize(const setting_t *val)
{
	size_t maxSize = 0;
	const lookupTableEntry_t *table = settingLookupTable(val);
	if (table) {
		for (unsigned ii = 0; ii < table->valueCount; ii++) {
			maxSize = MAX(maxSize, strlen(table->values[ii]));
		}
	}
	return maxSize;
}

const char * settingGetString(const setting_t *val)
{
	if (SETTING_TYPE(val) == VAR_STRING) {
		return settingGetValuePointer(val);
	}
	return NULL;
}

void settingSetString(const setting_t *val, const char *s, size_t size)
{
	if (SETTING_TYPE(val) == VAR_STRING) {
		char *p = settingGetValuePointer(val);
		size_t copySize = MIN(size, settingGetMax(val));
		memcpy(p, s, copySize);
		p[copySize] = '\0';
	}
}

setting_max_t settingGetStringMaxLength(const setting_t *val)
{
	if (SETTING_TYPE(val) == VAR_STRING) {
		// Max string length is stored as its max
		return settingGetMax(val);
	}
	return 0;
}

bool settingsGetParameterGroupIndexes(pgn_t pg, uint16_t *start, uint16_t *end)
{
	unsigned acc = 0;
	for (int ii = 0; ii < SETTINGS_PGN_COUNT; ii++) {
		if (settingsPgn[ii] == pg) {
			if (start) {
				*start = acc;
			}
			if (end) {
				*end = acc + settingsPgnCounts[ii] - 1;
			}
			return true;
		}
		acc += settingsPgnCounts[ii];
	}
	return false;
}
