# Build System Reference: CMAKE_BUILD_TYPE, Optimization, and Disk Usage

Reference for INAV's CMake build types, compiler flags, output artifacts,
and why full multi-target builds use a lot of disk space.

## CMAKE_BUILD_TYPE Options

INAV supports three build types, defined in `cmake/arm-none-eabi.cmake`:

| Build Type | Compiler Flags | Debug Symbols | Optimization | LTO | Use Case |
|------------|---------------|---------------|--------------|-----|----------|
| **Debug** | `-Og -g` | Yes (`-g`) | Minimal (`-Og`) | No | Active development, debugging |
| **Release** | `-DNDEBUG` | No | Target-specific (`-O2`/`-Os`) | Yes | Production releases |
| **RelWithDebInfo** | `-ggdb3 -DNDEBUG` | Yes (`-ggdb3`) | Target-specific (`-O2`/`-Os`) | Yes | **Default** — optimized but debuggable |

**Debug:** `-Og`, asserts enabled (no `NDEBUG`), basic debug symbols, no LTO,
largest binaries, fastest build.

**Release:** `-O2`/`-Os` (target-specific), asserts disabled, no debug
symbols, LTO enabled, smallest binaries, slowest build (LTO cost).

**RelWithDebInfo (default):** same optimization and LTO as Release, but with
`-ggdb3` debug info for GDB — optimized firmware you can still debug.

### How CMAKE_BUILD_TYPE Gets Set

**Default**, from `cmake/arm-none-eabi.cmake`:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
```

If you don't specify a build type, you get RelWithDebInfo. Neither
`build.sh` (Docker → `cmake/docker.sh`) nor the SITL build scripts pass
`-DCMAKE_BUILD_TYPE`, so normal builds use this default.

**Override explicitly:**

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..      # or Release, or RelWithDebInfo
make MATEKF405
```

**Common misconception:** the `make release` target (see below) does *not*
control optimization — it just builds the set of hardware targets flagged
for release, using whatever `CMAKE_BUILD_TYPE` is already configured.

### Compiler Flags by Build Type

From `cmake/arm-none-eabi.cmake`:

```cmake
set(arm_none_eabi_debug "-Og -g")
set(arm_none_eabi_release "-DNDEBUG")
set(arm_none_eabi_relwithdebinfo "-ggdb3 ${arm_none_eabi_release}")
```

Target-specific optimization flags are only applied for Release and
RelWithDebInfo (`cmake/stm32.cmake`):

```cmake
if (IS_RELEASE_BUILD)
    target_compile_options(${elf_target} PRIVATE ${args_OPTIMIZATION})
    target_link_options(${elf_target} PRIVATE ${args_OPTIMIZATION})
endif()
```

`IS_RELEASE_BUILD` is set in `CMakeLists.txt`:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(IS_RELEASE_BUILD ON)
endif()
```

Debug builds get no target-specific optimization flags at all.

### Optimization Level by MCU Family

| MCU Family | Optimization | Source |
|---|---|---|
| STM32F4 (most) | `-O2` | `cmake/stm32f4.cmake` |
| STM32F411 | `-Os` | `cmake/stm32f4.cmake` (size-constrained, 512KB flash) |
| STM32F7 | `-O2` or `-Os` | `cmake/stm32f7.cmake`, by flash size |
| STM32H7 | `-O2` | `cmake/stm32h7.cmake` |
| AT32F4 | `-O2` | `cmake/at32f4.cmake` |

F7's choice is a numeric flash-size check, not a letter-code lookup —
`cmake/stm32f7.cmake`:

```cmake
if(flash_size GREATER 512)
    set(opt -O2)   # more than 512KB flash — optimize for speed
else()
    set(opt -Os)   # 512KB or less — optimize for size
endif()
```

### Link-Time Optimization (LTO)

From `cmake/main.cmake`:

```cmake
if(IS_RELEASE_BUILD AND NOT (CMAKE_HOST_APPLE AND SITL))
    set_target_properties(${exe} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION ON
    )
