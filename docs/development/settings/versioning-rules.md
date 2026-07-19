# Parameter Group Version Increment Rules

## Quick Decision Chart

```
┌──────────────────────────────────────────────┐
│  Are you modifying a parameter group struct? │
└────────────────┬─────────────────────────────┘
                 │
                 ├─ NO  ──> No version increment needed
                 │
                 └─ YES ──> Continue below
                            │
        ┌───────────────────┴────────────────────┐
        │                                        │
        v                                        v
    ┌───────┐                              ┌──────────┐
    │ SAFE  │                              │ REQUIRES │
    │ (no   │                              │ VERSION  │
    │ incr) │                              │ INCREMENT│
    └───┬───┘                              └─────┬────┘
        │                                        │
        │                                        │
        ├─ Only comments changed                 ├─ Adding a field
        ├─ Only whitespace changed               ├─ Removing a field
        ├─ Field renamed (same type/size)        ├─ Changing field type
        ├─ settings.yaml entry updated           ├─ Changing field size
                (no struct change)                ├─ Reordering fields
                                                  ├─ Adding padding/packing
                                                  └─ Changing array size
```

## The Core Rule

**If the struct's memory layout changes in any way, increment the version.**

## Why Versions Matter

When INAV boots, it loads settings from EEPROM:

```c
void pgLoad(const pgRegistry_t* reg, int profileIndex,
            const void *from, int size, int version)
{
    pgReset(reg, profileIndex);  // Always reset to defaults first

    if (version == pgVersion(reg)) {
        // ONLY copy if versions match
        const int take = MIN(size, pgSize(reg));
        memcpy(pgOffset(reg, profileIndex), from, take);
    }
    // If version mismatch: defaults remain, no corruption
}
```

**Key insight**: Version mismatch triggers automatic fallback to defaults. This is the protection mechanism.

## Changes That REQUIRE Version Increment

### 1. Adding a Field

**Before** (version 3, 6 bytes total):
```c
typedef struct myConfig_s {
    uint16_t field1;  // offset 0-1
    uint32_t field2;  // offset 2-5
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);
```

**After** (version 4, 10 bytes total):
```c
typedef struct myConfig_s {
    uint16_t field1;  // offset 0-1
    uint32_t field2;  // offset 2-5
    uint32_t field3;  // offset 6-9  <-- NEW FIELD
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 4);  // Increment!
```

**Why**: Size changed. If you don't increment:
- Old EEPROM: 6 bytes
- New firmware: 10 bytes
- pgLoad() copies 6 bytes, leaving field3 uninitialized (undefined behavior)

### 2. Removing a Field

**Before** (version 4, 10 bytes total):
```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;               // offset 0-1
    uint16_t rate_denom;             // offset 2-3
    uint8_t device;                  // offset 4
    uint8_t invertedCardDetection;   // offset 5  <-- REMOVING
    uint32_t includeFlags;           // offset 6-9
    int8_t arm_control;              // offset 10
} blackboxConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 4);
```

**After** (version 5, 9 bytes total):
```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;               // offset 0-1
    uint16_t rate_denom;             // offset 2-3
    uint8_t device;                  // offset 4
    // invertedCardDetection removed
    uint32_t includeFlags;           // offset 5-8  <-- MOVED!
    int8_t arm_control;              // offset 9    <-- MOVED!
} blackboxConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 5);  // MUST increment!
```

**Why**: Field offsets changed. If you don't increment:
- Old EEPROM bytes 5-9: `[invertedCardDetection=1][includeFlags=0x12340000][arm_control=-1]`
- New firmware reads: `includeFlags` from offset 5-8 gets `[1][0x12][0x34][0x00]` = **wrong value!**
- Result: Settings corruption! (See [case-study-pr11236.md](case-study-pr11236.md))

### 3. Changing Field Type

**Before** (version 2):
```c
typedef struct myConfig_s {
    uint8_t timeout;  // offset 0, 1 byte
    uint16_t rate;    // offset 1-2, 2 bytes
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 2);
```

**After** (version 3):
```c
typedef struct myConfig_s {
    uint16_t timeout;  // offset 0-1, 2 bytes  <-- Changed uint8_t -> uint16_t
    uint16_t rate;     // offset 2-3, 2 bytes  <-- MOVED!
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);  // Increment!
```

**Why**: Size AND offsets changed. Old `rate` value would be misinterpreted.

### 4. Reordering Fields

**Before** (version 1):
```c
typedef struct myConfig_s {
    uint8_t fieldA;   // offset 0
    uint8_t fieldB;   // offset 1
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 1);
```

**After** (version 2):
```c
typedef struct myConfig_s {
    uint8_t fieldB;   // offset 0  <-- SWAPPED
    uint8_t fieldA;   // offset 1  <-- SWAPPED
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 2);  // Increment!
```

**Why**: Field offsets changed. Values would be swapped in memory.

### 5. Changing Array Size

**Before** (version 2):
```c
typedef struct myConfig_s {
    uint8_t values[4];  // 4 bytes
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 2);
```

