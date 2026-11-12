# Parameter Group Registration Guide

## Overview

This guide explains how to register a new parameter group in INAV using the PG system macros. The registration process is compile-time based, using linker sections to build the parameter group registry.

## Quick Start: 5-Step Process

1. **Define the struct** in your module's header file
2. **Declare the PG** using `PG_DECLARE` macro
3. **Register the PG** using `PG_REGISTER_*` macro in your .c file
4. **Provide defaults** using `PG_RESET_TEMPLATE`
5. **Add settings** to `settings.yaml` (if exposing to CLI)

## Detailed Steps

### Step 1: Define Your Configuration Struct

In your module's header file (e.g., `myfeature.h`):

```c
#include "config/parameter_group.h"

typedef struct myFeatureConfig_s {
    uint8_t enabled;
    uint16_t rate;
    int8_t offset;
    uint32_t flags;
} myFeatureConfig_t;
```

**Best practices**:
- Use fixed-size types (`uint8_t`, `int16_t`, etc.) not `int` or `unsigned`
- Consider field ordering for natural alignment (larger types first)
- Document each field with comments
- Keep struct focused and cohesive

### Step 2: Declare the Parameter Group

Still in your header file, declare the PG using the appropriate macro:

#### For System-Wide Configuration (Single Instance)

```c
PG_DECLARE(myFeatureConfig_t, myFeatureConfig);
```

This expands to:
```c
extern myFeatureConfig_t myFeatureConfig_System;
extern myFeatureConfig_t myFeatureConfig_Copy;
static inline const myFeatureConfig_t* myFeatureConfig(void) {
    return &myFeatureConfig_System;
}
static inline myFeatureConfig_t* myFeatureConfigMutable(void) {
    return &myFeatureConfig_System;
}
```

**Usage in other code**:
```c
// Read-only access
if (myFeatureConfig()->enabled) { ... }

// Mutable access (only when needed)
myFeatureConfigMutable()->rate = 100;
```

#### For Profile-Specific Configuration (Multiple Instances)

```c
PG_DECLARE_PROFILE(myFeatureConfig_t, myFeatureConfig);
```

This creates storage for `MAX_PROFILE_COUNT` instances, with a pointer to the current profile.

**Usage in other code**:
```c
// Automatically uses current profile
if (myFeatureConfig()->enabled) { ... }
```

### Step 3: Register the Parameter Group

In your module's .c file:

#### System Configuration with Defaults Template

```c
// Assign a unique PGN (parameter group number)
// Check src/main/config/parameter_group_ids.h for available IDs
#define PG_MY_FEATURE_CONFIG 100  // Example ID

PG_REGISTER_WITH_RESET_TEMPLATE(
    myFeatureConfig_t,      // Type
    myFeatureConfig,        // Name (matches declaration)
    PG_MY_FEATURE_CONFIG,   // PGN (unique ID)
    0                       // Version (start at 0)
);
```

This macro:
1. Creates `myFeatureConfig_System` storage
2. Creates `myFeatureConfig_Copy` storage
3. Registers the PG in `.pg_registry` linker section
4. Links to `pgResetTemplate_myFeatureConfig` for defaults

#### Profile Configuration with Defaults Template

```c
PG_REGISTER_PROFILE_WITH_RESET_TEMPLATE(
    myFeatureConfig_t,
    myFeatureConfig,
    PG_MY_FEATURE_CONFIG,
    0
);
```

#### Alternative: Custom Reset Function

For complex initialization that can't be expressed as a struct literal:

```c
PG_REGISTER_WITH_RESET_FN(
    myFeatureConfig_t,
    myFeatureConfig,
    PG_MY_FEATURE_CONFIG,
    0
);

// Implement the reset function
void pgResetFn_myFeatureConfig(myFeatureConfig_t *config)
{
    memset(config, 0, sizeof(*config));
    config->enabled = 1;
    config->rate = calculateDefaultRate();  // Complex logic
    config->offset = -10;
    config->flags = DEFAULT_FLAGS;
}
```

### Step 4: Provide Default Values

#### Using a Template (Recommended)

```c
PG_RESET_TEMPLATE(myFeatureConfig_t, myFeatureConfig,
    .enabled = 1,
    .rate = 100,
    .offset = -10,
    .flags = 0x12345678
);
```

This creates `pgResetTemplate_myFeatureConfig` in the `.pg_resetdata` linker section.

