#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_FRSKYOSD)

#include "common/crc.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"
#include "common/uvarint.h"

#include "drivers/time.h"

#include "io/frsky_osd.h"
#include "io/serial.h"

#define FRSKY_OSD_BAUDRATE 115200
#define FRSKY_OSD_SUPPORTED_API_VERSION 1

#define FRSKY_OSD_PREAMBLE_BYTE_0 '$'
#define FRSKY_OSD_PREAMBLE_BYTE_1 'A'

#define FRSKY_OSD_GRID_BUFFER_CHAR_BITS 9
#define FRSKY_OSD_GRID_BUFFER_CHAR_MASK ((1 << FRSKY_OSD_GRID_BUFFER_CHAR_BITS) - 1)
#define FRSKY_OSD_GRID_BUFFER_ENCODE(chr, attr) ((chr & FRSKY_OSD_GRID_BUFFER_CHAR_MASK) | (attr << FRSKY_OSD_GRID_BUFFER_CHAR_BITS))

#define FRSKY_OSD_CHAR_ATTRIBUTE_COLOR_INVERSE (1 << 0)
#define FRSKY_OSD_CHAR_ATTRIBUTE_SOLID_BACKGROUND (1 << 1)

#define FRSKY_OSD_CHAR_DATA_BYTES 54
#define FRSKY_OSD_CHAR_METADATA_BYTES 10
#define FRSKY_OSD_CHAR_TOTAL_BYTES (FRSKY_OSD_CHAR_DATA_BYTES + FRSKY_OSD_CHAR_METADATA_BYTES)

#define FRSKY_OSD_SEND_BUFFER_SIZE 192
#define FRSKY_OSD_RECV_BUFFER_SIZE 128

#define FRSKY_OSD_CMD_RESPONSE_ERROR 0

#define FRSKY_OSD_INFO_INTERVAL_MS 1000

#define FRSKY_OSD_TRACE(fmt, ...)
#define FRSKY_OSD_DEBUG(fmt, ...) LOG_D(OSD, "FrSky OSD: " fmt,  ##__VA_ARGS__)
#define FRSKY_OSD_ERROR(fmt, ...) LOG_E(OSD, "FrSky OSD: " fmt,  ##__VA_ARGS__)
#define FRSKY_OSD_ASSERT(x)

