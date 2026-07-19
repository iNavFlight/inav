# Common Target Issues and Fixes

This document catalogs the most common problems found in INAV target configurations, based on analysis of git history. Each problem includes real examples with commit references.

## Table of Contents

1. [Resource Conflicts](#resource-conflicts)
2. [Flash Size Issues](#flash-size-issues)
3. [Pin Configuration Errors](#pin-configuration-errors)
4. [Timer Configuration](#timer-configuration)
5. [I2C Bus Configuration](#i2c-bus-configuration)
6. [Flash Memory Configuration](#flash-memory-configuration)
7. [Sensor Initialization](#sensor-initialization)
8. [Build System Issues](#build-system-issues)

---

## Resource Conflicts

### Problem: UART/I2C Pin Sharing Not Declared

**Symptom:** I2C device not working when UART is enabled, or vice versa

**Cause:** STM32 pins can be used for multiple functions, but the firmware needs to know about conflicts

**Detection:**
- Check if UART TX/RX pins match I2C SCL/SDA pins
- Common conflict: UART3 (PB10/PB11) and I2C2 (PB10/PB11)

**Fix:** Add resource sharing declaration

```c
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define I2C_DEVICE_2_SHARES_UART3  // <-- Add this line
```

**Real Examples:**

**Commit 3c2c62b2b** - NEXUSX: fix UART3/I2C2 resource conflict
```diff
 #define USE_I2C_DEVICE_2
 #define I2C2_SCL                PB10
 #define I2C2_SDA                PB11
+#define I2C_DEVICE_2_SHARES_UART3
```

**Commit 95a979574** - VANTAC_RF007: Fix UART3/I2C2 resource conflict on port C
```diff
 #define I2C2_SCL                PB10
 #define I2C2_SDA                PB11
 #define EXTERNAL_I2C BUS_I2C2
+#define I2C_DEVICE_2_SHARES_UART3
```

**Prevention:**
- When adding I2C_DEVICE_2 on PB10/PB11, always add the sharing flag
- Check STM32 datasheet for alternate function mappings
- Look for similar targets as reference

---

## Flash Size Issues

### Problem: Flash Overflow on Small Targets

**Symptom:**
```
region `FLASH' overflowed by XXXXX bytes
```

**Cause:** Too many features enabled for available flash memory

**Detection:**
- Build fails with overflow error
- Target uses F405/F411 with 512KB flash
- Many USE_* features defined

**Fixes (in order of preference):**

#### 1. Disable Unused Features

Most effective approach - disable features not needed for this hardware:

```c
// Disable unneeded radio protocols
#undef USE_SERIALRX_FPORT      // Saves ~2-3KB
#undef USE_SERIALRX_IBUS       // Saves ~1-2KB

// Disable unneeded VTX protocols
#undef USE_VTX_SMARTAUDIO      // Saves ~2KB
#undef USE_VTX_TRAMP           // Saves ~2KB

// Disable unneeded telemetry
#undef USE_TELEMETRY_IBUS      // Saves ~1KB
#undef USE_TELEMETRY_JETIEXBUS // Saves ~1KB
```

**Real Example:**

**Commit a54adea74** - Disable Smartaudio for target OMNIBUS, due to flash size
```diff
 #define SPI1_MOSI_PIN           PA7

+#undef USE_VTX_SMARTAUDIO       // Disabled due to flash size
+
 #define USE_EXTI
```

#### 2. Reduce Sensor Support

Instead of `USE_BARO_ALL`, support only sensors actually present on hardware:

```c
// Before (supports all, uses more flash)
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_ALL

// After (supports only what's present)
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280         // Only what this board has
```

**Real Example:**

**Commit a497c58d9** - remove baro all again due to high flash use
```
Changed from USE_BARO_ALL to specific barometer defines
to save flash space on constrained target
```

#### 3. Disable CLI Help Text

Last resort - removes in-CLI help text:

```c
#define SKIP_CLI_COMMAND_HELP   // Saves ~3KB
```

**Real Example:**

**Commit 447cc8f89** - Disable CLI help -> saves 3kB of flash
```
Added SKIP_CLI_COMMAND_HELP to save 3KB on flash-limited target
```

#### 4. Global Flash Optimizations

These are already applied globally but documented here:
- Compiler optimization level (-Os for size)
- Link-time optimization (LTO)
- Dead code elimination
- Flash partitioning for split targets

**Real Example:**

**Commit 0551f8743** - Add flash partitioning support (#5474)
```
Implemented flash partitioning to better utilize available space
on F7 targets with split flash layout
```

**Prevention:**
- Start with minimal feature set
- Add features incrementally
- Test build after each addition
- Compare with similar targets of same flash size
- Use F7/H7 targets for feature-rich configurations

---

## Pin Configuration Errors

### Problem: Incorrect I2C Bus Assignment

**Symptom:** Sensor not detected at runtime despite being physically present

**Cause:** I2C bus configured on pins that don't exist or aren't accessible on hardware

**Detection:**
- Sensor fails to initialize
- I2C bus reads return 0xFF or timeout
- Check hardware schematic vs target.h pins

**Fix:** Change I2C bus to match actual hardware

**Real Example:**

**Commit ad8d8c2ba** - Fix NEXUSX magnetometer to use accessible I2C2 bus
```diff
 #define USE_MAG
-#define MAG_I2C_BUS             BUS_I2C3
+#define MAG_I2C_BUS             DEFAULT_I2C
+#define USE_MAG_ALL
```

**Explanation:** I2C3 was not physically accessible on NEXUSX hardware. Changed to DEFAULT_I2C (which is BUS_I2C2).

### Problem: Incorrect Sensor I2C Address

**Symptom:** Specific barometer model not detected

**Cause:** Using generic `BARO_I2C_ADDR` instead of sensor-specific define

**Detection:**
- Barometer detection fails
- Wrong I2C address used for sensor

**Fix:** Use sensor-specific address define

**Real Example:**

**Commit 8533e08f1** - Fixes for Brahma targets: F405 and F722
```diff
 #define USE_BARO
 #define BARO_I2C_BUS                    DEFAULT_I2C_BUS
-#define BARO_I2C_ADDR                   0x77
+#define DPS310_I2C_ADDR                 0x77
 #define USE_BARO_DPS310
```

**Explanation:** Generic `BARO_I2C_ADDR` doesn't work. Each sensor needs its specific address define like `DPS310_I2C_ADDR`.

### Problem: Incorrect Chip Select Pin

**Symptom:** SPI sensor not working

**Cause:** CS pin define points to the wrong physical pin for that sensor

**Detection:**
- SPI device not responding
- CS pin may be shared or wrong

**Fix:** Verify the CS pin against the schematic for that specific sensor

**Real Example:**

**Commit 8533e08f1** - Fixes for Brahma targets (ICM42605 CS pin)
```diff
 #define USE_IMU_ICM42605
 #define IMU_ICM42605_ALIGN      CW0_DEG
 #define ICM42605_SPI_BUS        BUS_SPI1
-#define ICM42605_CS_PIN         SPI1_NSS_PIN
+#define ICM42605_CS_PIN         PC13
```

**Explanation:** `SPI1_NSS_PIN` was a validly-defined pin (`PA4`) — just the wrong one for this sensor; the ICM42605's actual CS trace was `PC13`. Referencing `SPIx_NSS_PIN` for a `*_CS_PIN` define is a common, valid INAV pattern when it's genuinely that sensor's CS trace (widely used across current targets) — the bug here was an incorrect pin assignment, not the use of the macro itself.

**Prevention:**
- Verify each device's CS pin against the schematic individually — don't assume the bus's hardware-NSS pin is shared by every device on it
- Check hardware schematic for correct pins
- Verify with multimeter or logic analyzer if unsure

---

## Timer Configuration

### Problem: Incorrect Timer Usage Flags

**Symptom:** Motors/servos not working, or can't be configured in GUI

**Cause:** Using wrong `TIM_USE_` flags in timer definitions

**Detection:**
- Can't assign motor outputs in configurator
- Timers show as "unknown" type
- Motors don't respond to throttle

**Fix:** Use correct TIM_USE flags

**Real Example:**

**Commit eb6db70bf** - botwing f722 - fix timers, blackbox
```diff
 timerHardware_t timerHardware[] = {
-    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_ANY, 0, 0),
-    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_ANY, 0, 0),
-    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_ANY, 0, 0),
-    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_ANY, 0, 0),
+    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0),
+    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0),
+    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),
+    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),
```

**Correct Timer Flags:**
- `TIM_USE_OUTPUT_AUTO` - Auto-assign to motor or servo (recommended)
- `TIM_USE_MOTOR` - Force as motor output
- `TIM_USE_SERVO` - Force as servo output
- `TIM_USE_LED` - LED strip (WS2812)
- `TIM_USE_ANY` - **DEPRECATED** - Use OUTPUT_AUTO instead

**Prevention:**
- Always use `TIM_USE_OUTPUT_AUTO` for motor/servo pins
- Use `TIM_USE_LED` only for LED strip pins
- Don't use `TIM_USE_ANY` in new targets

---

## I2C Bus Configuration

### Problem: Missing USE_MAG_ALL for Auto-Detection

**Symptom:** Magnetometer works with specific define, but not on all boards

**Cause:** Only one magnetometer type defined, but hardware may have different model

**Detection:**
- Mag works on some boards, fails on others
- Target has generic mag support but specifies only one model

**Fix:** Use `USE_MAG_ALL` for auto-detection

**Real Example:**

**Commit ad8d8c2ba** - Fix NEXUSX magnetometer (added USE_MAG_ALL)
```diff
 #define USE_MAG
 #define MAG_I2C_BUS             DEFAULT_I2C
+#define USE_MAG_ALL
```

**Explanation:** Different board revisions may have different magnetometer chips. `USE_MAG_ALL` enables auto-detection.

**When to Use:**
- `USE_MAG_ALL` - Board may have various mag models (QMC5883, HMC5883, IST8310, etc.)
- `USE_BARO_ALL` - Board may have various baro models
- Specific defines - Only if you're certain of the exact model

---

## Flash Memory Configuration

### Problem: Wrong Flash Chip Type

**Symptom:** Blackbox not working, flash not detected

**Cause:** Target defines wrong flash chip model

**Detection:**
- Flash chip detection fails at boot
- Blackbox can't be enabled
- Check hardware vs target.h defines

**Fix:** Match flash chip to actual hardware

**Real Example:**

**Commit eb6db70bf** - botwing f722 - fix blackbox
```diff
 #define USE_FLASHFS
-#define USE_FLASH_W25Q128FV
-#define W25Q128FV_CS_PIN                PA4
-#define W25Q128FV_SPI_BUS               BUS_SPI1
+#define USE_FLASH_M25P16
+#define M25P16_CS_PIN                   SPI1_NSS_PIN
+#define M25P16_SPI_BUS                  BUS_SPI1
```

**Common Flash Chips:**
- M25P16 - 16Mbit (2MB)
- W25Q128 / W25Q128FV - 128Mbit (16MB)
- W25N01G - 1Gbit (128MB) NAND
- W25N02K - 2Gbit (256MB) NAND

**Prevention:**
- Check board schematic for flash chip part number
- Read chip markings with magnifying glass
- Test flash detection after defining

### Problem: Missing Blackbox Default Enable

**Symptom:** Blackbox available but not enabled by default

**Cause:** Forgot to set default enable flag

**Fix:** Add default enable define

```c
// For SPI flash
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// For SD card
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT
```

---

## Sensor Initialization

### Problem: Sensor on Wrong Bus

**Symptom:** Sensor present but not detected

**Cause:** Sensor configured on SPI bus when it should be I2C (or vice versa)

**Detection:**
- Check hardware: Is sensor connected via SPI or I2C?
- Look for I2C address on sensor PCB (indicates I2C)
- Trace PCB connections

**Fix:** Match bus type to hardware

---

## Build System Issues

### Problem: Wrong MCU Variant in CMakeLists.txt

**Symptom:**
- Build fails with linker errors
- Flash size mismatched
- Wrong peripheral definitions

**Cause:** CMakeLists.txt specifies wrong STM32 variant

**Detection:**
- Check actual MCU chip markings
- Compare flash size in error vs expected

**Fix:** Correct the MCU variant

```cmake
# Wrong - chip is actually F405, not F411
target_stm32f411ce(TARGETNAME)

# Correct
target_stm32f405rg(TARGETNAME)
```

**Common Variants:**
- `f405rg` - F405, 1MB flash
- `f411ce` - F411, 512KB flash
- `f722re` - F722, 512KB flash
- `f722xe` - F722, 512KB flash (alternate)
- `f745vg` - F745, 1MB flash
- `h743xi` - H743, 2MB flash

**Prevention:**
- Read MCU chip markings carefully
- Check manufacturer's datasheet
- Verify flash size with ST-Link utility

### Problem: Linker Script Flash Overflow

**Symptom:** `.text` section overflows FLASH region

**Cause:** Data being placed in wrong flash region on split-flash targets

**Fix:** Adjust linker script

**Real Example:**

**Commit 033dcda47** - Fix FLASH section overflow on flash_split targets
```diff
      . = ALIGN(4);
      _edata = .;        /* define a global symbol at data end */
-  } >RAM AT> FLASH
+  } >RAM AT> FLASH1
```

**Explanation:** On F7 targets with split flash layout, some sections must go in FLASH1 instead of FLASH.

---

## Summary Table

| Problem | Symptom | Quick Fix | Git Example |
|---------|---------|-----------|-------------|
| UART/I2C conflict | Device not working | Add `I2C_DEVICE_X_SHARES_UARTX` | 3c2c62b2b, 95a979574 |
| Flash overflow | Build fails | Disable unused features with `#undef` | a54adea74, 447cc8f89 |
| Wrong I2C bus | Sensor not detected | Fix bus assignment | ad8d8c2ba |
| Wrong I2C address | Specific sensor fails | Use sensor-specific address | 8533e08f1 |
| Wrong CS pin | SPI device fails | Use actual GPIO pin | 8533e08f1 |
| Wrong timer flags | Motors not working | Use `TIM_USE_OUTPUT_AUTO` | eb6db70bf |
| Wrong flash chip | Blackbox not working | Match chip model to hardware | eb6db70bf |
| Missing auto-detect | Sensor works sometimes | Add `USE_xxx_ALL` | ad8d8c2ba |

---

## How to Investigate Target Issues

1. **Check git history** for similar targets or recent fixes:
   ```bash
   git log --oneline -- "src/main/target/TARGETNAME/"
   git log --grep="TARGETNAME" --oneline
   ```

2. **Compare with working targets** using same MCU:
   ```bash
   diff src/main/target/WORKING/target.h src/main/target/BROKEN/target.h
   ```

3. **Check hardware schematic** if available

4. **Search for error message** in git history:
   ```bash
   git log --all --grep="error text" --oneline
   ```

5. **Look for similar chips/sensors** in other targets:
   ```bash
   grep -r "USE_BARO_DPS310" src/main/target/
   ```

---

## Related Documentation

- **overview.md** - Target system architecture
- **creating-targets.md** - How to create new targets
- **troubleshooting-guide.md** - Systematic debugging approach
- **examples.md** - Detailed walkthrough of specific fixes
