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

#define USE_OSD
#define USE_HDZERO_OSD
#if defined(USE_OSD) && defined(USE_HDZERO_OSD)

#include "common/utils.h"
#include "common/printf.h"
#include "common/time.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/osd_symbols.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

#include "displayport_hdzero_osd.h"

#define MSP_HEARTBEAT 0
#define MSP_RELEASE 1
#define MSP_CLEAR_SCREEN 2
#define MSP_WRITE_STRING 3
#define MSP_DRAW_SCREEN 4
#define MSP_SET_HD 5

#define FONT_PAGE_ATTRIBUTE 0x01

static mspPort_t hdzeroMspPort;
static displayPort_t hdzeroOsdDisplayPort;
static bool hdzeroVtxReady;

// HD screen size
#define ROWS 18
#define COLS 50
#define SCREENSIZE (ROWS*COLS)
static uint8_t screen[SCREENSIZE];
static uint8_t fontPage[SCREENSIZE / 8 + 1]; // page bits for each character (to address 512 char font)

extern uint8_t cliMode;

static int output(displayPort_t *displayPort, uint8_t cmd, uint8_t *subcmd, int len)
{
    UNUSED(displayPort);

    if (cliMode)
        return 0;

    return mspSerialPushPort(cmd, subcmd, len, &hdzeroMspPort, MSP_V1);
}

static int heartbeat(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_HEARTBEAT };
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static int release(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_RELEASE };
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static int clearScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);

    memset(screen, SYM_BLANK, sizeof(screen));
    memset(fontPage, 0, sizeof(fontPage));
    return 1;
}

/*
 * Write up to three populated rows at a time, skipping blank lines.
 * This gives a refresh rate to the VTX of approximately 10 to 62Hz
 * depending on how much data is displayed.
 */
static int drawScreen(displayPort_t *displayPort) // 62.5hz
{
    static uint8_t row = 0, clearSent = 0;
    uint8_t subcmd[COLS + 4], len, col, page, aPage, rowsToPrint;
    uint16_t lineIdx, idx, end;
    int charsOut = 0;

    rowsToPrint = 3;
    do {
        // Find a row with something to print
        do {
            // Strip leading and trailing spaces for the selected row
            lineIdx = row * COLS;
            idx = lineIdx;
            end = idx + COLS - 1;

            while ((screen[idx] == SYM_BLANK || screen[end] == SYM_BLANK) && idx <= end) {
                if (screen[idx] == SYM_BLANK)
                    idx++;
                if (screen[end] == SYM_BLANK)
                    end--;
            }
        } while (idx > end && ++row < ROWS);

        while (idx <= end) {
            if (!clearSent) {
                // Start the transaction
                subcmd[0] = MSP_CLEAR_SCREEN;
                charsOut += output(displayPort, MSP_DISPLAYPORT, subcmd, 1);
                clearSent = 1;
            }

            // Split the line up into strings from the same font page and output them.
            // (note spaces are printed to save overhead on small elements)
            len = 4;
            col = idx - lineIdx;
            page = (fontPage[idx >> 3] >> (idx & 0x07)) & FONT_PAGE_ATTRIBUTE;

            do {
                subcmd[len++] = screen[idx++];
                aPage = (fontPage[idx >> 3] >> (idx & 0x07)) & FONT_PAGE_ATTRIBUTE;
            } while (idx <= end && (aPage == page || screen[idx] == SYM_BLANK));

            subcmd[0] = MSP_WRITE_STRING;
            subcmd[1] = row;
            subcmd[2] = col;
            subcmd[3] = page;
            charsOut += output(displayPort, MSP_DISPLAYPORT, subcmd, len);
        }
    } while (++row < ROWS && --rowsToPrint);

    if (row >= ROWS) {
        // End the transaction if required and reset the counters
        if (clearSent > 0) {
            subcmd[0] = MSP_DRAW_SCREEN;
            charsOut += output(displayPort, MSP_DISPLAYPORT, subcmd, 1);
        }
        row = clearSent = 0;
    }

    return charsOut;
}

