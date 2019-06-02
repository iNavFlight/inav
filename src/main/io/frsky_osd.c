#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_FRSKYOSD)

#include "common/crc.h"
#include "common/log.h"
#include "common/time.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "io/frsky_osd.h"
#include "io/serial.h"

#define FRSKY_OSD_BAUDRATE 115200

#define FRSKY_OSD_PREAMBLE_BYTE_0 '$'
#define FRSKY_OSD_PREAMBLE_BYTE_1 'A'

#define FRSKY_OSD_RECV_BUFFER_SIZE 64

#define FRSKY_OSD_CMD_INFO 1
#define FRSKY_OSD_CMD_READ_FONT 2
#define FRSKY_OSD_CMD_WRITE_FONT 3
#define FRSKY_OSD_CMD_CLEAR 4
#define FRSKY_OSD_CMD_DRAW_GRID_CHAR 5
#define FRSKY_OSD_CMD_DRAW_GRID_STR 6

#define FRSKY_OSD_DEBUG(fmt, ...) LOG_D(OSD, fmt,  ##__VA_ARGS__)
#define FRSKY_OSD_ERROR(fmt, ...) LOG_E(OSD, fmt,  ##__VA_ARGS__)

typedef enum {
    RECV_STATE_NONE,
    RECV_STATE_SYNC,
    RECV_STATE_CMD,
    RECV_STATE_LENGTH,
    RECV_STATE_DATA,
    RECV_STATE_CHECKSUM,
    RECV_STATE_DONE,
} frskyOSDRecvState_e;

typedef struct frskyOSDInfoResponse_s {
    uint8_t magic[3];
    uint8_t versionMajor;
    uint8_t versionMinor;
    uint8_t versionPatch;
    uint8_t gridRows;
    uint8_t gridColumns;
    uint16_t pixelWidth;
    uint16_t pixelHeight;
} __attribute__((packed)) frskyOSDInfoResponse_t;

typedef struct frskyOSDFontCharacter_s {
    uint16_t addr;
    uint8_t data[54]; // 12x18 2bpp
} __attribute__((packed)) frskyOSDCharacter_t;

typedef struct frskyOSDDrawGridCharCmd_s {
    uint8_t gx;
    uint8_t gy;
    uint16_t chr;
    uint8_t attr;
} __attribute__((packed)) frskyOSDDrawGridCharCmd_t;

typedef struct frskyOSDState_s {
    struct {
        uint8_t state;
        uint8_t cmd;
        uint8_t crc;
        uint8_t expected;
        uint8_t data[FRSKY_OSD_RECV_BUFFER_SIZE];
        uint8_t pos;
    } recv_buffer;
    struct {
        uint8_t major;
        uint8_t minor;
        timeMs_t nextRequest;
        struct {
            uint8_t rows;
            uint8_t columns;
        } grid;
        struct {
            uint16_t width;
            uint16_t height;
        } viewport;
    } info;
    serialPort_t *port;
} frskyOSDState_t;

static frskyOSDState_t state;

static uint8_t frskyOSDChecksum(uint8_t crc, uint8_t c)
{
    return crc8_dvb_s2(crc, c);
}

static void frskyOSDResetReceiveBuffer(void)
{
    state.recv_buffer.state = RECV_STATE_NONE;
    state.recv_buffer.cmd = 0;
    state.recv_buffer.crc = 0;
    state.recv_buffer.expected = 0;
    state.recv_buffer.pos = 0;
}

static void frskyOSDStateReset(serialPort_t *port)
{
    frskyOSDResetReceiveBuffer();
    state.info.grid.rows = 0;
    state.info.grid.columns = 0;
    state.info.viewport.width = 0;
    state.info.viewport.height = 0;

    state.port = port;
}

