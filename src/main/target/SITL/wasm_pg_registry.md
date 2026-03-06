# WASM Parameter Group Registry Solution

## Problem

INAV's Parameter Group (PG) system relies on GNU LD linker script features that are not supported by WebAssembly's wasm-ld linker:

1. **Custom Sections**: Native builds use `.pg_registry` and `.pg_resetdata` sections
2. **Linker Boundary Symbols**: GNU LD provides `__pg_registry_start` and `__pg_registry_end` via `PROVIDE_HIDDEN()` in linker scripts
3. **wasm-ld Limitations**: Emscripten's wasm-ld does NOT support:
   - GNU LD linker scripts (`-T script.ld`)
   - `PROVIDE_HIDDEN()` symbol definitions
   - `__start_section` / `__stop_section` pseudo-symbols

## Solution: Script-Generated Manual Registry

Instead of relying on linker magic, we auto-generate a C file that manually lists all PG registries.

### How It Works

#### 1. Script: `src/utils/generate_wasm_pg_registry.sh`

**What it does:**
- Scans all `.c` files in `src/main/` for `PG_REGISTER*` macros
- Extracts the parameter group names (2nd or 3rd macro parameter)
- Generates `src/main/target/SITL/wasm_pg_registry.c` with:
  - `extern` declarations for all `*_Registry` symbols
  - A static array of pointers to all registries
  - Definitions for `__pg_registry_start` and `__pg_registry_end`

**Pattern Recognition:**
```c
// These macros register parameter groups:
PG_REGISTER(type, NAME, pgn, version)                    // NAME is param 2
PG_REGISTER_ARRAY(type, size, NAME, pgn, version)        // NAME is param 3
PG_REGISTER_WITH_RESET_FN(type, NAME, pgn, version)      // NAME is param 2
// ... and 10+ other variants
```

**Example extraction:**
```c
// From src/main/fc/settings.c:
PG_REGISTER(systemConfig_t, systemConfig, PG_SYSTEM_CONFIG, 2);

// Script extracts: "systemConfig"
// Generates:
extern const pgRegistry_t systemConfig_Registry;
```

#### 2. Generated File: `src/main/target/SITL/wasm_pg_registry.c`

**Structure:**
```c
#ifdef __EMSCRIPTEN__

#include "config/parameter_group.h"

// 1. Extern declarations for all 74 PG registries
extern const pgRegistry_t accelerometerConfig_Registry;
extern const pgRegistry_t adcChannelConfig_Registry;
// ... 72 more ...

// 2. Static array of pointers
static const pgRegistry_t* __wasm_pg_registry[] = {
    &accelerometerConfig_Registry,
    &adcChannelConfig_Registry,
    // ... 72 more ...
};

// 3. Define the linker symbols as pointers to array
const pgRegistry_t* const __pg_registry_start = &__wasm_pg_registry[0];
const pgRegistry_t* const __pg_registry_end = &__wasm_pg_registry[74];

// 4. Stub resetdata (TODO: implement properly)
static const uint8_t __wasm_pg_resetdata_stub[1] = {0};
const uint8_t* const __pg_resetdata_start = &__wasm_pg_resetdata_stub[0];
const uint8_t* const __pg_resetdata_end = &__wasm_pg_resetdata_stub[0];

#endif
```

**Key Design Decisions:**

1. **Pointer Types**: Uses `const pgRegistry_t* const` instead of arrays
   - Why: Matches how `PG_FOREACH` macro uses these symbols
   - Native: `extern const pgRegistry_t __pg_registry_start[]` (array)
   - WASM: `extern const pgRegistry_t* const __pg_registry_start` (pointer)

2. **Conditional Compilation**: `#ifdef __EMSCRIPTEN__`
   - Only compiled for WASM builds
   - Native builds continue using linker script approach

3. **Static Array**: Registry array is static, symbols point into it
   - Ensures all registries are in contiguous memory
   - Allows pointer arithmetic: `__pg_registry_end - __pg_registry_start`

#### 3. Header Changes: `src/main/config/parameter_group.h`

**Added WASM-specific declarations:**
```c
#ifdef __EMSCRIPTEN__
// WASM: Use pointer types (manual registry)
extern const pgRegistry_t* const __pg_registry_start;
extern const pgRegistry_t* const __pg_registry_end;
#define PG_REGISTER_ATTRIBUTES __attribute__ ((used, aligned(4)))
#else
// Native: Use array types (linker sections)
extern const pgRegistry_t __pg_registry_start[];
extern const pgRegistry_t __pg_registry_end[];
#define PG_REGISTER_ATTRIBUTES __attribute__ ((section(".pg_registry"), used, aligned(4)))
#endif
```

**Why Different Types?**
- `PG_FOREACH` macro works with both:
  ```c
  #define PG_FOREACH(_name) \
      for (const pgRegistry_t *(_name) = __pg_registry_start; (_name) < __pg_registry_end; _name++)
  ```
