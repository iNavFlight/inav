/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

#ifdef USE_MAX7456

#include "common/bitarray.h"
#include "common/printf.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/dma.h"
#include "drivers/io.h"
#include "drivers/light_led.h"
#include "drivers/nvic.h"
#include "drivers/time.h"

#include "max7456.h"

// VM0 bits
#define VIDEO_BUFFER_DISABLE        0x01
#define MAX7456_RESET               0x02
#define VERTICAL_SYNC_NEXT_VSYNC    0x04
#define OSD_ENABLE                  0x08

#define SYNC_MODE_AUTO              0x00
#define SYNC_MODE_INTERNAL          0x30
#define SYNC_MODE_EXTERNAL          0x20

#define VIDEO_MODE_PAL              0x40
#define VIDEO_MODE_NTSC             0x00
#define VIDEO_MODE_MASK             0x40
#define VIDEO_MODE_IS_PAL(val)      (((val) & VIDEO_MODE_MASK) == VIDEO_MODE_PAL)
#define VIDEO_MODE_IS_NTSC(val)     (((val) & VIDEO_MODE_MASK) == VIDEO_MODE_NTSC)

#define VIDEO_SIGNAL_DEBOUNCE_MS    100 // Time to wait for input to stabilize

// VM1 bits

// duty cycle is on_off
#define BLINK_DUTY_CYCLE_50_50 0x00
#define BLINK_DUTY_CYCLE_33_66 0x01
#define BLINK_DUTY_CYCLE_25_75 0x02
#define BLINK_DUTY_CYCLE_75_25 0x03

// blinking time
#define BLINK_TIME_0 0x00
#define BLINK_TIME_1 0x04
#define BLINK_TIME_2 0x08
#define BLINK_TIME_3 0x0C

// background mode brightness (percent)
#define BACKGROUND_BRIGHTNESS_0  (0x00 << 4)
#define BACKGROUND_BRIGHTNESS_7  (0x01 << 4)
#define BACKGROUND_BRIGHTNESS_14 (0x02 << 4)
#define BACKGROUND_BRIGHTNESS_21 (0x03 << 4)
#define BACKGROUND_BRIGHTNESS_28 (0x04 << 4)
#define BACKGROUND_BRIGHTNESS_35 (0x05 << 4)
#define BACKGROUND_BRIGHTNESS_42 (0x06 << 4)
#define BACKGROUND_BRIGHTNESS_49 (0x07 << 4)

#define BACKGROUND_MODE_GRAY    0x80

// STAT register bits

#define STAT_PAL      0x01
#define STAT_NTSC     0x02
#define STAT_LOS      0x04
#define STAT_NVR_BUSY 0x20

#define STAT_IS_PAL(val)  ((val) & STAT_PAL)
#define STAT_IS_NTSC(val) ((val) & STAT_NTSC)
#define STAT_IS_LOS(val)  ((val) & STAT_LOS)

#define VIN_IS_PAL(val)  (!STAT_IS_LOS(val) && STAT_IS_PAL(val))
#define VIN_IS_NTSC(val)  (!STAT_IS_LOS(val) && STAT_IS_NTSC(val))

// Kluege warning!
// There are occasions that NTSC is not detected even with !LOS (AB7456 specific?)
// When this happens, lower 3 bits of STAT register is read as zero.
// To cope with this case, this macro defines !LOS && !PAL as NTSC.
// Should be compatible with MAX7456 and non-problematic case.

#define VIN_IS_NTSC_alt(val)  (!STAT_IS_LOS(val) && !STAT_IS_PAL(val))

#define MAX7456_SIGNAL_CHECK_INTERVAL_MS 1000 // msec

// DMM special bits
#define DMM_8BIT_MODE (1 << 6)
#define DMM_BLINK (1 << 4)
#define DMM_INVERT_PIXEL_COLOR (1 << 3)
#define DMM_CLEAR_DISPLAY (1 << 2)
#define DMM_CLEAR_DISPLAY_VERT (DMM_CLEAR_DISPLAY | 1 << 1)
#define DMM_AUTOINCREMENT (1 << 0)

#define DMM_IS_8BIT_MODE(val) (val & DMM_8BIT_MODE)
#define DMM_CHAR_MODE_MASK (MAX7456_MODE_INVERT | MAX7456_MODE_BLINK | MAX7456_MODE_SOLID_BG)

