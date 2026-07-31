# Real Target Fix Examples from Git History

Detailed walkthroughs of actual target fixes, showing what was wrong and how it was fixed.

## Example 1: Resource Conflict (UART/I2C Pin Sharing)

**Commit:** 3c2c62b2b - NEXUSX: fix UART3/I2C2 resource conflict

**Problem:** I2C sensors (barometer, magnetometer) not working when UART3 enabled

**Root Cause:** UART3 and I2C2 share pins PB10/PB11 on STM32F4. Firmware didn't know about conflict and tried to use both simultaneously.

**Before:**
```c
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define DEFAULT_I2C BUS_I2C2
// Missing conflict declaration!
```

**After:**
```c
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define I2C_DEVICE_2_SHARES_UART3  // ← Added this line
#define DEFAULT_I2C BUS_I2C2
```

**Lesson:** Always declare resource sharing when pins overlap. Check STM32 datasheet for pin alternate functions.

**Detection:** I2C devices work when UART3 disabled, fail when enabled.

---

## Example 2: Flash Overflow - Disable Unused Features

**Commit:** a54adea74 - Disable Smartaudio for target OMNIBUS, due to flash size

**Problem:** Build failed with flash overflow on 512KB F4 target

**Error Message:**
```
region `FLASH' overflowed by 2847 bytes
```

**Root Cause:** OMNIBUS is flash-constrained F4 board. SmartAudio VTX support added ~2KB, pushing over limit.

**Fix:**
```c
#define SPI1_MOSI_PIN           PA7

#undef USE_VTX_SMARTAUDIO       // Disabled due to flash size

#define USE_EXTI
```

**Result:** Build succeeds, firmware fits in flash

**Lesson:** F4 512KB targets require aggressive feature pruning. Disable features not present on hardware.

**Alternative Fixes (in order of preference):**
1. `#undef` unused radio protocols (FPORT, IBUS, etc.)
2. `#undef` unused VTX protocols (TRAMP, SmartAudio)
3. Use specific sensor defines instead of `USE_xxx_ALL`
4. Add `SKIP_CLI_COMMAND_HELP` (last resort)

---

## Example 3: Wrong I2C Bus Assignment

**Commit:** ad8d8c2ba - Fix NEXUSX magnetometer to use accessible I2C2 bus

**Problem:** Magnetometer not detected despite being physically present

**Root Cause:** Target configured mag on I2C3, but I2C3 pins not accessible on NEXUSX hardware

**Before:**
```c
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C3  // ← Wrong bus!
```

**After:**
```c
#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C  // Uses BUS_I2C2
#define USE_MAG_ALL              // Auto-detect mag model
```

**Why This Failed:**
- I2C3 pins exist on STM32 chip
- But PCB doesn't route I2C3 to external connector
- Magnetometer physically connected to I2C2

**Detection:** Magnetometer shows "NOT DETECTED" in configurator, but hardware is present.

**Lesson:** Check hardware schematic, don't assume. Just because STM32 has I2C3 doesn't mean board uses it.

---

## Example 4: Wrong Sensor I2C Address

**Commit:** 8533e08f1 - Fixes for Brahma targets: F405 and F722

**Problem:** DPS310 barometer not detected

**Root Cause:** Using generic `BARO_I2C_ADDR` instead of sensor-specific address define

**Before:**
```c
#define USE_BARO
#define BARO_I2C_BUS                    DEFAULT_I2C_BUS
#define BARO_I2C_ADDR                   0x77  // ← Wrong approach
#define USE_BARO_DPS310
```

**After:**
```c
#define USE_BARO
#define BARO_I2C_BUS                    DEFAULT_I2C_BUS
#define DPS310_I2C_ADDR                 0x77  // ← Sensor-specific
#define USE_BARO_DPS310
```

**Why This Matters:**
- Generic `BARO_I2C_ADDR` doesn't work
- Each sensor driver looks for its own address define
- Must use `<SENSOR>_I2C_ADDR` format

**Also Fixed in Same Commit:**
```diff
-#define ICM42605_CS_PIN         SPI1_NSS_PIN  // ← Wrong pin for this sensor
+#define ICM42605_CS_PIN         PC13          // ← Actual pin
```

