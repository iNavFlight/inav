# INAV CMake Build System

This document describes the CMake build system architecture and how to extend it.

## Architecture Overview

The INAV CMake system supports building for three distinct platforms:

1. **ARM Microcontrollers** (default)
   - STM32 F4/F7/H7
   - AT32F43x
   - Uses: `arm-none-eabi-gcc`

2. **SITL (Software-In-The-Loop)** - Native builds
   - Linux/macOS/Windows
   - Uses: `gcc`/`clang` (native compiler)

3. **WebAssembly** (new)
   - Browser-based
   - Uses: Emscripten (`emcc`)

## Project Structure

```
CMakeLists.txt              # Main CMake entry point
cmake/
├── emscripten.cmake        # Emscripten toolchain (new)
├── wasm.cmake              # WASM build rules (new)
├── host.cmake              # SITL native compilation
├── arm-none-eabi.cmake     # ARM embedded cross-compilation
├── stm32.cmake             # STM32-specific rules
├── at32.cmake              # AT32-specific rules
├── sitl.cmake              # SITL simulation rules
├── main.cmake              # Common compilation options
├── settings.cmake          # Settings generator
└── ... (other toolchain files)

src/
├── CMakeLists.txt          # Main source configuration
├── main/
│   ├── CMakeLists.txt      # Main source list
│   ├── target/
│   │   ├── CMakeLists.txt  # Aggregate target configs
│   │   ├── SITL/           # SITL simulation target
│   │   └── WASM/           # WebAssembly target (new)
│   └── ... (source files)
└── test/
    └── CMakeLists.txt      # Test builds
```

## Build Flow

### 1. Main CMakeLists.txt

Entry point that:
- Parses build options (`SITL`, `WASM`)
- Selects appropriate toolchain
- Configures project parameters
- Includes platform-specific CMake modules

Key decision logic:

```cmake
if (WASM)
    set(TOOLCHAIN "emscripten")
elseif (SITL)
    set(TOOLCHAIN "host")
else()
    set(TOOLCHAIN "arm-none-eabi")
endif()
```

### 2. Toolchain Selection

Three main toolchain files:

| Toolchain | File | Target | Compiler |
|-----------|------|--------|----------|
| ARM | `arm-none-eabi.cmake` | Microcontrollers | `arm-none-eabi-gcc` |
| Host | `host.cmake` | SITL (Native) | `gcc`/`clang` |
| Emscripten | `emscripten.cmake` | WebAssembly | `emcc` |

### 3. Platform-Specific Rules

After toolchain selection, platform rules are included:

- `stm32.cmake` - STM32 microcontroller rules
- `at32.cmake` - AT32 microcontroller rules
- `sitl.cmake` - SITL simulation rules
- `wasm.cmake` - WebAssembly build rules

### 4. Target Definition

Each target (e.g., `MATEKF405SE`, `SITL`, `WASM`) is defined via:

- CMake functions: `target_arm()`, `target_sitl()`, `target_wasm()`
- Source configuration: Lists of `.c` files to include/exclude
- Definitions: Compiler defines (e.g., `WASM_BUILD`)

## Building

### From Root Directory

```bash
# Build for microcontroller
./build.sh MATEKF405SE

# Build SITL
cd build && cmake -DSITL=ON .. && make SITL_TARGET

# Build WebAssembly
./build_wasm.sh Release
```

### Manual CMake Build

```bash
mkdir -p build/custom && cd build/custom

# ARM microcontroller
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../cmake/arm-none-eabi.cmake

# SITL
cmake ../.. -DSITL=ON

# WASM
cmake ../.. -DWASM=ON -DCMAKE_TOOLCHAIN_FILE=../../cmake/emscripten.cmake
```

## Adding New Targets

### Adding a New Microcontroller Board

1. Create target directory:
```bash
mkdir -p src/main/target/MYBOARD
```

2. Create `target.h` with hardware definitions

3. Create `config.c` with default settings

4. Create `CMakeLists.txt`:
```cmake
target_arm(MYBOARD STM32F7)
```

5. Build:
```bash
./build.sh MYBOARD
```

### Adding a New WASM Feature

1. Create feature in `src/main/target/WASM/wasm/`:

```c
// wasm/my_feature.c
void my_wasm_function(void) {
    // WASM-specific code
}
```

2. Add to exported functions in `cmake/wasm.cmake`:

```cmake
-s EXPORTED_FUNCTIONS="[..., '_my_wasm_function']"
```

3. Call from JavaScript:

```javascript
Module.ccall('my_wasm_function', null, [], []);
```

## CMake Macros and Functions

### Common Macros

#### `main_sources(var [sources...])`

Prepends `${MAIN_SRC_DIR}` to source paths:

```cmake
main_sources(MY_SOURCES
    drivers/example.c
    flight/controller.c
)
# Expands to: src/main/drivers/example.c src/main/flight/controller.c
```

#### `exclude(var excludes)`

Removes items from list:

```cmake
exclude(COMMON_SRC "${EXCLUDES}")
```

### Target Functions

#### `target_arm(name mcu [args...])`

Creates ARM microcontroller target (from `stm32.cmake`/`at32.cmake`):

```cmake
target_arm(MATEKF405SE STM32F4)
```

#### `target_sitl(name [args...])`

Creates SITL simulation target (from `sitl.cmake`):

```cmake
target_sitl(SITL)
```

#### `target_wasm(name [args...])`

Creates WebAssembly target (from `wasm.cmake`):

```cmake
target_wasm(WASM)
```

### Configuration Macros

#### `PG_DECLARE()` / `PG_REGISTER()`

Configuration system (not CMake, but used in source):

```c
// In .h file
PG_DECLARE(myConfig_t, myConfig);

// In .c file
PG_REGISTER_WITH_RESET_TEMPLATE(myConfig_t, myConfig, PG_MY_CONFIG, 0);
```

## Compiler Flags

### Common Flags

All builds use:
```
-Wall -Wextra -Wdouble-promotion -Wstrict-prototypes -Werror=switch
```

### Architecture-Specific

**ARM:**
```
-mcpu=cortex-mX -mthumb -fPIC
```

**SITL (Host):**
```
-Wno-format -funsigned-char
```

**WebAssembly:**
```
-fPIC -Wno-format
```

### Build Type Flags

| Type | Flags |
|------|-------|
| Debug | `-O0 -g` |
| Release | `-O3 -DNDEBUG` (WASM), `-Os` (ARM) |
| RelWithDebInfo | `-g -Os` (ARM), `-g -O3` (WASM) |

## Linker Configuration

### ARM Microcontroller

- Linker script: `src/main/target/link/<target>.ld`
- Memory regions: Defined in linker script
- Sections: Startup code, flash, RAM

### SITL

- Linker script: `src/main/target/link/sitl.ld`
- Dynamic linking libraries: pthreads, math, libc

### WebAssembly

- No linker script
- Emscripten runtime provides memory model
- Heap grows dynamically (configurable)

## Settings System

Settings are generated from YAML at build time:

```yaml
# src/main/fc/settings.yaml
settings:
  - name: gyro_lpf
    type: uint16_t
    min: 0
    max: 1000
    default: 100
```

Generates: `settings_generated.h` and `settings_generated.c`

## Build Artifacts

### ARM Microcontroller Output

```
build/bin/<BOARDNAME>/
└── inav_X.X.X_<BOARDNAME>.hex    # Flash image
```

### SITL Output

```
build/bin/<BOARDNAME>/
└── inav_X.X.X_SITL               # Executable
```

### WebAssembly Output

```
build/
├── inav_X.X.X_WASM.js            # JavaScript wrapper
├── inav_X.X.X_WASM.wasm          # WebAssembly binary
└── inav_X.X.X_WASM.data          # Optional data file
```

## Extending the Build System

### Adding a New Platform

1. Create `cmake/newplatform.cmake` with compiler configuration
2. Update `CMakeLists.txt` to include new platform option
3. Create `cmake/newplatform_rules.cmake` for platform-specific build rules
4. Add target example in `src/main/target/`

### Adding New Build Options

```cmake
# In CMakeLists.txt
option(MY_OPTION "Description" DEFAULT_VALUE)

# Use in platform file
if(MY_OPTION)
    target_compile_definitions(${target} PRIVATE MY_OPTION_ENABLED)
endif()
```

### Custom Compilation Targets

```cmake
# In src/main/target/<TARGET>/CMakeLists.txt
add_custom_command(TARGET ${name} POST_BUILD
    COMMAND echo "Build complete for ${name}"
)
```

## Debugging Build Issues

### Verbose Output

```bash
cd build && cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
make VERBOSE=1
```

### CMake Debug

```bash
cmake --debug-output ../..
```

### Check Variables

```cmake
message(STATUS "TOOLCHAIN: ${TOOLCHAIN}")
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "COMMON_SRC: ${COMMON_SRC}")
```

### Compiler Verification

```bash
# Test compiler
${CMAKE_C_COMPILER} --version

# Check toolchain
arm-none-eabi-gcc --version
emcc --version
gcc --version
```

## CI/CD Integration

Example GitHub Actions workflow:

```yaml
name: Build INAV

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        target: [MATEKF405SE, SITL, WASM]
    
    steps:
      - uses: actions/checkout@v2
      - uses: mymindstorm/setup-emsdk@v7  # For WASM
      
      - name: Build ${{ matrix.target }}
        run: |
          if [[ "${{ matrix.target }}" == "WASM" ]]; then
            ./build_wasm.sh Release
          else
            ./build.sh ${{ matrix.target }}
          fi
```

## References

- [CMake Documentation](https://cmake.org/documentation/)
- [Emscripten CMake Integration](https://emscripten.org/docs/tools_reference/cmake_impl_plugins.html)
- [STM32 CMake Examples](https://github.com/STM32Cube/STM32CubeF4)