#define DMAH_8_BIT_DMDI_IS_CHAR_ATTR (1 << 1)

// Special address for terminating incremental write
#define END_STRING 0xff

#define MAX7456ADD_READ         0x80
#define MAX7456ADD_VM0          0x00  //0b0011100// 00 // 00             ,0011100
#define MAX7456ADD_VM1          0x01
#define MAX7456ADD_HOS          0x02
#define MAX7456ADD_VOS          0x03
#define MAX7456ADD_DMM          0x04
#define MAX7456ADD_DMAH         0x05
#define MAX7456ADD_DMAL         0x06
#define MAX7456ADD_DMDI         0x07
#define MAX7456ADD_CMM          0x08
#define MAX7456ADD_CMAH         0x09
#define MAX7456ADD_CMAL         0x0a
#define MAX7456ADD_CMDI         0x0b
#define MAX7456ADD_OSDM         0x0c
#define MAX7456ADD_RB0          0x10
#define MAX7456ADD_RB1          0x11
#define MAX7456ADD_RB2          0x12
#define MAX7456ADD_RB3          0x13
#define MAX7456ADD_RB4          0x14
#define MAX7456ADD_RB5          0x15
#define MAX7456ADD_RB6          0x16
#define MAX7456ADD_RB7          0x17
#define MAX7456ADD_RB8          0x18
#define MAX7456ADD_RB9          0x19
#define MAX7456ADD_RB10         0x1a
#define MAX7456ADD_RB11         0x1b
#define MAX7456ADD_RB12         0x1c
#define MAX7456ADD_RB13         0x1d
#define MAX7456ADD_RB14         0x1e
#define MAX7456ADD_RB15         0x1f
#define MAX7456ADD_OSDBL        0x6c
#define MAX7456ADD_STAT         0xA0
#define MAX7456ADD_CMDO         0xC0

#define NVM_RAM_SIZE            54
#define READ_NVR                0x50
#define WRITE_NVR               0xA0

// Maximum time to wait for video sync. After this we
// default to PAL
#define MAX_SYNC_WAIT_MS        1500

// Maximum time to wait for a software reset to complete
// Under normal conditions, it should take 20us
#define MAX_RESET_TIMEOUT_MS    50

#define MAKE_CHAR_MODE_U8(c, m) ((((uint16_t)c) << 8) | m)
#define MAKE_CHAR_MODE(c, m)    (MAKE_CHAR_MODE_U8(c, m) | (c > 255 ? CHAR_MODE_EXT : 0))
#define CHAR_BLANK              MAKE_CHAR_MODE(0x20, 0)
#define CHAR_BYTE(x)            (x >> 8)
#define MODE_BYTE(x)            (x & 0xFF)
#define CHAR_IS_BLANK(x)        ((CHAR_BYTE(x) == 0x20 || CHAR_BYTE(x) == 0x00) && !CHAR_MODE_IS_EXT(MODE_BYTE(x)))
#define CHAR_MODE_EXT           (1 << 2)
#define CHAR_MODE_IS_EXT(m)     ((m) & CHAR_MODE_EXT)

// we write everything in osdCharacterGridBuffer and set a dirty bit
// in screenIsDirty to update only changed chars. This solution
// is faster than redrawing the whole screen on each frame.
static BITARRAY_DECLARE(screenIsDirty, MAX7456_BUFFER_CHARS_PAL);

//max chars to update in one idle
#define MAX_CHARS2UPDATE        10
#define BYTES_PER_CHAR2UPDATE   (7 * 2) // SPI regs + values for them

typedef struct max7456Registers_s {
    uint8_t vm0;
    uint8_t dmm;
} max7456Registers_t;

typedef struct max7456State_s {
    busDevice_t *dev;
    videoSystem_e videoSystem;
    bool isInitialized;
    bool mutex;
    max7456Registers_t registers;
} max7456State_t;

static max7456State_t state;

static bool max7456ReadVM0(uint8_t *vm0)
{
    return busRead(state.dev, MAX7456ADD_VM0 | MAX7456ADD_READ, vm0);
}

static bool max7456IsBusy(void)
{
    uint8_t status;

    if (busRead(state.dev, MAX7456ADD_STAT, &status)) {
        return status & STAT_NVR_BUSY;
    }
    // Read failed or busy
    return true;
}

