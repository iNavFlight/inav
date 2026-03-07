# AGENT.md - Guide for AI Agents Working with INAV

## Project Overview

**INAV** is a navigation-capable flight controller firmware for multirotor, fixed-wing, and other RC vehicles. It is a community-driven project written primarily in C (C99/C11 standard) with support for STM32 F4, F7, H7, and AT32 microcontrollers.

### Key Characteristics
- **Type**: Embedded firmware for flight controllers
- **Language**: C (C99/C11), with some C++ for unit tests
- **Build System**: CMake (version 3.13+)
- **License**: GNU GPL v3
- **Version**: 9.0.1 (as of this writing)
- **Codebase History**: Evolved from Cleanflight/Baseflight

## Architecture and Structure

### Core Components

The INAV codebase is organized into the following major subsystems:

1. **Flight Control (`fc/`)**: Core flight controller logic, initialization, MSP protocol handling
2. **Sensors (`sensors/`)**: Gyro, accelerometer, compass, barometer, GPS, rangefinder, pitot tube
3. **Flight (`flight/`)**: PID controllers, mixers, altitude hold, position hold, navigation
4. **Navigation (`navigation/`)**: Waypoint missions, RTH (Return to Home), position hold
5. **Drivers (`drivers/`)**: Hardware abstraction layer for MCU peripherals (SPI, I2C, UART, timers, etc.)
6. **IO (`io/`)**: Serial communication, OSD, LED strips, telemetry
7. **RX (`rx/`)**: Radio receiver protocols (CRSF, SBUS, IBUS, etc.)
8. **Scheduler (`scheduler/`)**: Real-time task scheduling
9. **Configuration (`config/`)**: Parameter groups, EEPROM storage, settings management
10. **MSP (`msp/`)**: MultiWii Serial Protocol implementation
11. **Telemetry (`telemetry/`)**: SmartPort, FPort, MAVLink, LTM, CRSF telemetry
12. **Blackbox (`blackbox/`)**: Flight data recorder
13. **Programming (`programming/`)**: Logic conditions and global functions for in-flight programming

### Directory Structure

```
/src/main/              - Main source code
  ├── build/            - Build configuration and macros
  ├── common/           - Common utilities (math, filters, utils)
  ├── config/           - Configuration system (parameter groups)
  ├── drivers/          - Hardware drivers (MCU-specific)
  ├── fc/               - Flight controller core
  ├── flight/           - Flight control algorithms
  ├── io/               - Input/output (serial, OSD, LED)
  ├── msp/              - MSP protocol
  ├── navigation/       - Navigation and autopilot
  ├── rx/               - Radio receiver protocols
  ├── sensors/          - Sensor drivers and processing
  ├── scheduler/        - Task scheduler
  ├── telemetry/        - Telemetry protocols
  └── target/           - Board-specific configurations

/docs/                  - Documentation
  └── development/      - Developer documentation

/cmake/                 - CMake build scripts
/lib/                   - External libraries
/tools/                 - Build and utility tools
```

## Coding Conventions

### Naming Conventions

1. **Types**: Use `_t` suffix for typedef'd types: `gyroConfig_t`, `pidController_t`
2. **Enums**: Use `_e` suffix for enum types: `portMode_e`, `cfTaskPriority_e`
3. **Functions**: Use camelCase with verb-phrase names: `gyroInit()`, `deleteAllPages()`
4. **Variables**: Use camelCase nouns, avoid noise words like "data" or "info"
5. **Booleans**: Question format: `isOkToArm()`, `canNavigate()`
6. **Constants**: Upper case with underscores: `MAX_GYRO_COUNT`
7. **Macros**: Upper case with underscores: `FASTRAM`, `STATIC_UNIT_TESTED`

### Code Style

- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style (opening brace on same line, except for functions)
- **Line Length**: Keep reasonable (typically under 120 characters)
- **Comments**: Explain WHY, not WHAT. Document variables at declaration, not at extern usage
- **Header Guards**: Use `#pragma once` (modern convention used throughout codebase)

### Memory Attributes

INAV uses special memory attributes for performance-critical code on resource-constrained MCUs:

```c
FASTRAM                     // Fast RAM section (aligned)
EXTENDED_FASTRAM            // Extended fast RAM (STM32F4/F7 only)
DMA_RAM                     // DMA-accessible RAM (STM32H7, AT32F43x)
SLOW_RAM                    // Slower external RAM - deprecated, o not use
STATIC_FASTRAM              // Static variable in fast RAM
STATIC_FASTRAM_UNIT_TESTED  // Static fast RAM variable visible in unit tests
```

### Test Visibility Macros

```c
STATIC_UNIT_TESTED         // static in production, visible in unit tests
STATIC_INLINE_UNIT_TESTED  // static inline in production, visible in tests
INLINE_UNIT_TESTED         // inline in production, visible in tests
UNIT_TESTED                // Always visible (no storage class)
```

## Build System

### CMake Build Process

INAV uses CMake with custom target definition functions:

1. **Target Definition**: Each board has a `target.h` and optionally `CMakeLists.txt`
2. **Hardware Function**: `target_stm32f405xg(NAME optional_params)`
3. **Build Directory**: Always use out-of-source builds in `/build` directory

### Building a Target

```bash
# From workspace root
cd build
cmake ..
make MATEKF722SE     # Build specific target
make                 # Build all targets
```

### Target Configuration

Targets are defined in `/src/main/target/TARGETNAME/`:
- `target.h`: Hardware pin definitions, feature enables, MCU configuration
- `target.c`: Board-specific initialization code
- `CMakeLists.txt`: Build configuration (optional)

Example target definition:
```c
#define TARGET_BOARD_IDENTIFIER "MF7S"
#define LED0 PA14
#define BEEPER PC13
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN PA5
```

### Conditional Compilation

Feature flags control code inclusion:
```c
#ifdef USE_GPS
    // GPS code
#endif

#if defined(STM32F4)
    // F4-specific code
#elif defined(STM32F7)
    // F7-specific code
#endif
```

## Key Concepts

### Parameter Groups (PG)

INAV uses a sophisticated configuration system called "Parameter Groups" for persistent storage:

```c
// Define a configuration structure
typedef struct {
    uint8_t gyro_lpf_hz;
    uint16_t gyro_kalman_q;
    // ...
} gyroConfig_t;

// Register with reset template
PG_REGISTER_WITH_RESET_TEMPLATE(gyroConfig_t, gyroConfig, PG_GYRO_CONFIG, 12);

// Define default values
PG_RESET_TEMPLATE(gyroConfig_t, gyroConfig,
    .gyro_lpf_hz = 60,
    .gyro_kalman_q = 200,
    // ...
);

// Access in code
gyroConfig()->gyro_lpf_hz
```

**Key Functions:**
- `PG_REGISTER_WITH_RESET_TEMPLATE()`: Register with static defaults
- `PG_REGISTER_WITH_RESET_FN()`: Register with function-based initialization
- `PG_REGISTER_ARRAY()`: For arrays of configuration items
- Parameter group IDs are in `config/parameter_group_ids.h`

### Scheduler

INAV uses a priority-based cooperative task scheduler:

```c
typedef enum {
    TASK_PRIORITY_IDLE = 0,
    TASK_PRIORITY_LOW = 1,
    TASK_PRIORITY_MEDIUM = 3,
    TASK_PRIORITY_HIGH = 5,
    TASK_PRIORITY_REALTIME = 18,
} cfTaskPriority_e;
```

Tasks are defined in `fc/fc_tasks.c` with priority and desired execution period.

### Sensors and Calibration

Sensors use a common pattern:
1. **Detection**: Auto-detect hardware at boot (`gyroDetect()`, `baroDetect()`)
2. **Initialization**: Configure sensor parameters
3. **Calibration**: Zero-offset calibration (gyro, accelerometer)
4. **Data Processing**: Apply calibration, alignment, and filtering

### Flight Control Loop

The main control loop follows this pattern:
1. **Gyro Task** (highest priority): Read gyro, apply filters
2. **PID Task**: Calculate PID corrections
3. **RX Task**: Process radio input
4. **Other Tasks**: Sensors, telemetry, OSD, etc. (lower priority)

### MSP Protocol

MultiWii Serial Protocol is used for configuration and telemetry:
- Request/response model
- Message types defined in `msp/msp_protocol.h`
- Handlers in `fc/fc_msp.c`

## Common Patterns

### Hardware Abstraction

INAV abstracts hardware through resource allocation:

```c
// IO pins
IO_t pin = IOGetByTag(IO_TAG(PA5));
IOInit(pin, OWNER_SPI, RESOURCE_SPI_SCK, 1);

// Timers
const timerHardware_t *timer = timerGetByTag(IO_TAG(PA8), TIM_USE_ANY);

// DMA
dmaIdentifier_e dma = dmaGetIdentifier(DMA1_Stream0);
```

### Error Handling

INAV uses several approaches:
1. **Return codes**: Boolean success/failure or error enums
2. **Diagnostics**: `sensors/diagnostics.c` for sensor health
3. **Status indicators**: Beeper codes, LED patterns for user feedback
4. **Logging**: CLI-based logging system

### Filter Chains

Sensor data typically goes through multiple filtering stages:

```c
// Low-pass filters
gyroLpfApplyFn = lowpassFilterGetApplyFn(filterType);
gyroLpfApplyFn(&gyroLpfState[axis], sample);

// Notch filters (for dynamic filtering)
for (int i = 0; i < dynamicNotchCount; i++) {
    sample = biquadFilterApply(&notchFilter[i], sample);
}
```

### Board Alignment

All sensors go through board alignment transforms to correct for mounting orientation:

```c
// Apply board alignment rotation matrix
applySensorAlignment(gyroData, gyroData, gyroAlign);
```

### SITL (Software In The Loop)

INAV can be compiled for host system simulation:
```bash
cmake -DSITL=ON ..
make
```

## Development Workflow

### Branching Strategy

- **`maintenance-X.x`**: Current version development (e.g., `maintenance-9.x`)
- **`maintenance-Y.x`**: Next major version (e.g., `maintenance-10.x`)
- **`master`**: Tracks current version, receives merges from maintenance branches

### Pull Request Guidelines

1. **Target Branch**: 
   - Bug fixes and backward-compatible features → current maintenance branch
   - Breaking changes → next major version maintenance branch
   - **Never** target `master` directly

2. **Keep PRs Focused**: One feature/fix per PR

3. **Code Quality**:
   - Follow existing code style
   - Add unit tests where possible
   - Update documentation in `/docs`
   - Test on real hardware when possible

4. **Commit Messages**: Clear, descriptive messages

### Important Files to Check

Before making changes, review:
- `docs/development/Development.md` - Development principles
- `docs/development/Contributing.md` - Contribution guidelines
- Target-specific files in `/src/main/target/`

## Important Files and Directories

### Configuration Files

- `platform.h`: Platform-specific includes and defines
- `target.h`: Board-specific hardware configuration
- `config/parameter_group.h`: Parameter group system
- `config/parameter_group_ids.h`: PG ID definitions
- `fc/settings.yaml`: CLI settings definitions

### Core Flight Control

- `fc/fc_core.c`: Main flight control loop
- `fc/fc_init.c`: System initialization
- `fc/fc_tasks.c`: Task scheduler configuration
- `flight/pid.c`: PID controller implementation
- `flight/mixer.c`: Motor mixing

### Key Headers

- `build/build_config.h`: Build-time configuration macros
- `common/axis.h`: Axis definitions (X, Y, Z, ROLL, PITCH, YAW)
- `common/maths.h`: Math utilities and constants
- `common/filter.h`: Digital filter implementations
- `drivers/accgyro/accgyro.h`: Gyro/accelerometer interface

## AI Agent Guidelines

### When Adding Features

1. **Understand the Module**: Read related files in the same directory
2. **Check for Similar Code**: Search for similar features to maintain consistency
3. **Follow Parameter Group Pattern**: New settings should use PG system
4. **Add to Scheduler**: New periodic tasks go in `fc/fc_tasks.c`
5. **Update Documentation**: Add/update files in `/docs` 
6. **Consider Target Support**: Use `#ifdef USE_FEATURE` for optional features
7. **Generate CLI setting docs**: Remember to inform user to run `python src/utils/update_cli_docs.py`
8. **Follow conding standard**: Follow MISRA C rules
9. **Increase Paremeterer Group Version**: When changing PG structure, increase version corresponding in `PG_REGISTER`, `PG_REGISTER_WITH_RESET_FN`, `PG_REGISTER_WITH_RESET_TEMPLATE`, `PG_REGISTER_ARRAY` or `PG_REGISTER_ARRAY_WITH_RESET_FN`

