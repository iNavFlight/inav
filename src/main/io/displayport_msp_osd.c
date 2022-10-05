/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

FILE_COMPILE_FOR_SPEED

//#define MSP_DISPLAYPORT_STATS

#if defined(USE_OSD) && defined(USE_MSP_OSD)

#include "common/utils.h"
#include "common/printf.h"
#include "common/time.h"
#include "common/bitarray.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/osd_symbols.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

#include "displayport_msp_osd.h"

#define FONT_VERSION 3

#define MSP_CLEAR_SCREEN 2
#define MSP_WRITE_STRING 3
#define MSP_DRAW_SCREEN 4
#define MSP_SET_OPTIONS 5

typedef enum {          // defines are from hdzero code
    SD_3016,
    HD_5018,
    HD_3016,
    HD_6022            // added to support DJI wtfos 60x22 grid
} resolutionType_e;

#define DRAW_FREQ_DENOM 4 // 60Hz
#define TX_BUFFER_SIZE 1024
#define VTX_TIMEOUT 1000 // 1 second timer

static mspProcessCommandFnPtr mspProcessCommand;
static mspPort_t mspPort;
static displayPort_t mspOsdDisplayPort;
static bool vtxSeen, vtxActive, vtxReset;
static timeMs_t vtxHeartbeat;

// PAL screen size
#define PAL_COLS 30
#define PAL_ROWS 16
// NTSC screen size
#define NTSC_COLS 30
#define NTSC_ROWS 13
// HDZERO screen size
#define HDZERO_COLS 50
#define HDZERO_ROWS 18
// Avatar screen size
#define AVATAR_COLS 54
#define AVATAR_ROWS 20
// DJIWTF screen size
#define DJI_COLS 60
#define DJI_ROWS 22
// set COLS and ROWS to largest size available
#define COLS DJI_COLS
#define ROWS DJI_ROWS
// set screen size
#define SCREENSIZE (ROWS*COLS)
static uint8_t screen[SCREENSIZE];
static BITARRAY_DECLARE(fontPage, SCREENSIZE); // font page for each character on the screen
static BITARRAY_DECLARE(dirty, SCREENSIZE); // change status for each character on the screen
static bool screenCleared;
static uint8_t screenRows;
static uint8_t screenCols;
static videoSystem_e osdVideoSystem;

extern uint8_t cliMode;

#ifdef MSP_DISPLAYPORT_STATS
static uint32_t dataSent;
static uint8_t resetCount;
#endif

static int output(displayPort_t *displayPort, uint8_t cmd, uint8_t *subcmd, int len)
{
    UNUSED(displayPort);

    int sent = 0;

    if (!cliMode && vtxActive) {
        sent = mspSerialPushPort(cmd, subcmd, len, &mspPort, MSP_V1);
    }

#ifdef MSP_DISPLAYPORT_STATS
    dataSent += sent;
#endif

    return sent;
}

static void checkVtxPresent(void)
{
    if (vtxActive && (millis()-vtxHeartbeat) > VTX_TIMEOUT) {
        vtxActive = false;
    }
}

static int setHdMode(displayPort_t *displayPort)
{
    uint8_t subSubcmd = 0;

    switch(osdVideoSystem)
    {
    case VIDEO_SYSTEM_DJIWTF:
        subSubcmd = HD_6022;
        break;
    case VIDEO_SYSTEM_HDZERO:
        subSubcmd = HD_5018;
        break;
    default:
        subSubcmd = SD_3016;
        break;
    }

    checkVtxPresent();
    uint8_t subcmd[] = { MSP_SET_OPTIONS, 0, subSubcmd }; // font selection, mode (SD/HD)
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static void init(void)
{
    memset(screen, SYM_BLANK, sizeof(screen));
    BITARRAY_CLR_ALL(fontPage);
    BITARRAY_CLR_ALL(dirty);
}

static int clearScreen(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_CLEAR_SCREEN };

    init();
    setHdMode(displayPort);
    screenCleared = true;
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static bool readChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);

    uint16_t pos = (row * screenCols) + col;
    if (pos >= SCREENSIZE) {
        return false;
    }

    *c = screen[pos];
    if (bitArrayGet(fontPage, pos)) {
        *c |= 0x100;
    }

    if (attr) {
        *attr = TEXT_ATTRIBUTES_NONE;
    }

    return true;
}