typedef enum
{
    OSD_CMD_RESPONSE_ERROR = 0,

    OSD_CMD_INFO = 1,
    OSD_CMD_READ_FONT = 2,
    OSD_CMD_WRITE_FONT = 3,
    OSD_CMD_GET_CAMERA = 4,
    OSD_CMD_SET_CAMERA = 5,
    OSD_CMD_GET_ACTIVE_CAMERA = 6,
    OSD_CMD_GET_OSD_ENABLED = 7,
    OSD_CMD_SET_OSD_ENABLED = 8,

    OSD_CMD_TRANSACTION_BEGIN = 16,
    OSD_CMD_TRANSACTION_COMMIT = 17,
    OSD_CMD_TRANSACTION_BEGIN_PROFILED = 18,
    OSD_CMD_TRANSACTION_BEGIN_RESET_DRAWING = 19,

    OSD_CMD_DRAWING_SET_STROKE_COLOR = 22,
    OSD_CMD_DRAWING_SET_FILL_COLOR = 23,
    OSD_CMD_DRAWING_SET_STROKE_AND_FILL_COLOR = 24,
    OSD_CMD_DRAWING_SET_COLOR_INVERSION = 25,
    OSD_CMD_DRAWING_SET_PIXEL = 26,
    OSD_CMD_DRAWING_SET_PIXEL_TO_STROKE_COLOR = 27,
    OSD_CMD_DRAWING_SET_PIXEL_TO_FILL_COLOR = 28,
    OSD_CMD_DRAWING_SET_STROKE_WIDTH = 29,
    OSD_CMD_DRAWING_SET_LINE_OUTLINE_TYPE = 30,
    OSD_CMD_DRAWING_SET_LINE_OUTLINE_COLOR = 31,

    OSD_CMD_DRAWING_CLIP_TO_RECT = 40,
    OSD_CMD_DRAWING_CLEAR_SCREEN = 41,
    OSD_CMD_DRAWING_CLEAR_RECT = 42,
    OSD_CMD_DRAWING_RESET = 43,
    OSD_CMD_DRAWING_DRAW_BITMAP = 44,
    OSD_CMD_DRAWING_DRAW_BITMAP_MASK = 45,
    OSD_CMD_DRAWING_DRAW_CHAR = 46,
    OSD_CMD_DRAWING_DRAW_CHAR_MASK = 47,
    OSD_CMD_DRAWING_DRAW_STRING = 48,
    OSD_CMD_DRAWING_DRAW_STRING_MASK = 49,
    OSD_CMD_DRAWING_MOVE_TO_POINT = 50,
    OSD_CMD_DRAWING_STROKE_LINE_TO_POINT = 51,
    OSD_CMD_DRAWING_STROKE_TRIANGLE = 52,
    OSD_CMD_DRAWING_FILL_TRIANGLE = 53,
    OSD_CMD_DRAWING_FILL_STROKE_TRIANGLE = 54,
    OSD_CMD_DRAWING_STROKE_RECT = 55,
    OSD_CMD_DRAWING_FILL_RECT = 56,
    OSD_CMD_DRAWING_FILL_STROKE_RECT = 57,
    OSD_CMD_DRAWING_STROKE_ELLIPSE_IN_RECT = 58,
    OSD_CMD_DRAWING_FILL_ELLIPSE_IN_RECT = 59,
    OSD_CMD_DRAWING_FILL_STROKE_ELLIPSE_IN_RECT = 60,

    OSD_CMD_CTM_RESET = 80,
    OSD_CMD_CTM_SET = 81,
    OSD_CMD_CTM_TRANSLATE = 82,
    OSD_CMD_CTM_SCALE = 83,
    OSD_CMD_CTM_ROTATE = 84,
    OSD_CMD_CTM_ROTATE_ABOUT = 85,
    OSD_CMD_CTM_SHEAR = 86,
    OSD_CMD_CTM_SHEAR_ABOUT = 87,
    OSD_CMD_CTM_MULTIPLY = 88,

    OSD_CMD_CONTEXT_PUSH = 100,
    OSD_CMD_CONTEXT_POP = 101,

    // MAX7456 emulation commands
    OSD_CMD_DRAW_GRID_CHR = 110,
    OSD_CMD_DRAW_GRID_STR = 111,
} osdCommand_e;

typedef enum {
    RECV_STATE_NONE,
    RECV_STATE_SYNC,
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
    uint8_t tvStandard;
    uint8_t hasDetectedCamera;
    uint16_t maxFrameSize;
    uint8_t contextStackSize;
} __attribute__((packed)) frskyOSDInfoResponse_t;

typedef struct frskyOSDFontCharacter_s {
    uint16_t addr;
    struct {
        uint8_t bitmap[FRSKY_OSD_CHAR_DATA_BYTES]; // 12x18 2bpp
        uint8_t metadata[FRSKY_OSD_CHAR_METADATA_BYTES];
    } data;
} __attribute__((packed)) frskyOSDCharacter_t;

typedef struct frskyOSDDrawGridCharCmd_s {
    uint8_t gx;
    uint8_t gy;
    uint16_t chr;
    uint8_t opts;
} __attribute__((packed)) frskyOSDDrawGridCharCmd_t;

typedef struct frskyOSDDrawGridStrHeaderCmd_s {
    uint8_t gx;
    uint8_t gy;
    uint8_t opts;
    // uvarint with size and blob folow
} __attribute__((packed)) frskyOSDDrawGridStrHeaderCmd_t;

typedef struct frskyOSDPoint_s {
    int x : 12;
    int y : 12;
} __attribute__((packed)) frskyOSDPoint_t;

typedef struct frskyOSDSize_s {
    int w : 12;
    int h : 12;
} __attribute__((packed)) frskyOSDSize_t;

typedef struct frskyOSDRect_s {
    frskyOSDPoint_t origin;
    frskyOSDSize_t size;
} __attribute__((packed)) frskyOSDRect_t;

typedef struct frskyOSDTriangle_s {
    frskyOSDPoint_t p1;
    frskyOSDPoint_t p2;
    frskyOSDPoint_t p3;
} __attribute__((packed)) frskyOSDTriangle_t;

typedef struct frskyOSDSetPixel_s {
    frskyOSDPoint_t p;
    uint8_t color;
}  __attribute__((packed)) frskyOSDSetPixel_t;

typedef struct frskyOSDDrawCharacterCmd_s {
    frskyOSDPoint_t p;
    uint16_t chr;
    uint8_t opts;
}  __attribute__((packed)) frskyOSDDrawCharacterCmd_t;

