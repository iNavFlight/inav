# AT32F43x Flash Recovery - Flash Protection / Lock Recovery

## Problem Summary

Certain flash operations on AT32F43x (e.g. writing the User System Data / option-byte area early in boot, before the flash controller and clock system are fully initialized) can leave the flash controller in a protected/locked state that prevents DFU flashing.

**Symptoms:**
- `dfu-util: ERASE_PAGE not correctly executed`
- `dfu-util: MASS_ERASE not correctly executed`
- Flash status shows `dfuERROR, status = 11`
- Cannot flash any firmware via DFU

Once flash protection triggers, DFU cannot recover it — a hardware debugger (ST-Link/J-Link or equivalent) is required.

## Recovery Methods

### Method 1: STM32CubeProgrammer (Recommended - Easiest)

**Note:** AT32F43x chips are compatible with STM32 programming tools. Despite being from different manufacturers (Artery vs STMicroelectronics), they use the same ARM Cortex-M4 core and DFU protocol.

**Download:**
https://www.st.com/en/development-tools/stm32cubeprog.html

**Steps:**

1. **Install STM32CubeProgrammer**
   - Download for your platform (Windows/Linux/macOS)
   - Run installer
   - Launch STM32CubeProgrammer GUI

2. **Connect to Flight Controller**
   - Put FC into DFU mode (hold BOOT button while plugging USB)
   - In STM32CubeProgrammer:
     - Select "USB" connection type
     - Click "Refresh" to detect device
     - Device should show as `USB1` with VID: `2E3C`, PID: `DF11`
     - Click "Connect"

3. **Clear Flash Protection**
   - If connection fails with protection error:
     - Go to "OB" (Option Bytes) tab
     - Look for Read Protection (RDP) setting
     - If RDP is set to Level 1, change to Level 0
     - Click "Apply"
     - **Warning:** This will erase all flash content

   - For write protection:
     - In "OB" tab, check "Write Protection" settings
     - Disable all write protection bits
     - Click "Apply"

4. **Mass Erase Flash**
   - Go to "Erasing & Programming" tab
   - Select "Full chip erase"
   - Click "Start"
   - Wait for completion

5. **Flash New Firmware**
   - In "Erasing & Programming" tab:
     - Select your `.hex` file
     - Start address: `0x08000000`
     - Check "Verify programming"
     - Click "Start Programming"
   - Or use `.bin` file with explicit address `0x08000000`

6. **Disconnect and Test**
   - Click "Disconnect"
   - Unplug FC
   - Replug without holding BOOT button
   - FC should boot normally

### Method 2: ST-Link Hardware Debugger

**Hardware Required:**
- ST-Link V2 or compatible (e.g., ST-Link V3, J-Link)
- 4-pin SWD connection: SWDIO, SWCLK, GND, 3.3V

**Connection:**

```
ST-Link          AT32F435 (Flight Controller)
--------         ----------------------------
SWDIO       -->  SWDIO (SWD Data)
SWCLK       -->  SWCLK (SWD Clock)
GND         -->  GND
3.3V        -->  3.3V (optional - can power from USB)
```

**Check your flight controller pinout** - SWD pins are usually:
- Labeled on PCB silkscreen
- In a 4-pin or 6-pin debug header
- Sometimes shared with unused UART pads

**Using STM32CubeProgrammer with ST-Link:**

1. Connect ST-Link to FC via SWD
2. Power FC via USB
3. In STM32CubeProgrammer:
   - Select "ST-LINK" connection type
   - Click "Connect"
4. Follow same procedure as Method 1 (steps 3-6)

**Using OpenOCD (Command Line):**

```bash
# Install OpenOCD
sudo apt install openocd    # Ubuntu/Debian
brew install openocd        # macOS

# Create openocd.cfg
cat > openocd.cfg << 'EOF'
source [find interface/stlink.cfg]
source [find target/at32f435_437.cfg]

init
reset halt
stm32f4x unlock 0
reset halt
flash erase_sector 0 0 last
reset
shutdown
EOF

# Run recovery
openocd -f openocd.cfg

# Flash new firmware
openocd -f openocd.cfg -c "program inav_9.0.0_BLUEBERRYF435WING.hex verify reset exit"
```

**Note:** You may need to create `at32f435_437.cfg` if OpenOCD doesn't have it:

```tcl
# target/at32f435_437.cfg
source [find target/stm32f4x.cfg]

# AT32F43x is compatible with STM32F4 configuration
```

### Method 3: Commercial Programmers

If ST-Link is unavailable:

**J-Link (Segger):**
- More expensive but very robust
- Excellent vendor support
- Use J-Link Commander or J-Flash software

**Black Magic Probe:**
- Open source debugger/programmer
- Uses GDB protocol
- Good alternative to ST-Link

### Method 4: Serial Bootloader (if SWD unavailable)

AT32F43x has a ROM bootloader accessible via UART (if not locked out).

**Requirements:**
- UART connection (TX, RX, GND)
- Boot0 pin pulled HIGH at power-on

**Tools:**
- **stm32flash** (Linux command-line)
- **Flash Loader Demonstrator** (Windows GUI from ST)

**Steps:**

```bash
# Install stm32flash
sudo apt install stm32flash

# Connect FC with BOOT0 = HIGH
# Identify serial port
ls /dev/ttyUSB* /dev/ttyACM*

# Unlock and erase
stm32flash -k /dev/ttyUSB0
stm32flash -o /dev/ttyUSB0

# Flash firmware
stm32flash -w inav_9.0.0_BLUEBERRYF435WING.bin -v -g 0x08000000 /dev/ttyUSB0
```

**Limitations:**
- Some FC boards don't expose BOOT0 pin
- May not work if bootloader is disabled via option bytes

## Why This Happens

Writing AT32F43x's User System Data area (which includes flash-partitioning option bytes like EOPB0) during early boot, before the flash controller and clock system are fully initialized, is a well-documented way to trigger flash protection. Consult AT32's own application notes on the specific option byte you're changing before attempting a similar operation, and always have a hardware debugger on hand as a recovery path before writing to this area on real hardware.

## References

- AT32F435/437 Datasheet: https://www.arterychip.com/en/product/AT32F435.jsp
- AT32F435/437 Reference Manual: Flash Controller chapter
- STM32CubeProgrammer: https://www.st.com/en/development-tools/stm32cubeprog.html