static int setChar(const uint16_t pos, const uint16_t c)
{
    if (pos < SCREENSIZE) {
        uint8_t ch = c & 0xFF;
        bool page = (c >> 8);
        if (screen[pos] != ch || bitArrayGet(fontPage, pos) !=  page) {
            screen[pos] = ch;
            (page) ? bitArraySet(fontPage, pos) : bitArrayClr(fontPage, pos);
            bitArraySet(dirty, pos);
        }
    }
    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);
    UNUSED(attr);

    return setChar((row * screenCols) + col, c);
}

static int writeString(displayPort_t *displayPort, uint8_t col, uint8_t row, const char *string, textAttributes_t attr)
{
    UNUSED(displayPort);
    UNUSED(attr);

    uint16_t pos = (row * screenCols) + col;
    while (*string) {
        setChar(pos++, *string++);
    }
    return 0;
}

#ifdef MSP_DISPLAYPORT_STATS
static void printStats(displayPort_t *displayPort, uint32_t updates)
{
    static timeMs_t lastTime;
    static uint32_t maxDataSent, maxBufferUsed, maxUpdates;
    timeMs_t currentTime = millis();
    char lineBuffer[100];

    if (updates > maxUpdates) {
        maxUpdates = updates; // updates sent per displayWrite
    }

    uint32_t bufferUsed = TX_BUFFER_SIZE - serialTxBytesFree(mspPort.port);
    if (bufferUsed > maxBufferUsed) {
        maxBufferUsed = bufferUsed; // serial buffer used after displayWrite
    }

    uint32_t diff = (currentTime - lastTime);
    if (diff > 1000) { // Data sampled in 1 second
        if (dataSent > maxDataSent) {
            maxDataSent = dataSent; // bps (max 11520 allowed)
        }

        dataSent = 0;
        lastTime = currentTime;
    }


    tfp_sprintf(lineBuffer, "R:%2d %4ld %5ld(%5ld) U:%2ld(%2ld) B:%3ld(%4ld,%4ld)", resetCount, (millis()-vtxHeartbeat),
            dataSent, maxDataSent, updates, maxUpdates, bufferUsed, maxBufferUsed, mspPort.port->txBufferSize);
    writeString(displayPort, 0, 17, lineBuffer, 0);
}
#endif

/**
 * Write only changed characters to the VTX
 */
static int drawScreen(displayPort_t *displayPort) // 250Hz
{
    static uint8_t counter = 0;

    if ((counter++ % DRAW_FREQ_DENOM) == 0) {
        uint8_t subcmd[COLS + 4];
        uint8_t updateCount = 0;
        subcmd[0] = MSP_WRITE_STRING;

        int next = BITARRAY_FIND_FIRST_SET(dirty, 0);
        while (next >= 0) {
            // Look for sequential dirty characters on the same line for the same font page
            int pos = next;
            uint8_t row = pos / screenCols;
            uint8_t col = pos % screenCols;
            int endOfLine = row * screenCols + screenCols;
            bool page = bitArrayGet(fontPage, pos);

            uint8_t len = 4;
            do {
                bitArrayClr(dirty, pos);
                subcmd[len++] = screen[pos++];

                if (bitArrayGet(dirty, pos)) {
                    next = pos;
                }
            } while (next == pos && next < endOfLine && bitArrayGet(fontPage, next) == page);

            subcmd[1] = row;
            subcmd[2] = col;
            subcmd[3] = page;
            output(displayPort, MSP_DISPLAYPORT, subcmd, len);
            updateCount++;
            next = BITARRAY_FIND_FIRST_SET(dirty, pos);
        }

        if (updateCount > 0 || screenCleared) {
            if (screenCleared) {
                screenCleared = false;
            }
            subcmd[0] = MSP_DRAW_SCREEN;
            output(displayPort, MSP_DISPLAYPORT, subcmd, 1);
        }

#ifdef MSP_DISPLAYPORT_STATS
        printStats(displayPort, updateCount);
#endif
        checkVtxPresent();

        if (vtxReset) {
#ifdef MSP_DISPLAYPORT_STATS
            resetCount++;
#endif
            clearScreen(displayPort);
            vtxReset = false;
        }
    }

    return 0;
}

