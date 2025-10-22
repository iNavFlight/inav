#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_MSP_OVER_TELEMETRY)

#include "build/build_config.h"

#include "common/utils.h"

#include "fc/fc_msp.h"

#include "msp/msp.h"

#include "telemetry/crsf.h"
#include "telemetry/msp_shared.h"
#include "telemetry/smartport.h"

#define TELEMETRY_MSP_VERSION    2
#define TELEMETRY_MSP_VER_SHIFT  5
#define TELEMETRY_MSP_VER_MASK   (0x7 << TELEMETRY_MSP_VER_SHIFT)
#define TELEMETRY_MSP_ERROR_FLAG (1 << 5)
#define TELEMETRY_MSP_START_FLAG (1 << 4)
#define TELEMETRY_MSP_SEQ_MASK   0x0F
#define TELEMETRY_MSP_RES_ERROR (-10)

#define TELEMETRY_MSP_MIN_FRAME_LENGTH 2
#define TELEMETRY_MSP_MIN_LENGTH_REQUEST_V1 3
#define TELEMETRY_MSP_MIN_LENGTH_REQUEST_V2 6

enum {
    TELEMETRY_MSP_VER_MISMATCH=0,
    TELEMETRY_MSP_CRC_ERROR=1,
    TELEMETRY_MSP_ERROR=2
};

STATIC_UNIT_TESTED uint8_t checksum = 0;
STATIC_UNIT_TESTED mspPackage_t mspPackage;
static mspRxBuffer_t mspRxBuffer;
static mspTxBuffer_t mspTxBuffer;
static mspPacket_t mspRxPacket;
static mspPacket_t mspTxPacket;

static uint8_t lastRequestVersion = 0;

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

bool handleMspFrame(uint8_t *frameStart, int frameLength)
{
    if (frameLength < TELEMETRY_MSP_MIN_FRAME_LENGTH) {
        return false;
    }

    static uint8_t mspStarted = 0;
    static uint8_t lastSeq = 0;

    if (sbufBytesRemaining(&mspPackage.responsePacket->buf) > 0) {
        mspStarted = 0;
    }

    if (mspStarted == 0) {
        initSharedMsp();
    }

    mspPacket_t *packet = mspPackage.requestPacket;
    sbuf_t *frameBuf = sbufInit(&mspPackage.requestFrame, frameStart, frameStart + (uint8_t)frameLength);
    sbuf_t *rxBuf = &mspPackage.requestPacket->buf;
    const uint8_t header = sbufReadU8(frameBuf);
    const uint8_t seqNumber = header & TELEMETRY_MSP_SEQ_MASK;
    lastRequestVersion = (header & TELEMETRY_MSP_VER_MASK) >> TELEMETRY_MSP_VER_SHIFT;

    if (lastRequestVersion == 0 || lastRequestVersion > TELEMETRY_MSP_VERSION) {
        sendMspErrorResponse(TELEMETRY_MSP_VER_MISMATCH, 0);
        return true;
    }

    if (header & TELEMETRY_MSP_START_FLAG) {
        // first packet in sequence
        uint16_t mspPayloadSize = sbufReadU8(frameBuf);

        if (lastRequestVersion == 1) { //MSP V1
            if (frameLength < TELEMETRY_MSP_MIN_LENGTH_REQUEST_V1) {
                return false;
            }
            
            mspPayloadSize = sbufBytesRemaining(frameBuf);

            packet->cmd = sbufReadU8(frameBuf);
            packet->result = 0;
            packet->buf.ptr = mspPackage.requestBuffer;
            packet->buf.end = mspPackage.requestBuffer + mspPayloadSize;

            checksum = mspPayloadSize ^ packet->cmd;
            mspStarted = 1;
        } else if (lastRequestVersion == 2) { //MSP V2
            if (frameLength < TELEMETRY_MSP_MIN_LENGTH_REQUEST_V2) {
                return false;
            }

            mspPayloadSize = sbufBytesRemaining(frameBuf);

            packet->flags = sbufReadU8(frameBuf);
            packet->cmd = sbufReadU16(frameBuf);

            packet->buf.ptr = mspPackage.requestBuffer;
            packet->buf.end = mspPackage.requestBuffer + mspPayloadSize;

            checksum = mspPayloadSize ^ packet->cmd;
            mspStarted = 1;
        }
        
        if (!mspStarted) {
            // no start packet yet, throw this one away
            return false;
        } else if (((lastSeq + 1) & TELEMETRY_MSP_SEQ_MASK) != seqNumber) {
            // packet loss detected!
            mspStarted = 0;
            return false;
        }
    }

    const uint8_t bufferBytesRemaining = sbufBytesRemaining(rxBuf);
    const uint8_t frameBytesRemaining = sbufBytesRemaining(frameBuf);
    uint8_t payload[frameBytesRemaining];

    if (bufferBytesRemaining >= frameBytesRemaining) {
        sbufReadData(frameBuf, payload, frameBytesRemaining);
        sbufAdvance(frameBuf, frameBytesRemaining);
        sbufWriteData(rxBuf, payload, frameBytesRemaining);
        lastSeq = seqNumber;

        return false;
    } else {
        sbufReadData(frameBuf, payload, bufferBytesRemaining);
        sbufAdvance(frameBuf, bufferBytesRemaining);
        sbufWriteData(rxBuf, payload, bufferBytesRemaining);
        sbufSwitchToReader(rxBuf, mspPackage.requestBuffer);
        while (sbufBytesRemaining(rxBuf)) {
            checksum ^= sbufReadU8(rxBuf);
        }

        if (checksum != *frameBuf->ptr) {
            mspStarted = 0;
            sendMspErrorResponse(TELEMETRY_MSP_CRC_ERROR, packet->cmd);
            return true;
        }
    }

    mspStarted = 0;
    sbufSwitchToReader(rxBuf, mspPackage.requestBuffer);
    processMspPacket();
    return true;
}

