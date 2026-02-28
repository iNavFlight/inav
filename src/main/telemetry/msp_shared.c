#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_MSP_OVER_TELEMETRY)

#include "build/build_config.h"

#include "common/utils.h"

#include "fc/fc_msp.h"

#include "msp/msp.h"

#include "telemetry/msp_shared.h"

#define TELEMETRY_MSP_VERSION    2
#define TELEMETRY_MSP_RES_ERROR (-10)

enum { // constants for status of msp-over-telemetry frame
    TELEMETRY_MSP_SEQ_MASK      = 0x0f, // 0b00001111,   // sequence number mask
    TELEMETRY_MSP_VER_MASK      = 0x60, // 0b01100000,   // MSP version mask
    TELEMETRY_MSP_START_MASK    = 0x10, // 0b00010000,   // bit of starting frame (if 1, the frame is a first/single chunk of msp-frame)
    TELEMETRY_MSP_ERROR_MASK    = 0x80, // 0b10000000,   // Error bit (1 if error)
    TELEMETRY_MSP_VER_SHIFT     = 5,    // MSP version shift
};

enum {
    TELEMETRY_MSP_VER_MISMATCH=0,
    TELEMETRY_MSP_CRC_ERROR=1,
    TELEMETRY_MSP_ERROR=2,
    TELEMETRY_MSP_REQUEST_IS_TOO_BIG = 3,
};

enum { // minimum length for a frame.
    MIN_LENGTH_CHUNK         = 2, // status + at_least_one_byte
    MIN_LENGTH_REQUEST_V1    = 3, // status + length + ID
    MIN_LENGTH_REQUEST_V2    = 6, // status + flag + ID_lo + ID_hi + size_lo + size_hi
};

enum { // byte position(index) in msp-over-telemetry request payload
    // MSPv1
    MSP_INDEX_STATUS        = 0,                           // status byte
    MSP_INDEX_SIZE_V1       = MSP_INDEX_STATUS        + 1, // MSPv1 payload size
    MSP_INDEX_ID_V1         = MSP_INDEX_SIZE_V1       + 1, // MSPv1 ID/command/function byte
    MSP_INDEX_PAYLOAD_V1    = MSP_INDEX_ID_V1         + 1, // MSPv1 Payload start / CRC for zero payload

    // MSPv2
    MSP_INDEX_FLAG_V2       = MSP_INDEX_SIZE_V1,           // MSPv2 flags byte
    MSP_INDEX_ID_LO         = MSP_INDEX_ID_V1,             // MSPv2 Lo byte of ID/command/function
    MSP_INDEX_ID_HI         = MSP_INDEX_ID_LO         + 1, // MSPv2 Hi byte of ID/command/function
    MSP_INDEX_SIZE_V2_LO    = MSP_INDEX_ID_HI         + 1, // MSPv2 Lo byte of payload size
    MSP_INDEX_SIZE_V2_HI    = MSP_INDEX_SIZE_V2_LO    + 1, // MSPv2 Hi byte of payload size
    MSP_INDEX_PAYLOAD_V2    = MSP_INDEX_SIZE_V2_HI    + 1, // MSPv2 first byte of payload itself
};

static uint8_t lastRequestVersion; // MSP version of last request. Temporary solution. It's better to keep it in requestPacket.
STATIC_UNIT_TESTED mspPackage_t mspPackage;
static mspRxBuffer_t mspRxBuffer;
static mspTxBuffer_t mspTxBuffer;
static mspPacket_t mspRxPacket;
static mspPacket_t mspTxPacket;

void initSharedMsp(void)
{
    mspPackage.requestBuffer = (uint8_t *)&mspRxBuffer;
    mspPackage.requestPacket = &mspRxPacket;
    mspPackage.requestPacket->buf.ptr = mspPackage.requestBuffer;
    mspPackage.requestPacket->buf.end = mspPackage.requestBuffer;

    mspPackage.responseBuffer = (uint8_t *)&mspTxBuffer;
    mspPackage.responsePacket = &mspTxPacket;
    mspPackage.responsePacket->buf.ptr = mspPackage.responseBuffer;
    mspPackage.responsePacket->buf.end = mspPackage.responseBuffer;
}