// Wait until max7456IsBusy() returns false, toggling a LED on each iteration
static void max7456WaitUntilNoBusy(void)
{
    while (1) {
        if (!max7456IsBusy()) {
            break;
        }
#ifdef LED0_TOGGLE
        LED0_TOGGLE;
#else
        LED1_TOGGLE;
#endif
    }
}

// Sets wether the OSD drawing is enabled. Returns true iff
// changes were succesfully performed.
static bool max7456OSDSetEnabled(bool enabled)
{
    if (enabled && !(state.registers.vm0 | OSD_ENABLE)) {
        state.registers.vm0 |= OSD_ENABLE;
    } else if (!enabled && (state.registers.vm0 | OSD_ENABLE)) {
        state.registers.vm0 &= ~OSD_ENABLE;
    } else {
        // No changes needed
        return false;
    }
    return busWrite(state.dev, MAX7456ADD_VM0, state.registers.vm0);
}

static bool max7456OSDIsEnabled(void)
{
    return state.registers.vm0 & OSD_ENABLE;
}

static void max7456Lock(void)
{
    while(state.mutex);

    state.mutex = true;
}

static void max7456Unlock(void)
{
    state.mutex = false;
}

static bool max7456TryLock(void)
{
    if (!state.mutex) {
        state.mutex = true;
        return true;
    }
    return false;
}

static int max7456PrepareBuffer(uint8_t * buf, int bufPtr, uint8_t add, uint8_t data)
{
    buf[bufPtr++] = add;
    buf[bufPtr++] = data;
    return bufPtr;
}

uint16_t max7456GetScreenSize(void)
{
    // Default to PAL while the display is not yet initialized. This
    // was the initial behavior and not all callers might be able to
    // deal with a zero returned from here.
    // TODO: Inspect all callers, make sure they can handle zero and
    // change this function to return zero before initialization.
    if (state.isInitialized && ((state.registers.vm0 & VIDEO_MODE_PAL) == 0)) {
        return MAX7456_BUFFER_CHARS_NTSC;
    }
    return MAX7456_BUFFER_CHARS_PAL;
}

uint8_t max7456GetRowsCount(void)
{
    if (!state.isInitialized) {
        // Not initialized yet
        return 0;
    }
    if (state.registers.vm0 & VIDEO_MODE_PAL) {
        return MAX7456_LINES_PAL;
    }

    return MAX7456_LINES_NTSC;
}

//because MAX7456 need some time to detect video system etc. we need to wait for a while to initialize it at startup
//and in case of restart we need to reinitialize chip. Note that we can't touch osdCharacterGridBuffer here, since
//it might already have some data by the first time this function is called.
static void max7456ReInit(void)
{
    uint8_t buf[2 * 2];
    int bufPtr = 0;
    uint8_t statVal;


    // Check if device is available
    if (state.dev == NULL) {
        return;
    }

    uint8_t vm0Mode;

    switch (state.videoSystem) {
        case PAL:
            vm0Mode = VIDEO_MODE_PAL;
            break;
        case NTSC:
            vm0Mode = VIDEO_MODE_NTSC;
            break;
        default:
            busRead(state.dev, MAX7456ADD_STAT, &statVal);
            if (VIN_IS_PAL(statVal)) {
                vm0Mode = VIDEO_MODE_PAL;
            } else if (VIN_IS_NTSC_alt(statVal)) {
                vm0Mode = VIDEO_MODE_NTSC;
            } else if ( millis() > MAX_SYNC_WAIT_MS) {
                // Detection timed out, default to PAL
                vm0Mode = VIDEO_MODE_PAL;
            } else {
                // No signal detected yet, wait for detection timeout
                return;
            }
    }

    state.registers.vm0 = vm0Mode | OSD_ENABLE;

    // Enable OSD drawing and clear the display
    bufPtr = max7456PrepareBuffer(buf, bufPtr, MAX7456ADD_VM0, state.registers.vm0);
    bufPtr = max7456PrepareBuffer(buf, bufPtr, MAX7456ADD_DMM, DMM_CLEAR_DISPLAY);

    // Transfer data to SPI
    busTransfer(state.dev, NULL, buf, bufPtr);

    // force redrawing all screen
    BITARRAY_SET_ALL(screenIsDirty);
    if (!state.isInitialized) {
        max7456RefreshAll();
        state.isInitialized = true;
    }
}