static void resync(displayPort_t *displayPort)
{
    displayPort->rows = screenRows;
    displayPort->cols = screenCols;
    setHdMode(displayPort);
}

static int screenSize(const displayPort_t *displayPort)
{
    return (displayPort->rows * displayPort->cols);
}

static uint32_t txBytesFree(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return mspSerialTxBytesFree();
}

static bool getFontMetadata(displayFontMetadata_t *metadata, const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    metadata->charCount = 512;
    metadata->version = FONT_VERSION;
    return true;
}

static textAttributes_t supportedTextAttributes(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return TEXT_ATTRIBUTES_NONE;
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return false;
}

static bool isReady(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return vtxActive;
}

static int grab(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static int heartbeat(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static int release(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static const displayPortVTable_t mspOsdVTable = {
    .grab = grab,
    .release = release,
    .clearScreen = clearScreen,
    .drawScreen = drawScreen,
    .screenSize = screenSize,
    .writeString = writeString,
    .writeChar = writeChar,
    .readChar = readChar,
    .isTransferInProgress = isTransferInProgress,
    .heartbeat = heartbeat,
    .resync = resync,
    .txBytesFree = txBytesFree,
    .supportedTextAttributes = supportedTextAttributes,
    .getFontMetadata = getFontMetadata,
    .isReady = isReady,
};

bool mspOsdSerialInit(void)
{
    static volatile uint8_t txBuffer[TX_BUFFER_SIZE];
    memset(&mspPort, 0, sizeof(mspPort_t));

    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP_OSD);
    if (portConfig) {
        serialPort_t *port = openSerialPort(portConfig->identifier, FUNCTION_MSP_OSD, NULL, NULL,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (port) {
            // Use a bigger TX buffer size to accommodate the configuration menus
            port->txBuffer = txBuffer;
            port->txBufferSize = TX_BUFFER_SIZE;
            port->txBufferTail = 0;
            port->txBufferHead = 0;

            resetMspPort(&mspPort, port);

            return true;
        }
    }

    return false;
}

displayPort_t* mspOsdDisplayPortInit(const videoSystem_e videoSystem)
{
    if (mspOsdSerialInit()) {
        init();
        displayInit(&mspOsdDisplayPort, &mspOsdVTable);
        osdVideoSystem = videoSystem;
        switch (videoSystem)
        {
        case VIDEO_SYSTEM_AUTO:
        case VIDEO_SYSTEM_PAL:
            screenRows = PAL_ROWS;
            screenCols = PAL_COLS;
            break;
        case VIDEO_SYSTEM_NTSC:
            screenRows = NTSC_ROWS;
            screenCols = NTSC_COLS;
            break;
        case VIDEO_SYSTEM_HDZERO:
            screenRows = HDZERO_ROWS;
            screenCols = HDZERO_COLS;
            break;
        case VIDEO_SYSTEM_DJIWTF:
            screenRows = DJI_ROWS;
            screenCols = DJI_COLS;
            break;
        default:
            break;
        }

        return &mspOsdDisplayPort;
    }
    return NULL;
}

/*
 * Intercept MSP processor.
 * VTX sends an MSP command every 125ms or so.
 * VTX will have be marked as not ready if no commands received within VTX_TIMEOUT.
 */
static mspResult_e processMspCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    if (vtxSeen && !vtxActive) {
        vtxReset = true;
    }

    vtxSeen = vtxActive = true;
    vtxHeartbeat = millis();

    // Process MSP command
    return mspProcessCommand(cmd, reply, mspPostProcessFn);
}

void mspOsdSerialProcess(mspProcessCommandFnPtr mspProcessCommandFn)
{
    if (mspPort.port) {
        mspProcessCommand = mspProcessCommandFn;
        mspSerialProcessOnePort(&mspPort, MSP_SKIP_NON_MSP_DATA, processMspCommand);
    }
}

#endif // USE_MSP_OSD
