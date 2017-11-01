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
#include "drivers/light_led.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/dma.h"
#include "drivers/vcd.h"

#include "max7456.h"
#include "max7456_symbols.h"

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
#define BACKGROUND_BRIGHTNESS_0 0x00
#define BACKGROUND_BRIGHTNESS_7 0x01
#define BACKGROUND_BRIGHTNESS_14 0x02
#define BACKGROUND_BRIGHTNESS_21 0x03
#define BACKGROUND_BRIGHTNESS_28 0x04
#define BACKGROUND_BRIGHTNESS_35 0x05
#define BACKGROUND_BRIGHTNESS_42 0x06
#define BACKGROUND_BRIGHTNESS_49 0x07

#define BACKGROUND_MODE_GRAY 0x40

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
#define DMM_BLINK (1 << 4)
#define DMM_INVERT_PIXEL_COLOR (1 << 3)
#define DMM_CLEAR_DISPLAY (1 << 2)
#define DMM_CLEAR_DISPLAY_VERT (DMM_CLEAR_DISPLAY | 1 << 1)
#define DMM_AUTOINCREMENT (1 << 0)

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

#define NVM_RAM_SIZE            54
#define WRITE_NVR               0xA0

#define CHARS_PER_LINE          30 // XXX Should be related to VIDEO_BUFFER_CHARS_*?
#define MAKE_CHAR_MODE(c, m)    ((((uint16_t)c) << 8) | m)
#define CHAR_BLANK              MAKE_CHAR_MODE(0x20, 0)
#define CHAR_BYTE(x)            (x >> 8)
#define MODE_BYTE(x)            (x & 0xFF)

// On shared SPI buss we want to change clock for OSD chip and restore for other devices.

#ifdef MAX7456_SPI_CLK
    #define ENABLE_MAX7456()    do { \
                                    spiSetSpeed(MAX7456_SPI_INSTANCE, MAX7456_SPI_CLK); \
                                    IOLo(max7456CsPin); \
                                } while(0)
#else
    #define ENABLE_MAX7456()    do { \
                                    IOLo(max7456CsPin); \
                                } while(0)
#endif

#ifdef MAX7456_RESTORE_CLK
    #define DISABLE_MAX7456()   do { \
                                    IOHi(max7456CsPin); \
                                    spiSetSpeed(MAX7456_SPI_INSTANCE, MAX7456_RESTORE_CLK); \
                                } while(0)
#else
    #define DISABLE_MAX7456()   do { \
                                    IOHi(max7456CsPin); \
                                } while(0)
#endif

uint16_t maxScreenSize = VIDEO_BUFFER_CHARS_PAL;

// we write everything in screenBuffer and set a dirty bit
// in screenIsDirty to upgrade only changed chars this solution
// is faster than redrawing the whole screen on each frame
static uint16_t screenBuffer[VIDEO_BUFFER_CHARS_PAL] ALIGNED(4);
static BITARRAY_DECLARE(screenIsDirty, VIDEO_BUFFER_CHARS_PAL);

//max chars to update in one idle
#define MAX_CHARS2UPDATE        10
#define BYTES_PER_CHAR2UPDATE   8 // [3-4] spi regs + values for them
#ifdef MAX7456_DMA_CHANNEL_TX
volatile bool dmaTransactionInProgress = false;
#endif

#define SPI_BUFF_SIZE (MAX_CHARS2UPDATE*BYTES_PER_CHAR2UPDATE)
static uint8_t spiBuff[SPI_BUFF_SIZE];

static uint8_t  videoSignalCfg   = 0;
static uint8_t  videoSignalReg   = VIDEO_MODE_PAL | OSD_ENABLE; //PAL by default
static bool  max7456Lock        = false;
static bool fontIsLoading       = false;
static IO_t max7456CsPin        = IO_NONE;

// Register bus device descriptor
BUSDEV_REGISTER_SPI(max7456_busdev, DEVHW_MAX7456, 0, MAX7456_SPI_CS_PIN);

static uint8_t max7456Send(uint8_t add, uint8_t data)
{
    spiTransferByte(MAX7456_SPI_INSTANCE, add);
    return spiTransferByte(MAX7456_SPI_INSTANCE, data);
}