static void processMspPacket(void)
{
    mspPackage.responsePacket->cmd = 0;
    mspPackage.responsePacket->result = 0;
    mspPackage.responsePacket->buf.end = mspPackage.responseBuffer;

    mspPostProcessFnPtr mspPostProcessFn = NULL;
    if (mspFcProcessCommand(mspPackage.requestPacket, mspPackage.responsePacket, &mspPostProcessFn) == MSP_RESULT_ERROR) {
        sbufWriteU8(&mspPackage.responsePacket->buf, TELEMETRY_MSP_ERROR);
    }
    if (mspPostProcessFn) {
        mspPostProcessFn(NULL);
    }

    sbufSwitchToReader(&mspPackage.responsePacket->buf, mspPackage.responseBuffer);
}

void sendMspErrorResponse(uint8_t error, int16_t cmd)
{
    mspPackage.responsePacket->cmd = cmd;
    mspPackage.responsePacket->result = 0;
    mspPackage.responsePacket->buf.end = mspPackage.responseBuffer;

    sbufWriteU8(&mspPackage.responsePacket->buf, error);
    mspPackage.responsePacket->result = TELEMETRY_MSP_RES_ERROR;
    sbufSwitchToReader(&mspPackage.responsePacket->buf, mspPackage.responseBuffer);
}

bool handleMspFrame(uint8_t *const frameStart, const int payloadLength)
{
    static uint8_t mspStarted = 0;
    static uint8_t lastSeq = 0;

    if (sbufBytesRemaining(&mspPackage.responsePacket->buf) > 0) {
        mspStarted = 0;
    }

    if (mspStarted == 0) {
        initSharedMsp();
    }

    if (payloadLength < MIN_LENGTH_CHUNK) {
        return false;   // prevent analyzing garbage data
    }

    mspPacket_t *requestPacket = mspPackage.requestPacket;

    const uint8_t status = frameStart[MSP_INDEX_STATUS];
    const uint8_t seqNumber = status & TELEMETRY_MSP_SEQ_MASK;
    lastRequestVersion = (status & TELEMETRY_MSP_VER_MASK) >> TELEMETRY_MSP_VER_SHIFT;

    if (lastRequestVersion > TELEMETRY_MSP_VERSION) {
        sendMspErrorResponse(TELEMETRY_MSP_VER_MISMATCH, 0);
        return true;
    }

    if (status & TELEMETRY_MSP_START_MASK) { // first packet in sequence
        uint16_t mspPayloadSize;

        if (lastRequestVersion == 1) { // MSPv1

            mspPayloadSize = frameStart[MSP_INDEX_SIZE_V1];
            requestPacket->cmd = frameStart[MSP_INDEX_ID_V1];
            sbufInit(&mspPackage.requestFrame, frameStart + MSP_INDEX_PAYLOAD_V1, frameStart + payloadLength);

        } else { // MSPv2
            if (payloadLength < MIN_LENGTH_REQUEST_V2) {
                return false;   // prevent analyzing garbage data
            }
            requestPacket->flags = frameStart[MSP_INDEX_FLAG_V2];
            requestPacket->cmd = *(uint16_t *) &frameStart[MSP_INDEX_ID_LO];
            mspPayloadSize = *(uint16_t *) &frameStart[MSP_INDEX_SIZE_V2_LO];
            sbufInit(&mspPackage.requestFrame, frameStart + MSP_INDEX_PAYLOAD_V2, frameStart + payloadLength);
        }

        if (mspPayloadSize <= sizeof(mspRxBuffer)) { // prevent buffer overrun
            requestPacket->result = 0;
            requestPacket->buf.ptr = mspPackage.requestBuffer;
            requestPacket->buf.end = mspPackage.requestBuffer + mspPayloadSize;
            mspStarted = 1;
        } else { // this MSP packet is too big to fit in the buffer.
            sendMspErrorResponse(TELEMETRY_MSP_REQUEST_IS_TOO_BIG, mspPackage.requestPacket->cmd);
            return true;
        }
        mspStarted = 1;
    } else { // second onward chunk
        if (!mspStarted) { // no start packet yet, throw this one away
            return false;
        } else {
            if (((lastSeq + 1) & TELEMETRY_MSP_SEQ_MASK) != seqNumber) {
                // packet loss detected!
                mspStarted = 0;
                return false;
            }
        }
        sbufInit(&mspPackage.requestFrame, frameStart + 1, frameStart + payloadLength);
    }

    const uint8_t payloadExpecting = sbufBytesRemaining(&requestPacket->buf);
    const uint8_t payloadIncoming = sbufBytesRemaining(&mspPackage.requestFrame);
    uint8_t payload[payloadIncoming];

    if (payloadExpecting > payloadIncoming) {
        sbufReadData(&mspPackage.requestFrame, payload, payloadIncoming);
        sbufAdvance(&mspPackage.requestFrame, payloadIncoming);
        sbufWriteData(&requestPacket->buf, payload, payloadIncoming);
        lastSeq = seqNumber;

        return false;
    }

    mspStarted = 0;

    sbufReadData(&mspPackage.requestFrame, payload, payloadExpecting);
    sbufAdvance(&mspPackage.requestFrame, payloadExpecting);
    sbufWriteData(&requestPacket->buf, payload, payloadExpecting);
    sbufSwitchToReader(&requestPacket->buf, mspPackage.requestBuffer);
    processMspPacket();
    return true;
}