endif()
```

LTO is enabled for Release and RelWithDebInfo (except macOS SITL builds).

## Output Files

Building a target (e.g. `MATEKF405`) produces:

- **`.elf`** (`build/obj/main/<TARGET>.elf`) — the full ELF binary. Contains
  loadable sections plus, for Debug/RelWithDebInfo, DWARF debug sections
  (`.debug_info`, `.debug_line`, etc.) and the symbol table. Needed for GDB,
  `arm-none-eabi-nm`/`objdump`, and for producing the .hex/.bin below.
- **`.map`** (`build/obj/main/<TARGET>.map`) — linker map with memory
  layout, symbol names and addresses, generated via `-Wl,-Map,${map}`
  (`cmake/stm32.cmake`). Always generated regardless of build type. Useful
  for mapping crash addresses to functions without a full .elf.
- **`.hex`** / **`.bin`** (`build/inav_<version>_<TARGET>.{hex,bin}`) — what
  actually gets flashed. Produced via:

  ```bash
  ${CMAKE_OBJCOPY} -Oihex --set-start 0x08000000 <target>.elf out.hex
  ${CMAKE_OBJCOPY} -Obinary <target>.elf out.bin
  ```

  These are the same size regardless of build type: `objcopy` only extracts
  the loadable sections (`.text`/`.rodata`/`.data`/`.bss`), so DWARF debug
  info and the symbol table — which only exist in non-loadable sections —
  never make it into the .hex/.bin in the first place. There's no separate
  "strip debug info" step; it's simply that debug info was never in a
  section `objcopy` copies.

**For releases**, keep the `.elf` and `.map` alongside the `.hex` you
distribute — users report crashes with addresses, and you need the matching
`.elf`/`.map` from that exact build to map them back to source.

## Detecting Build Type From Artifacts

| Method | Works With | Reliability |
|---|---|---|
| Check `.elf` for `.debug_*` sections (`arm-none-eabi-readelf -S`) | `.elf` only | High — distinguishes Debug/RelWithDebInfo from Release |
| Compare `.elf` file size | `.elf` only | Medium — rough estimate |
| Search for assert strings | any | Medium — only distinguishes Debug from Release/RelWithDebInfo |
| `grep CMAKE_BUILD_TYPE build/CMakeCache.txt` | build directory | High — exact value, if you have the build dir |

## Why Full Multi-Target Builds Use So Much Disk Space

Building all ~100 hardware targets with the default RelWithDebInfo
configuration produces build directories in the tens-of-gigabytes range,
even though each target's final `.hex`/`.bin` is only a few hundred KB.
Reproduced independently (not the original session's exact numbers, but the
same effect, same toolchain used by this repo):

```
$ arm-none-eabi-gcc -c -ggdb3 -Os test.c -o test_debug.o    # struct with a few arrays
19776 bytes

$ arm-none-eabi-gcc -c -Os test.c -o test_release.o         # same file, no -ggdb3
752 bytes
```

—roughly a 26x size difference from `-ggdb3` alone, on a single small
translation unit. Scaled across ~1000 source files per target and ~100
targets, this is why RelWithDebInfo intermediate object files dominate disk
usage: DWARF debug info (line tables, type info, full symbol data) is large
relative to the actual generated code, especially at `-Os`/`-O2` where the
code itself is kept small.

**None of this affects final binaries** — as covered above, `.hex`/`.bin`
only contain loadable sections regardless of build type, so the extra debug
data in RelWithDebInfo `.o`/`.elf` files never reaches what you flash.

**No object-file sharing between targets.** Each target's `add_executable()`
compiles the entire source tree independently with its own `-D<TARGET>`
define and optimization flags, even though most targets share the large
majority of their code (common drivers, HAL, flight controller logic).
CMake has no built-in mechanism to share object files across separately
defined executables with different compile flags, so this is inherent to
the current per-target `add_executable()` structure, not a bug.

### Practical Options

- **Build Release instead of RelWithDebInfo** for full-target builds where
  you don't need to debug the result (e.g. CI, checking that everything
  still builds): `cmake -DCMAKE_BUILD_TYPE=Release ..` before building.
  Note this changes the build type for the whole build directory — there
  isn't a way to run only some targets in Release from the same
  configuration.
- **Delete intermediate object files after a successful build** if you only
  need the final binaries: `find build -name "*.o" -delete` (or
  target-specific object-file patterns). You'll need a full rebuild if you
  need to relink.
- **Build only the targets you need** with `-DTARGETS="MATEKF405SE,SITL"`
  rather than the full target list.
- **`ccache`** speeds up rebuilds significantly but doesn't reduce build
  directory disk usage — its cache is stored separately.

A CMake `OBJECT` library refactor (compiling shared code once and linking
it into each target) could substantially reduce disk usage even at
RelWithDebInfo, but this hasn't been attempted — it's a nontrivial
restructuring given the per-target optimization flags and preprocessor
defines currently used.

### Note on the Existing `release` Target

`cmake/main.cmake` already defines a custom target named `release`:

```cmake
set(release_target_name "release")
add_custom_target(${release_target_name}
    ${CMAKE_COMMAND} -E true
    DEPENDS ${release_targets}
)
```

This just builds the set of hardware targets not flagged `SKIP_RELEASES` —
it does **not** change `CMAKE_BUILD_TYPE`. A "rebuild everything in Release
mode" convenience target would need a different name (e.g. `release-slim`)
to avoid colliding with this existing one; naming it `release` would fail
CMake's configure step outright.
