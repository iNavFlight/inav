# Parameter Groups System

## Overview

Parameter Groups (PG) is INAV's system for managing persistent configuration settings. Each configuration struct is registered with a unique ID and version number, stored in EEPROM/flash, and automatically validated on boot.

## Quick Reference

**When modifying a parameter group struct:**
- ✅ **Increment version** - Safest option, always works
- ⚠️ **Don't increment** - Only safe for comments or non-structural changes

**Key files:**
- `src/main/config/parameter_group.h` - Macros and registry structure
- `src/main/config/parameter_group.c` - Runtime load/store functions
- `src/main/config/parameter_group_ids.h` - PGN definitions
- `src/main/fc/settings.yaml` - Maps CLI settings to PG fields

## How It Works

### Registration

Parameter groups use linker sections to build a compile-time registry:

```c
// In yourfeature.h
typedef struct yourFeatureConfig_s {
    uint8_t enabled;
    uint16_t rate;
} yourFeatureConfig_t;

PG_DECLARE(yourFeatureConfig_t, yourFeatureConfig);

// In yourfeature.c
PG_REGISTER_WITH_RESET_TEMPLATE(
    yourFeatureConfig_t,
    yourFeatureConfig,
    PG_YOUR_FEATURE_CONFIG,  // Unique ID from parameter_group_ids.h
    0                        // Version number
);

PG_RESET_TEMPLATE(yourFeatureConfig_t, yourFeatureConfig,
    .enabled = 1,
    .rate = 100
);
```

### Version Checking

On boot, `pgLoad()` validates versions before loading settings:

```c
void pgLoad(const pgRegistry_t* reg, int profileIndex,
            const void *from, int size, int version)
{
    pgReset(reg, profileIndex);  // Always reset to defaults first

    if (version == pgVersion(reg)) {
        // Only copy if versions match
        const int take = MIN(size, pgSize(reg));
        memcpy(pgOffset(reg, profileIndex), from, take);
    }
    // If version mismatch: defaults remain, no corruption
}
```

**Key behavior:** Version mismatch causes settings to remain at defaults. This prevents corruption when struct layout changes.

## Version Management

### When to Increment Version

Version numbers are stored in the top 4 bits of the PGN field (valid range: 0-15).

**The safe rule:** When modifying a parameter group struct, increment the version.

**Changes requiring version increment:**
- Removing a field
- Changing field type or size
- Reordering fields
- Adding packing attributes

**Example - Removing a field:**

```c
// Before: version 4
typedef struct blackboxConfig_s {
    uint16_t rate_num;
    uint16_t rate_denom;
    uint8_t device;
    uint8_t invertedCardDetection;  // Removing this
    uint32_t includeFlags;
    int8_t arm_control;
} blackboxConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 4);

// After: version 5 (MUST increment!)
typedef struct blackboxConfig_s {
    uint16_t rate_num;
    uint16_t rate_denom;
    uint8_t device;
    // invertedCardDetection removed - field offsets changed!
    uint32_t includeFlags;  // Now at different offset
    int8_t arm_control;     // Now at different offset
} blackboxConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 5);
```

**Why:** Without version increment, `pgLoad()` sees matching versions (4==4) and copies old data, but field offsets have changed. Result: `includeFlags` reads wrong bytes from EEPROM, causing corruption.

With version increment (4→5), `pgLoad()` detects mismatch and keeps defaults instead.

### Changes NOT Requiring Version Increment

- Comments only
- Default values in `PG_RESET_TEMPLATE` (doesn't affect EEPROM layout)
- settings.yaml entries (CLI mapping, not struct layout)

### Technical Note on Appending

The `pgLoad()` function uses `MIN(size, pgSize(reg))` when copying data. This means:
- If new firmware has larger struct, it copies available bytes and keeps defaults for new fields
- If new firmware has smaller struct, it copies only what fits

This behavior theoretically allows appending fields at the end without version increment. However, **the recommended practice is to always increment** unless you have specific reason not to and understand the implications.

## Complete Example

### yourfeature.h
```c
#pragma once

#include <stdint.h>
#include "config/parameter_group.h"

typedef struct yourFeatureConfig_s {
    uint8_t enabled;
    uint16_t updateRate;
    uint32_t flags;
} yourFeatureConfig_t;

PG_DECLARE(yourFeatureConfig_t, yourFeatureConfig);
```

### yourfeature.c
```c
#include "yourfeature.h"
#include "config/parameter_group_ids.h"

PG_REGISTER_WITH_RESET_TEMPLATE(
    yourFeatureConfig_t,
    yourFeatureConfig,
    PG_YOUR_FEATURE_CONFIG,
    0
);

PG_RESET_TEMPLATE(yourFeatureConfig_t, yourFeatureConfig,
    .enabled = 1,
    .updateRate = 100,
    .flags = 0
);

void yourFeatureInit(void)
{
    if (!yourFeatureConfig()->enabled) {
        return;
    }
    // Use configuration...
}
```

### parameter_group_ids.h
```c
typedef enum {
    // ... existing entries ...
    PG_YOUR_FEATURE_CONFIG = 150,  // Choose unused ID
    // ...
} pgn_e;
```

### settings.yaml
```yaml
- name: your_feature_enable
  description: "Enable yourFeature"
  field: yourFeatureConfig.enabled
  type: uint8_t
  min: 0
  max: 1
  default_value: 1

- name: your_feature_rate
  description: "Update rate in Hz"
  field: yourFeatureConfig.updateRate
  type: uint16_t
  min: 1
  max: 1000
  default_value: 100
```

## Accessing Configuration

### Read-only (preferred)
```c
if (yourFeatureConfig()->enabled) {
    processAtRate(yourFeatureConfig()->updateRate);
}
```

### Mutable (use sparingly)
```c
yourFeatureConfigMutable()->updateRate = newRate;
```

## Registration Macros

| Macro | Use |
|-------|-----|
| `PG_DECLARE(type, name)` | Declare system PG in header |
| `PG_DECLARE_PROFILE(type, name)` | Declare profile PG in header |
| `PG_REGISTER_WITH_RESET_TEMPLATE(...)` | Register with template defaults |
| `PG_REGISTER_WITH_RESET_FN(...)` | Register with custom reset function |
| `PG_RESET_TEMPLATE(type, name, ...)` | Define default values |

## Common Patterns

### Boolean Fields
```c
uint8_t enabled;  // Use uint8_t, not bool (EEPROM needs fixed sizes)
```

### Enums
```c
typedef enum { MODE_A = 0, MODE_B = 1 } mode_e;
uint8_t mode;  // Store enum as uint8_t
```

### Bitfields
```c
#define FLAG_A (1 << 0)
#define FLAG_B (1 << 1)
uint32_t flags;
```

## Testing Version Changes

When incrementing a version:

1. Flash old firmware, configure non-default settings, save
2. Flash new firmware with incremented version
3. Verify settings reset to defaults (not corrupted)
4. Check for errors in CLI

## See Also

- `src/main/config/parameter_group.h` - Full macro definitions
- `src/main/config/parameter_group.c` - Implementation
- Betaflight PG documentation: https://betaflight.com/docs/development/ParameterGroups
