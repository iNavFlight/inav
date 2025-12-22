# INAV Flight Controller - Copilot Instructions

## Project Overview
**INAV** is a feature-rich open-source flight controller for navigation-capable UAVs (multirotor, fixed-wing, rovers, boats). It runs on ARM Cortex (F4, F7, H7) and AT32 MCUs and supports advanced features like Position Hold, RTH, waypoint missions, OSD, and Blackbox logging.

**Current Version:** 9.0.0 | **Current Branch:** Webassembly (WASM target in development)

## Architecture & Component Organization

### Core Directory Structure
- **`src/main/`** – Primary firmware source
  - `main.c` – Entry point; initializes system and runs main scheduler loop
  - `fc/` – Flight controller core (initialization, config, runtime state, PID/mixer)
  - `flight/` – Flight logic (IMU, mixer, servos, navigation, failsafe)
  - `navigation/` – Advanced guidance (waypoints, RTH, geo-fencing, fixed-wing-specific)
  - `sensors/` – IMU, baro, compass, GPS, optical flow, rangefinder drivers
  - `drivers/` – Hardware abstraction (SPI, I2C, UART, PWM, USB VCP, SD card, etc.)
  - `config/` – Parameter system (parameter groups, feature flags, settings)
  - `target/` – Board-specific hardware config (~200 supported boards)

- **`src/test/unit/`** – Unit tests using GoogleTest framework
- **`cmake/`** – Build system configuration (toolchain selection: ARM, SITL, WebAssembly)
- **`docs/`** – User-facing documentation (installation, configuration, debugging)

### Build System
**CMake-based** with three primary targets:
1. **Embedded (ARM):** Cross-compile for STM32F4/F7/H7, AT32 MCUs
   - Build: `./build.sh <TARGET_NAME>` (Docker-based)
   - Or manually: `cmake .. && make MATEKF405` from `build/` dir
   
2. **SITL** (Software-In-The-Loop simulation for host):
   - `cmake .. -DSITL=ON && make`
   - Useful for testing navigation/control logic without hardware

3. **WebAssembly (WASM):** Experimental browser-based UI simulator
   - `./build_wasm.sh [Debug|Release]`
   - Emscripten required; outputs `.js` and `.wasm` files

## Key Architectural Patterns

### 1. Parameter Groups (Settings System)
INAV uses a compile-time registry of configurable parameters. Settings are automatically persisted to EEPROM:

```c
// In src/main/config/parameter_group_ids.h - declare a group ID
#define PG_GYRO_CONFIG 5

// In driver/gyro.h - define the structure with version
typedef struct gyroConfig_s {
    uint8_t gyro_lpf;
    uint16_t gyro_sync_denom;
    // ...
} gyroConfig_t;

// In driver/gyro.c - register and provide reset function
PG_REGISTER(gyroConfig_t, gyroConfig, PG_GYRO_CONFIG, 3);
```

**Rule:** Settings accessed via accessor functions like `gyroConfig()` return the global instance. Version field (embedded in `pgn`) supports migration when struct layout changes.

### 2. Driver Initialization Pattern
All hardware drivers follow a consistent layered approach:

```c
// High-level API (e.g., sensors/gyro.c)
void gyroInit(void);          // Initialize driver
void gyroRead(gyroADC_t *);   // Read raw data

// Medium-level bus abstraction (drivers/bus.c, drivers/bus_spi.c)
busDevice_t *gyro_bus;
bool busReadBuf(busDevice_t *, uint8_t addr, uint8_t *buf, uint8_t len);

// Low-level HAL (drivers/spi_hal.c, drivers/uart_hal.c, etc.)
// Typically implemented for each MCU family (STM32F4, STM32F7, STM32H7, AT32)
```

**Key convention:** MCU-specific files have suffixes: `_stm32f4xx.c`, `_stm32f7xx.c`, `_stm32h7xx.c`, `_at32f43x.c`