bool sendMspReply(uint8_t payloadSize, mspResponseFnPtr responseFn)
{
    static uint8_t checksum = 0;
    static uint8_t seq = 0;

    uint8_t payloadOut[payloadSize];
    sbuf_t payload;
    sbuf_t *payloadBuf = sbufInit(&payload, payloadOut, payloadOut + payloadSize);
    sbuf_t *txBuf = &mspPackage.responsePacket->buf;

    // detect first reply packet
    if (txBuf->ptr == mspPackage.responseBuffer) {

        // header
        uint8_t head = TELEMETRY_MSP_START_FLAG | (seq++ & TELEMETRY_MSP_SEQ_MASK);
        if (mspPackage.responsePacket->result < 0) {
            head |= TELEMETRY_MSP_ERROR_FLAG;
        }
        sbufWriteU8(payloadBuf, head);

        const int size = sbufBytesRemaining(txBuf);
        if (lastRequestVersion == 1) { // MSP V1
            if (size >= 0xff) {
                sbufWriteU8(payloadBuf, 0xff);
                sbufWriteU8(payloadBuf, (uint8_t)mspPackage.responsePacket->cmd);
                sbufWriteU16(payloadBuf, (uint16_t)size);
            } else {
                sbufWriteU8(payloadBuf, (uint8_t)size);
                sbufWriteU8(payloadBuf, (uint8_t)mspPackage.responsePacket->cmd);
            }
        } else if (lastRequestVersion == 2) { // MSP V2
            sbufWriteU8(payloadBuf, mspPackage.responsePacket->flags);
            sbufWriteU16(payloadBuf, mspPackage.responsePacket->cmd);
            sbufWriteU16(payloadBuf, (int16_t)size);
        }
    } else {
        // header
        sbufWriteU8(payloadBuf, (seq++ & TELEMETRY_MSP_SEQ_MASK));
    }

    const uint8_t bufferBytesRemaining = sbufBytesRemaining(txBuf);
    const uint8_t payloadBytesRemaining = sbufBytesRemaining(payloadBuf);
    uint8_t frame[payloadBytesRemaining];

    if (bufferBytesRemaining >= payloadBytesRemaining) {

        sbufReadData(txBuf, frame, payloadBytesRemaining);
        sbufAdvance(txBuf, payloadBytesRemaining);
        sbufWriteData(payloadBuf, frame, payloadBytesRemaining);
        responseFn(payloadOut);

        return true;

    } else {

        sbufReadData(txBuf, frame, bufferBytesRemaining);
        sbufAdvance(txBuf, bufferBytesRemaining);
        sbufWriteData(payloadBuf, frame, bufferBytesRemaining);
        sbufSwitchToReader(txBuf, mspPackage.responseBuffer);

        checksum = sbufBytesRemaining(txBuf) ^ mspPackage.responsePacket->cmd;

        while (sbufBytesRemaining(txBuf)) {
            checksum ^= sbufReadU8(txBuf);
        }
        sbufWriteU8(payloadBuf, checksum);

        while (sbufBytesRemaining(payloadBuf)>1) {
            sbufWriteU8(payloadBuf, 0);
        }

    }

    responseFn(payloadOut);
    return false;
}

#endif
