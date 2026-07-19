# Creating New INAV Targets

Step-by-step guide to create a new hardware target configuration.

## Prerequisites

- Hardware schematic or board documentation
- STM32 MCU datasheet
- Knowledge of which sensors/chips are present
- Working Linux/macOS dev environment

## Quick Start: Copy Similar Target

**Best approach:** Start from a similar working target

```bash
cd inav/src/main/target/
cp -r SIMILAR_TARGET/ NEWTARGET/
```

**Choose reference target based on:**
- Same MCU family (F4/F7/H7)
- Similar flash size
- Similar peripheral set (OSD, flash, gyro type)

## Step 1: Create Target Directory

```bash
mkdir -p inav/src/main/target/NEWTARGETNAME
cd inav/src/main/target/NEWTARGETNAME
```

## Step 2: Create CMakeLists.txt

**File:** `CMakeLists.txt`

```cmake
target_stm32f722xe(NEWTARGETNAME)
```

**MCU Variants:**
- `f405rg` - STM32F405, 1MB
- `f411ce` - STM32F411, 512KB
- `f722xe` - STM32F722, 512KB
- `f745vg` - STM32F745, 1MB
- `h743xi` - STM32H743, 2MB

Verify MCU marking on chip for exact variant.

## Step 3: Create target.h

**File:** `target.h`

Start with this template:

```c
#pragma once

#define TARGET_BOARD_IDENTIFIER "XXXX"  // 4-char unique ID
#define USBD_PRODUCT_STRING     "Board Name"

#define LED0                    PXX  // Status LED pin

#define BEEPER                  PXX
#define BEEPER_INVERTED              // If beeper is active-low

// Copy remaining sections from reference target
// Modify pins/buses to match your hardware
```

**Sections to configure (in order):**
1. Board ID and LEDs
2. SPI buses (gyro, OSD, flash)
3. Gyro/accelerometer
4. I2C buses (baro, mag)
5. Barometer
6. Magnetometer
7. Flash memory or SD card
8. OSD (MAX7456)
9. UARTs
10. ADC (voltage, current, RSSI)
11. CAN bus (if present)
12. Default features
13. PWM outputs

See `common-issues.md` for common mistakes to avoid.

## Step 4: Pin Mapping from Schematic

**For each peripheral:**

1. Find chip on schematic
2. Trace connections to STM32
3. Note STM32 pin (e.g., PA5, PB12)
4. Verify pin supports required function (check datasheet)

**Example - Gyro on SPI1:**

Schematic shows:
- MPU6000 CS → PA4
- SCK → PA5
- MISO → PA6
- MOSI → PA7

target.h:
```c
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW0_DEG  // Adjust based on mounting
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1
```

See `reading-schematics.md` for a detailed workflow when working from raw KiCad/Altium schematic files rather than a finished pinout table.

## Step 5: Create target.c (Optional)

**File:** `target.c`

Define timer outputs for motors/servos:

```c
#include "platform.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0), // M1
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0), // M2
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0), // M3
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0), // M4
    DEF_TIM(TIM2, CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0), // M5
    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_OUTPUT_AUTO, 0, 0), // M6
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_LED, 0, 0),         // LED
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
```

**Check for DMA conflicts** — see `timer-dma-conflicts.md` for details.

## Step 6: Create config.c (Optional)

**File:** `config.c`

Set default serial port functions:

```c
#include "platform.h"
#include "fc/config.h"
#include "io/serial.h"

void targetConfiguration(void)
{
    // UART1 for MSP
    serialConfigMutable()->portConfigs[
        findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)
    ].functionMask = FUNCTION_MSP;

    // UART2 for RX
    serialConfigMutable()->portConfigs[
        findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)
    ].functionMask = FUNCTION_RX_SERIAL;
}
```

## Step 7: Build Target

**Use inav-builder agent:**

```
Use inav-builder agent to build NEWTARGETNAME
```

**Fix any build errors:**
- Undefined pins → Check schematic
- Flash overflow → Remove unnecessary features (see `common-issues.md`)
- DMA conflicts → See `timer-dma-conflicts.md`

## Step 8: Test on Hardware

1. **Flash firmware** to board
2. **Connect USB** - should enumerate as USB device
3. **Open INAV Configurator**
4. **Check sensors tab** - gyro, baro, mag should show data
5. **Test motors tab** - all outputs should respond
6. **Check ports tab** - UARTs should be detected

## Step 9: Iterate and Fix

Common issues:
- Gyro not detected → Check SPI pins, CS pin, alignment
- Barometer not working → Check I2C pins, bus assignment
- Motors not responding → Check timer definitions, DMA conflicts
- Flash not detected → Check flash chip model, SPI pins

See `troubleshooting-guide.md` for systematic debugging.

## Checklist

- [ ] CMakeLists.txt created with correct MCU
- [ ] target.h defines all hardware correctly
- [ ] Pins verified against schematic
- [ ] SPI/I2C buses configured
- [ ] All sensors defined
- [ ] Flash/SD card configured
- [ ] UARTs defined
- [ ] ADC channels set
- [ ] CANbus defined, if present
- [ ] target.c created (if needed)
- [ ] DMA conflicts checked against reference manual DMA request mapping
- [ ] config.c created (if needed)
- [ ] Target builds without errors
- [ ] Tested on actual hardware
- [ ] All sensors working
- [ ] All motor outputs working
- [ ] USB connection stable

## Tips

- **Start simple** - Get basic build working first, add features later
- **Compare with similar targets** - See how others solved similar problems
- **Test incrementally** - Add one feature, build, test, repeat
- **Document as you go** - Add comments explaining hardware-specific choices
- **Ask for help** - Search INAV GitHub issues for similar boards

## Related Documentation

- **overview.md** - Target system architecture
- **reading-schematics.md** - How to analyze manufacturer KiCad/Altium schematics and build a verified pin map before writing target.h
- **common-issues.md** - Avoid these mistakes
- **timer-dma-conflicts.md** - Resolve DMA conflicts
- **troubleshooting-guide.md** - Debug issues systematically
