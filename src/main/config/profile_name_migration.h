#pragma once

#include <stddef.h>
#include <string.h>

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "fc/control_profile_config_struct.h"
#include "sensors/battery_config_structs.h"
#include "flight/mixer_profile.h"

// Only the immediately preceding, unnamed layout is compatible. Profile PGs
// contain arrays: copying the old record contiguously would shift later slots.
static inline bool pgMigrateProfileNames(const pgRegistry_t *reg, void *to,
    const void *from, int size, int version)
{
    size_t prefix, stride, alignment;
    int currentVersion;
    switch (pgN(reg)) {
    case PG_CONTROL_PROFILES:
        prefix = offsetof(controlConfig_t, name);
        stride = sizeof(controlConfig_t);
        alignment = __alignof__(controlConfig_t);
        currentVersion = 1;
        break;
    case PG_BATTERY_PROFILES:
        prefix = offsetof(batteryProfile_t, name);
        stride = sizeof(batteryProfile_t);
        alignment = __alignof__(batteryProfile_t);
        currentVersion = 5;
        break;
    case PG_MIXER_PROFILE:
        prefix = offsetof(mixerProfile_t, name);
        stride = sizeof(mixerProfile_t);
        alignment = __alignof__(mixerProfile_t);
#ifdef USE_AUTO_TRANSITION
        currentVersion = 5;
#else
        currentVersion = 2;
#endif
        break;
    default:
        return false;
    }
    // Reconstruct the old sizeof, including its trailing padding. Do not copy
    // that padding into the newly appended name, which must remain empty.
    const size_t oldStride = (prefix + alignment - 1) / alignment * alignment;
    const size_t count = pgSize(reg) / stride;
    if (pgVersion(reg) != currentVersion || version != currentVersion - 1 ||
        !pgIsSystem(reg) || !count || pgSize(reg) % stride ||
        size < 0 || (size_t)size != count * oldStride) {
        return false;
    }
    for (size_t i = 0; i < count; i++) {
        uint8_t *dest = (uint8_t *)to + i * stride;
        memcpy(dest, (const uint8_t *)from + i * oldStride, prefix);
        memset(dest + prefix, 0, MAX_PROFILE_NAME_LENGTH + 1);
    }
    return true;
}
