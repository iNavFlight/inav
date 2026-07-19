# Target Troubleshooting Guide

Systematic approach to debugging target build and runtime issues.

## Build-Time Issues

### Flash Overflow

**Error:** `region 'FLASH' overflowed by XXXXX bytes`

**Diagnosis:**
```bash
# Check target flash size
grep "target_stm32" CMakeLists.txt
# f411ce, f722xe = 512KB (tight)
# f405rg, f745vg = 1MB (comfortable)
```

**Solutions (in order):**
1. Disable unused radio protocols (`#undef USE_SERIALRX_FPORT`)
2. Disable unused VTX protocols (`#undef USE_VTX_SMARTAUDIO`)
3. Remove sensor auto-detection (`USE_BARO_BMP280` instead of `USE_BARO_ALL`)
4. Add `SKIP_CLI_COMMAND_HELP` (saves 3KB)

See `common-issues.md` → "Flash Size Issues" for details.

### Pin Already Defined

**Error:** `redefinition of 'PIN'`

**Cause:** Pin used by multiple peripherals without conflict handling

**Solution:** Add resource sharing flag or use different pin

```c
#define I2C_DEVICE_2_SHARES_UART3  // Add this if I2C2 and UART3 share PB10/PB11
```

### Timer Channel Silently Not Working

**Symptom:** Motor/servo output defined with `TIM_USE_ANY` builds fine but never gets assigned as a motor or servo

**Cause:** `TIM_USE_ANY` is still a declared enum value (`= 0` in `drivers/timer.h`), so it compiles — but a value of 0 sets no usage bits, making the channel a no-op rather than an error

**Fix:** Use `TIM_USE_OUTPUT_AUTO` instead — `TIM_USE_ANY` is a legacy leftover, not a real "any" wildcard

## Runtime Issues

### Gyro Not Detected

**Symptoms:** Configurator shows "NO GYRO", won't arm

**Diagnosis steps:**
1. **Check target.h SPI configuration**
   ```c
   #define USE_SPI_DEVICE_1
   #define SPI1_SCK_PIN            PA5  // Verify against schematic
   #define SPI1_MISO_PIN           PA6
   #define SPI1_MOSI_PIN           PA7
   ```

2. **Check gyro chip select pin**
   ```c
   #define MPU6000_CS_PIN          PA4  // Must match hardware
   ```

3. **Check gyro is enabled**
   ```c
   #define USE_IMU_MPU6000  // Must have this
   ```

4. **Test with multimeter:**
   - CS pin should be HIGH when idle
   - SCK should toggle during boot
   - 3.3V power present on gyro

**Common fixes:**
- Wrong CS pin
- Wrong SPI bus (BUS_SPI1 vs BUS_SPI2)
- Gyro model mismatch (MPU6000 vs MPU6500 vs ICM42605)
- Missing `USE_IMU_xxx` define

### Barometer Not Working

**Symptoms:** Altitude shows 0m, no variometer

**Diagnosis:**
1. **Check I2C configuration**
   ```c
   #define USE_I2C_DEVICE_1
   #define I2C1_SCL                PB6  // Verify pins
   #define I2C1_SDA                PB7
   ```

2. **Check baro bus assignment**
   ```c
   #define BARO_I2C_BUS            BUS_I2C1  // Must match
   ```

3. **Check baro models enabled**
   ```c
   #define USE_BARO_BMP280  // Or whichever chip is present
   ```

**Common fixes:**
- Wrong I2C bus (I2C1 vs I2C2 vs I2C3)
- Bus not accessible on hardware (check schematic)
- Wrong I2C address (use sensor-specific define like `DPS310_I2C_ADDR`)

See `common-issues.md` → "I2C Bus Configuration"

### Motors Not Working

**Symptoms:** Some or all motors don't respond to throttle

**Diagnosis:**
1. **Check for DMA conflicts**
   ```bash
   # In CLI
   status
   # Look for "DMA conflict" warnings
   ```

2. **Cross-check your timer table against the MCU's DMA request mapping**
   - See `timer-dma-conflicts.md` for the methodology