//here we init only CS and try to init MAX for first time
void max7456Init(const videoSystem_e videoSystem)
{
    uint8_t buf[(MAX7456_LINES_PAL + 1) * 2];
    int bufPtr;
    state.dev = busDeviceInit(BUSTYPE_SPI, DEVHW_MAX7456, 0, OWNER_OSD);

    if (state.dev == NULL) {
        return;
    }

    busSetSpeed(state.dev, BUS_SPEED_STANDARD);

    // force soft reset on Max7456
    busWrite(state.dev, MAX7456ADD_VM0, MAX7456_RESET);

    // DMM defaults to all zeroes on reset
    state.registers.dmm = 0;
    state.videoSystem = videoSystem;

    // Set screen buffer to all blanks
    for (uint_fast16_t ii = 0; ii < ARRAYLEN(osdCharacterGridBuffer); ii++) {
        osdCharacterGridBuffer[ii] = CHAR_BLANK;
    }

    // Wait for software reset to finish
    timeMs_t timeout = millis() + MAX_RESET_TIMEOUT_MS;
    while(max7456ReadVM0(&state.registers.vm0) &&
        (state.registers.vm0 | MAX7456_RESET) &&
        millis() < timeout);

    // Set all rows to same charactor black/white level. We
    // can do this without finding wether the display is PAL
    // NTSC because all the brightness registers can be written
    // regardless of the video mode.
    bufPtr = 0;
    for (int ii = 0; ii < MAX7456_LINES_PAL; ii++) {
        bufPtr = max7456PrepareBuffer(buf, bufPtr, MAX7456ADD_RB0 + ii, MAX7456_BWBRIGHTNESS);
    }

    // Set the blink duty cycle
    bufPtr = max7456PrepareBuffer(buf, bufPtr, MAX7456ADD_VM1, BLINK_DUTY_CYCLE_50_50 | BLINK_TIME_3 | BACKGROUND_BRIGHTNESS_28);
    busTransfer(state.dev, NULL, buf, bufPtr);
}

void max7456ClearScreen(void)
{
    for (uint_fast16_t ii = 0; ii < ARRAYLEN(osdCharacterGridBuffer); ii++) {
        if (osdCharacterGridBuffer[ii] != CHAR_BLANK) {
            osdCharacterGridBuffer[ii] = CHAR_BLANK;
            bitArraySet(screenIsDirty, ii);
        }
    }
}

void max7456WriteChar(uint8_t x, uint8_t y, uint16_t c, uint8_t mode)
{
    unsigned pos = y * MAX7456_CHARS_PER_LINE + x;
    uint16_t val = MAKE_CHAR_MODE(c, mode);
    if (osdCharacterGridBuffer[pos] != val) {
        osdCharacterGridBuffer[pos] = val;
        bitArraySet(screenIsDirty, pos);
    }
}

bool max7456ReadChar(uint8_t x, uint8_t y, uint16_t *c, uint8_t *mode)
{
    unsigned pos = y * MAX7456_CHARS_PER_LINE + x;
    if (pos < ARRAYLEN(osdCharacterGridBuffer)) {
        uint16_t val = osdCharacterGridBuffer[pos];
        *c = CHAR_BYTE(val);
        *mode = MODE_BYTE(val);
        if (CHAR_MODE_IS_EXT(*mode)) {
            *c |= 1 << 8;
            *mode &= ~CHAR_MODE_EXT;
        }
        return true;
    }
    return false;
}

void max7456Write(uint8_t x, uint8_t y, const char *buff, uint8_t mode)
{
    uint8_t i = 0;
    uint16_t c;
    unsigned pos = y * MAX7456_CHARS_PER_LINE + x;
    for (i = 0; *buff; i++, buff++, pos++) {
        //do not write past screen's end of line
        if (x + i >= MAX7456_CHARS_PER_LINE) {
            break;
        }
        c = MAKE_CHAR_MODE_U8(*buff, mode);
        if (osdCharacterGridBuffer[pos] != c) {
            osdCharacterGridBuffer[pos] = c;
            bitArraySet(screenIsDirty, pos);
        }
    }
}