### 3. Target-Specific Configuration
Each board lives in `src/main/target/<BOARDNAME>/` with files:
- `target.h` – Pin mappings, hardware defines (USE_SPI, USE_UART1, etc.), default features
- `target.c` – Board-specific init code (systemInit, timerInit, etc.)

Board selection is determined by CMake; target name becomes a macro like `-DAOCODARCH7DUAL`.

### 4. Scheduler & Timing
The scheduler (scheduler/scheduler.c) manages periodic tasks with microsecond precision:

```c
// Register a task that runs every N microseconds
// cfTasksInit() in fc/fc_tasks.c defines all persistent tasks
schedulerAddTask("IMU_UPDATE", imuUpdateCallback, 2000);  // every 2ms
```

**Pattern:** Times use `timeUs_t` (microseconds), and `micros()` provides the current time. Always check `timeDelta_t` calculations for overflow in loops.

## Configuration & Runtime State

### Feature Flags
Features are bit-packed in `fc/runtime_config.h`:

```c
FEATURE_RX_PARALLEL_PWM
FEATURE_RX_PPM
FEATURE_TELEMETRY
FEATURE_FAILSAFE
// etc.
```

**Check:** `featureIsEnabled(FEATURE_FAILSAFE)` or `feature(FEATURE_BARO | FEATURE_PITOT)`

### Settings vs. Features vs. Defines
- **Settings (`config/general_settings.h`):** Runtime-changeable parameters persisted to EEPROM (PGs)
- **Features (`config/feature.h`):** Runtime toggles (bits in EEPROM)
- **Compile-time Defines (`target/<BOARD>/target.h`):** Board-specific hardware config (USE_BARO, USE_COMPASS pins, etc.)

## Core Initialization Sequence

1. `main()` → `init()` in `fc/fc_init.c`
2. `initialisePreBootHardware()` – Basic clocking, GPIO, UART for debug
3. `systemInit()` – Target-specific MCU init
4. `boardInit()` – Apply board-specific config
5. `configLoad()` – Load settings from EEPROM (or defaults)
6. Hardware inits: `busInit()`, `adcInit()`, `serialInit()`, `compassInit()`, `barometerInit()`, etc.
7. `imuInit()` – IMU/Gyro calibration loop
8. `fcTasksInit()` – Register scheduler tasks
9. Main loop: `while(true) { scheduler(); }`

## Navigation & Flight Logic

### Data Flow
GPS/Baro/IMU sensors → `sensors_update()` → `imuUpdateCallback()` → **Navigation (pos estimator)** → Flight control (PID) → Mixer → PWM outputs

**Key files:**
- `navigation/navigation_pos_estimator.c` – Kalman filter for position/velocity estimation
- `navigation/navigation.c` – High-level waypoint, RTH, position-hold logic
- `navigation/navigation_multicopter.c`, `navigation_fixedwing.c` – Vehicle-specific guidance
- `flight/pid.c` – PID controller (supports cascaded loops for velocity/position)
- `flight/mixer.c` – Mixes stabilized output to individual motors/servos

## Important Coding Conventions

### Naming
- **Settings:** `SETTING_<GROUP>_<PARAM>` (e.g., `SETTING_GYRO_LPF`)
- **Constants:** `<SCOPE>_<NAME>` (e.g., `MAX_MOTOR_PROFILE_COUNT`)
- **Enums:** lowercase with `_e` suffix (e.g., `flightMode_e`)
- **Structs:** CamelCase with `_s` suffix (e.g., `gyroConfig_s`)
- **Functions:** camelCase (e.g., `navigationInit()`)

### C Standard
- **Embedded targets:** C99 with extensions (`-std=gnu99`)
- **SITL/WASM:** C11 (`-std=gnu11`)
- Avoid C++ features in main firmware

### Compiler Flags
Warnings are **strict**: `-Wall -Wextra -Werror=switch`. No implicit function declarations. Test changes against all three toolchains (arm, SITL, WASM).