3. **Verify timer pins match schematic**

**Common fixes:**
- DMA conflicts (see `timer-dma-conflicts.md`)
- Wrong TIM_USE flag (use `TIM_USE_OUTPUT_AUTO`)
- Pin doesn't support that timer (check datasheet AF table)

### Blackbox Not Working

**Symptoms:** Can't enable blackbox in configurator

**Diagnosis:**
1. **Check flash chip defined**
   ```c
   #define USE_FLASHFS
   #define USE_FLASH_M25P16  // Or W25Q128, etc.
   ```

2. **Check flash pins**
   ```c
   #define M25P16_CS_PIN           PA4
   #define M25P16_SPI_BUS          BUS_SPI1
   ```

3. **Check default enable**
   ```c
   #define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
   ```

**Common fixes:**
- Wrong flash chip model
- Flash chip on wrong SPI bus
- Missing default enable flag

### USB Not Connecting

**Symptoms:** Board doesn't show up as USB device

**Diagnosis:**
1. **Check VCP enabled**
   ```c
   #define USE_VCP
   ```

2. **Check VBUS sensing**
   ```c
   #define VBUS_SENSING_PIN        PB12  // Must match hardware
   #define VBUS_SENSING_ENABLED
   ```

3. **Test with different USB cable/port**

**Common fixes:**
- Missing `USE_VCP`
- Wrong VBUS sensing pin
- Faulty USB connector

## Systematic Debugging Process

### Step 1: Verify Basic Build

```bash
# Use inav-builder agent
"Build TARGETNAME target"
# Should complete without errors
```

### Step 2: Compare with Working Target

```bash
# Find similar working target
grep -r "USE_IMU_MPU6000" inav/src/main/target/*/target.h

# Compare configurations
diff inav/src/main/target/WORKING/target.h inav/src/main/target/BROKEN/target.h
```

### Step 3: Check Git History

```bash
# Look for similar issues
git log --all --grep="target name" --oneline

# Look for similar sensor fixes
git log --all --grep="MPU6000" -- "src/main/target/" --oneline
```

### Step 4: Isolate Problem

**Method:** Comment out sections of target.h until build works or sensor appears

```c
// Temporarily disable to isolate issue
// #define USE_MAG
// #define USE_BARO
// #define USE_OSD
```

Then re-enable one at a time to find culprit.

### Step 5: Test on Hardware

**Minimal test procedure:**
1. Flash firmware
2. Connect USB
3. Open configurator
4. Check sensors tab
5. Check motors tab
6. Check CLI status

## Common Error Messages

| Error | Meaning | Fix |
|-------|---------|-----|
| `region 'FLASH' overflowed` | Too many features | Remove features, see flash optimization |
| `undefined reference to 'XXX'` | Missing driver | Add `USE_XXX` define |
| `redefinition of 'PIN'` | Pin conflict | Check for duplicate pin assignments |
| `NO GYRO` in configurator | Gyro not detected | Check SPI pins, CS pin, chip model |
| `DMA conflict` in CLI | Timer DMA conflict | See `timer-dma-conflicts.md` |

## Debugging Tools

1. **Git history search** - Find similar fixes
2. **Diff tool** - Compare with working targets
3. **Multimeter** - Verify hardware connections
4. **Logic analyzer** - Trace SPI/I2C communication
5. **STM32 CubeMX** - Visualize pin assignments and DMA request mapping

## When to Ask for Help

If after systematic debugging you still have issues:

1. **Document what you tried**
2. **Collect logs** (build output, CLI status)
3. **Provide schematic** (if available)
4. **Post on INAV Discord/GitHub** with:
   - Target name
   - MCU variant
   - Problem description
   - What you've tried
   - Build output or CLI logs

## Related Documentation

- **common-issues.md** - Catalog of known problems and fixes
- **creating-targets.md** - Step-by-step target creation
- **timer-dma-conflicts.md** - DMA conflict resolution
- **overview.md** - Target system architecture