typedef struct frskyOSDDrawCharacterMaskCmd_s {
    frskyOSDDrawCharacterCmd_t dc;
    uint8_t maskColor;
}  __attribute__((packed)) frskyOSDDrawCharacterMaskCmd_t;

typedef struct frskyOSDDrawStrCommandHeaderCmd_s {
    frskyOSDPoint_t p;
    uint8_t opts;
    // uvarint with size and blob follow
} __attribute__((packed)) frskyOSDDrawStrCommandHeaderCmd_t;

typedef struct frskyOSDDrawStrMaskCommandHeaderCmd_s {
    frskyOSDPoint_t p;
    uint8_t opts;
    uint8_t maskColor;
    // uvarint with size and blob follow
} __attribute__((packed)) frskyOSDDrawStrMaskCommandHeaderCmd_t;


typedef struct frskyOSDState_s {
    struct {
        uint8_t data[FRSKY_OSD_SEND_BUFFER_SIZE];
        uint8_t pos;
    } sendBuffer;
    struct {
        uint8_t state;
        uint8_t crc;
        uint16_t expected;
        uint8_t expectedShift;
        uint8_t data[FRSKY_OSD_RECV_BUFFER_SIZE];
        uint8_t pos;
    } recvBuffer;
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
    struct {
        uint16_t addr;
        osdCharacter_t *chr;
    } recvOsdCharacter;
    serialPort_t *port;
    bool initialized;
    timeMs_t nextInfoRequest;
} frskyOSDState_t;

static frskyOSDState_t state;

static uint8_t frskyOSDChecksum(uint8_t crc, uint8_t c)
{
    return crc8_dvb_s2(crc, c);
}

static void frskyOSDResetReceiveBuffer(void)
{
    state.recvBuffer.state = RECV_STATE_NONE;
    state.recvBuffer.crc = 0;
    state.recvBuffer.expected = 0;
    state.recvBuffer.expectedShift = 0;
    state.recvBuffer.pos = 0;
}

