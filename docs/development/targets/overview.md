# INAV Target System Overview

This document explains how INAV hardware targets work and how they integrate with the build system.

## What is a Target?

A "target" is a specific flight controller hardware board configuration that defines:
- **MCU type and variant** (STM32F4, F7, H7)
- **Hardware peripherals** (gyro, barometer, OSD chip, flash memory)
- **Pin mappings** (which MCU pins connect to which hardware)
- **Feature availability** (enabled/disabled functionality)
- **Default configuration** (serial ports, sensors, blackbox)

## Target Directory Structure

All targets live in `/src/main/target/` with one subdirectory per target:

```
src/main/target/
├── MATEKF722/
│   ├── CMakeLists.txt     # Build system integration (REQUIRED)
│   ├── target.h           # Hardware configuration (REQUIRED)
│   ├── target.c           # Timer definitions (optional)
│   └── config.c           # Default settings (optional)
```

## Target Files

### 1. CMakeLists.txt - Build System Integration (REQUIRED)

Tells CMake which MCU to build for:

```cmake
target_stm32f722xe(MATEKF722)
```

Format: `target_stm32<CHIP><SIZE>(<TARGET_NAME>)`

Common MCU variants:
- `target_stm32f405rg()` - F405, 1MB flash
- `target_stm32f411ce()` - F411, 512KB flash
- `target_stm32f722xe()` - F722, 512KB flash
- `target_stm32h743xi()` - H743, 2MB flash

### 2. target.h - Hardware Configuration (REQUIRED)

The heart of every target.

Key sections:
- Board identification
- SPI/I2C/UART peripheral configuration
- Sensor definitions (gyro, baro, mag)
- Flash memory / SD card setup
- OSD configuration
- ADC channels (voltage, current, RSSI)
- Default features
- PWM outputs (motors/servos)

Example structure:
```c
#pragma once

#define TARGET_BOARD_IDENTIFIER "MKF7"
#define USBD_PRODUCT_STRING     "MATEKF722"

// SPI buses
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// IMU
#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW180_DEG
#define MPU6500_CS_PIN          PC2
#define MPU6500_SPI_BUS         BUS_SPI1
```

### 3. target.c - Timer Definitions (OPTIONAL)

Defines which MCU timers control which pins for motors/servos/LEDs.

See `timer-dma-conflicts.md` for:
- How timer/DMA conflicts occur
- How to check a timer table against your MCU's DMA request mapping
- Timer configuration best practices

Example:
```c
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0), // Motor 1
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0), // Motor 2
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_LED, 0, 0),         // LED strip
};
const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
```

**TIM_USE values:**
- `TIM_USE_OUTPUT_AUTO` - Auto-assigned to motor or servo (recommended)
- `TIM_USE_MOTOR` - Force motor output
- `TIM_USE_SERVO` - Force servo output
- `TIM_USE_LED` - LED strip (WS2812)

### 4. config.c - Default Configuration (OPTIONAL)

Sets up default serial port configuration and features:

```c
void targetConfiguration(void)
{
    // Set UART1 to MSP at 115200
    serialConfigMutable()->portConfigs[
        findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)
    ].functionMask = FUNCTION_MSP;

    serialConfigMutable()->portConfigs[
        findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)
    ].msp_baudrateIndex = BAUD_115200;
}
```

## How Targets Are Built

1. **User selects target:** `cmake -DINAVTARGET=MATEKF722 ..`
2. **CMake finds target directory:** `src/main/target/MATEKF722/`
3. **CMake reads CMakeLists.txt:** Determines MCU type (F722), flash size (512KB)
4. **Preprocessor includes target.h:** All hardware #defines become active
5. **Build system compiles:**
   - Common code uses target.h defines
   - Conditionally includes drivers based on USE_* flags
   - Links with appropriate STM32 HAL libraries
6. **Linker creates binary:** Creates .hex/.bin files for flashing

## Flash Memory Constraints

STM32F4/F7 targets are **CRITICALLY FLASH-LIMITED**:
- F405/F411 with 512KB → Very tight (~500KB usable)
- F722 with 512KB → Tight, can fit most features
- F745/H743 with 1-2MB → Comfortable

### Flash Optimization Strategies

When flash is tight, disable unused features:

```c
#undef USE_SERIALRX_FPORT      // Saves ~2-3KB
#undef USE_VTX_SMARTAUDIO       // Saves ~2KB
#undef USE_TELEMETRY_IBUS       // Saves ~1KB
```

Or reduce sensor support:
```c
// Instead of USE_BARO_ALL, support only what's present
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280         // Only this model
```

See `common-issues.md` for real examples from git history.

## Common Target Patterns

### Multi-Variant Targets

Some hardware has multiple versions:

```c
#ifdef KAKUTEH7MINI
    #define USE_FLASHFS
    #define USE_FLASH_M25P16
#else
    #define USE_SDCARD
    #define SDCARD_CS_PIN   PA4
#endif
```

### Resource Sharing

Some pins are shared between peripherals:

```c
// UART3 and I2C2 share pins PB10/PB11
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define I2C_DEVICE_2_SHARES_UART3  // Important flag!
```

See `common-issues.md` for more resource conflict examples.

## Key Differences Between MCU Families

| Feature | STM32F4 | STM32F7 | STM32H7 |
|---------|---------|---------|---------|
| **Clock** | 100-168 MHz | 216 MHz | 480 MHz |
| **Flash** | 512KB-1MB | 512KB-1MB | 128KB-2MB |
| **RAM** | 128-192KB | 256KB | 1MB |
| **FPU** | 32-bit | 64-bit | 64-bit |
| **Status** | Mature, flash-limited | Current gen, balanced | High-end, plenty of resources |

## Target Naming Conventions

- **Manufacturer + Model:** `MATEKF722`, `AIRBOTF4`
- **Version suffixes:** `V2`, `V3`, `MINI`, `PRO`
- **Feature suffixes:** `AIO` (all-in-one), `WING` (fixed wing focused)

## Related Documentation

- **common-issues.md** - Catalog of target problems and fixes with git examples
- **creating-targets.md** - Step-by-step guide to create new target
- **troubleshooting-guide.md** - Debug target build and runtime issues
- **examples.md** - Detailed walkthrough of real fixes
- **timer-dma-conflicts.md** - Understanding and fixing timer/DMA issues

## Summary

A target is defined by:
1. **CMakeLists.txt** - MCU selection
2. **target.h** - Hardware configuration
3. **target.c** - Timer mappings (optional)
4. **config.c** - Default settings (optional)

The build system uses these files to create firmware tailored to specific hardware.

## Tools

- **STM32 CubeMX** - Pin and peripheral visualization, and the authoritative source for each MCU's DMA request mapping (see `timer-dma-conflicts.md`)
- **ST-Link Utility** - Flash size verification
