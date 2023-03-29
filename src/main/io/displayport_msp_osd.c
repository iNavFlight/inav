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

#if defined(USE_OSD) && defined(USE_MSP_OSD)

#include "common/utils.h"
#include "common/printf.h"
#include "common/time.h"
#include "common/bitarray.h"

#include "cms/cms.h"

#include "drivers/display.h"
#include "drivers/display_font_metadata.h"
#include "drivers/osd_symbols.h"

#include "fc/rc_modes.h"

#include "io/osd.h"
#include "io/displayport_msp.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

#include "displayport_msp_osd.h"
#include "displayport_msp_bf_compat.h"

#define FONT_VERSION 3

typedef enum {          // defines are from hdzero code
    SD_3016,
    HD_5018,
    HD_3016,           // Special HDZERO mode that just sends the centre 30x16 of the 50x18 canvas to the VRX
    HD_6022,            // added to support DJI wtfos 60x22 grid
    HD_5320            // added to support Avatar and BetaflightHD
} resolutionType_e;

#define DRAW_FREQ_DENOM 4 // 60Hz
#define TX_BUFFER_SIZE 1024
#define VTX_TIMEOUT 1000 // 1 second timer

static mspProcessCommandFnPtr mspProcessCommand;
static mspPort_t mspPort;
static displayPort_t mspOsdDisplayPort;
static bool vtxSeen, vtxActive, vtxReset;
static timeMs_t vtxHeartbeat;
static timeMs_t sendSubFrameMs = 0;

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
#define AVATAR_COLS 53
#define AVATAR_ROWS 20
// DJIWTF screen size
#define DJI_COLS 60
#define DJI_ROWS 22

// set COLS and ROWS to largest size available
#define COLS DJI_COLS
#define ROWS DJI_ROWS

// set screen size
#define SCREENSIZE (ROWS*COLS)

static uint8_t currentOsdMode; // HDZero screen mode can change across layouts

static uint8_t screen[SCREENSIZE];
static BITARRAY_DECLARE(fontPage, SCREENSIZE);  // font page for each character on the screen
static BITARRAY_DECLARE(dirty, SCREENSIZE);     // change status for each character on the screen
static BITARRAY_DECLARE(blinkChar, SCREENSIZE); // Does the character blink?
static bool screenCleared;
static uint8_t screenRows, screenCols;
static videoSystem_e osdVideoSystem;

extern uint8_t cliMode;

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
        sent = mspSerialPushPort(cmd, subcmd, len, &mspPort, MSP_V1);
    }

    return sent;
}