static void frskyOSDResetSendBuffer(void)
{
    state.sendBuffer.pos = 0;
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

static void frskyOSDSendCommand(uint8_t cmd, const void *payload, size_t size)
{
    int required = size + 1;
    FRSKY_OSD_ASSERT(required <= sizeof(state.sendBuffer.data));
    int rem = sizeof(state.sendBuffer.data) - state.sendBuffer.pos;
    if (rem < required) {
        frskyOSDFlushSendBuffer();
    }
    state.sendBuffer.data[state.sendBuffer.pos++] = cmd;
    const uint8_t *ptr = payload;
    for (size_t ii = 0; ii < size; ii++, ptr++) {
        state.sendBuffer.data[state.sendBuffer.pos++] = *ptr;
    }
}

static void frskyOSDStateReset(serialPort_t *port)
{
    frskyOSDResetReceiveBuffer();
    frskyOSDResetSendBuffer();
    state.info.grid.rows = 0;
    state.info.grid.columns = 0;
    state.info.viewport.width = 0;
    state.info.viewport.height = 0;

    state.port = port;
    state.initialized = false;
}

static void frskyOSDUpdateReceiveBuffer(void)
{
    while (serialRxBytesWaiting(state.port) > 0) {
        uint8_t c = serialRead(state.port);
        switch ((frskyOSDRecvState_e)state.recvBuffer.state) {
            case RECV_STATE_NONE:
                if (c != FRSKY_OSD_PREAMBLE_BYTE_0) {
                    break;
                }
                state.recvBuffer.state = RECV_STATE_SYNC;
                break;
            case RECV_STATE_SYNC:
                if (c != FRSKY_OSD_PREAMBLE_BYTE_1) {
                    frskyOSDResetReceiveBuffer();
                    break;
                }
                state.recvBuffer.state = RECV_STATE_LENGTH;
                break;
            case RECV_STATE_LENGTH:
                state.recvBuffer.crc = frskyOSDChecksum(state.recvBuffer.crc, c);
                state.recvBuffer.expected |= (c & 0x7F) << state.recvBuffer.expectedShift;
                state.recvBuffer.expectedShift += 7;
                if (c < 0x80) {
                    // Full uvarint decoded. Check against buffer size.
                    if (state.recvBuffer.expected > sizeof(state.recvBuffer.data)) {
                        FRSKY_OSD_ERROR("Can't handle payload of size %u with a buffer of size %u",
                            state.recvBuffer.expected, sizeof(state.recvBuffer.data));
                        frskyOSDResetReceiveBuffer();
                        break;
                    }
                    FRSKY_OSD_TRACE("Payload of size %u", state.recvBuffer.expected);
                    state.recvBuffer.state = state.recvBuffer.expected > 0 ? RECV_STATE_DATA : RECV_STATE_CHECKSUM;
                }
                break;
            case RECV_STATE_DATA:
                state.recvBuffer.data[state.recvBuffer.pos++] = c;
                state.recvBuffer.crc = frskyOSDChecksum(state.recvBuffer.crc, c);
                if (state.recvBuffer.pos == state.recvBuffer.expected) {
                    state.recvBuffer.state = RECV_STATE_CHECKSUM;
                }
                break;
            case RECV_STATE_CHECKSUM:
                if (c != state.recvBuffer.crc) {
                    FRSKY_OSD_DEBUG("Checksum error %u != %u. Discarding %u bytes",
                        c, state.recvBuffer.crc, state.recvBuffer.pos);
                    frskyOSDResetReceiveBuffer();
                    break;
                }
                state.recvBuffer.state = RECV_STATE_DONE;
                break;
            case RECV_STATE_DONE:
                FRSKY_OSD_DEBUG("Received unexpected byte %u after data", c);
                break;
        }
    }
}

static bool frskyOSDIsResponseAvailable(void)
{
    return state.recvBuffer.state == RECV_STATE_DONE;
}

static bool frskyOSDHandleCommand(osdCommand_e cmd, const void *payload, size_t size)
{
    const uint8_t *ptr = payload;

    switch (cmd) {
        case OSD_CMD_RESPONSE_ERROR:
        {
            if (size >= 2) {
                FRSKY_OSD_ERROR("Received an error %02x in response to command %u", *(ptr + 1), *ptr);
                return true;
            }
            break;
        }
        case OSD_CMD_INFO:
        {
            if (size < sizeof(frskyOSDInfoResponse_t)) {
                break;
            }
            const frskyOSDInfoResponse_t *resp = payload;
            if (resp->magic[0] != 'A' || resp->magic[1] != 'G' || resp->magic[2] != 'H') {
                FRSKY_OSD_ERROR("invalid magic number %x %x %x, expecting AGH",
                    resp->magic[0], resp->magic[1], resp->magic[2]);
                return false;
            }
            state.info.major = resp->versionMajor;
            state.info.minor = resp->versionMinor;
            state.info.grid.rows = resp->gridRows;
            state.info.grid.columns = resp->gridColumns;
            state.info.viewport.width = resp->pixelWidth;
            state.info.viewport.height = resp->pixelHeight;
            if (!state.initialized) {
                FRSKY_OSD_DEBUG("initialized. Version %u.%u.%u, pixels=%ux%u, grid=%ux%u",
                    resp->versionMajor, resp->versionMinor, resp->versionPatch,
                    resp->pixelWidth, resp->pixelHeight, resp->gridColumns, resp->gridRows);
                state.initialized = true;
                frskyOSDClearScreen();
                frskyOSDResetDrawingState();
            }
            return true;
        }
        case OSD_CMD_READ_FONT:
        {
            if (!state.recvOsdCharacter.chr) {
                FRSKY_OSD_DEBUG("Got unexpected font character");
                break;
            }
            if (size < sizeof(uint16_t) + FRSKY_OSD_CHAR_TOTAL_BYTES) {
                FRSKY_OSD_TRACE("Received buffer too small for a character: %u bytes", size);
                break;
            }
            const frskyOSDCharacter_t *chr = payload;
            state.recvOsdCharacter.addr = chr->addr;
            FRSKY_OSD_TRACE("Received character %u", chr->addr);
            // Skip character address
            memcpy(state.recvOsdCharacter.chr->data, &chr->data, MIN(sizeof(state.recvOsdCharacter.chr->data), (size_t)FRSKY_OSD_CHAR_TOTAL_BYTES));
            return true;
        }
        case OSD_CMD_WRITE_FONT:
        {
            // We only wait for the confirmation, we're not interested in the data
            return true;
        }
        default:
            break;
    }
    return false;
}

static bool frskyOSDDispatchResponse(void)
{
    const uint8_t *payload = state.recvBuffer.data;
    int remaining = (int)state.recvBuffer.pos;
    bool ok = false;
    if (remaining > 0) {
        // OSD sends commands one by one, so we don't need to handle
        // a frame with multiple ones.
        uint8_t cmd = *payload;
        payload++;
        remaining--;
        if (frskyOSDHandleCommand(cmd, payload, remaining)) {
            ok = true;
        } else {
            FRSKY_OSD_DEBUG("Discarding buffer due to unhandled command %u (%d bytes remaining)", cmd, remaining);
        }
    }
    frskyOSDResetReceiveBuffer();
    return ok;
}

static void frskyOSDClearReceiveBuffer(void)
{
    frskyOSDUpdateReceiveBuffer();

    if (frskyOSDIsResponseAvailable()) {
        frskyOSDDispatchResponse();
    } else if (state.recvBuffer.pos > 0) {
        FRSKY_OSD_DEBUG("Discarding receive buffer with %u bytes", state.recvBuffer.pos);
        frskyOSDResetReceiveBuffer();
    }
}

static void frskyOSDSendAsyncCommand(uint8_t cmd, const void *data, size_t size)
{
    FRSKY_OSD_TRACE("Send async cmd %u", cmd);
    frskyOSDSendCommand(cmd, data, size);
}

static bool frskyOSDSendSyncCommand(uint8_t cmd, const void *data, size_t size, timeMs_t timeout)
{
    FRSKY_OSD_TRACE("Send sync cmd %u", cmd);
    frskyOSDClearReceiveBuffer();
    frskyOSDSendCommand(cmd, data, size);
    frskyOSDFlushSendBuffer();
    timeMs_t end = millis() + timeout;
    while (millis() < end) {
        frskyOSDUpdateReceiveBuffer();
        if (frskyOSDIsResponseAvailable() && frskyOSDDispatchResponse()) {
            FRSKY_OSD_DEBUG("Got sync response");
            return true;
        }
    }
    FRSKY_OSD_DEBUG("Sync response failed");
    return false;
}

static bool frskyOSDShouldRequestInfo(void)
{
    return !frskyOSDIsReady() || millis() > state.nextInfoRequest;
}

static void frskyOSDRequestInfo(void)
{
    timeMs_t now = millis();
    if (state.info.nextRequest < now) {
        uint8_t version = FRSKY_OSD_SUPPORTED_API_VERSION;
        frskyOSDSendAsyncCommand(OSD_CMD_INFO, &version, sizeof(version));
        frskyOSDFlushSendBuffer();
        state.info.nextRequest = now + FRSKY_OSD_INFO_INTERVAL_MS;
    }
}

static uint8_t frskyOSDEncodeAttr(textAttributes_t attr)
{
    uint8_t frskyOSDAttr = 0;
    if (TEXT_ATTRIBUTES_HAVE_INVERTED(attr)) {
        frskyOSDAttr |= FRSKY_OSD_CHAR_ATTRIBUTE_COLOR_INVERSE;
    }
    if (TEXT_ATTRIBUTES_HAVE_SOLID_BG(attr)) {
        frskyOSDAttr |= FRSKY_OSD_CHAR_ATTRIBUTE_SOLID_BACKGROUND;
    }
    return frskyOSDAttr;
}

bool frskyOSDInit(videoSystem_e videoSystem)
{
    UNUSED(videoSystem);
    // TODO: Use videoSystem to set the signal standard when
    // no input is detected.
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_FRSKY_OSD);
    if (portConfig) {
        FRSKY_OSD_TRACE("configured, trying to connect...");
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
        frskyOSDDispatchResponse();
    }

    if (frskyOSDShouldRequestInfo()) {
        frskyOSDRequestInfo();
    }
}