static void frskyOSDUpdateReceiveBuffer(void)
{
    while (serialRxBytesWaiting(state.port) > 0) {
        uint8_t c = serialRead(state.port);
        switch ((frskyOSDRecvState_e)state.recv_buffer.state) {
            case RECV_STATE_NONE:
                if (c != FRSKY_OSD_PREAMBLE_BYTE_0) {
                    break;
                }
                state.recv_buffer.state = RECV_STATE_SYNC;
                break;
            case RECV_STATE_SYNC:
                if (c != FRSKY_OSD_PREAMBLE_BYTE_1) {
                    frskyOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.state = RECV_STATE_CMD;
                break;
            case RECV_STATE_CMD:
                state.recv_buffer.crc = frskyOSDChecksum(state.recv_buffer.crc, c);
                state.recv_buffer.cmd = c;
                state.recv_buffer.state = RECV_STATE_LENGTH;
                break;
            case RECV_STATE_LENGTH:
                if (c > sizeof(state.recv_buffer.data)) {
                    FRSKY_OSD_ERROR("Can't handle payload of size %u with a buffer of size %u",
                        c, sizeof(state.recv_buffer.data));
                    frskyOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.crc = frskyOSDChecksum(state.recv_buffer.crc, c);
                state.recv_buffer.expected = c;
                state.recv_buffer.state = c > 0 ? RECV_STATE_DATA : RECV_STATE_CHECKSUM;
                break;
            case RECV_STATE_DATA:
                state.recv_buffer.data[state.recv_buffer.pos++] = c;
                state.recv_buffer.crc = frskyOSDChecksum(state.recv_buffer.crc, c);
                if (state.recv_buffer.pos == state.recv_buffer.expected) {
                    state.recv_buffer.state = RECV_STATE_CHECKSUM;
                }
                break;
            case RECV_STATE_CHECKSUM:
                if (c != state.recv_buffer.crc) {
                    FRSKY_OSD_DEBUG("Checksum error %u != %u. Discarding %u bytes",
                        c, state.recv_buffer.crc, state.recv_buffer.pos);
                    frskyOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.state = RECV_STATE_DONE;
                break;
            case RECV_STATE_DONE:
                FRSKY_OSD_DEBUG("Received unexpected byte %u after data", c);
                break;
        }
    }
}

static bool frskyOSDIsResponseAvailable(void)
{
    return state.recv_buffer.state == RECV_STATE_DONE;
}

static bool frskyOSDHandleCommand(void)
{
    const void *data = state.recv_buffer.data;
    switch (state.recv_buffer.cmd) {
        case FRSKY_OSD_CMD_INFO:
            if (state.recv_buffer.expected >= sizeof(frskyOSDInfoResponse_t)) {
                const frskyOSDInfoResponse_t *resp = data;
                if (resp->magic[0] != 'A' || resp->magic[1] != 'G' || resp->magic[2] != 'H') {
                    FRSKY_OSD_ERROR("Invalid magic number %x %x %x, expecting AGH",
                        resp->magic[0], resp->magic[1], resp->magic[2]);
                    return false;
                }
                state.info.major = resp->versionMajor;
                state.info.minor = resp->versionMinor;
                state.info.grid.rows = resp->gridRows;
                state.info.grid.columns = resp->gridColumns;
                state.info.viewport.width = resp->pixelWidth;
                state.info.viewport.height = resp->pixelHeight;
                FRSKY_OSD_DEBUG("FrSky OSD initialized. Version %u.%u.%u, pixels=%ux%u, grid=%ux%u",
                    resp->versionMajor, resp->versionMinor, resp->versionPatch,
                    resp->pixelWidth, resp->pixelHeight, resp->gridColumns, resp->gridRows);
                return true;
            }
            break;
    }
    return false;
}

static void frskyOSDDispatchCommand(void)
{
    if (!frskyOSDHandleCommand()) {
        FRSKY_OSD_DEBUG("Discarding unknown command %u (%u bytes)",
            state.recv_buffer.cmd, state.recv_buffer.pos);
    }
    frskyOSDResetReceiveBuffer();
}

static void frskyOSDClearReceiveBuffer(void)
{
    frskyOSDUpdateReceiveBuffer();

    if (frskyOSDIsResponseAvailable()) {
        frskyOSDDispatchCommand();
    } else if (state.recv_buffer.pos > 0) {
        FRSKY_OSD_DEBUG("Discarding receive buffer with %u bytes", state.recv_buffer.pos);
        frskyOSDResetReceiveBuffer();
    }
}

static void frskyOSDProcessCommandU8(uint8_t *crc, uint8_t c)
{
    while (serialTxBytesFree(state.port) == 0) {
    };
    serialWrite(state.port, c);
    if (crc) {
        *crc = crc8_dvb_s2(*crc, c);
    }
}

static void frskyOSDProcessCommandU16(uint8_t *crc, uint16_t c)
{
    frskyOSDProcessCommandU8(crc, c & 0xFF);
    frskyOSDProcessCommandU8(crc, c >> 8);
}

static void frskyOSDSendPreamble(uint8_t *crc, uint8_t cmd)
{
    // TODO: Implement uvarint
    frskyOSDProcessCommandU8(NULL, FRSKY_OSD_PREAMBLE_BYTE_0);
    frskyOSDProcessCommandU8(NULL, FRSKY_OSD_PREAMBLE_BYTE_1);
    frskyOSDProcessCommandU8(crc, cmd);
}

static void frskyOSDSendAsyncCommand(uint8_t cmd, const void *data, size_t size)
{
    FRSKY_OSD_DEBUG("Send async cmd %u", cmd);
    uint8_t crc = 0;

    frskyOSDSendPreamble(&crc, cmd);

    if (data && size > 0) {
        frskyOSDProcessCommandU8(&crc, size & 0x7F);
        const uint8_t *p = data;
        const uint8_t *end = p + size;
        for(; p != end; p++) {
            frskyOSDProcessCommandU8(&crc, *p);
        }
    } else {
        frskyOSDProcessCommandU8(&crc, 0);
    }
    frskyOSDProcessCommandU8(NULL, crc);
}

static bool frskyOSDSendSyncCommand(uint8_t cmd, const void *data, size_t size, timeMs_t timeout)
{
    frskyOSDClearReceiveBuffer();
    frskyOSDSendAsyncCommand(cmd, data, size);
    timeMs_t end = millis() + timeout;
    while (millis() < end) {
        frskyOSDUpdateReceiveBuffer();
        if (frskyOSDIsResponseAvailable()) {
            FRSKY_OSD_DEBUG("Got sync response");
            return true;
        }
    }
    FRSKY_OSD_DEBUG("Sync response failed");
    return false;
}

static void frskyOSDRequestInfo(void)
{
    timeMs_t now = millis();
    if (state.info.nextRequest < now) {
        frskyOSDSendAsyncCommand(FRSKY_OSD_CMD_INFO, NULL, 0);
        state.info.nextRequest = now + 1000;
    }
}

bool frskyOSDInit(videoSystem_e videoSystem)
{
    // TODO: Use videoSystem to set the signal standard when
    // no input is detected.
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_FRSKY_OSD);
    if (portConfig) {
        portOptions_t portOptions = 0;
        serialPort_t *port = openSerialPort(portConfig->identifier,
            FUNCTION_FRSKY_OSD, NULL, NULL, FRSKY_OSD_BAUDRATE,
            MODE_RXTX, portOptions);

        if (port) {
            frskyOSDStateReset(port);
            frskyOSDRequestInfo();
            return true;
        }
    }
    return false;
}

bool frskyOSDIsReady(void)
{
    return state.info.minor > 0 || state.info.major > 0;
}

void frskyOSDUpdate(void)
{
    if (!state.port) {
        return;
    }
    frskyOSDUpdateReceiveBuffer();

    if (frskyOSDIsResponseAvailable()) {
        frskyOSDDispatchCommand();
    }

    if (!frskyOSDIsReady()) {
        // Info not received yet
        frskyOSDRequestInfo();
    }
}

bool frskyOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr)
{
    if (!frskyOSDIsReady()) {
        return false;
    }

    uint16_t addr = char_address;

    // 500ms should be more than enough to receive ~60 bytes @ 115200 bps
    if (frskyOSDSendSyncCommand(FRSKY_OSD_CMD_READ_FONT, &addr, sizeof(addr), 500)) {
        FRSKY_OSD_DEBUG("CMD %d got %u, expect %u", state.recv_buffer.cmd, state.recv_buffer.expected, sizeof(*chr) + sizeof(addr));
        if (state.recv_buffer.cmd != FRSKY_OSD_CMD_READ_FONT ||
            state.recv_buffer.expected < sizeof(*chr) + sizeof(addr)) {

            frskyOSDResetReceiveBuffer();
            FRSKY_OSD_DEBUG("Bad font character at position %u", char_address);
            return false;
        }
        // Skip character address
        memcpy(chr, &state.recv_buffer.data[2], sizeof(*chr));
        frskyOSDResetReceiveBuffer();
        return true;
    }
    return false;
}

bool frskyOSDWriteFontCharacter(unsigned char_address, const osdCharacter_t *chr)
{
    if (!frskyOSDIsReady()) {
        return false;
    }

    frskyOSDCharacter_t c;
    STATIC_ASSERT(sizeof(*chr) == sizeof(c.data), invalid_character_size);

    memcpy(c.data, chr, sizeof(c.data));
    c.addr = char_address;
    FRSKY_OSD_DEBUG("WRITE FONT CHR %u", char_address);
    frskyOSDSendSyncCommand(FRSKY_OSD_CMD_WRITE_FONT, &c, sizeof(c), 1000);
//    frskyOSDSendAsyncCommand(FRSKY_OSD_CMD_WRITE_FONT, &c, sizeof(c));
    // Wait until all bytes have been sent. Otherwise uploading
    // the very last character of the font might fail.
    // TODO: Investigate if we can change the max7456 handling to
    // stop rebooting when a font is uploaded
    //waitForSerialPortToFinishTransmitting(state.port);
    return true;
}

unsigned frskyOSDGetGridRows(void)
{
    return state.info.grid.rows;
}

unsigned frskyOSDGetGridCols(void)
{
    return state.info.grid.columns;
}

void frskyOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr)
{
    uint8_t crc = 0;
    frskyOSDSendPreamble(&crc, FRSKY_OSD_CMD_DRAW_GRID_STR);
    uint8_t size = 1 + 1 + strlen(buff) + 1;
    if (attr != 0) {
        size += 1;
    }
    frskyOSDProcessCommandU8(&crc, size);
    frskyOSDProcessCommandU8(&crc, x);
    frskyOSDProcessCommandU8(&crc, y);
    for (const char *p = buff; *p; p++) {
        frskyOSDProcessCommandU8(&crc, *p);
    }
    // Terminate string
    frskyOSDProcessCommandU8(&crc, 0);
    if (attr != 0) {
        frskyOSDProcessCommandU8(&crc, attr);
    }
    frskyOSDProcessCommandU8(NULL, crc);
}

void frskyOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr)
{
    uint8_t crc = 0;
    frskyOSDSendPreamble(&crc, FRSKY_OSD_CMD_DRAW_GRID_CHAR);
    uint8_t size = 1 + 1 + 2;
    if (attr != 0) {
        size += 1;
    }
    frskyOSDProcessCommandU8(&crc, size);
    frskyOSDProcessCommandU8(&crc, x);
    frskyOSDProcessCommandU8(&crc, y);
    frskyOSDProcessCommandU16(&crc, chr);
    if (attr != 0) {
        frskyOSDProcessCommandU8(&crc, attr);
    }
    frskyOSDProcessCommandU8(NULL, crc);
}

void frskyOSDClearScreen(void)
{
    frskyOSDSendAsyncCommand(FRSKY_OSD_CMD_CLEAR, NULL, 0);
}

#endif