#ifdef MAX7456_DMA_CHANNEL_TX
static void max7456SendDma(void* tx_buffer, void* rx_buffer, uint16_t buffer_size)
{
    DMA_InitTypeDef DMA_InitStructure;
#ifdef MAX7456_DMA_CHANNEL_RX
    static uint16_t dummy[] = {0xffff};
#else
    UNUSED(rx_buffer);
#endif
    while (dmaTransactionInProgress); // Wait for prev DMA transaction

    DMA_DeInit(MAX7456_DMA_CHANNEL_TX);
#ifdef MAX7456_DMA_CHANNEL_RX
    DMA_DeInit(MAX7456_DMA_CHANNEL_RX);
#endif

    // Common to both channels
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(MAX7456_SPI_INSTANCE->DR));
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_BufferSize = buffer_size;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;

#ifdef MAX7456_DMA_CHANNEL_RX
    // Rx Channel
#ifdef STM32F4
    DMA_InitStructure.DMA_Memory0BaseAddr = rx_buffer ? (uint32_t)rx_buffer : (uint32_t)(dummy);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
#else
    DMA_InitStructure.DMA_MemoryBaseAddr = rx_buffer ? (uint32_t)rx_buffer : (uint32_t)(dummy);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
#endif
    DMA_InitStructure.DMA_MemoryInc = rx_buffer ? DMA_MemoryInc_Enable : DMA_MemoryInc_Disable;

    DMA_Init(MAX7456_DMA_CHANNEL_RX, &DMA_InitStructure);
    DMA_Cmd(MAX7456_DMA_CHANNEL_RX, ENABLE);
#endif
    // Tx channel

#ifdef STM32F4
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)tx_buffer; //max7456_screen;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
#else
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tx_buffer; //max7456_screen;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
#endif
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    DMA_Init(MAX7456_DMA_CHANNEL_TX, &DMA_InitStructure);
    DMA_Cmd(MAX7456_DMA_CHANNEL_TX, ENABLE);

#ifdef MAX7456_DMA_CHANNEL_RX
    DMA_ITConfig(MAX7456_DMA_CHANNEL_RX, DMA_IT_TC, ENABLE);
#else
    DMA_ITConfig(MAX7456_DMA_CHANNEL_TX, DMA_IT_TC, ENABLE);
#endif

    // Enable SPI TX/RX request
    ENABLE_MAX7456();
    dmaTransactionInProgress = true;

    SPI_I2S_DMACmd(MAX7456_SPI_INSTANCE,
#ifdef MAX7456_DMA_CHANNEL_RX
            SPI_I2S_DMAReq_Rx |
#endif
            SPI_I2S_DMAReq_Tx, ENABLE);
}

void max7456_dma_irq_handler(dmaChannelDescriptor_t* descriptor)
{
    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
#ifdef MAX7456_DMA_CHANNEL_RX
        DMA_Cmd(MAX7456_DMA_CHANNEL_RX, DISABLE);
#endif
        // make sure spi dmd transfer is complete
        while (SPI_I2S_GetFlagStatus (MAX7456_SPI_INSTANCE, SPI_I2S_FLAG_TXE) == RESET) {};
        while (SPI_I2S_GetFlagStatus (MAX7456_SPI_INSTANCE, SPI_I2S_FLAG_BSY) == SET) {};

        //Empty RX buffer. RX DMA takes care of it if enabled
        //this should be done after transmission finish!!!
        while (SPI_I2S_GetFlagStatus(MAX7456_SPI_INSTANCE, SPI_I2S_FLAG_RXNE) == SET) {
            MAX7456_SPI_INSTANCE->DR;
        }

        DMA_Cmd(MAX7456_DMA_CHANNEL_TX, DISABLE);

        DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);

        SPI_I2S_DMACmd(MAX7456_SPI_INSTANCE,
#ifdef MAX7456_DMA_CHANNEL_RX
                SPI_I2S_DMAReq_Rx |
#endif
                SPI_I2S_DMAReq_Tx, DISABLE);

        DISABLE_MAX7456();
        dmaTransactionInProgress = false;
    }

    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_HTIF)) {
        DMA_CLEAR_FLAG(descriptor, DMA_IT_HTIF);
    }
    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TEIF)) {
        DMA_CLEAR_FLAG(descriptor, DMA_IT_TEIF);
    }
}

