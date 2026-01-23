# PG Struct Size Extraction Using nm (No Extra Dependencies)

## Overview

Extract PG struct sizes directly from compiled binaries using `nm` from standard binutils.

## Advantages Over pahole Approach

✅ **No extra dependencies** - `nm` is part of binutils (already required for builds)
✅ **Works without debug info** - Reads symbol table, not DWARF
✅ **Faster** - Direct symbol table lookup
✅ **Simpler** - Standard tool, no special packages needed

## Dependencies

- `nm` (part of binutils, already required)
- For ARM targets: `arm-none-eabi-nm` (already required)

## How It Works

1. PG system stores reset templates in `.data` section
2. Each template symbol is named: `pgResetTemplate_<configName>`
3. Symbol table includes size of each reset template
4. Reset template size = `sizeof(struct)`

## Usage

### Extract All PG Struct Sizes

```bash
cmake/extract-pg-sizes-nm.sh build_sitl/bin/SITL.elf
```

**Sample Output:**
```
adcChannelConfig_t             4
armingConfig_t                 6
barometerConfig_t              8
batteryMetersConfig_t          24
beeperConfig_t                 12
blackboxConfig_t               16
compassConfig_t                24
displayConfig_t                1
djiOsdConfig_t                 6
...
```

### Compare Across Platforms

```bash
cmake/compare-pg-sizes-nm.sh build/bin/SPEEDYBEEF405V3.elf build/bin/MATEKF722.elf build/bin/MATEKH743.elf
```

**Sample Output:**
```
Comparing PG struct sizes across platforms...

Struct                         SPEEDYBEEF405V3       MATEKF722       MATEKH743
============================== =============== =============== ===============
adcChannelConfig_t                          4B              4B              4B
armingConfig_t                              6B              6B              6B
barometerConfig_t                           8B              8B              8B
batteryMetersConfig_t                      24B             24B             24B
blackboxConfig_t                           16B             16B             16B
...
```

## Verified Results (2026-01-22)

Compared PG struct sizes across three platforms:
- **SPEEDYBEEF405V3** (STM32F405 - Cortex-M4)
- **MATEKF722** (STM32F722 - Cortex-M7)
- **MATEKH743** (STM32H743 - Cortex-M7)

**Finding:** ✅ **All PG struct sizes are identical across F4, F7, and H7 platforms**

No platform-specific size differences detected across 47 PG structs.

## Implementation Details

### Symbol Format

```
$ nm --print-size build_sitl/bin/SITL.elf | grep pgResetTemplate_blackboxConfig
00000000000df71c 0000000000000010 D pgResetTemplate_blackboxConfig
      ↑                  ↑          ↑ ↑
   address          size (hex)   type name
```

- **Address**: Location in memory (not relevant for size extraction)
- **Size**: Template size in hexadecimal = `sizeof(struct)`
- **Type**: D = data section
- **Name**: `pgResetTemplate_<configName>`

### Conversion Process

1. Extract symbol name: `pgResetTemplate_blackboxConfig` → `blackboxConfig`
2. Convert hex size to decimal: `0x10` → `16`
3. Find struct type from `PG_REGISTER`: `blackboxConfig` → `blackboxConfig_t`
4. Output: `blackboxConfig_t 16`

## Build-Time Validation Workflow

### 1. Generate Initial Database

```bash
# Build with any target
make sitl

# Extract sizes
cmake/extract-pg-sizes-nm.sh build_sitl/bin/SITL.elf > cmake/pg_struct_sizes.db

# Add PG versions from source
# (Script to be created that merges sizes with versions from PG_REGISTER)
```

### 2. Validate On Each Build

```bash
# After compilation
cmake/check-pg-struct-sizes.sh build_sitl/bin/SITL.elf

# Exits with error if:
# - Struct size changed BUT
# - PG version was NOT incremented
```

### 3. Update Database After Valid Changes

```bash
# After incrementing PG version for legitimate changes
cmake/extract-pg-sizes-nm.sh build_sitl/bin/SITL.elf > cmake/pg_struct_sizes.db
git add cmake/pg_struct_sizes.db
git commit -m "Update PG sizes after blackbox version increment"
```

## Limitations

- **Size only, not layout** - Cannot detect field reordering with same total size
- **Platform consistency assumed** - Works because INAV PG structs don't use platform-specific types

## Files Created

- `extract-pg-sizes-nm.sh` - Extract sizes from single binary
- `compare-pg-sizes-nm.sh` - Compare sizes across multiple binaries
- `check-pg-struct-sizes.sh` - Validate sizes against database (TODO)
- `pg_struct_sizes.db` - Database of struct sizes + versions (TODO)

## Build-Time Validation - IMPLEMENTED

### Created Scripts

1. **`check-pg-struct-sizes.sh`** - Validates sizes against database
   - Extracts current sizes from binary
   - Compares against `pg_struct_sizes.db`
   - Gets PG versions from source code
   - Fails build if size changed without version increment

2. **`generate-pg-database.sh`** - Generates database from binary
   - Extracts sizes using `extract-pg-sizes-nm.sh`
   - Looks up PG versions from source
   - Output format: `struct_type size version`

3. **`pg_struct_sizes.db`** - Database of 47 PG structs with sizes and versions

### Integration

Modified `cmake/main.cmake` - Added POST_BUILD validation to `setup_firmware_target()`:

```cmake
# Add PG struct size validation
add_custom_command(TARGET ${exe} POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/cmake/check-pg-struct-sizes.sh $<TARGET_FILE:${exe}>
    COMMENT "Validating PG struct sizes for ${name}"
    VERBATIM
)
```

### Testing Required

Build SITL to verify validation runs:
```bash
make sitl
```

Expected output at end of build:
```
🔍 Checking PG struct sizes against database...
  ✓ adcChannelConfig_t (4B)
  ...
✅ All PG struct sizes validated successfully
```

## Comparison with pahole

| Feature | nm Approach | pahole Approach |
|---------|-------------|-----------------|
| Extra dependencies | ❌ None | ✅ Requires dwarves package |
| Debug info required | ❌ No | ✅ Yes |
| Shows field layout | ❌ No | ✅ Yes (with offsets, padding) |
| Detects reordering | ❌ No | ✅ Yes |
| Build time | ⚡ Fast | 🐌 Slower (DWARF parsing) |
| Works on stripped binaries | ✅ Yes (symbol table sufficient) | ❌ No |

**Recommendation:** Use nm approach for build-time validation (fast, no dependencies). Use pahole for detailed struct analysis during development.
