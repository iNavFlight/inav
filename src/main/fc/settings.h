#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config/parameter_group.h"

#include "settings_generated.h"

typedef struct lookupTableEntry_s {
    const char * const *values;
    const uint8_t valueCount;
} lookupTableEntry_t;

#define SETTING_TYPE_OFFSET 0
#define SETTING_SECTION_OFFSET 4
#define SETTING_MODE_OFFSET 6

typedef enum {
    // value type, bits 0-3
    VAR_UINT8 = (0 << SETTING_TYPE_OFFSET),
    VAR_INT8 = (1 << SETTING_TYPE_OFFSET),
    VAR_UINT16 = (2 << SETTING_TYPE_OFFSET),
    VAR_INT16 = (3 << SETTING_TYPE_OFFSET),
    VAR_UINT32 = (4 << SETTING_TYPE_OFFSET),
    VAR_FLOAT = (5 << SETTING_TYPE_OFFSET), // 0x05
    VAR_STRING = (6 << SETTING_TYPE_OFFSET) // 0x06
} setting_type_e;

typedef enum {
    // value section, bits 4-5
    MASTER_VALUE = (0 << SETTING_SECTION_OFFSET),
    PROFILE_VALUE = (1 << SETTING_SECTION_OFFSET),
    CONTROL_RATE_VALUE = (2 << SETTING_SECTION_OFFSET), // 0x20
    BATTERY_CONFIG_VALUE = (3 << SETTING_SECTION_OFFSET),
} setting_section_e;

typedef enum {
    // value mode, bits 6-7
    MODE_DIRECT = (0 << SETTING_MODE_OFFSET),
    MODE_LOOKUP = (1 << SETTING_MODE_OFFSET), // 0x40
} setting_mode_e;

#define SETTING_TYPE_MASK (0x0F)
#define SETTING_SECTION_MASK (0x30)
#define SETTING_MODE_MASK (0xC0)

typedef struct settingMinMaxConfig_s {
    const uint8_t indexes[SETTING_MIN_MAX_INDEX_BYTES];
} settingMinMaxConfig_t;

typedef struct settingLookupTableConfig_s {
    const uint8_t tableIndex;
} settingLookupTableConfig_t;

typedef union {
    settingLookupTableConfig_t lookup;
    settingMinMaxConfig_t minmax;
} settingConfig_t;

typedef struct {
    const uint8_t encoded_name[SETTING_ENCODED_NAME_MAX_BYTES];
    const uint8_t type; // see settingFlag_e
    const settingConfig_t config;
    const setting_offset_t offset;

} __attribute__((packed)) setting_t;

static inline setting_type_e SETTING_TYPE(const setting_t *s) { return s->type &  SETTING_TYPE_MASK; }
static inline setting_section_e SETTING_SECTION(const setting_t *s) { return s->type & SETTING_SECTION_MASK; }
static inline setting_mode_e SETTING_MODE(const setting_t *s) { return s->type & SETTING_MODE_MASK; }

void settingGetName(const setting_t *val, char *buf);
bool settingNameContains(const setting_t *val, char *buf, const char *cmdline);
bool settingNameIsExactMatch(const setting_t *val, char *buf, const char *cmdline, uint8_t var_name_length);
// Returns a setting_t with the exact name (case sensitive), or
// NULL if no setting with that name exists.
const setting_t *settingFind(const char *name);
// Returns the setting at the given index, or NULL if
// the index is greater than the total count.
const setting_t *settingGet(unsigned index);
// Returns the setting index for the given setting.
unsigned settingGetIndex(const setting_t *val);
// Checks if all settings have values in their valid ranges.
// If they don't, invalidIndex is filled with the first invalid
// settings index and false is returned.
bool settingsValidate(unsigned *invalidIndex);
// Returns the size in bytes of the setting value.
size_t settingGetValueSize(const setting_t *val);
pgn_t settingGetPgn(const setting_t *val);
// Returns a pointer to the actual value stored by
// the setting_t. The returned value might be modified.
void * settingGetValuePointer(const setting_t *val);
// Returns a pointer to the backed up copy of the value. Note that
// this will contain random garbage unless a copy of the parameter
// group for the value has been manually performed. Currently, this
// is only used by cli.c during config dumps.
const void * settingGetCopyValuePointer(const setting_t *val);
// Returns the minimum valid value for the given setting_t. setting_min_t
// depends on the target and build options, but will always be a signed
// integer (e.g. intxx_t,)
setting_min_t settingGetMin(const setting_t *val);
// Returns the maximum valid value for the given setting_t. setting_max_t
// depends on the target and build options, but will always be an unsigned
// integer (e.g. uintxx_t,)
setting_max_t settingGetMax(const setting_t *val);
// Returns the lookup table for the given setting. If the setting mode
// is not MODE_LOOKUP, it returns NULL;
const lookupTableEntry_t * settingLookupTable(const setting_t *val);
// Returns the string in the table which corresponds to the value v
// for the given setting. If the setting mode is not MODE_LOOKUP or
// if the value is out of range, it returns NULL.
const char * settingLookupValueName(const setting_t *val, unsigned v);
// Returns the length of the longest value name for the given setting,
// or 0 if the setting does not use value names.
size_t settingGetValueNameMaxSize(const setting_t *val);
// Returns the setting value as a const char * iff the setting is of type
// VAR_STRING. Otherwise it returns NULL.
const char * settingGetString(const setting_t *val);
// Sets the value for the given string setting. Size indicates the number of
// bytes in the string without the '\0' terminator (i.e. its strlen()).
// If the setting is not of type VAR_STRING, this function does nothing.
void settingSetString(const setting_t *val, const char *s, size_t size);
// Returns the max string length (without counting the '\0' terminator)
// for setting of type VAR_STRING. Otherwise it returns 0.
setting_max_t settingGetStringMaxLength(const setting_t *val);

// Retrieve the setting indexes for the given PG. If the PG is not
// found, these function returns false.
bool settingsGetParameterGroupIndexes(pgn_t pg, uint16_t *start, uint16_t *end);