#endif

uint8_t max7456GetRowsCount(void)
{
    if (videoSignalReg & VIDEO_MODE_PAL)
        return VIDEO_LINES_PAL;

    return VIDEO_LINES_NTSC;
}

//because MAX7456 need some time to detect video system etc. we need to wait for a while to initialize it at startup
//and in case of restart we need to reinitialize chip. Note that we can't touch screenBuffer here, since
//it might already have some data by the first time this function is called.
void max7456ReInit(void)
{
    uint8_t maxScreenRows;
    uint8_t srdata = 0;
    uint16_t x;
    static bool firstInit = true;

    //do not init MAX before camera power up correctly
    if (millis() < 1500)
        return;

    ENABLE_MAX7456();

    switch (videoSignalCfg) {
        case PAL:
            videoSignalReg = VIDEO_MODE_PAL | OSD_ENABLE;
            break;
        case NTSC:
            videoSignalReg = VIDEO_MODE_NTSC | OSD_ENABLE;
            break;
        default:
            srdata = max7456Send(MAX7456ADD_STAT, 0x00);
            if ((0x02 & srdata) == 0x02)
                videoSignalReg = VIDEO_MODE_NTSC | OSD_ENABLE;
    }

    if (videoSignalReg & VIDEO_MODE_PAL) { //PAL
        maxScreenSize = VIDEO_BUFFER_CHARS_PAL;
        maxScreenRows = VIDEO_LINES_PAL;
    } else {              // NTSC
        maxScreenSize = VIDEO_BUFFER_CHARS_NTSC;
        maxScreenRows = VIDEO_LINES_NTSC;
    }

    // set all rows to same charactor black/white level
    for (x = 0; x < maxScreenRows; x++) {
        max7456Send(MAX7456ADD_RB0 + x, BWBRIGHTNESS);
    }

    // make sure the Max7456 is enabled
    max7456Send(MAX7456ADD_VM0, videoSignalReg);
    max7456Send(MAX7456ADD_DMM, DMM_CLEAR_DISPLAY);
    DISABLE_MAX7456();

    // force redrawing all screen in non-dma mode
    memset(screenIsDirty, 0xFF, sizeof(screenIsDirty));
    if (firstInit)
    {
        max7456RefreshAll();
        firstInit = false;
    }
}


//here we init only CS and try to init MAX for first time
void max7456Init(const vcdProfile_t *pVcdProfile)
{
#ifdef MAX7456_SPI_CS_PIN
    max7456CsPin = IOGetByTag(IO_TAG(MAX7456_SPI_CS_PIN));
#endif
    IOInit(max7456CsPin, OWNER_OSD, RESOURCE_SPI_CS, 0);
    IOConfigGPIO(max7456CsPin, SPI_IO_CS_CFG);

    spiSetSpeed(MAX7456_SPI_INSTANCE, SPI_CLOCK_STANDARD);
    // force soft reset on Max7456
    ENABLE_MAX7456();
    max7456Send(MAX7456ADD_VM0, MAX7456_RESET);
    DISABLE_MAX7456();
    videoSignalCfg = pVcdProfile->video_system;

    // Set screenbuffer to all blanks
    for (uint_fast16_t ii = 0; ii < ARRAYLEN(screenBuffer); ii++) {
        screenBuffer[ii] = CHAR_BLANK;
    }

#ifdef MAX7456_DMA_CHANNEL_TX
    dmaSetHandler(MAX7456_DMA_IRQ_HANDLER_ID, max7456_dma_irq_handler, NVIC_PRIO_MAX7456_DMA, 0);
#endif
    //real init will be made letter when driver idle detect
}

void max7456ClearScreen(void)
{
    for (uint_fast16_t ii = 0; ii < ARRAYLEN(screenBuffer); ii++) {
        if (screenBuffer[ii] != CHAR_BLANK) {
            screenBuffer[ii] = CHAR_BLANK;
            bitArraySet(screenIsDirty, ii);
        }
    }
}

