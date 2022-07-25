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

//#define HDZERO_STATS

#if defined(USE_OSD) && defined(USE_HDZERO_OSD)

#include "common/utils.h"
#include "common/printf.h"
#include "common/time.h"
#include "common/bitarray.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/osd_symbols.h"

#include "fc/rc_modes.h"

#include "io/osd.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

#include "displayport_hdzero_osd.h"

#define FONT_VERSION 3

#define MSP_CLEAR_SCREEN 2
#define MSP_WRITE_STRING 3
#define MSP_DRAW_SCREEN 4
#define MSP_SET_OPTIONS 5
#define DRAW_FREQ_DENOM 4 // approx. 60Hz
#define TX_BUFFER_SIZE 1024
#define VTX_TIMEOUT 1000 // 1 second timer

static mspProcessCommandFnPtr mspProcessCommand;
static mspPort_t hdZeroMspPort;
static displayPort_t hdZeroOsdDisplayPort;
static bool vtxSeen, vtxActive, vtxReset;
static timeMs_t vtxHeartbeat;

// Screen modes
static uint8_t osdMode;
#define HD_5018 1
#define HD_3016 2 // Special mode that just sends the centre 30x16 of the 50x18 canvas to the VRX.
#define ROWS 18
#define COLS 50
#define SCREENSIZE (ROWS*COLS)

static uint8_t screen[SCREENSIZE];
static BITARRAY_DECLARE(fontPage, SCREENSIZE); // font page for each character on the screen
static BITARRAY_DECLARE(dirty, SCREENSIZE); // change status for each character on the screen
static bool screenCleared;

extern uint8_t cliMode;

#ifdef HDZERO_STATS
static uint32_t dataSent;
static timeMs_t maxVtxAbsence;
static uint8_t resetCount;
#endif

static void checkVtxPresent(void)
{
    if (vtxActive && (millis()-vtxHeartbeat) > VTX_TIMEOUT) {
        vtxActive = false;
    }
}

static int output(displayPort_t *displayPort, uint8_t cmd, uint8_t *subcmd, int len)
{
    UNUSED(displayPort);

    checkVtxPresent();

    int sent = 0;
    if (!cliMode && vtxActive) {
        sent = mspSerialPushPort(cmd, subcmd, len, &hdZeroMspPort, MSP_V1);
    }

#ifdef HDZERO_STATS
    dataSent += sent;
#endif

    return sent;
}

static int chk = 0;

static uint8_t determineOsdMode(void)
{
    chk++;
    // Check if all visible widgets are in the center 30x16 chars of the canvas.
    int activeLayout = osdGetActiveLayout(NULL);
    osd_items_e index = 0;
    do {
        index = osdIncElementIndex(index);
        uint16_t pos = osdLayoutsConfig()->item_pos[activeLayout][index];
        if (OSD_VISIBLE(pos)) {
            uint8_t elemPosX = OSD_X(pos);
            uint8_t elemPosY = OSD_Y(pos);
            if (!osdItemIsFixed(index) && (elemPosX < 10 || elemPosX > 39 || elemPosY == 0 || elemPosY == 17)) {
                return HD_5018;
            }
        }
    } while (index > 0);

    return HD_3016;
}

static int setDisplayMode(displayPort_t *displayPort)
{
    osdMode = determineOsdMode();

    uint8_t subcmd[] = { MSP_SET_OPTIONS, 0,  osdMode }; // font selection, mode (SD/HD)
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static void hdZeroInit(void)
{
    osdMode = HD_5018;
    memset(screen, SYM_BLANK, sizeof(screen));
    BITARRAY_CLR_ALL(fontPage);
    BITARRAY_CLR_ALL(dirty);
}

static int clearScreen(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_CLEAR_SCREEN };

    if (IS_RC_MODE_ACTIVE(BOXOSD)) {
        output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
        subcmd[0] = MSP_DRAW_SCREEN;
        vtxReset = true;
    }
    else {
        hdZeroInit();
        setDisplayMode(displayPort);
        screenCleared = true;
    }

    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static bool readChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);

    uint16_t pos = (row * COLS) + col;
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

    return setChar((row * COLS) + col, c);
}

static int writeString(displayPort_t *displayPort, uint8_t col, uint8_t row, const char *string, textAttributes_t attr)
{
    UNUSED(displayPort);
    UNUSED(attr);

    uint16_t pos = (row * COLS) + col;
    while (*string) {
        setChar(pos++, *string++);
    }
    return 0;
}