**Best practices**:
- Initialize ALL fields explicitly (don't rely on zero-initialization)
- Use `#define` constants instead of magic numbers
- Document why each default was chosen

Example with constants:
```c
#define DEFAULT_MY_FEATURE_RATE 100
#define DEFAULT_MY_FEATURE_OFFSET -10

PG_RESET_TEMPLATE(myFeatureConfig_t, myFeatureConfig,
    .enabled = 1,
    .rate = DEFAULT_MY_FEATURE_RATE,
    .offset = DEFAULT_MY_FEATURE_OFFSET,
    .flags = 0
);
```

#### Using a Reset Function

Already shown in Step 3. Use this when:
- Defaults depend on runtime detection (e.g., hardware features)
- Initialization requires complex logic or calculations
- You need to call other initialization functions

### Step 5: Add Settings to settings.yaml

Map your PG fields to CLI settings in `src/main/fc/settings.yaml`:

```yaml
- name: my_feature_enable
  description: "Enable myFeature functionality"
  field: myFeatureConfig.enabled
  type: uint8_t
  min: 0
  max: 1
  default_value: 1

- name: my_feature_rate
  description: "MyFeature update rate in Hz"
  field: myFeatureConfig.rate
  type: uint16_t
  min: 10
  max: 1000
  default_value: 100

- name: my_feature_offset
  description: "MyFeature offset correction"
  field: myFeatureConfig.offset
  type: int8_t
  min: -50
  max: 50
  default_value: -10
```

**Important**: The `field` entry uses the PG name (not the struct name).

## Registration Macro Reference

### System Configuration Macros

| Macro | Use Case |
|-------|----------|
| `PG_DECLARE(type, name)` | Declare system PG in header |
| `PG_REGISTER(type, name, pgn, ver)` | Register with zero defaults |
| `PG_REGISTER_WITH_RESET_TEMPLATE(type, name, pgn, ver)` | Register with template defaults (recommended) |
| `PG_REGISTER_WITH_RESET_FN(type, name, pgn, ver)` | Register with custom reset function |
| `PG_RESET_TEMPLATE(type, name, ...)` | Define default values template |

### Profile Configuration Macros

| Macro | Use Case |
|-------|----------|
| `PG_DECLARE_PROFILE(type, name)` | Declare profile PG in header |
| `PG_REGISTER_PROFILE(type, name, pgn, ver)` | Register with zero defaults |
| `PG_REGISTER_PROFILE_WITH_RESET_TEMPLATE(type, name, pgn, ver)` | Register with template defaults (recommended) |
| `PG_REGISTER_PROFILE_WITH_RESET_FN(type, name, pgn, ver)` | Register with custom reset function |

### Array Configuration Macros

For arrays of configuration (less common):

```c
PG_DECLARE_ARRAY(myType_t, ARRAY_SIZE, myArray);
PG_REGISTER_ARRAY_WITH_RESET_TEMPLATE(myType_t, ARRAY_SIZE, myArray, PGN, VERSION);
```

## Complete Example

Here's a complete example showing all files and steps:

### myfeature.h
```c
#pragma once

#include <stdint.h>
#include "config/parameter_group.h"

// Configuration struct
typedef struct myFeatureConfig_s {
    uint8_t enabled;          // 0=disabled, 1=enabled
    uint16_t updateRate;      // Update rate in Hz
    int8_t calibrationOffset; // Calibration offset in units
    uint32_t featureFlags;    // Bitfield of feature flags
} myFeatureConfig_t;

// Declare the parameter group
PG_DECLARE(myFeatureConfig_t, myFeatureConfig);

// Feature flag definitions
#define MY_FEATURE_FLAG_AUTO_TUNE    (1 << 0)
#define MY_FEATURE_FLAG_VERBOSE      (1 << 1)
#define MY_FEATURE_FLAG_STRICT_MODE  (1 << 2)

// Public API
void myFeatureInit(void);
void myFeatureUpdate(uint32_t currentTimeUs);
```

### myfeature.c
```c
#include "myfeature.h"
#include "config/parameter_group_ids.h"

// Default values as defines
#define DEFAULT_UPDATE_RATE 100
#define DEFAULT_CALIBRATION_OFFSET 0
#define DEFAULT_FEATURE_FLAGS (MY_FEATURE_FLAG_AUTO_TUNE | MY_FEATURE_FLAG_VERBOSE)

// Register the parameter group
PG_REGISTER_WITH_RESET_TEMPLATE(
    myFeatureConfig_t,
    myFeatureConfig,
    PG_MY_FEATURE_CONFIG,  // Defined in parameter_group_ids.h
    0                      // Version 0 (first version)
);

// Provide default values
PG_RESET_TEMPLATE(myFeatureConfig_t, myFeatureConfig,
    .enabled = 1,
    .updateRate = DEFAULT_UPDATE_RATE,
    .calibrationOffset = DEFAULT_CALIBRATION_OFFSET,
    .featureFlags = DEFAULT_FEATURE_FLAGS
);

void myFeatureInit(void)
{
    if (!myFeatureConfig()->enabled) {
        return;
    }
    // Initialize feature using config values
    // ...
}

void myFeatureUpdate(uint32_t currentTimeUs)
{
    // Use config values
    if (myFeatureConfig()->featureFlags & MY_FEATURE_FLAG_VERBOSE) {
        // Verbose logging
    }
    // ...
}
```

### src/main/config/parameter_group_ids.h
```c
// Add to the enum:
typedef enum {
    // ... existing IDs ...
    PG_MY_FEATURE_CONFIG = 100,  // Choose unused ID
    // ...
} pgn_e;
```

### src/main/fc/settings.yaml
```yaml
# Add to the appropriate section:
- name: my_feature_enable
  description: "Enable myFeature functionality"
  field: myFeatureConfig.enabled
  type: uint8_t
  min: 0
  max: 1
  default_value: 1

- name: my_feature_rate
  description: "MyFeature update rate (Hz)"
  field: myFeatureConfig.updateRate
  type: uint16_t
  min: 1
  max: 1000
  default_value: 100

- name: my_feature_offset
  description: "MyFeature calibration offset"
  field: myFeatureConfig.calibrationOffset
  type: int8_t
  min: -127
  max: 127
  default_value: 0

- name: my_feature_flags
  description: "MyFeature flags bitfield"
  field: myFeatureConfig.featureFlags
  type: uint32_t
  min: 0
  max: 4294967295
  default_value: 3
```

## Common Patterns

### Boolean Settings

```c
typedef struct myConfig_s {
    uint8_t featureEnabled;      // Use uint8_t, not bool
    uint8_t autoCalibrate;       // 0=off, 1=on
} myConfig_t;

PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .featureEnabled = 1,
    .autoCalibrate = 0
);
```

**Why uint8_t not bool**: EEPROM storage needs fixed sizes. `bool` size is implementation-defined.

### Enum Settings

```c
typedef enum {
    MODE_DISABLED = 0,
    MODE_BASIC = 1,
    MODE_ADVANCED = 2
} myFeatureMode_e;

typedef struct myConfig_s {
    uint8_t mode;  // Store enum as uint8_t
} myConfig_t;

PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .mode = MODE_BASIC
);
```

### Bitfield Flags

```c
#define FLAG_BIT_1  (1 << 0)
#define FLAG_BIT_2  (1 << 1)
#define FLAG_BIT_3  (1 << 2)

typedef struct myConfig_s {
    uint32_t flags;
} myConfig_t;

PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .flags = FLAG_BIT_1 | FLAG_BIT_2  // Default flags enabled
);
```

### Arrays

```c
#define MAX_CHANNELS 8

typedef struct myConfig_s {
    int16_t channelOffsets[MAX_CHANNELS];
    uint8_t channelEnabled[MAX_CHANNELS];
} myConfig_t;

PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .channelOffsets = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .channelEnabled = { 1, 1, 1, 1, 0, 0, 0, 0 }
);
```

## Accessing Configuration at Runtime

### Read-Only Access (Preferred)

```c
// System config
if (myFeatureConfig()->enabled) {
    processAtRate(myFeatureConfig()->updateRate);
}

// Profile config
pidGain = pidConfig()->P[ROLL];  // Uses current profile
```

### Mutable Access (Use Sparingly)

```c
// Only when you need to modify (e.g., auto-calibration)
myFeatureConfigMutable()->calibrationOffset = calculatedOffset;
```

**Important**: Mutations should be rare. Most code should read config, not modify it.

### Resetting to Defaults

```c
// Reset specific group to defaults
PG_RESET_CURRENT(myFeatureConfig);

// Reset all parameter groups (full EEPROM reset)
pgResetAll(MAX_PROFILE_COUNT);
```

## Troubleshooting

### Linker Error: Undefined Reference to Registry

**Error**: `undefined reference to 'myFeatureConfig_Registry'`

**Cause**: You declared with `PG_DECLARE` but forgot to register with `PG_REGISTER_*`.

**Fix**: Add `PG_REGISTER_WITH_RESET_TEMPLATE(...)` in your .c file.

### Linker Error: Undefined Reference to Reset Template

**Error**: `undefined reference to 'pgResetTemplate_myFeatureConfig'`

**Cause**: You used `PG_REGISTER_WITH_RESET_TEMPLATE` but forgot `PG_RESET_TEMPLATE`.

**Fix**: Add `PG_RESET_TEMPLATE(myFeatureConfig_t, myFeatureConfig, ...)`.

### Settings Not Persisting

**Cause**: No settings.yaml entry, or wrong field name in settings.yaml.

**Fix**: Add entry to settings.yaml with correct `field: myFeatureConfig.fieldName`.

### PGN Already in Use

**Error**: Multiple definitions of the same PGN.

**Fix**: Choose a unique PGN from `parameter_group_ids.h`. Add yours to the enum.

## Version Management

When you modify a registered parameter group struct, **you must increment the version**. See [versioning-rules.md](versioning-rules.md) for details.

Example of version increment:
```c
// Version 0: Initial
typedef struct myConfig_s {
    uint8_t field1;
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 0);

// Version 1: Added field2
typedef struct myConfig_s {
    uint8_t field1;
    uint16_t field2;  // NEW FIELD
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 1);  // Incremented!
```

## See Also

- [README.md](README.md) - System overview
- [versioning-rules.md](versioning-rules.md) - When to increment versions
- [case-study-pr11236.md](case-study-pr11236.md) - Real-world example