void frskyOSDBeginTransaction(frskyOSDTransactionOptions_e opts)
{
    if (opts & FRSKY_OSD_TRANSACTION_OPT_PROFILED) {
        frskyOSDPoint_t p = { .x = 0, .y = 10};
        frskyOSDSendAsyncCommand(OSD_CMD_TRANSACTION_BEGIN_PROFILED, &p, sizeof(p));
        if (opts & FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING) {
            frskyOSDResetDrawingState();
        }
    } else if (opts & FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING) {
        frskyOSDSendAsyncCommand(OSD_CMD_TRANSACTION_BEGIN_RESET_DRAWING, NULL, 0);
    } else {
        frskyOSDSendAsyncCommand(OSD_CMD_TRANSACTION_BEGIN, NULL, 0);
    }
}

void frskyOSDCommitTransaction(void)
{
    // Check wether the only command in the queue is a transaction begin.
    // In that, case, discard the send buffer since it will make generate
    // an empty transaction.
    if (state.sendBuffer.pos == 1) {
        if (state.sendBuffer.data[0] == OSD_CMD_TRANSACTION_BEGIN ||
            state.sendBuffer.data[0] == OSD_CMD_TRANSACTION_BEGIN_RESET_DRAWING) {

            state.sendBuffer.pos = 0;
            return;
        }
    }
    frskyOSDSendAsyncCommand(OSD_CMD_TRANSACTION_COMMIT, NULL, 0);
    frskyOSDFlushSendBuffer();
}

