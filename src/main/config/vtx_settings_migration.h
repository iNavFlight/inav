#pragma once

#include <stddef.h>
#include <string.h>
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "io/vtx.h"

// Version 2 placed frequencyGroup immediately after maxPowerOverride.
// Version 3 inserts the optional Tramp table before frequencyGroup.
typedef struct vtxSettingsConfigV2_s {
    uint8_t band;
    uint8_t channel;
    uint8_t power;
    uint16_t pitModeChan;
    uint8_t lowPowerDisarm;
    uint16_t maxPowerOverride;
    uint8_t frequencyGroup;
} vtxSettingsConfigV2_t;

static inline bool pgMigrateVtxSettings(const pgRegistry_t *reg, void *to,
    const void *from, int size, int version)
{
    if (pgN(reg) != PG_VTX_SETTINGS_CONFIG || pgVersion(reg) != 3 || version != 2 ||
        pgSize(reg) != sizeof(vtxSettingsConfig_t) || size != sizeof(vtxSettingsConfigV2_t)) {
        return false;
    }
    // Copy from potentially unaligned EEPROM data through a local old record.
    vtxSettingsConfigV2_t old;
    memcpy(&old, from, sizeof(old));
    vtxSettingsConfig_t *dest = (vtxSettingsConfig_t *)to;
    dest->band = old.band;
    dest->channel = old.channel;
    dest->power = old.power;
    dest->pitModeChan = old.pitModeChan;
    dest->lowPowerDisarm = old.lowPowerDisarm;
    dest->maxPowerOverride = old.maxPowerOverride;
    dest->frequencyGroup = old.frequencyGroup;
    memset(dest->trampPowerLevels, 0, sizeof(dest->trampPowerLevels));
    return true;
}