// Must be called with the lock held. Returns wether any new characters
// were drawn.
static bool max7456DrawScreenPartial(void)
{
    uint8_t spiBuff[MAX_CHARS2UPDATE * BYTES_PER_CHAR2UPDATE];
    int bufPtr = 0;
    int pos;
    uint_fast16_t updatedCharCount;
    uint8_t charMode;

    for (pos = 0, updatedCharCount = 0;;) {
        pos = BITARRAY_FIND_FIRST_SET(screenIsDirty, pos);
        if (pos < 0) {
            // No more dirty chars.
            break;
        }

        // Found one dirty character to send
        uint8_t ph = pos >> 8;
        uint8_t pl = pos & 0xff;

        charMode = MODE_BYTE(osdCharacterGridBuffer[pos]);
        uint8_t chr = CHAR_BYTE(osdCharacterGridBuffer[pos]);
        if (CHAR_MODE_IS_EXT(charMode)) {
            if (!DMM_IS_8BIT_MODE(state.registers.dmm)) {
                state.registers.dmm |= DMM_8BIT_MODE;
                bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMM, state.registers.dmm);
            }

            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAH, ph | DMAH_8_BIT_DMDI_IS_CHAR_ATTR);
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAL, pl);
            // Attribute bit positions on DMDI are 2 bits up relative to DMM.
            // DMM uses [5:3] while DMDI uses [7:4] - one bit more for referencing
            // characters in the [256, 511] range (which is not possible via DMM).
            // Since we write mostly to DMM, the internal representation uses
            // the format of the former and we shift it up here.
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMDI, charMode << 2);

            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAH, ph);
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAL, pl);
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMDI, chr);

        } else {
            if (DMM_IS_8BIT_MODE(state.registers.dmm) || (DMM_CHAR_MODE_MASK & state.registers.dmm) != charMode) {
                state.registers.dmm &= ~DMM_8BIT_MODE;
                state.registers.dmm = (state.registers.dmm & ~DMM_CHAR_MODE_MASK) | charMode;
                // Send the attributes for the character run. They
                // will be applied to all characters until we change
                // the DMM register.
                bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMM, state.registers.dmm);
            }

            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAH, ph);
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMAL, pl);
            bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_DMDI, chr);
        }

        bitArrayClr(screenIsDirty, pos);
        if (++updatedCharCount == MAX_CHARS2UPDATE) {
            break;
        }
        // Start next search at next bit
        pos++;
    }

    if (bufPtr) {
        busTransfer(state.dev, NULL, spiBuff, bufPtr);
        return true;
    }
    return false;
}

// Must be called with the lock held
static void max7456StallCheck(void)
{
    static uint32_t lastSigCheckMs = 0;
    static uint32_t videoDetectTimeMs = 0;

    uint8_t vm0;
    uint8_t statReg;

    if (!state.isInitialized || !max7456ReadVM0(&vm0) || vm0 != state.registers.vm0) {
        max7456ReInit();
        return;
    }

    if (state.videoSystem == VIDEO_SYSTEM_AUTO) {
        timeMs_t nowMs = millis();
        if ((nowMs - lastSigCheckMs) > MAX7456_SIGNAL_CHECK_INTERVAL_MS) {

            // Adjust output format based on the current input format.
            busRead(state.dev, MAX7456ADD_STAT, &statReg);

            if (statReg & STAT_LOS) {
                videoDetectTimeMs = 0;
            } else {
                if ((VIN_IS_PAL(statReg) && VIDEO_MODE_IS_NTSC(state.registers.vm0))
                    || (VIN_IS_NTSC_alt(statReg) && VIDEO_MODE_IS_PAL(state.registers.vm0))) {
                    if (videoDetectTimeMs) {
                        if (nowMs - videoDetectTimeMs > VIDEO_SIGNAL_DEBOUNCE_MS) {
                            max7456ReInit();
                        }
                    } else {
                        // Wait for signal to stabilize
                        videoDetectTimeMs = nowMs;
                    }
                }
            }

            lastSigCheckMs = nowMs;
        }
    }
}

void max7456Update(void)
{
    // Check if device is available
    if (state.dev == NULL) {
        return;
    }

    if ((max7456OSDIsEnabled() && max7456TryLock()) || !state.isInitialized) {
        // (Re)Initialize MAX7456 at startup or stall is detected.
        max7456StallCheck();
        if (state.isInitialized) {
            max7456DrawScreenPartial();
        }
        max7456Unlock();
    }
}