static int setHdMode(displayPort_t *displayPort)
{
    uint8_t subcmd[3];
    subcmd[0] = MSP_SET_HD;
    subcmd[1] = 0; // future font index
    subcmd[2] = 1; // 0 SD 1 HD

    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static int grab(displayPort_t *displayPort)
{
    return heartbeat(displayPort);
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    return SCREENSIZE;
}

// Intercept writeString and write to a buffer instead (1st page of font file only)
static int writeString(displayPort_t *displayPort, uint8_t col, uint8_t row, const char *string, textAttributes_t attr)
{
    UNUSED(displayPort);
    UNUSED(attr);

    uint16_t i, pos, len, end, idx;

    pos = (row * COLS) + col;
    if (pos >= SCREENSIZE)
        return 0;

    len = strlen(string);

    // Allow word wrap and truncate of past the screen end
    end = pos + len - 1;
    if (end >= SCREENSIZE)
        len = end - SCREENSIZE;

    // Copy the string into the screen buffer
    memcpy(screen + pos, string, len);

    // Clear the page bits for all the characters in the string
    for (i = 0; i < len; i++) {
        idx = pos + i;
        fontPage[idx >> 3] &= ~(1 << (idx & 0x07));
    }

    return (int) len;
}

// Write character to screen and page buffers (supports 512 char fonts)
static int writeChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);
    UNUSED(attr);

    uint16_t pos, idx;
    uint8_t bitmask;

    pos = (row * COLS) + col;
    if (pos >= SCREENSIZE)
        return 0;

    // Copy character into screen buffer
    screen[pos] = c;

    idx = pos >> 3;
    bitmask = 1 << (pos & 0x07);

    // Save index of the character's font page
    if (c & 0x0100)
        fontPage[idx] |= bitmask;
    else
        fontPage[idx] &= ~bitmask;

    return (int) 1;
}

static bool readChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t *c, textAttributes_t *attr)
{
    UNUSED(displayPort);

    uint16_t pos, chr;

    pos = (row * COLS) + col;
    if (pos >= SCREENSIZE)
        *c = SYM_BLANK;
    else {
        chr = (fontPage[pos >> 3] >> (pos & 0x07)) & FONT_PAGE_ATTRIBUTE;
        *c = (chr << 8) | screen[pos];
    }

    if (attr)
        *attr = TEXT_ATTRIBUTES_NONE;

    return true;
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    return false;
}

static void resync(displayPort_t *displayPort)
{
    displayPort->rows = ROWS;
    displayPort->cols = COLS;
    setHdMode(displayPort);
}

static uint32_t txBytesFree(const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    return mspSerialTxBytesFree();
}

static textAttributes_t supportedTextAttributes(const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    textAttributes_t attr = TEXT_ATTRIBUTES_NONE;
    //TEXT_ATTRIBUTES_ADD_INVERTED(attr);
    //TEXT_ATTRIBUTES_ADD_SOLID_BG(attr);
    return attr;
}

static bool getFontMetadata(displayFontMetadata_t *metadata, const displayPort_t *displayPort)
{
    UNUSED(displayPort);

    metadata->charCount = 512;
    metadata->version = 1;

    return true;
}

static bool isReady(displayPort_t *displayPort)
{
    UNUSED(displayPort);

    return hdzeroVtxReady;
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

void hdzeroOsdSerialInit(void)
{
    memset(&hdzeroMspPort, 0, sizeof(mspPort_t));

    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_HDZERO_OSD);

    if (portConfig) {
        serialPort_t *port = openSerialPort(portConfig->identifier, FUNCTION_HDZERO_OSD, NULL, NULL,
                baudRates[portConfig->msp_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);
        if (port)
            resetMspPort(&hdzeroMspPort, port);
    }
}

displayPort_t* hdzeroOsdDisplayPortInit(void)
{
    memset(screen, SYM_BLANK, sizeof(screen));
    memset(fontPage, 0, sizeof(fontPage));
    displayInit(&hdzeroOsdDisplayPort, &hdzeroOsdVTable);
    return &hdzeroOsdDisplayPort;
}

void hdzeroOsdSerialProcess(mspEvaluateNonMspData_e evaluateNonMspData, mspProcessCommandFnPtr mspProcessCommandFn)
{
    if (hdzeroMspPort.port) {
        // Process normal MSP command
        mspSerialProcessOnePort(&hdzeroMspPort, evaluateNonMspData, mspProcessCommandFn);
        hdzeroVtxReady = true;
    }
}

#endif // USE_HDZERO_OSD