### When Fixing Bugs

1. **Review Recent Changes**: Check git history for related modifications
2. **Test Signal Path**: For sensor issues, trace from hardware through filters to consumer
3. **Consider All Platforms**: STM32F4, F7, H7, and AT32 may behave differently
4. **Check Memory Usage**: Embedded system has limited RAM/flash

### When Refactoring

1. **Maintain API Compatibility**: Unless targeting next major version
2. **Preserve Unit Tests**: Update tests to match refactored code
3. **Update Documentation**: Keep docs synchronized with code changes
4. **Consider Performance**: Profile on target hardware, not just host compilation
5. **Check All Callers**: Use grep/search to find all usage sites

### Common Pitfalls to Avoid

1. **Don't Ignore Memory Attributes**: FASTRAM placement affects real-time performance
2. **Don't Break Parameter Groups**: Changing PG structure requires version bump
3. **Don't Assume Hardware**: Always check feature flags (`#ifdef USE_GPS`)
4. **Don't Break MSP**: Changes to MSP protocol affect configurator compatibility
5. **Don't Commit Wrong Branch**: Target maintenance branch, not master
6. **Don't Skip Documentation**: Code without docs increases support burden
7. **Don't Hardcode Values**: Use parameter groups for configurable values

### Searching the Codebase

**Find definitions:**
```bash
grep -r "typedef.*_t" src/main/  # Find all type definitions
grep -r "PG_REGISTER" src/main/  # Find parameter groups
grep -r "TASK_" src/main/        # Find scheduled tasks
```

**Find usage:**
```bash
grep -r "functionName" src/main/
grep -r "USE_GPS" src/main/target/  # Feature support by target
```

**Find similar code:**
- Look in the same directory first
- Check for similar sensor/peripheral implementations
- Review git history: `git log --all --oneline --grep="keyword"`

### Understanding Control Flow

1. **Startup**: `main.c` → `fc_init.c:init()` → `fc_tasks.c:tasksInit()`
2. **Main Loop**: `scheduler.c:scheduler()` executes tasks by priority
3. **Critical Path**: Gyro → PID → Mixer → Motors (highest priority)
4. **Configuration**: CLI/MSP → Parameter Groups → EEPROM

### Cross-Platform Considerations

Different MCU families have different characteristics:

- **STM32F4**: Most common, 84-168 MHz, FPU, no cache
- **STM32F7**: Faster, 216 MHz, FPU, I/D cache, requires cache management
- **STM32H7**: Fastest, 480 MHz, more RAM, complex memory architecture (DTCM, SRAM)
- **AT32F43x**: Chinese MCU, STM32F4-compatible, different peripherals

Always test on target or use `#if defined()` guards for MCU-specific code.

## Quick Reference

### File Naming Patterns

- `*_config.h`: Configuration structures (usually with PG)
- `*_impl.h`: Implementation headers (MCU-specific)
- `*_hal.h`: Hardware abstraction layer
- `accgyro_*.c`: Gyro/accelerometer drivers
- `bus_*.c`: Communication bus drivers (SPI, I2C)

### Common Abbreviations

- **FC**: Flight Controller
- **PG**: Parameter Group
- **MSP**: MultiWii Serial Protocol
- **CLI**: Command Line Interface
- **OSD**: On-Screen Display
- **RTH**: Return to Home
- **PID**: Proportional-Integral-Derivative (controller)
- **IMU**: Inertial Measurement Unit (gyro + accel)
- **AHRS**: Attitude and Heading Reference System
- **ESC**: Electronic Speed Controller
- **SITL**: Software In The Loop
- **HITL**: Hardware In The Loop

## Resources

- **Main Repository**: https://github.com/iNavFlight/inav
- **Configurator**: https://github.com/iNavFlight/inav-configurator
- **Discord**: https://discord.gg/peg2hhbYwN
- **Documentation**: https://github.com/iNavFlight/inav/wiki
- **Release Notes**: https://github.com/iNavFlight/inav/releases

## Version Information

This document is accurate for INAV 9.0.1. As the project evolves, some details may change. Always refer to the latest documentation and code for authoritative information.

---

**Remember**: INAV flies aircraft that people build and fly. Code quality, safety, and reliability are paramount. When in doubt, ask the community or maintainers for guidance.