void max7456WriteChar(uint8_t x, uint8_t y, uint8_t c, uint8_t mode)
{
    unsigned pos = y * CHARS_PER_LINE + x;
    uint16_t val = MAKE_CHAR_MODE(c, mode);
    if (screenBuffer[pos] != val) {
        screenBuffer[pos] = val;
        bitArraySet(screenIsDirty, pos);
    }
}

void max7456Write(uint8_t x, uint8_t y, const char *buff, uint8_t mode)
{
    uint8_t i = 0;
    uint16_t c;
    unsigned pos = y * CHARS_PER_LINE + x;
    for (i = 0; *buff; i++, buff++, pos++) {
        //do not write past screen's end of line
        if (x + i >= CHARS_PER_LINE) {
            break;
        }
        c = MAKE_CHAR_MODE(*buff, mode);
        if (screenBuffer[pos] != c) {
            screenBuffer[pos] = c;
            bitArraySet(screenIsDirty, pos);
        }
    }
}

bool max7456DmaInProgress(void)
{
#ifdef MAX7456_DMA_CHANNEL_TX
    return dmaTransactionInProgress;
#else
    return false;
#endif
}

void max7456DrawScreenPartial(void)
{
    static uint32_t lastSigCheckMs = 0;
    static uint32_t videoDetectTimeMs = 0;
    // Save this between updates. The default value
    // in the MAX7456 is all bits to zero.
    static uint8_t setMode = 0;

    uint8_t stallCheck;
    uint8_t videoSense;
    uint32_t nowMs;
    int pos;
    int buff_len = 0;
    uint_fast16_t updatedCharCount;
    uint8_t currentMode;

    if (!max7456Lock && !fontIsLoading) {
        // (Re)Initialize MAX7456 at startup or stall is detected.

        max7456Lock = true;
        ENABLE_MAX7456();
        stallCheck = max7456Send(MAX7456ADD_VM0|MAX7456ADD_READ, 0x00);
        DISABLE_MAX7456();

        nowMs = millis();

        if (stallCheck != videoSignalReg) {
            max7456ReInit();
        } else if ((videoSignalCfg == VIDEO_SYSTEM_AUTO)
                  && ((nowMs - lastSigCheckMs) > MAX7456_SIGNAL_CHECK_INTERVAL_MS)) {

            // Adjust output format based on the current input format.

            ENABLE_MAX7456();
            videoSense = max7456Send(MAX7456ADD_STAT, 0x00);
            DISABLE_MAX7456();

            if (videoSense & STAT_LOS) {
                videoDetectTimeMs = 0;
            } else {
                if ((VIN_IS_PAL(videoSense) && VIDEO_MODE_IS_NTSC(videoSignalReg))
                  || (VIN_IS_NTSC_alt(videoSense) && VIDEO_MODE_IS_PAL(videoSignalReg))) {
                    if (videoDetectTimeMs) {
                        if (millis() - videoDetectTimeMs > VIDEO_SIGNAL_DEBOUNCE_MS) {
                            max7456ReInit();
                        }
                    } else {
                        // Wait for signal to stabilize
                        videoDetectTimeMs = millis();
                    }
                }
            }

            lastSigCheckMs = nowMs;
        }

        //------------   end of (re)init-------------------------------------

        for (pos = 0, updatedCharCount = 0;;) {
            pos = BITARRAY_FIND_FIRST_SET(screenIsDirty, pos);
            if (pos < 0 || pos >= maxScreenSize) {
                // No more dirty chars.
                break;
            }
            currentMode = MODE_BYTE(screenBuffer[pos]);
            // Found one dirty character to send
            if (setMode != currentMode) {
                setMode = currentMode;
                // Send the attributes for the character run. They
                // will be applied to all characters until we change
                // the DMM register.
                spiBuff[buff_len++] = MAX7456ADD_DMM;
                spiBuff[buff_len++] = currentMode;
            }
            spiBuff[buff_len++] = MAX7456ADD_DMAH;
            spiBuff[buff_len++] = pos >> 8;
            spiBuff[buff_len++] = MAX7456ADD_DMAL;
            spiBuff[buff_len++] = pos & 0xff;
            spiBuff[buff_len++] = MAX7456ADD_DMDI;
            spiBuff[buff_len++] = CHAR_BYTE(screenBuffer[pos]);
            bitArrayClr(screenIsDirty, pos);
            if (++updatedCharCount == MAX_CHARS2UPDATE) {
                break;
            }
            // Start next search at next bit
            pos++;
        }

        if (buff_len) {
            #ifdef MAX7456_DMA_CHANNEL_TX
            if (buff_len > 0)
                max7456SendDma(spiBuff, NULL, buff_len);
            #else
            ENABLE_MAX7456();
            spiTransfer(MAX7456_SPI_INSTANCE, NULL, spiBuff, buff_len);
            DISABLE_MAX7456();
            #endif // MAX7456_DMA_CHANNEL_TX
        }
        max7456Lock = false;
    }
}