bool sendMspReply(uint8_t payloadSize, mspResponseFnPtr responseFn)
{
    static uint8_t seq = 0;
    static bool headerSent = false;

    uint8_t payloadOut[payloadSize];
    sbuf_t payload;
    sbuf_t *payloadBuf = sbufInit(&payload, payloadOut, payloadOut + payloadSize);
    sbuf_t *txBuf = &mspPackage.responsePacket->buf;

    // detect first reply packet
    if (!headerSent) {

        // header
        uint8_t status = TELEMETRY_MSP_START_MASK | (seq++ & TELEMETRY_MSP_SEQ_MASK) | (lastRequestVersion << TELEMETRY_MSP_VER_SHIFT);;
        if (mspPackage.responsePacket->result < 0) {
            status |= TELEMETRY_MSP_ERROR_MASK;
        }
        sbufWriteU8(payloadBuf, status);

        const uint8_t size = sbufBytesRemaining(txBuf);
        if (lastRequestVersion == 1) { // MSPv1
            sbufWriteU8(payloadBuf, size);
            sbufWriteU8(payloadBuf, mspPackage.responsePacket->cmd);
        } else { // MSPv2
            sbufWriteU8(payloadBuf, mspPackage.responsePacket->flags);  // MSPv2 flags
            sbufWriteU16(payloadBuf, mspPackage.responsePacket->cmd);    // command is 16 bit in MSPv2
            sbufWriteU16(payloadBuf, (uint16_t) size);        // size is 16 bit in MSPv2
        }
        headerSent = true;
    } else {
        sbufWriteU8(payloadBuf, (seq++ & TELEMETRY_MSP_SEQ_MASK) | (lastRequestVersion << TELEMETRY_MSP_VER_SHIFT)); // header without 'start' flag
    }

    const uint8_t bufferBytesRemaining = sbufBytesRemaining(txBuf);
    const uint8_t payloadBytesRemaining = sbufBytesRemaining(payloadBuf);
    uint8_t frame[payloadBytesRemaining];

    if (bufferBytesRemaining >= payloadBytesRemaining) {

        sbufReadData(txBuf, frame, payloadBytesRemaining);
        sbufAdvance(txBuf, payloadBytesRemaining);
        sbufWriteData(payloadBuf, frame, payloadBytesRemaining);
        responseFn(payloadOut, payloadSize);

        return true;
    }

    // last/only chunk
    sbufReadData(txBuf, frame, bufferBytesRemaining);
    sbufAdvance(txBuf, bufferBytesRemaining);
    sbufWriteData(payloadBuf, frame, bufferBytesRemaining);
    sbufSwitchToReader(txBuf, mspPackage.responseBuffer);

    responseFn(payloadOut, payloadBuf->ptr - payloadOut);
    headerSent = false; // <-- added: reset for the next response
    return false;
}

#endif
