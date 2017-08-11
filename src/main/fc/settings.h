#pragma once

#include <stdint.h>

typedef struct lookupTableEntry_s {
    const char * const *values;
    const uint8_t valueCount;
} lookupTableEntry_t;

#define VALUE_TYPE_OFFSET 0
#define VALUE_SECTION_OFFSET 4
#define VALUE_MODE_OFFSET 6

typedef enum {
    // value type, bits 0-3
    VAR_UINT8 = (0 << VALUE_TYPE_OFFSET),
    VAR_INT8 = (1 << VALUE_TYPE_OFFSET),
    VAR_UINT16 = (2 << VALUE_TYPE_OFFSET),
    VAR_INT16 = (3 << VALUE_TYPE_OFFSET),
    VAR_UINT32 = (4 << VALUE_TYPE_OFFSET),
    VAR_FLOAT = (5 << VALUE_TYPE_OFFSET), // 0x05

    // value section, bits 4-5
    MASTER_VALUE = (0 << VALUE_SECTION_OFFSET),
    PROFILE_VALUE = (1 << VALUE_SECTION_OFFSET),
    CONTROL_RATE_VALUE = (2 << VALUE_SECTION_OFFSET), // 0x20
    // value mode, bits 6-7
    MODE_DIRECT = (0 << VALUE_MODE_OFFSET),
    MODE_LOOKUP = (1 << VALUE_MODE_OFFSET), // 0x40
    MODE_MAX = (2 << VALUE_MODE_OFFSET), // 0x80
} cliValueFlag_e;

#define VALUE_TYPE_MASK (0x0F)
#define VALUE_SECTION_MASK (0x30)
#define VALUE_MODE_MASK (0xC0)

typedef struct cliMinMaxConfig_s {
    const int16_t min;
    const int16_t max;
} cliMinMaxConfig_t;

typedef struct cliMaxConfig_s {
    const uint32_t max;
} cliMaxConfig_t;

typedef struct cliLookupTableConfig_s {
    const uint8_t tableIndex;
} cliLookupTableConfig_t;

typedef union {
    cliLookupTableConfig_t lookup;
    cliMinMaxConfig_t minmax;
    cliMaxConfig_t max;
} cliValueConfig_t;

typedef struct {
#ifdef CLIVALUE_COMPACT_NAMES
    const uint8_t compact_name[CLIVALUE_MAX_NAME_BYTES];
#else
    const char *name;
#endif
    const uint8_t type; // see cliValueFlag_e
    const cliValueConfig_t config;

    pgn_t pgn;
    uint16_t offset;
} __attribute__((packed)) clivalue_t;

void clivalue_get_name(const clivalue_t *val, char *buf);
uint8_t clivalue_name_contains(const clivalue_t *val, const char *cmdline);
uint8_t clivalue_name_exact_match(const clivalue_t *val, const char *cmdline, uint8_t var_name_length);