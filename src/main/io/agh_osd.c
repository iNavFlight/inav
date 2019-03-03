#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_AGHOSD)

#include "build/debug.h"

#include "common/crc.h"
#include "common/time.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "io/agh_osd.h"
#include "io/serial.h"

#define AGH_OSD_BAUDRATE 115200

#define AGH_OSD_PREAMBLE_BYTE_0 '$'
#define AGH_OSD_PREAMBLE_BYTE_1 'A'

#define AGH_OSD_RECV_BUFFER_SIZE 64

#define AGH_OSD_CMD_INFO 1
#define AGH_OSD_CMD_READ_FONT 2
#define AGH_OSD_CMD_WRITE_FONT 3
#define AGH_OSD_CMD_CLEAR 4
#define AGH_OSD_CMD_DRAW_GRID_CHAR 5
#define AGH_OSD_CMD_DRAW_GRID_STR 6

#define AGH_OSD_DEBUG(fmt, ...) DEBUG_TRACE(fmt,  ##__VA_ARGS__)
#define AGH_OSD_ERROR(fmt, ...) DEBUG_TRACE(fmt,  ##__VA_ARGS__)

typedef enum {
    RECV_STATE_NONE,
    RECV_STATE_SYNC,
    RECV_STATE_CMD,
    RECV_STATE_LENGTH,
    RECV_STATE_DATA,
    RECV_STATE_CHECKSUM,
    RECV_STATE_DONE,
} aghOSDRecvState_e;

typedef struct aghOSDInfoResponse_s {
    uint8_t magic[3];
    uint8_t versionMajor;
    uint8_t versionMinor;
    uint8_t versionPatch;
    uint8_t gridRows;
    uint8_t gridColumns;
    uint16_t pixelWidth;
    uint16_t pixelHeight;
} __attribute__((packed)) aghOSDInfoResponse_t;

typedef struct aghOSDFontCharacter_s {
    uint16_t addr;
    uint8_t data[54]; // 12x18 2bpp
} __attribute__((packed)) aghOSDCharacter_t;

typedef struct aghOSDDrawGridCharCmd_s {
    uint8_t gx;
    uint8_t gy;
    uint16_t chr;
    uint8_t attr;
} __attribute__((packed)) aghOSDDrawGridCharCmd_t;