void frskyOSDFlushSendBuffer(void)
{
    if (state.sendBuffer.pos > 0) {
        frskyOSDProcessCommandU8(NULL, FRSKY_OSD_PREAMBLE_BYTE_0);
        frskyOSDProcessCommandU8(NULL, FRSKY_OSD_PREAMBLE_BYTE_1);

        uint8_t crc = 0;
        uint8_t buffer[4];
        int lengthSize = uvarintEncode(state.sendBuffer.pos, buffer, sizeof(buffer));
        for (int ii = 0; ii < lengthSize; ii++) {
            frskyOSDProcessCommandU8(&crc, buffer[ii]);
        }
        for (unsigned ii = 0; ii < state.sendBuffer.pos; ii++) {
            frskyOSDProcessCommandU8(&crc, state.sendBuffer.data[ii]);
        }
        frskyOSDProcessCommandU8(NULL, crc);
        state.sendBuffer.pos = 0;
    }
}

bool frskyOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr)
{
    if (!frskyOSDIsReady()) {
        return false;
    }

    uint16_t addr = char_address;

    state.recvOsdCharacter.addr = UINT16_MAX;
    state.recvOsdCharacter.chr = chr;

    // 500ms should be more than enough to receive ~70 bytes @ 115200 bps
    bool ok = frskyOSDSendSyncCommand(OSD_CMD_READ_FONT, &addr, sizeof(addr), 500);

    state.recvOsdCharacter.chr = NULL;

    if (ok && state.recvOsdCharacter.addr == addr) {
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

    memcpy(&c.data, chr, sizeof(c.data));
    c.addr = char_address;
    FRSKY_OSD_TRACE("Writing font character %u", char_address);
    frskyOSDSendSyncCommand(OSD_CMD_WRITE_FONT, &c, sizeof(c), 1000);
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

unsigned frskyOSDGetPixelWidth(void)
{
    return state.info.viewport.width;
}

unsigned frskyOSDGetPixelHeight(void)
{
    return state.info.viewport.height;
}

static void frskyOSDSendCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr)
{
    uint8_t payload[] = {
        x,
        y,
        chr & 0xFF,
        chr >> 8,
        frskyOSDEncodeAttr(attr),
    };
    frskyOSDSendAsyncCommand(OSD_CMD_DRAW_GRID_CHR, payload, sizeof(payload));
}

static void frskyOSDSendAsyncBlobCommand(uint8_t cmd, const void *header, size_t headerSize, const void *blob, size_t blobSize)
{
    uint8_t payload[128];

    memcpy(payload, header, headerSize);

    int uvarintSize = uvarintEncode(blobSize, &payload[headerSize], sizeof(payload) - headerSize);
    memcpy(&payload[headerSize + uvarintSize], blob, blobSize);
    frskyOSDSendAsyncCommand(cmd, payload,  headerSize + uvarintSize + blobSize);
}

void frskyOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr)
{
    unsigned charsUpdated = 0;
    const char *updatedCharAt = NULL;
    uint16_t *entry = osdCharacterGridBufferGetEntryPtr(x, y);
    const char *p;
    unsigned xx;
    for (p = buff, xx = x; *p && xx < state.info.grid.columns; p++, entry++, xx++) {
        unsigned val = FRSKY_OSD_GRID_BUFFER_ENCODE(*p, attr);
        if (*entry != val) {
            if (++charsUpdated == 1) {
                // First character that needs to be updated, save it
                // in case we can issue a single update.
                updatedCharAt = p;
            }
            *entry = val;
        }
    }

    if (charsUpdated == 0) {
        return;
    }

    if (charsUpdated == 1) {
        frskyOSDSendCharInGrid(x + (updatedCharAt - buff), y, *updatedCharAt, attr);
        return;
    }

    frskyOSDDrawGridStrHeaderCmd_t cmd;
    cmd.gx = x;
    cmd.gy = y;
    cmd.opts = frskyOSDEncodeAttr(attr);

    frskyOSDSendAsyncBlobCommand(OSD_CMD_DRAW_GRID_STR, &cmd, sizeof(cmd), buff, strlen(buff) + 1);
}

void frskyOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr)
{
    uint16_t *entry = osdCharacterGridBufferGetEntryPtr(x, y);
    unsigned val = FRSKY_OSD_GRID_BUFFER_ENCODE(chr, attr);

    if (*entry == val) {
        return;
    }

    frskyOSDSendCharInGrid(x, y, chr, attr);

    *entry = val;
}

bool frskyOSDReadCharInGrid(unsigned x, unsigned y, uint16_t *c, textAttributes_t *attr)
{
    uint16_t val = *osdCharacterGridBufferGetEntryPtr(x, y);
    // We use the lower 10 bits for characters
    *c = val & FRSKY_OSD_GRID_BUFFER_CHAR_MASK;
    *attr = val >> FRSKY_OSD_GRID_BUFFER_CHAR_BITS;
    return true;
}

void frskyOSDClearScreen(void)
{
    osdCharacterGridBufferClear();
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_CLEAR_SCREEN, NULL, 0);
}

void frskyOSDSetStrokeColor(frskyOSDColor_e color)
{
    uint8_t c = color;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_STROKE_COLOR, &c, sizeof(c));
}

void frskyOSDSetFillColor(frskyOSDColor_e color)
{
    uint8_t c = color;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_FILL_COLOR, &c, sizeof(c));
}

void frskyOSDSetStrokeAndFillColor(frskyOSDColor_e color)
{
    uint8_t c = color;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_STROKE_AND_FILL_COLOR, &c, sizeof(c));
}

void frskyOSDSetColorInversion(bool inverted)
{
    uint8_t c = inverted ? 1 : 0;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_COLOR_INVERSION, &c, sizeof(c));
}

void frskyOSDSetPixel(int x, int y, frskyOSDColor_e color)
{
    frskyOSDSetPixel_t sp = {.p = {.x = x, .y = y}, .color = color};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_PIXEL, &sp, sizeof(sp));
}

void frskyOSDSetPixelToStrokeColor(int x, int y)
{
    frskyOSDPoint_t p = { .x = x, .y = y};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_PIXEL_TO_STROKE_COLOR, &p, sizeof(p));
}

void frskyOSDSetPixelToFillColor(int x, int y)
{
    frskyOSDPoint_t p = { .x = x, .y = y};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_PIXEL_TO_FILL_COLOR, &p, sizeof(p));
}

void frskyOSDSetStrokeWidth(unsigned width)
{
    uint8_t w = width;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_STROKE_WIDTH, &w, sizeof(w));
}

void frskyOSDSetLineOutlineType(frskyOSDLineOutlineType_e outlineType)
{
    uint8_t type = outlineType;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_LINE_OUTLINE_TYPE, &type, sizeof(type));
}

void frskyOSDSetLineOutlineColor(frskyOSDColor_e outlineColor)
{
    uint8_t color = outlineColor;
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_SET_LINE_OUTLINE_COLOR, &color, sizeof(color));
}

void frskyOSDClipToRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_CLIP_TO_RECT, &r, sizeof(r));
}

void frskyOSDClearRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_CLEAR_RECT, &r, sizeof(r));
}

void frskyOSDResetDrawingState(void)
{
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_RESET, NULL, 0);
}

void frskyOSDDrawCharacter(int x, int y, uint16_t chr, uint8_t opts)
{
    frskyOSDDrawCharacterCmd_t dc = { .p = {.x = x, .y = y}, .chr = chr, .opts = opts};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_DRAW_CHAR, &dc, sizeof(dc));
}

void frskyOSDDrawCharacterMask(int x, int y, uint16_t chr, frskyOSDColor_e color, uint8_t opts)
{
    frskyOSDDrawCharacterMaskCmd_t dc = { .dc = { .p = {.x = x, .y = y}, .chr = chr, .opts = opts}, .maskColor = color};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_DRAW_CHAR_MASK, &dc, sizeof(dc));
}