## Build & Test Workflow

### Building for a Specific Board
```bash
cd build
cmake ..
make MATEKF405              # or any TARGET_NAME from `./build.sh valid_targets`
# Output: build/bin/inav_*.hex, *.elf
```

### Building SITL
```bash
cd build
cmake .. -DSITL=ON
make
./bin/inav_SITL  # runs local simulation
```

### WebAssembly Build
```bash
./build_wasm.sh Release
# Requires Emscripten SDK; outputs build/wasm/inav_*_WASM.js + .wasm
```

### Running Unit Tests
```bash
cd build
cmake ..
make -j
ctest  # or cd test && ./unit_test_executable
```

### Docker Build (All Targets)
```bash
./build.sh release_targets  # builds official release set
```

## Critical Gotchas & Patterns

### 1. Parameter Group Versioning
When modifying a `typedef struct` in a PG, **increment the version number** in the `PG_REGISTER` macro. Old EEPROM data must be migrated or defaults applied.

### 2. Time Calculations
- Always use `timeUs_t` for microsecond storage (32-bit, wraps ~4295 seconds)
- Use `timeDelta_t` for differences; check for overflow in long-running calculations
- Never subtract time variables directly without checking for wrap-around

### 3. Cross-Platform Defines
The same source compiles for ARM, SITL, and WASM. Use platform checks:
```c
#if defined(SITL_BUILD)
  // SITL-specific code
#elif defined(WASM_BUILD)
  // WASM-specific code
#else
  // ARM-specific code (default)
#endif
```

### 4. Sensor/Driver Initialization Order
Some drivers depend on others (e.g., compass requires I2C; I2C requires GPIO). The `fc_init.c` sequence is carefully ordered—**do not reorder** without understanding dependencies. Check `busInit()` before sensor inits.

### 5. MSP & Configurator Integration
INAV exposes telemetry and configuration via MSP protocol (MSPv1/v2). When adding new settings, register them in `fc/settings.h` with a unique setting ID and `SETTING_*` macros so the Configurator can discover them.

## Testing & Debugging

### Adding Unit Tests
Place new tests in `src/test/unit/` with GoogleTest syntax:
```cpp
#include <gtest/gtest.h>
#include "flight/pid.h"

TEST(PIDTest, BasicProportionalControl) {
  // setup, execute, assert
}
```

Compile with `-DUNIT_TEST` (automatic in CMake build).

### Blackbox Logging
For flight data capture: configure blackbox via CLI/Configurator. Data is written to SD card (if available) or flash. Use `blackbox-log-viewer` to replay and debug flight behavior.

### Debug Output
Use `DEBUG` mode in `build/debug.h` to log debug values:
```c
debug[0] = someValue;  // writes to blackbox and telemetry
```

## External Dependencies
- **ARM GCC Toolchain:** `arm-none-eabi-gcc`
- **Emscripten:** For WASM builds (separate SDK)
- **CMake:** ≥3.13
- **GoogleTest:** Auto-fetched by CMake for unit tests
- **STM32/AT32 HALs:** Bundled in project
- **MAVLink:** Bundled in `lib/main/MAVLink`

## Key Files to Know
- **`src/main/main.c`** – Entry point and main loop
- **`src/main/fc/fc_init.c`** – Comprehensive initialization
- **`src/main/fc/fc_tasks.c`** – Task scheduler setup
- **`CMakeLists.txt`** – Build configuration (version, architecture)
- **`cmake/settings.cmake`** – Auto-generation of settings enums
- **`src/main/config/parameter_group.h`** – PG system core
- **`src/main/navigation/navigation.c`** – Navigation core
- **`src/main/flight/mixer.c`** – Motor/servo mixing
- **`docs/Installation.md`** – User-facing build docs

---

**Last Updated:** 2025-12-05 | **Version:** INAV 9.0.0 | **Branch:** Webassembly