- Arrays decay to pointers in expressions
- WASM version explicitly uses pointers for clarity

#### 4. Build Integration: `cmake/sitl.cmake`

**WASM-specific source inclusion:**
```cmake
if(NOT ${TOOLCHAIN} STREQUAL "wasm")
    main_sources(SITL_SRC
        drivers/serial_tcp.c
        target/SITL/sim/realFlight.c
        # ... native-only sources
    )
else()
    # WASM-specific: Manual PG registry
    main_sources(SITL_SRC
        target/SITL/wasm_pg_registry.c
    )
endif()
```

### Build Process

1. **Run script** (manual or in CMake):
   ```bash
   cd inav
   ./src/utils/generate_wasm_pg_registry.sh
   ```

2. **Script output**: `src/main/target/SITL/wasm_pg_registry.c` (auto-generated)

3. **CMake includes** `wasm_pg_registry.c` in WASM builds only

4. **Compiler builds** wasm_pg_registry.c with `__EMSCRIPTEN__` defined

5. **Linker resolves** `__pg_registry_start` and `__pg_registry_end` symbols

### Maintenance

**When to regenerate:**
- After adding new parameter groups (`PG_REGISTER*` calls)
- When PG names change
- Before WASM builds if PG changes are suspected

**Automatic regeneration:**
- Could be integrated into CMake as a PRE_BUILD step
- Currently manual to avoid dependency on bash in all build environments

**Validation:**
```bash
# Check PG count in native binary
objdump -h inav/build_sitl/bin/SITL.elf | grep pg_registry
# Size should match: <count> * 32 bytes (on 64-bit)

# Check PG count in source
grep -rh "^PG_REGISTER" inav/src/main/ --include="*.c" | wc -l
# Should equal count in wasm_pg_registry.c
```

## Limitations & Future Work

### Current Limitations

1. **Reset Data Stubbed**: `__pg_resetdata_*` symbols point to empty array
   - Impact: Default values for PG structs won't work
   - Workaround: PG reset functions still work
   - Fix: Extract `.pg_resetdata` section from native binary

2. **Conditional PGs**: Some PGs only compile with certain features
   - Impact: Script may include registries that don't exist
   - Current status: Causes linker errors (8 missing registries found)
   - Fix: Filter PG list based on active build features

3. **Manual Process**: Script must be run manually
   - Impact: Can forget to regenerate after PG changes
   - Fix: Integrate into CMake as custom command

### Future Improvements

1. **CMake Integration**:
   ```cmake
   add_custom_command(
       OUTPUT wasm_pg_registry.c
       COMMAND ${CMAKE_SOURCE_DIR}/src/utils/generate_wasm_pg_registry.sh
       DEPENDS ${ALL_SOURCE_FILES}
   )
   ```

2. **Feature-Aware Generation**: Parse `#ifdef` guards to exclude conditional PGs

3. **Reset Data Handling**:
   - Extract from native build
   - Or parse `pgResetTemplate_*` symbols
   - Or use constructor registration pattern

4. **Validation Tests**: Compare native vs WASM registry contents

## Alternative Approaches Considered

### ❌ Extract Binary Blob from Native Build
- **Idea**: Use `objcopy` to extract `.pg_registry` section from native SITL
- **Problem**: Contains native memory addresses (64-bit pointers)
- **Reason Rejected**: Would need complex pointer relocation

### ❌ Use wasm-ld `__start/__stop` Symbols
- **Idea**: Use GNU LD's automatic section boundary symbols
- **Problem**: Not supported by wasm-ld despite 2019 "fix" claim
- **Testing**: Confirmed still undefined in Emscripten 4.0.20
- **Reason Rejected**: Feature doesn't exist

### ❌ Constructor-Based Registration
- **Idea**: Use `__attribute__((constructor))` to register at runtime
- **Problem**: Would require modifying all 74 `PG_REGISTER` macro invocations
- **Reason Rejected**: Too invasive for upstream contribution

### ✅ Script-Generated Manual Registry (Chosen)
- **Pros**:
  - No runtime overhead
  - Minimal code changes (one header, one script, one generated file)
  - Easy to maintain
  - Transparent to upstream (can be WASM-specific)
- **Cons**:
  - Requires regeneration when PGs change
  - Doesn't handle conditional compilation automatically

## References

- Original issue: SITL WASM Phase 1 POC
- Emscripten custom sections: https://github.com/emscripten-core/emscripten/issues/5572
- wasm-ld documentation: https://lld.llvm.org/WebAssembly.html
- Parameter Group architecture: `src/main/config/parameter_group.h`
- Linker script (native): `src/main/target/link/sitl.ld`

## Author

Auto-generated by Claude Code as part of SITL WASM Phase 1 POC (2025-12-02)