#ifdef HDZERO_STATS
static void printStats(displayPort_t *displayPort, uint32_t updates)
{
    static timeMs_t lastTime;
    static uint32_t maxDataSent, maxBufferUsed, maxUpdates;
    timeMs_t currentTime = millis();
    char lineBuffer[31];

    if (updates > maxUpdates) {
        maxUpdates = updates; // updates sent per displayWrite
    }

    uint32_t bufferUsed = TX_BUFFER_SIZE - serialTxBytesFree(hdZeroMspPort.port);
    if (bufferUsed > maxBufferUsed) {
        maxBufferUsed = bufferUsed; // serial buffer used after displayWrite
    }

    tfp_sprintf(lineBuffer, "CHECKS: %d", chk);
    writeString(displayPort, 10, ((osdMode == HD_5018) ? 15 : 14), lineBuffer, 0);

    tfp_sprintf(lineBuffer, "RES:%2d HB:%4ld DAT:%5ld %5ld",
        resetCount, maxVtxAbsence, dataSent, maxDataSent);
    writeString(displayPort, 10, ((osdMode == HD_5018) ? 16 : 15), lineBuffer, 0);

    tfp_sprintf(lineBuffer, "UPD:%2ld %2ld BUF:%3ld %4ld %4ld %2s",
            updates, maxUpdates, bufferUsed, maxBufferUsed, hdZeroMspPort.port->txBufferSize,
            (osdMode == HD_5018) ? "HD" : "SD");
    writeString(displayPort, 10, ((osdMode == HD_5018) ? 17 : 16), lineBuffer, 0);

    uint32_t diff = (currentTime - lastTime);
    if (diff > 1000) { // Data sampled in 1 second
        if (dataSent > maxDataSent) {
            maxDataSent = dataSent; // bps (max 11520 allowed)
        }
        dataSent = 0;
        lastTime = currentTime;
    }
}
#endif

/**
 * Write only changed characters to the VTX
 */
static int drawScreen(displayPort_t *displayPort) // 250Hz
{
    static uint8_t counter = 0;

    if (IS_RC_MODE_ACTIVE(BOXOSD) || (counter++ % DRAW_FREQ_DENOM)) {
        return 0;
    }

    uint8_t subcmd[COLS + 4];
    uint8_t updateCount = 0;
    subcmd[0] = MSP_WRITE_STRING;

    int next = BITARRAY_FIND_FIRST_SET(dirty, 0);
    while (next >= 0) {
        // Look for sequential dirty characters on the same line for the same font page
        int pos = next;
        uint8_t row = pos / COLS;
        uint8_t col = pos % COLS;
        int endOfLine = row * COLS + COLS;
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

#ifdef HDZERO_STATS
    printStats(displayPort, updateCount);
#endif

    if (vtxReset) {
#ifdef HDZERO_STATS
        resetCount++;
#endif
        clearScreen(displayPort);
        vtxReset = false;
    }

return 0;
}

static void resync(displayPort_t *displayPort)
{
    displayPort->rows = ROWS;
    displayPort->cols = COLS;
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return SCREENSIZE;
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

static const displayPortVTable_t hdzeroOsdVTable = {
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

bool hdzeroOsdSerialInit(void)
{
    static volatile uint8_t txBuffer[TX_BUFFER_SIZE];
    memset(&hdZeroMspPort, 0, sizeof(mspPort_t));

    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_HDZERO_OSD);
    if (portConfig) {
        serialPort_t *port = openSerialPort(portConfig->identifier, FUNCTION_HDZERO_OSD, NULL, NULL,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (port) {
            // Use a bigger TX buffer size to accommodate the configuration menus
            port->txBuffer = txBuffer;
            port->txBufferSize = TX_BUFFER_SIZE;
            port->txBufferTail = 0;
            port->txBufferHead = 0;

            resetMspPort(&hdZeroMspPort, port);

            return true;
        }
    }

    return false;
}

displayPort_t* hdzeroOsdDisplayPortInit(void)
{
    if (hdzeroOsdSerialInit()) {
        hdZeroInit();
        displayInit(&hdZeroOsdDisplayPort, &hdzeroOsdVTable);
        return &hdZeroOsdDisplayPort;
    }
    return NULL;
}

/*
 * Intercept MSP processor.
 * VTX sends an MSP command every 125ms or so.
 * VTX will have be marked as not ready if no commands received within VTX_TIMEOUT ms.
 */
static mspResult_e hdZeroProcessMspCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    if ((vtxSeen && !vtxActive) || (cmd->cmd == MSP_EEPROM_WRITE)) {
        vtxReset = true;
    }
#ifdef HDZERO_STATS
    if (vtxSeen) {
        timeMs_t vtxAbsence = millis() - vtxHeartbeat;
        if (maxVtxAbsence < vtxAbsence)
        {
            maxVtxAbsence = vtxAbsence;
        }
    }
#endif
    vtxSeen = vtxActive = true;
    vtxHeartbeat = millis();

    // Process MSP command
    return mspProcessCommand(cmd, reply, mspPostProcessFn);
}

void hdzeroOsdSerialProcess(mspProcessCommandFnPtr mspProcessCommandFn)
{
    if (hdZeroMspPort.port) {
        mspProcessCommand = mspProcessCommandFn;
        mspSerialProcessOnePort(&hdZeroMspPort, MSP_SKIP_NON_MSP_DATA, hdZeroProcessMspCommand);
    }
}

#endif // USE_HDZERO_OSD