typedef struct aghOSDState_s {
    struct {
        uint8_t state;
        uint8_t cmd;
        uint8_t crc;
        uint8_t expected;
        uint8_t data[AGH_OSD_RECV_BUFFER_SIZE];
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
} aghOSDState_t;

static aghOSDState_t state;

static uint8_t aghOSDChecksum(uint8_t crc, uint8_t c)
{
    return crc8_dvb_s2(crc, c);
}

static void aghOSDResetReceiveBuffer(void)
{
    state.recv_buffer.state = RECV_STATE_NONE;
    state.recv_buffer.cmd = 0;
    state.recv_buffer.crc = 0;
    state.recv_buffer.expected = 0;
    state.recv_buffer.pos = 0;
}

static void aghOSDStateReset(serialPort_t *port)
{
    aghOSDResetReceiveBuffer();
    state.info.grid.rows = 0;
    state.info.grid.columns = 0;
    state.info.viewport.width = 0;
    state.info.viewport.height = 0;

    state.port = port;
}

static void aghOSDUpdateReceiveBuffer(void)
{
    while (serialRxBytesWaiting(state.port) > 0) {
        uint8_t c = serialRead(state.port);
        switch ((aghOSDRecvState_e)state.recv_buffer.state) {
            case RECV_STATE_NONE:
                if (c != AGH_OSD_PREAMBLE_BYTE_0) {
                    break;
                }
                state.recv_buffer.state = RECV_STATE_SYNC;
                break;
            case RECV_STATE_SYNC:
                if (c != AGH_OSD_PREAMBLE_BYTE_1) {
                    aghOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.state = RECV_STATE_CMD;
                break;
            case RECV_STATE_CMD:
                state.recv_buffer.crc = aghOSDChecksum(state.recv_buffer.crc, c);
                state.recv_buffer.cmd = c;
                state.recv_buffer.state = RECV_STATE_LENGTH;
                break;
            case RECV_STATE_LENGTH:
                if (c > sizeof(state.recv_buffer.data)) {
                    AGH_OSD_ERROR("Can't handle payload of size %u with a buffer of size %u",
                        c, sizeof(state.recv_buffer.data));
                    aghOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.crc = aghOSDChecksum(state.recv_buffer.crc, c);
                state.recv_buffer.expected = c;
                state.recv_buffer.state = c > 0 ? RECV_STATE_DATA : RECV_STATE_CHECKSUM;
                break;
            case RECV_STATE_DATA:
                state.recv_buffer.data[state.recv_buffer.pos++] = c;
                state.recv_buffer.crc = aghOSDChecksum(state.recv_buffer.crc, c);
                if (state.recv_buffer.pos == state.recv_buffer.expected) {
                    state.recv_buffer.state = RECV_STATE_CHECKSUM;
                }
                break;
            case RECV_STATE_CHECKSUM:
                if (c != state.recv_buffer.crc) {
                    AGH_OSD_DEBUG("Checksum error %u != %u. Discarding %u bytes",
                        c, state.recv_buffer.crc, state.recv_buffer.pos);
                    aghOSDResetReceiveBuffer();
                    break;
                }
                state.recv_buffer.state = RECV_STATE_DONE;
                break;
            case RECV_STATE_DONE:
                AGH_OSD_DEBUG("Received unexpected byte %u after data", c);
                break;
        }
    }
}

static bool aghOSDIsResponseAvailable(void)
{
    return state.recv_buffer.state == RECV_STATE_DONE;
}

static bool aghOSDHandleCommand(void)
{
    const void *data = state.recv_buffer.data;
    switch (state.recv_buffer.cmd) {
        case AGH_OSD_CMD_INFO:
            if (state.recv_buffer.expected >= sizeof(aghOSDInfoResponse_t)) {
                const aghOSDInfoResponse_t *resp = data;
                if (resp->magic[0] != 'A' || resp->magic[1] != 'G' || resp->magic[2] != 'H') {
                    AGH_OSD_ERROR("Invalid magic number %x %x %x, expecting AGH",
                        resp->magic[0], resp->magic[1], resp->magic[2]);
                    return false;
                }
                state.info.major = resp->versionMajor;
                state.info.minor = resp->versionMinor;
                state.info.grid.rows = resp->gridRows;
                state.info.grid.columns = resp->gridColumns;
                state.info.viewport.width = resp->pixelWidth;
                state.info.viewport.height = resp->pixelHeight;
                AGH_OSD_DEBUG("AGH OSD initialized. Version %u.%u.%u, pixels=%ux%u, grid=%ux%u",
                    resp->versionMajor, resp->versionMinor, resp->versionPatch,
                    resp->pixelWidth, resp->pixelHeight, resp->gridColumns, resp->gridRows);
                return true;
            }
            break;
    }
    return false;
}

static void aghOSDDispatchCommand(void)
{
    if (!aghOSDHandleCommand()) {
        AGH_OSD_DEBUG("Discarding unknown command %u (%u bytes)",
            state.recv_buffer.cmd, state.recv_buffer.pos);
    }
    aghOSDResetReceiveBuffer();
}

static void aghOSDClearReceiveBuffer(void)
{
    aghOSDUpdateReceiveBuffer();

    if (aghOSDIsResponseAvailable()) {
        aghOSDDispatchCommand();
    } else if (state.recv_buffer.pos > 0) {
        AGH_OSD_DEBUG("Discarding receive buffer with %u bytes", state.recv_buffer.pos);
        aghOSDResetReceiveBuffer();
    }
}

static void aghOSDProcessCommandU8(uint8_t *crc, uint8_t c)
{
    while (serialTxBytesFree(state.port) == 0) {
    };
    serialWrite(state.port, c);
    if (crc) {
        *crc = crc8_dvb_s2(*crc, c);
    }
}

static void aghOSDProcessCommandU16(uint8_t *crc, uint16_t c)
{
    aghOSDProcessCommandU8(crc, c & 0xFF);
    aghOSDProcessCommandU8(crc, c >> 8);
}

static void aghOSDSendPreamble(uint8_t *crc, uint8_t cmd)
{
    // TODO: Implement uvarint
    aghOSDProcessCommandU8(NULL, AGH_OSD_PREAMBLE_BYTE_0);
    aghOSDProcessCommandU8(NULL, AGH_OSD_PREAMBLE_BYTE_1);
    aghOSDProcessCommandU8(crc, cmd);
}

static void aghOSDSendAsyncCommand(uint8_t cmd, const void *data, size_t size)
{
    AGH_OSD_DEBUG("Send async cmd %u", cmd);
    uint8_t crc = 0;

    aghOSDSendPreamble(&crc, cmd);

    if (data && size > 0) {
        aghOSDProcessCommandU8(&crc, size & 0x7F);
        const uint8_t *p = data;
        const uint8_t *end = p + size;
        for(; p != end; p++) {
            aghOSDProcessCommandU8(&crc, *p);
        }
    } else {
        aghOSDProcessCommandU8(&crc, 0);
    }
    aghOSDProcessCommandU8(NULL, crc);
}

static bool aghOSDSendSyncCommand(uint8_t cmd, const void *data, size_t size, timeMs_t timeout)
{
    aghOSDClearReceiveBuffer();
    aghOSDSendAsyncCommand(cmd, data, size);
    timeMs_t end = millis() + timeout;
    while (millis() < end) {
        aghOSDUpdateReceiveBuffer();
        if (aghOSDIsResponseAvailable()) {
            return true;
        }
    }
    return false;
}

static void aghOSDRequestInfo(void)
{
    timeMs_t now = millis();
    if (state.info.nextRequest < now) {
        aghOSDSendAsyncCommand(AGH_OSD_CMD_INFO, NULL, 0);
        state.info.nextRequest = now + 1000;
    }
}

bool aghOSDInit(videoSystem_e videoSystem)
{
    // TODO: Use videoSystem to set the signal standard when
    // no input is detected.
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_AGH_OSD);
    if (portConfig) {
        portOptions_t portOptions = 0;
        serialPort_t *port = openSerialPort(portConfig->identifier,
            FUNCTION_AGH_OSD, NULL, NULL, AGH_OSD_BAUDRATE,
            MODE_RXTX, portOptions);

        if (port) {
            aghOSDStateReset(port);
            aghOSDRequestInfo();
            return true;
        }
    }
    return false;
}

bool aghOSDIsReady(void)
{
    return state.info.minor > 0 || state.info.major > 0;
}

void aghOSDUpdate(void)
{
    if (!state.port) {
        return;
    }
    aghOSDUpdateReceiveBuffer();

    if (aghOSDIsResponseAvailable()) {
        aghOSDDispatchCommand();
    }

    if (!aghOSDIsReady()) {
        // Info not received yet
        aghOSDRequestInfo();
    }
}

bool aghOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr)
{
    if (!aghOSDIsReady()) {
        return false;
    }

    uint16_t addr = char_address;

    // 200ms should be more than enough to receive ~60 bytes @ 115200 bps
    if (aghOSDSendSyncCommand(AGH_OSD_CMD_READ_FONT, &addr, sizeof(addr), 200)) {
        if (state.recv_buffer.cmd != AGH_OSD_CMD_READ_FONT ||
            state.recv_buffer.expected < sizeof(*chr) + sizeof(addr)) {

            aghOSDResetReceiveBuffer();
            AGH_OSD_DEBUG("Bad font character at position %u", char_address);
            return false;
        }
        // Skip character address
        memcpy(chr, &state.recv_buffer.data[2], sizeof(*chr));
        aghOSDResetReceiveBuffer();
        return true;
    }
    return false;
}

bool aghOSDWriteFontCharacter(unsigned char_address, const osdCharacter_t *chr)
{
    if (!aghOSDIsReady()) {
        return false;
    }

    aghOSDCharacter_t c;
    STATIC_ASSERT(sizeof(*chr) == sizeof(c.data), invalid_character_size);

    memcpy(c.data, chr, sizeof(c.data));
    c.addr = char_address;
    aghOSDSendAsyncCommand(AGH_OSD_CMD_WRITE_FONT, &c, sizeof(c));
    // Wait until all bytes have been sent. Otherwise uploading
    // the very last character of the font might fail.
    // TODO: Investigate if we can change the max7456 handling to
    // stop rebooting when a font is uploaded
    waitForSerialPortToFinishTransmitting(state.port);
    return true;
}

unsigned aghOSDGetGridRows(void)
{
    return state.info.grid.rows;
}

unsigned aghOSDGetGridCols(void)
{
    return state.info.grid.columns;
}

void aghOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr)
{
    uint8_t crc = 0;
    aghOSDSendPreamble(&crc, AGH_OSD_CMD_DRAW_GRID_STR);
    uint8_t size = 1 + 1 + strlen(buff) + 1;
    if (attr != 0) {
        size += 1;
    }
    aghOSDProcessCommandU8(&crc, size);
    aghOSDProcessCommandU8(&crc, x);
    aghOSDProcessCommandU8(&crc, y);
    for (const char *p = buff; *p; p++) {
        aghOSDProcessCommandU8(&crc, *p);
    }
    // Terminate string
    aghOSDProcessCommandU8(&crc, 0);
    if (attr != 0) {
        aghOSDProcessCommandU8(&crc, attr);
    }
    aghOSDProcessCommandU8(NULL, crc);
}

void aghOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr)
{
    DEBUG_TRACE("WRITE CHAR GRID CHR %u %u %u", x, y, chr);
    uint8_t crc = 0;
    aghOSDSendPreamble(&crc, AGH_OSD_CMD_DRAW_GRID_CHAR);
    uint8_t size = 1 + 1 + 2;
    if (attr != 0) {
        size += 1;
    }
    aghOSDProcessCommandU8(&crc, size);
    aghOSDProcessCommandU8(&crc, x);
    aghOSDProcessCommandU8(&crc, y);
    aghOSDProcessCommandU16(&crc, chr);
    if (attr != 0) {
        aghOSDProcessCommandU8(&crc, attr);
    }
    aghOSDProcessCommandU8(NULL, crc);
}

void aghOSDClearScreen(void)
{
    aghOSDSendAsyncCommand(AGH_OSD_CMD_CLEAR, NULL, 0);
}

#endif