void frskyOSDDrawString(int x, int y, const char *s, uint8_t opts)
{
    frskyOSDDrawStrCommandHeaderCmd_t cmd;
    cmd.p.x = x;
    cmd.p.y = y;
    cmd.opts = opts;

    frskyOSDSendAsyncBlobCommand(OSD_CMD_DRAWING_DRAW_STRING, &cmd, sizeof(cmd), s, strlen(s) + 1);
}

void frskyOSDDrawStringMask(int x, int y, const char *s, frskyOSDColor_e color, uint8_t opts)
{
    frskyOSDDrawStrMaskCommandHeaderCmd_t cmd;
    cmd.p.x = x;
    cmd.p.y = y;
    cmd.opts = opts;
    cmd.maskColor = color;

    frskyOSDSendAsyncBlobCommand(OSD_CMD_DRAWING_DRAW_STRING_MASK, &cmd, sizeof(cmd), s, strlen(s) + 1);
}

void frskyOSDMoveToPoint(int x, int y)
{
    frskyOSDPoint_t p = { .x = x, .y = y};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_MOVE_TO_POINT, &p, sizeof(p));
}

void frskyOSDStrokeLineToPoint(int x, int y)
{
    frskyOSDPoint_t p = { .x = x, .y = y};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_STROKE_LINE_TO_POINT, &p, sizeof(p));
}

void frskyOSDStrokeTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    frskyOSDTriangle_t t = {.p1 = {.x = x1, .y = y1}, .p2 = {.x = x2, .y = y2}, .p3 = { .x = x3, .y = y3}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_STROKE_TRIANGLE, &t, sizeof(t));
}

void frskyOSDFillTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    frskyOSDTriangle_t t = {.p1 = {.x = x1, .y = y1}, .p2 = {.x = x2, .y = y2}, .p3 = { .x = x3, .y = y3}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_TRIANGLE, &t, sizeof(t));
}

void frskyOSDFillStrokeTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    frskyOSDTriangle_t t = {.p1 = {.x = x1, .y = y1}, .p2 = {.x = x2, .y = y2}, .p3 = { .x = x3, .y = y3}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_STROKE_TRIANGLE, &t, sizeof(t));
}

void frskyOSDStrokeRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_STROKE_RECT, &r, sizeof(r));
}

void frskyOSDFillRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_RECT, &r, sizeof(r));
}

void frskyOSDFillStrokeRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_STROKE_RECT, &r, sizeof(r));
}

void frskyOSDStrokeEllipseInRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_STROKE_ELLIPSE_IN_RECT, &r, sizeof(r));
}

void frskyOSDFillEllipseInRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_ELLIPSE_IN_RECT, &r, sizeof(r));
}

void frskyOSDFillStrokeEllipseInRect(int x, int y, int w, int h)
{
    frskyOSDRect_t r = { .origin = { .x = x, .y = y}, .size = {.w = w, .h = h}};
    frskyOSDSendAsyncCommand(OSD_CMD_DRAWING_FILL_STROKE_ELLIPSE_IN_RECT, &r, sizeof(r));
}

void frskyOSDCtmReset(void)
{
    frskyOSDSendAsyncCommand(OSD_CMD_CTM_RESET, NULL, 0);
}

void frskyOSDCtmSet(float m11, float m12, float m21, float m22, float m31, float m32)
{
    float values[] = {
        m11, m12,
        m21, m22,
        m31, m32,
    };
    frskyOSDSendAsyncCommand(OSD_CMD_CTM_SET, values, sizeof(values));
}

void frskyOSDCtmTranslate(float tx, float ty)
{
    float values[] = {
        tx,
        ty,
    };
    frskyOSDSendAsyncCommand(OSD_CMD_CTM_TRANSLATE, values, sizeof(values));
}

void frskyOSDCtmScale(float sx, float sy)
{
    float values[] = {
        sx,
        sy,
    };
    frskyOSDSendAsyncCommand(OSD_CMD_CTM_SCALE, values, sizeof(values));
}

void frskyOSDCtmRotate(float r)
{
    frskyOSDSendAsyncCommand(OSD_CMD_CTM_ROTATE, &r, sizeof(r));
}

void frskyOSDContextPush(void)
{
    frskyOSDSendAsyncCommand(OSD_CMD_CONTEXT_PUSH, NULL, 0);
}

void frskyOSDContextPop(void)
{
    frskyOSDSendAsyncCommand(OSD_CMD_CONTEXT_POP, NULL, 0);
}


#endif
