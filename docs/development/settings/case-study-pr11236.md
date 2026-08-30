# Case Study: PR #11236 - Blackbox invertedCardDetection Removal

## Summary

PR #11236 proposed removing the unused `invertedCardDetection` field from `blackboxConfig_t`. This case study demonstrates why removing a field from a parameter group struct **requires incrementing the version number** to prevent settings corruption.

**PR**: https://github.com/iNavFlight/inav/pull/11236
**Status**: Closed, not merged
**Key Learning**: Field removal changes struct layout → must increment version

The PR itself never landed, but the review discussion it produced is still a clear, real-world illustration of the version-increment rule below — the struct and the field it discusses are both still present in the codebase today, unchanged.

## Background

### The invertedCardDetection Field

The `invertedCardDetection` setting controlled SD card detection polarity for blackbox logging:

```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;
    uint16_t rate_denom;
    uint8_t device;
    uint8_t invertedCardDetection;  // This field
    uint32_t includeFlags;
    int8_t arm_control;
} blackboxConfig_t;
```

### Why It Was Proposed for Removal

The PR proposed refactoring the setting away from a runtime option toward a compile-time define:

**Old approach** (runtime setting):
```c
// User could set via CLI: set sdcard_detect_inverted = 1
if (blackboxConfig()->invertedCardDetection) {
    return !sdcardIsInserted();
}
return sdcardIsInserted();
```

**Proposed approach** (compile-time define):
```c
// Target-specific: #define SDCARD_DETECT_INVERTED in target.h
#if defined(SDCARD_DETECT_INVERTED)
    return !result;
#else
    return result;
#endif
```

The rationale: SD card detection polarity is **hardware-specific**, not user-configurable, so it arguably belongs per-target rather than as a user setting.

## The Problem: Removing a Field Without Version Increment

### Current State (Unmodified)

```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;               // offset 0-1   (2 bytes)
    uint16_t rate_denom;             // offset 2-3   (2 bytes)
    uint8_t device;                  // offset 4     (1 byte)
    uint8_t invertedCardDetection;   // offset 5     (1 byte)
    uint32_t includeFlags;           // offset 6-9   (4 bytes)
    int8_t arm_control;              // offset 10    (1 byte)
} blackboxConfig_t;                  // Total: 11 bytes

PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 4);
//                                                                                      ^
//                                                                                  Version 4
```

**Memory layout in EEPROM** for a user with custom settings:
```
Offset  Field                       Example Value
------  -------------------------   -------------
0-1     rate_num                    1
2-3     rate_denom                  1
4       device                      1 (SDCARD)
5       invertedCardDetection       1 (inverted)
6-9     includeFlags                0x000003FF (all features enabled)
10      arm_control                 0 (always arm)
```

### Hypothetical Change (Without Version Increment - WRONG!)

If the field were removed but the version left at 4:

```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;               // offset 0-1   (2 bytes)
    uint16_t rate_denom;             // offset 2-3   (2 bytes)
    uint8_t device;                  // offset 4     (1 byte)
    // invertedCardDetection REMOVED
    uint32_t includeFlags;           // offset 5-8   (4 bytes) <-- MOVED!
    int8_t arm_control;              // offset 9     (1 byte)  <-- MOVED!
} blackboxConfig_t;                  // Total: 10 bytes

PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 4);
//                                                                                      ^
//                                                                                  Still version 4 - WRONG!
```

### What Would Happen: Settings Corruption

When a user with **old EEPROM data** flashes **new firmware**:

1. **Boot sequence** calls `pgLoad()` to restore settings
2. **EEPROM has**: Version 4, 11 bytes of data
3. **New firmware expects**: Version 4, 10 bytes of data
4. **pgLoad() logic**:
   ```c
   void pgLoad(const pgRegistry_t* reg, int profileIndex,
               const void *from, int size, int version)
   {
       pgReset(reg, profileIndex);  // Reset to defaults

       if (version == pgVersion(reg)) {  // 4 == 4 -> TRUE!
           const int take = MIN(size, pgSize(reg));  // MIN(11, 10) = 10
           memcpy(pgOffset(reg, profileIndex), from, take);  // Copy 10 bytes
       }
   }
   ```

5. **Memory copy occurs** (because versions match):
   ```
   Old EEPROM (11 bytes):  [rate_num][rate_denom][device][invertedCardDetection=1][includeFlags=0x000003FF][arm_control=0]
   Bytes being copied:      0──1     2─────3      4      5                         6────────────9           [10 not copied]

   New struct (10 bytes):  [rate_num][rate_denom][device][includeFlags ][arm_control]
   After memcpy:            0──1     2─────3      4       5────────8     9
   ```

6. **Result: Field values corrupted!**
   ```
   includeFlags should be: 0x000003FF
   includeFlags actually:  0x0003FF01  (read bytes 5-8: [0x01][0xFF][0x03][0x00])
                                         ^-- byte 5 is the old invertedCardDetection value (1),
                                             now read as the low byte of includeFlags!

   arm_control should be:  0
   arm_control actually:   0x00  (read byte 9: [0x00])
                                  ^-- This is the high byte of old includeFlags — happens to
                                      also be 0, which is why this particular field looks
                                      unaffected. A different includeFlags value could just
                                      as easily have left arm_control non-zero.
   ```

### User Impact