`SPI1_NSS_PIN` was a validly-defined pin (`PA4`) — just not the ICM42605's real CS trace. Referencing `SPIx_NSS_PIN` for a `*_CS_PIN` is a common, valid INAV pattern when it genuinely is that sensor's CS pin; the bug here was the wrong pin assignment, not the macro itself.

**Lesson:** Use sensor-specific address defines, and verify each device's CS pin against the schematic individually.

---

## Example 5: Timer Configuration Errors

**Commit:** eb6db70bf - botwing f722 - fix timers, blackbox

**Problem:** Motors not responding, blackbox not working

**Root Cause 1 - Wrong Timer Flags:**

**Before:**
```c
DEF_TIM(TIM3, CH1, PB4,  TIM_USE_ANY, 0, 0),  // ← Deprecated flag
DEF_TIM(TIM3, CH2, PB5,  TIM_USE_ANY, 0, 0),
```

**After:**
```c
DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0),  // ← Correct
DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0),
```

**Root Cause 2 - Wrong Flash Chip:**

**Before:**
```c
#define USE_FLASH_W25Q128FV
#define W25Q128FV_CS_PIN                PA4
#define W25Q128FV_SPI_BUS               BUS_SPI1
```

**After:**
```c
#define USE_FLASH_M25P16
#define M25P16_CS_PIN                   SPI1_NSS_PIN
#define M25P16_SPI_BUS                  BUS_SPI1
```

**Also Fixed:**
```diff
 #define USE_ADC
+#define ADC_INSTANCE                ADC1
+#define ADC1_DMA_STREAM             DMA2_Stream0
```

**Lessons:**
- Use `TIM_USE_OUTPUT_AUTO`, not `TIM_USE_ANY`
- Verify flash chip model against hardware
- Some targets need explicit ADC instance defines

---

## Example 6: Linker Script Flash Section

**Commit:** 033dcda47 - Fix FLASH section overflow on flash_split targets

**Problem:** F7 split-flash targets failing to link

**Root Cause:** Data section placed in wrong flash region

**Before:**
```ld
  . = ALIGN(4);
  _edata = .;
} >RAM AT> FLASH
```

**After:**
```ld
  . = ALIGN(4);
  _edata = .;
} >RAM AT> FLASH1  // ← Changed to FLASH1
```

**Context:** F7 targets with >512KB flash use split layout:
- FLASH (0x08000000) - First bank
- FLASH1 (0x08080000) - Second bank

Some sections must explicitly target FLASH1 to avoid overflow.

**Lesson:** Build system issues can look like target problems. Check linker scripts on split-flash targets.

---

## Example 7: config.c PINIO Configuration

**Commit:** eb6db70bf - botwing f722 (partial)

**Problem:** Only one PINIO box available, hardware has two

**Before:**
```c
void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    // Missing second PINIO!
}
```

**After:**
```c
void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;  // Added
}
```

**What is PINIO:** Programmable I/O pins that can be controlled via modes (USER1, USER2). Common uses:
- VTX power control
- Camera switching
- LED control
- Buzzer

**Lesson:** Check how many PINIO pins hardware has, configure all in config.c

---

## Summary: Common Patterns

| Problem Type | Symptoms | First Check | Typical Fix |
|--------------|----------|-------------|--------------|
| Resource conflict | Peripheral fails when another enabled | Pin assignments | Add `XXX_SHARES_YYY` flag |
| Flash overflow | Build fails | Flash size vs features | `#undef` unused features |
| Wrong bus | Sensor not detected | Schematic vs target.h | Change bus assignment |
| Wrong address | Specific sensor fails | Generic vs specific define | Use sensor-specific address |
| Timer issues | Motors don't work | Timer flags | Use `TIM_USE_OUTPUT_AUTO` |
| Wrong chip | Flash/sensor not working | Hardware vs defines | Match chip model exactly |

## How to Learn from Git History

```bash
# Find target-related fixes
git log --oneline -- "src/main/target/"

# Search for specific issues
git log --all --grep="flash" -- "src/main/target/"
git log --all --grep="fix" -- "src/main/target/"

# Examine a specific fix
git show <commit-hash>

# Find when a line changed
git blame src/main/target/TARGETNAME/target.h
```

## Related Documentation

- **common-issues.md** - Catalog of all known target problems
- **troubleshooting-guide.md** - Systematic debugging approach
- **creating-targets.md** - Avoid these issues from the start