**After** (version 3):
```c
typedef struct myConfig_s {
    uint8_t values[8];  // 8 bytes  <-- CHANGED
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);  // Increment!
```

**Why**: Size changed. Behavior depends on increase vs decrease:
- Increase: Extra elements uninitialized
- Decrease: Data truncated

### 6. Adding Packing or Alignment Attributes

**Before** (version 1, compiler adds padding):
```c
typedef struct myConfig_s {
    uint8_t field1;   // offset 0
    // [compiler padding: 1 byte]
    uint16_t field2;  // offset 2-3
} myConfig_t;  // Total: 4 bytes
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 1);
```

**After** (version 2, packed):
```c
typedef struct myConfig_s {
    uint8_t field1;   // offset 0
    uint16_t field2;  // offset 1-2  <-- MOVED!
} __attribute__((packed)) myConfig_t;  // Total: 3 bytes
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 2);  // Increment!
```

**Why**: Size changed, offsets changed.

## Changes That DO NOT Require Version Increment

### 1. Renaming a Field (Same Type)

```c
// Before (version 3)
typedef struct myConfig_s {
    uint8_t oldName;  // offset 0
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);

// After (version 3 - no increment needed)
typedef struct myConfig_s {
    uint8_t newName;  // offset 0, same type/size
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);  // Same version OK
```

**Why**: Memory layout unchanged. Binary compatibility maintained.

**Important**: You MUST update settings.yaml to reflect the field rename!

### 2. Updating Comments

```c
// Before
typedef struct myConfig_s {
    uint8_t field;  // Old comment
} myConfig_t;

// After - no version increment
typedef struct myConfig_s {
    uint8_t field;  // Updated comment explaining usage
} myConfig_t;
```

**Why**: Comments don't affect memory layout.

### 3. Updating Default Values Only

```c
// Before
PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .field1 = 10,
    .field2 = 20
);

// After - no version increment
PG_RESET_TEMPLATE(myConfig_t, myConfig,
    .field1 = 15,  // Changed default
    .field2 = 25   // Changed default
);
```

**Why**: Defaults only affect new installs or version mismatches. Existing users keep their settings.

**Note**: This is safe because default changes don't affect users with existing valid settings.

### 4. Updating settings.yaml Entry (No Struct Change)

```yaml
# Before
- name: my_setting
  field: myConfig.field1
  min: 0
  max: 100

# After - no version increment
- name: my_setting
  field: myConfig.field1
  min: 0
  max: 200  # Expanded range
```

**Why**: settings.yaml doesn't affect EEPROM binary layout.

## Special Cases

### Changing Field Range Without Type Change

If the field type remains the same but you need a larger range:

```c
// Before (version 2): uint8_t (0-255) was sufficient
typedef struct myConfig_s {
    uint8_t timeout;
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 2);

// After: Need values > 255, must change type
typedef struct myConfig_s {
    uint16_t timeout;  // Type changed -> INCREMENT VERSION
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);
```

This requires a version increment because the type changed (see rule #3).

### Replacing Field with Union (Same Size)

```c
// Before (version 2)
typedef struct myConfig_s {
    uint32_t value;  // 4 bytes at offset 0-3
} myConfig_t;

// After (version 3)
typedef struct myConfig_s {
    union {
        uint32_t value;    // 4 bytes
        uint8_t bytes[4];  // 4 bytes
    } data;  // Still 4 bytes at offset 0-3
} myConfig_t;
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 3);  // Increment!
```

**Why**: Field name changed (`value` → `data.value`). settings.yaml needs updating, and to be safe, increment version.

**Recommendation**: Always increment when introducing unions to avoid subtle bugs.

## Checklist for Struct Modifications

Before committing changes to a parameter group struct:

- [ ] Did I add a field? → **Increment version**
- [ ] Did I remove a field? → **Increment version**
- [ ] Did I change a field type? → **Increment version**
- [ ] Did I reorder fields? → **Increment version**
- [ ] Did I change an array size? → **Increment version**
- [ ] Did I add/remove packing attributes? → **Increment version**
- [ ] Did I only rename a field (same type)? → Update settings.yaml, version optional
- [ ] Did I only update comments? → No increment needed
- [ ] Did I only change default values? → No increment needed

**When in doubt, increment the version.** It's better to force users to reconfigure than to corrupt their settings.

## Version Number Limits

Versions are stored in 4 bits (top bits of the `pgn` field):
- **Valid range**: 0-15
- **Maximum version**: 15

If you reach version 15:
1. Consider if the parameter group can be split into smaller groups
2. Consult with maintainers about restructuring
3. Version overflow is a signal that the design may need refactoring

## Testing Version Increments

When you increment a version, test the migration:

1. **Flash old firmware** with old version
2. **Configure settings** to non-default values
3. **Save to EEPROM** (`save` command)
4. **Flash new firmware** with incremented version
5. **Boot and verify**: Settings should revert to defaults (not corrupt)
6. **Check no errors** in CLI or logs

## See Also

- [README.md](README.md) - System overview
- [case-study-pr11236.md](case-study-pr11236.md) - Real-world example of why versions matter
- [registration-guide.md](registration-guide.md) - How to register parameter groups