The user would end up with corrupted settings:
- **includeFlags** is wrong → some blackbox features unexpectedly disabled
- **arm_control** happened to work (by luck) but could have been wrong too

**Critical point**: The corruption is **silent**. No error message, no indication that settings are wrong. The user just sees unexpected behavior.

## The Correct Fix: Increment Version

```c
typedef struct blackboxConfig_s {
    uint16_t rate_num;
    uint16_t rate_denom;
    uint8_t device;
    // invertedCardDetection removed
    uint32_t includeFlags;
    int8_t arm_control;
} blackboxConfig_t;

PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 5);
//                                                                                      ^
//                                                                                  Version 5 - CORRECT!
```

### Why This Works

When a user with **old EEPROM data** (version 4) flashes **new firmware** (version 5):

1. **pgLoad() is called**:
   ```c
   void pgLoad(const pgRegistry_t* reg, int profileIndex,
               const void *from, int size, int version)
   {
       pgReset(reg, profileIndex);  // Reset to defaults

       if (version == pgVersion(reg)) {  // 4 == 5 -> FALSE!
           // memcpy NOT executed
       }
   }
   ```

2. **Version mismatch detected**: 4 ≠ 5
3. **memcpy skipped**: Old data NOT copied
4. **Result**: Settings remain at defaults (from `PG_RESET_TEMPLATE`)

### User Impact (Correct)

The user sees:
- All blackbox settings reset to defaults
- No corruption
- Obvious behavior: settings changed (user can reconfigure)

**Much better than silent corruption!**

## Developer Review Comment

DzikuVx's review comment on PR #11236:

> "In this file, `PG_REGISTER_WITH_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 4);` needs to use bigger PG version or settings will be broken after flashing"

This comment highlights why code review is critical for PG changes. The version increment requirement isn't obvious without understanding the PG system internals. The PR was ultimately closed without merging, but the review comment's point stands on its own as the general rule below.

## Lesson: When to Increment Version

This case study demonstrates the rule:

**Removing a field → struct layout changes → MUST increment version**

More generally:
- ✅ **Increment version** when struct layout changes (add/remove/reorder fields, change types)
- ❌ **Don't increment** when only comments, defaults, or settings.yaml changes

See [versioning-rules.md](versioning-rules.md) for complete rules.

## Testing the Fix

To verify a version increment works correctly:

### Test Procedure

1. **Flash old firmware** (version 4 with invertedCardDetection)
2. **Configure blackbox**:
   ```
   set blackbox_device = SDCARD
   set sdcard_detect_inverted = 1
   set blackbox_include_flags = 1023
   set blackbox_arm_control = 0
   save
   ```
3. **Power cycle** and verify settings persist
4. **Flash new firmware** (version 5 without invertedCardDetection)
5. **Boot and check**:
   ```
   get blackbox_device
   get blackbox_include_flags
   get blackbox_arm_control
   ```

**Expected result**:
- All settings should be at **defaults** (from `PG_RESET_TEMPLATE`)
- No weird values
- No corruption
- CLI shows no errors

### What You're Verifying

- pgLoad() detected version mismatch (4 vs 5)
- pgLoad() skipped memcpy, kept defaults
- No data corruption occurred
- User can reconfigure settings if needed

## Additional Changes Required

When removing a field, you must also:

1. **Remove from struct** (shown above)
2. **Increment version** (shown above)
3. **Remove from settings.yaml**:
   ```yaml
   # DELETE THIS ENTRY
   - name: sdcard_detect_inverted
     field: blackboxConfig.invertedCardDetection
     # ...
   ```
4. **Remove from documentation** (`docs/Settings.md`)
5. **Remove from reset template**:
   ```c
   PG_RESET_TEMPLATE(blackboxConfig_t, blackboxConfig,
       .device = DEFAULT_BLACKBOX_DEVICE,
       .rate_num = SETTING_BLACKBOX_RATE_NUM_DEFAULT,
       .rate_denom = SETTING_BLACKBOX_RATE_DENOM_DEFAULT,
       // .invertedCardDetection = BLACKBOX_INVERTED_CARD_DETECTION,  <-- REMOVE
       .arm_control = SETTING_BLACKBOX_ARM_CONTROL_DEFAULT,
       .includeFlags = SETTING_BLACKBOX_INCLUDE_FLAGS_DEFAULT
   );
   ```

## Relationship to Compile-Time Defines

This kind of refactoring replaces a **runtime setting** with a **compile-time define**. This is a good pattern when:

- The setting is **hardware-specific** (different per target)
- Users **shouldn't change it** (could damage hardware or cause malfunction)
- It reduces **EEPROM usage** and **RAM usage**

**Target-specific configuration** (e.g., in `target.h`):
```c
#ifdef MY_TARGET_NAME
  // This target has inverted SD card detection
  #define SDCARD_DETECT_INVERTED
#endif
```

The PG system remains for **user-configurable** settings only.

## Key Takeaways

1. **Removing fields is NOT free** - requires version increment
2. **Version mismatch = automatic defaults** - this is the safety mechanism
3. **Silent corruption is worse than reset** - users prefer obvious resets over subtle bugs
4. **Code review catches this** - maintainers know to check version increments
5. **Test with real migration** - flash old→new firmware to verify behavior

## See Also

- [versioning-rules.md](versioning-rules.md) - Complete version increment rules
- [README.md](README.md) - PG system overview
- [registration-guide.md](registration-guide.md) - How to register parameter groups