// this function redraws the whole display at once and
// might a long time to complete. It should not the used
// when copter is armed.
void max7456RefreshAll(void)
{
    uint8_t dmm;

    // Check if device is available
    if (state.dev == NULL) {
        return;
    }

    if (max7456TryLock()) {

        // Issue an OSD clear command
        busRead(state.dev, MAX7456ADD_DMM | MAX7456ADD_READ, &dmm);
        busWrite(state.dev, MAX7456ADD_DMM, state.registers.dmm | DMM_CLEAR_DISPLAY);

        // Wait for clear to complete (20us)
        while (1) {
             busRead(state.dev, MAX7456ADD_DMM | MAX7456ADD_READ, &dmm);
             if (!(dmm & DMM_CLEAR_DISPLAY)) {
                 state.registers.dmm = dmm;
                 break;
             }
        }

        // Mark non-blank characters as dirty
        BITARRAY_CLR_ALL(screenIsDirty);
        for (unsigned ii = 0; ii < ARRAYLEN(osdCharacterGridBuffer); ii++) {
            if (!CHAR_IS_BLANK(osdCharacterGridBuffer[ii])) {
                bitArraySet(screenIsDirty, ii);
            }
        }

        // Now perform partial writes until there are no dirty ones
        while(max7456DrawScreenPartial());

        max7456Unlock();
    }
}

void max7456ReadNvm(uint16_t char_address, osdCharacter_t *chr)
{
    // Check if device is available
    if (state.dev == NULL) {
        return;
    }

    max7456Lock();
    // OSD must be disabled to read or write to NVM
    bool enabled = max7456OSDSetEnabled(false);

    busWrite(state.dev, MAX7456ADD_CMAH, char_address);
    if (char_address > 255) {
        // AT7456E and AB7456 have 512 characters of NVM.
        // To read/write to NVM they use CMAL[6] as the
        // high bits of the character address.
        uint8_t addr_h = char_address >> 8;
        busWrite(state.dev, MAX7456ADD_CMAL, addr_h << 6);
    }
    busWrite(state.dev, MAX7456ADD_CMM, READ_NVR);

    max7456WaitUntilNoBusy();

    for (unsigned ii = 0; ii < OSD_CHAR_VISIBLE_BYTES; ii++) {
        busWrite(state.dev, MAX7456ADD_CMAL, ii);
        busRead(state.dev, MAX7456ADD_CMDO, &chr->data[ii]);
    }

    max7456OSDSetEnabled(enabled);
    max7456Unlock();
}

void max7456WriteNvm(uint16_t char_address, const osdCharacter_t *chr)
{
    uint8_t spiBuff[(sizeof(chr->data) * 2 + 2) * 2];
    int bufPtr = 0;

    // Check if device is available
    if (state.dev == NULL) {
        return;
    }

    max7456Lock();
    // OSD must be disabled to read or write to NVM
    max7456OSDSetEnabled(false);

    bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_CMAH, char_address & 0xFF); // set start address high

    uint8_t or_val = 0;
    if (char_address > 255) {
        // AT7456E and AB7456 have 512 characters of NVM.
        // To read/write to NVM they use CMAL[6] as the
        // high bit of the character address.
        //
        // Instead of issuing an additional write to CMAL when
        // we're done uploading to shadow RAM, we set the high
        // bits of CMAL on every write since they have no side
        // effects while writing from CMDI to RAM and when we
        // issue the copy command to NVM, CMAL[6] is already
        // set.
        uint8_t addr_h = char_address >> 8;
        or_val = addr_h << 6;
    }

    for (unsigned x = 0; x < OSD_CHAR_VISIBLE_BYTES; x++) {
        bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_CMAL, x | or_val); //set start address low
        bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_CMDI, chr->data[x]);
    }

    // transfer 54 bytes from shadow ram to NVM
    bufPtr = max7456PrepareBuffer(spiBuff, bufPtr, MAX7456ADD_CMM, WRITE_NVR);

    busTransfer(state.dev, NULL, spiBuff, bufPtr);

    max7456WaitUntilNoBusy();

    /* XXX: Don't call max7456OSDEnable(), it's intentionally ommited.
     * If we continue drawing while characters are being uploaded, we
     * get some corrupted characters from time to time. As a workaround,
     * we require a reboot after characters have been uploaded to NVM.
     */

    max7456Unlock();
}


#endif