static uint8_t determineHDZeroOsdMode(void)
{
    if (cmsInMenu) {
        return HD_5018;
    }

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
    if (osdVideoSystem == VIDEO_SYSTEM_HDZERO) {
        currentOsdMode = determineHDZeroOsdMode(); // Can change between layouts
    }

    uint8_t subcmd[] = { MSP_DP_OPTIONS, 0, currentOsdMode }; // Font selection, mode (SD/HD)
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static void init(void)
{
    memset(screen, SYM_BLANK, sizeof(screen));
    BITARRAY_CLR_ALL(fontPage);
    BITARRAY_CLR_ALL(dirty);
    BITARRAY_CLR_ALL(blinkChar);
}

static int clearScreen(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_DP_CLEAR_SCREEN };

    if (!cmsInMenu && IS_RC_MODE_ACTIVE(BOXOSD)) { // OSD is off
        output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
        subcmd[0] = MSP_DP_DRAW_SCREEN;
        vtxReset = true;
    }
    else {
        init();
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

static int setChar(const uint16_t pos, const uint16_t c, textAttributes_t attr)
{
    if (pos < SCREENSIZE) {
        uint8_t ch = c & 0xFF;
        bool page = (c >> 8);
        if (screen[pos] != ch || bitArrayGet(fontPage, pos) != page) {
            screen[pos] = ch;
            (page) ? bitArraySet(fontPage, pos) : bitArrayClr(fontPage, pos);
            (TEXT_ATTRIBUTES_HAVE_BLINK(attr)) ? bitArraySet(blinkChar, pos) : bitArrayClr(blinkChar, pos);
            bitArraySet(dirty, pos);
        }
    }
    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t col, uint8_t row, uint16_t c, textAttributes_t attr)
{
    UNUSED(displayPort);

    return setChar((row * COLS) + col, c, attr);
}

static int writeString(displayPort_t *displayPort, uint8_t col, uint8_t row, const char *string, textAttributes_t attr)
{
    UNUSED(displayPort);

    uint16_t pos = (row * COLS) + col;
    while (*string) {
        setChar(pos++, *string++, attr);
    }
    return 0;
}

/**
 * Write only changed characters to the VTX
 */
static int drawScreen(displayPort_t *displayPort) // 250Hz
{
    static uint8_t counter = 0;

    if ((!cmsInMenu && IS_RC_MODE_ACTIVE(BOXOSD)) || (counter++ % DRAW_FREQ_DENOM)) { // 62.5Hz
        return 0;
    }

    if (osdConfig()->msp_displayport_fullframe_interval >= 0 && (millis() > sendSubFrameMs)) {
        // For full frame update, first clear the OSD completely
        uint8_t refreshSubcmd[1];
        refreshSubcmd[0] = MSP_DP_CLEAR_SCREEN;
        output(displayPort, MSP_DISPLAYPORT, refreshSubcmd, sizeof(refreshSubcmd));
        
        // Then dirty the characters that are not blank, to send all data on this draw.
        for (unsigned int pos = 0; pos < sizeof(screen); pos++) {
            if (screen[pos] != SYM_BLANK) {
                bitArraySet(dirty, pos);
            }
        }
            
        sendSubFrameMs = (osdConfig()->msp_displayport_fullframe_interval > 0) ? (millis() + DS2MS(osdConfig()->msp_displayport_fullframe_interval)) : 0;
    }

    uint8_t subcmd[COLS + 4];
    uint8_t updateCount = 0;
    subcmd[0] = MSP_DP_WRITE_STRING;

    int next = BITARRAY_FIND_FIRST_SET(dirty, 0);
    while (next >= 0) {
        // Look for sequential dirty characters on the same line for the same font page
        int pos = next;
        uint8_t row = pos / COLS;
        uint8_t col = pos % COLS;
        uint8_t attributes = 0;
        int endOfLine = row * COLS + screenCols;
        bool page = bitArrayGet(fontPage, pos);
        bool blink = bitArrayGet(blinkChar, pos);

        uint8_t len = 4;
        do {
            bitArrayClr(dirty, pos);
            subcmd[len++] = isBfCompatibleVideoSystem(osdConfig()) ? getBfCharacter(screen[pos++], page): screen[pos++];

            if (bitArrayGet(dirty, pos)) {
                next = pos;
            }
        } while (next == pos && next < endOfLine && bitArrayGet(fontPage, next) == page && bitArrayGet(blinkChar, next) == blink);

        if (!isBfCompatibleVideoSystem(osdConfig())) {
            attributes |= (page << DISPLAYPORT_MSP_ATTR_FONTPAGE);
        }

        if (blink) {
            attributes |= (1 << DISPLAYPORT_MSP_ATTR_BLINK);
        }

        subcmd[1] = row;
        subcmd[2] = col;
        subcmd[3] = attributes;
        output(displayPort, MSP_DISPLAYPORT, subcmd, len);
        updateCount++;
        next = BITARRAY_FIND_FIRST_SET(dirty, pos);
    }

    if (updateCount > 0 || screenCleared) {
        if (screenCleared) {
            screenCleared = false;
        }

        subcmd[0] = MSP_DP_DRAW_SCREEN;
        output(displayPort, MSP_DISPLAYPORT, subcmd, 1);
    }

    if (vtxReset) {
        clearScreen(displayPort);
        vtxReset = false;
    }

return 0;
}

static void resync(displayPort_t *displayPort)
{
    displayPort->rows = screenRows;
    displayPort->cols = screenCols;
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

static int heartbeat(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_DP_HEARTBEAT };

    // heartbeat is used to:
    // a) ensure display is not released by MW OSD software
    // b) prevent OSD Slave boards from displaying a 'disconnected' status.
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
}

static int grab(displayPort_t *displayPort)
{
    return heartbeat(displayPort);
}

static int release(displayPort_t *displayPort)
{
    uint8_t subcmd[] = { MSP_DP_RELEASE };
    return output(displayPort, MSP_DISPLAYPORT, subcmd, sizeof(subcmd));
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
        switch(videoSystem) {
        case VIDEO_SYSTEM_AUTO:
        case VIDEO_SYSTEM_BFCOMPAT:
        case VIDEO_SYSTEM_PAL:
            currentOsdMode = SD_3016;
            screenRows = PAL_ROWS;
            screenCols = PAL_COLS;
            break;
        case VIDEO_SYSTEM_NTSC:
            currentOsdMode = SD_3016;
            screenRows = NTSC_ROWS;
            screenCols = NTSC_COLS;
            break;
        case VIDEO_SYSTEM_HDZERO:
            currentOsdMode = HD_5018;
            screenRows = HDZERO_ROWS;
            screenCols = HDZERO_COLS;
            break;
        case VIDEO_SYSTEM_DJIWTF:
            currentOsdMode = HD_6022;
            screenRows = DJI_ROWS;
            screenCols = DJI_COLS;
            break;
        case VIDEO_SYSTEM_BFCOMPAT_HD:
        case VIDEO_SYSTEM_AVATAR:
            currentOsdMode = HD_5320;
            screenRows = AVATAR_ROWS;
            screenCols = AVATAR_COLS;
            break;
        default:
            break;
        }

        osdVideoSystem = videoSystem;
        init();
        displayInit(&mspOsdDisplayPort, &mspOsdVTable);

        if (osdVideoSystem == VIDEO_SYSTEM_BFCOMPAT) {
            mspOsdDisplayPort.displayPortType = "MSP DisplayPort: BetaFlight Compatability mode";
        } else if (osdVideoSystem == VIDEO_SYSTEM_BFCOMPAT_HD) {
            mspOsdDisplayPort.displayPortType = "MSP DisplayPort: BetaFlight Compatability mode (HD)";
        } else {
            mspOsdDisplayPort.displayPortType = "MSP DisplayPort";
        }

        return &mspOsdDisplayPort;
    }
    return NULL;
}

/*
 * Intercept MSP processor.
 * VTX sends an MSP command every 125ms or so.
 * VTX will have be marked as not ready if no commands received within VTX_TIMEOUT ms.
 */
static mspResult_e processMspCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    if ((vtxSeen && !vtxActive) || (cmd->cmd == MSP_EEPROM_WRITE)) {
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