// this function redraws the whole display at once and
// might a long time to complete. It should not the used
// when copter is armed.
void max7456RefreshAll(void)
{
    if (!max7456Lock) {
#ifdef MAX7456_DMA_CHANNEL_TX
        while (dmaTransactionInProgress);
#endif
        uint16_t xx;
        max7456Lock = true;
        ENABLE_MAX7456();

        // Write characters. Start at character zero.
        max7456Send(MAX7456ADD_DMAH, 0);
        max7456Send(MAX7456ADD_DMAL, 0);
        // Enable auto-increment mode
        max7456Send(MAX7456ADD_DMM, DMM_AUTOINCREMENT);
        for (xx = 0; xx < maxScreenSize; ++xx) {
            max7456Send(MAX7456ADD_DMDI, CHAR_BYTE(screenBuffer[xx]));
        }
        // Exit auto-increment mode by writing the 0xFF escape
        // sequence to DMDI.
        max7456Send(MAX7456ADD_DMDI, 0xFF);

        // Write character attributes. Start at zero, but
        // set DMAH[1] = 1, to signal that we're sending
        // attributes rather than characters. Process is the
        // same as for the characters a few lines up.
        max7456Send(MAX7456ADD_DMAH, 1<<1);
        max7456Send(MAX7456ADD_DMAL, 0);
        max7456Send(MAX7456ADD_DMM, DMM_AUTOINCREMENT);
        for (xx = 0; xx < maxScreenSize; ++xx) {
            // Note that atttribute bits in DMDI are in different
            // positions than in DMM (DMM is used for partial writes),
            // and we store the attributes in the format expected by
            // DMM.
            //      | LBC | BLK | INV
            // ----------------------
            // DMDI:|  7  |  6  |  5
            // DMM: |  5  |  4  |  3
            //
            // Thus, we need to shift the bits by 2 when writing character
            // attributes to DMDI.
            max7456Send(MAX7456ADD_DMDI, MODE_BYTE(screenBuffer[xx]) << 2);
        }
        max7456Send(MAX7456ADD_DMDI, 0xFF);

        DISABLE_MAX7456();

        // All characters have been set to the MAX7456, none is dirty now.
        memset(screenIsDirty, 0, sizeof(screenIsDirty));
        max7456Lock = false;
    }
}

void max7456WriteNvm(uint8_t char_address, const uint8_t *font_data)
{
    uint8_t x;

#ifdef MAX7456_DMA_CHANNEL_TX
    while (dmaTransactionInProgress);
#endif
    while (max7456Lock);
    max7456Lock = true;

    ENABLE_MAX7456();
    // disable display
    fontIsLoading = true;
    max7456Send(MAX7456ADD_VM0, 0);

    max7456Send(MAX7456ADD_CMAH, char_address); // set start address high

    for (x = 0; x < 54; x++) {
        max7456Send(MAX7456ADD_CMAL, x); //set start address low
        max7456Send(MAX7456ADD_CMDI, font_data[x]);
#ifdef LED0_TOGGLE
        LED0_TOGGLE;
#else
        LED1_TOGGLE;
#endif
    }

    // transfer 54 bytes from shadow ram to NVM
    max7456Send(MAX7456ADD_CMM, WRITE_NVR);

    // wait until bit 5 in the status register returns to 0 (12ms)
    while ((max7456Send(MAX7456ADD_STAT, 0x00) & STAT_NVR_BUSY) != 0x00);

    DISABLE_MAX7456();

    max7456Lock = false;
}


#endif
