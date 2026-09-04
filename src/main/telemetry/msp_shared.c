#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_MSP_OVER_TELEMETRY)

#include "build/build_config.h"

#include "common/maths.h"
#include "common/utils.h"

#include "fc/fc_msp.h"

#include "msp/msp.h"

#include "telemetry/msp_shared.h"

#define TELEMETRY_MSP_VERSION    2
#define TELEMETRY_MSP_RES_ERROR (-10)

enum {
    TELEMETRY_MSP_SEQ_MASK   = 0x0f,
    TELEMETRY_MSP_VER_MASK   = 0x60,
    TELEMETRY_MSP_START_MASK = 0x10,
    TELEMETRY_MSP_ERROR_MASK = 0x80,
    TELEMETRY_MSP_VER_SHIFT  = 5,
};

enum {
    TELEMETRY_MSP_VER_MISMATCH = 0,
    TELEMETRY_MSP_ERROR = 2,
    TELEMETRY_MSP_REQUEST_IS_TOO_BIG = 3,
};

enum {
    MIN_LENGTH_CHUNK      = 2,
    MIN_LENGTH_REQUEST_V1 = 3,
    MIN_LENGTH_REQUEST_V2 = 6,
};

enum {
    MSP_INDEX_STATUS     = 0,
    MSP_INDEX_SIZE_V1    = MSP_INDEX_STATUS + 1,
    MSP_INDEX_ID_V1      = MSP_INDEX_SIZE_V1 + 1,
    MSP_INDEX_PAYLOAD_V1 = MSP_INDEX_ID_V1 + 1,

    MSP_INDEX_FLAG_V2    = MSP_INDEX_SIZE_V1,
    MSP_INDEX_ID_LO      = MSP_INDEX_ID_V1,
    MSP_INDEX_ID_HI      = MSP_INDEX_ID_LO + 1,
    MSP_INDEX_SIZE_V2_LO = MSP_INDEX_ID_HI + 1,
    MSP_INDEX_SIZE_V2_HI = MSP_INDEX_SIZE_V2_LO + 1,
    MSP_INDEX_PAYLOAD_V2 = MSP_INDEX_SIZE_V2_HI + 1,
};

void resetSharedMsp(mspSharedContext_t *context)
{
    if (!context) {
        return;
    }

    // Preserve caller-owned storage across a protocol-state reset.
    uint8_t *requestBuffer = context->requestBuffer;
    const uint16_t requestBufferSize = context->requestBufferSize;
    uint8_t *responseBuffer = context->responseBuffer;
    const uint16_t responseBufferSize = context->responseBufferSize;

    memset(context, 0, sizeof(*context));
    context->requestBuffer = requestBuffer;
    context->requestBufferSize = requestBufferSize;
    context->responseBuffer = responseBuffer;
    context->responseBufferSize = responseBufferSize;

    if (requestBuffer && requestBufferSize) {
        sbufInit(&context->requestPacket.buf, requestBuffer, requestBuffer + requestBufferSize);
    }
    if (responseBuffer) {
        sbufInit(&context->responsePacket.buf, responseBuffer, responseBuffer);
    }
}

void initSharedMsp(mspSharedContext_t *context,
    uint8_t *requestBuffer, uint16_t requestBufferSize,
    uint8_t *responseBuffer, uint16_t responseBufferSize)
{
    if (!context) {
        return;
    }

    memset(context, 0, sizeof(*context));
    context->requestBuffer = requestBuffer;
    context->requestBufferSize = requestBufferSize;
    context->responseBuffer = responseBuffer;
    context->responseBufferSize = responseBufferSize;
    resetSharedMsp(context);
}

bool sharedMspRequestPending(const mspSharedContext_t *context)
{
    return context && context->requestPending;
}

bool sharedMspReplyPending(const mspSharedContext_t *context)
{
    return context && context->responsePending;
}

bool sharedMspEndpointBusy(const mspSharedContext_t *context)
{
    return context && (context->requestPending || context->responsePending);
}

static bool processMspPacket(mspSharedContext_t *context, mspResponseCompleteFnPtr completeFn)
{
    mspPacket_t *response = &context->responsePacket;

    response->cmd = 0;
    response->flags = 0;
    response->result = 0;
    sbufInit(&response->buf,
        context->responseBuffer,
        context->responseBuffer + context->responseBufferSize);

    mspPostProcessFnPtr mspPostProcessFn = NULL;
    const mspResult_e status = mspFcProcessCommand(&context->requestPacket, response, &mspPostProcessFn);
    if (status == MSP_RESULT_ERROR) {
        sbufWriteU8(&response->buf, TELEMETRY_MSP_ERROR);
    }
    if (mspPostProcessFn) {
        mspPostProcessFn(NULL);
    }

    if (status == MSP_RESULT_NO_REPLY) {
        context->responsePending = false;
        sbufInit(&response->buf, context->responseBuffer, context->responseBuffer);
        // As with the last reply fragment, transport-owned transaction state
        // must be released before requestPending exposes a free endpoint to an
        // RX interrupt.
        if (completeFn) {
            completeFn();
        }
        context->requestPending = false;
        return false;
    }

    sbufSwitchToReader(&response->buf, context->responseBuffer);
    // Commit the response before releasing the request buffer. An RX callback
    // sees requestPending || responsePending and therefore cannot overwrite the
    // request while mspFcProcessCommand() is reading it.
    context->responsePending = true;
    context->requestPending = false;
    return true;
}

static void sendMspErrorResponse(mspSharedContext_t *context, uint8_t error, int16_t cmd)
{
    mspPacket_t *response = &context->responsePacket;
    response->cmd = cmd;
    response->flags = 0;
    response->result = TELEMETRY_MSP_RES_ERROR;
    sbufInit(&response->buf,
        context->responseBuffer,
        context->responseBuffer + context->responseBufferSize);
    sbufWriteU8(&response->buf, error);
    sbufSwitchToReader(&response->buf, context->responseBuffer);
    context->responsePending = true;
}

static void beginRequest(mspSharedContext_t *context, uint16_t payloadSize)
{
    mspPacket_t *request = &context->requestPacket;
    sbufInit(&request->buf,
        context->requestBuffer,
        context->requestBuffer + payloadSize);
    request->result = 0;
    context->receivingRequest = true;
}

bool receiveMspFrame(mspSharedContext_t *context, const uint8_t *frameStart, int payloadLength)
{
    if (!context || !frameStart || payloadLength < MIN_LENGTH_CHUNK) {
        return false;
    }

    /*
     * Do not let a new request overwrite a response that is still being
     * fragmented onto the transport. Endpoints are request/reply channels.
     */
    if (sharedMspEndpointBusy(context)) {
        return false;
    }

    mspPacket_t *request = &context->requestPacket;
    const uint8_t status = frameStart[MSP_INDEX_STATUS];
    const uint8_t seqNumber = status & TELEMETRY_MSP_SEQ_MASK;
    const uint8_t requestVersion = (status & TELEMETRY_MSP_VER_MASK) >> TELEMETRY_MSP_VER_SHIFT;

    if (requestVersion > TELEMETRY_MSP_VERSION) {
        context->receivingRequest = false;
        context->requestVersion = requestVersion;
        sendMspErrorResponse(context, TELEMETRY_MSP_VER_MISMATCH, 0);
        return true;
    }

    const uint8_t *payloadStart;
    int incomingPayloadLength;

    if (status & TELEMETRY_MSP_START_MASK) {
        uint16_t mspPayloadSize;

        context->receivingRequest = false;
        context->requestVersion = requestVersion;
        context->lastRxSeq = seqNumber;

        if (requestVersion == 1) {
            if (payloadLength < MIN_LENGTH_REQUEST_V1) {
                return false;
            }
            mspPayloadSize = frameStart[MSP_INDEX_SIZE_V1];
            request->cmd = frameStart[MSP_INDEX_ID_V1];
            request->flags = 0;
            payloadStart = frameStart + MSP_INDEX_PAYLOAD_V1;
            incomingPayloadLength = payloadLength - MSP_INDEX_PAYLOAD_V1;
        } else {
            if (payloadLength < MIN_LENGTH_REQUEST_V2) {
                return false;
            }
            request->flags = frameStart[MSP_INDEX_FLAG_V2];
            request->cmd = (uint16_t)frameStart[MSP_INDEX_ID_LO] |
                ((uint16_t)frameStart[MSP_INDEX_ID_HI] << 8);
            mspPayloadSize = (uint16_t)frameStart[MSP_INDEX_SIZE_V2_LO] |
                ((uint16_t)frameStart[MSP_INDEX_SIZE_V2_HI] << 8);
            payloadStart = frameStart + MSP_INDEX_PAYLOAD_V2;
            incomingPayloadLength = payloadLength - MSP_INDEX_PAYLOAD_V2;
        }

        if (mspPayloadSize > context->requestBufferSize) {
            sendMspErrorResponse(context, TELEMETRY_MSP_REQUEST_IS_TOO_BIG, request->cmd);
            return true;
        }

        beginRequest(context, mspPayloadSize);
    } else {
        if (!context->receivingRequest || requestVersion != context->requestVersion) {
            return false;
        }
        if (((context->lastRxSeq + 1) & TELEMETRY_MSP_SEQ_MASK) != seqNumber) {
            context->receivingRequest = false;
            return false;
        }
        context->lastRxSeq = seqNumber;
        payloadStart = frameStart + 1;
        incomingPayloadLength = payloadLength - 1;
    }

    const int payloadExpected = sbufBytesRemaining(&request->buf);
    const int bytesToCopy = MIN(payloadExpected, incomingPayloadLength);
    if (bytesToCopy > 0) {
        sbufWriteData(&request->buf, payloadStart, bytesToCopy);
    }

    if (sbufBytesRemaining(&request->buf) > 0) {
        return false;
    }

    context->receivingRequest = false;
    sbufSwitchToReader(&request->buf, context->requestBuffer);
    context->requestPending = true;
    return true;
}

bool processMspRequest(mspSharedContext_t *context, mspResponseCompleteFnPtr completeFn)
{
    if (!sharedMspRequestPending(context)) {
        return sharedMspReplyPending(context);
    }
    return processMspPacket(context, completeFn);
}

bool handleMspFrame(mspSharedContext_t *context, const uint8_t *frameStart, int payloadLength)
{
    if (receiveMspFrame(context, frameStart, payloadLength) && sharedMspRequestPending(context)) {
        processMspRequest(context, NULL);
    }
    return sharedMspReplyPending(context);
}

bool sendMspReply(mspSharedContext_t *context, uint8_t payloadSize, mspResponseFnPtr responseFn, mspResponseCompleteFnPtr completeFn)
{
    if (!context || !responseFn || payloadSize == 0 || !sharedMspReplyPending(context)) {
        return false;
    }

    uint8_t payloadOut[payloadSize];
    sbuf_t payload;
    sbuf_t *payloadBuf = sbufInit(&payload, payloadOut, payloadOut + payloadSize);
    mspPacket_t *response = &context->responsePacket;
    sbuf_t *txBuf = &response->buf;

    if (!context->responseHeaderSent) {
        uint8_t status = TELEMETRY_MSP_START_MASK |
            (context->txSeq++ & TELEMETRY_MSP_SEQ_MASK) |
            (context->requestVersion << TELEMETRY_MSP_VER_SHIFT);
        if (response->result < 0) {
            status |= TELEMETRY_MSP_ERROR_MASK;
        }
        sbufWriteU8(payloadBuf, status);

        const uint16_t size = sbufBytesRemaining(txBuf);
        if (context->requestVersion == 1) {
            sbufWriteU8(payloadBuf, size);
            sbufWriteU8(payloadBuf, response->cmd);
        } else {
            sbufWriteU8(payloadBuf, response->flags);
            sbufWriteU16(payloadBuf, response->cmd);
            sbufWriteU16(payloadBuf, size);
        }
        context->responseHeaderSent = true;
    } else {
        sbufWriteU8(payloadBuf,
            (context->txSeq++ & TELEMETRY_MSP_SEQ_MASK) |
            (context->requestVersion << TELEMETRY_MSP_VER_SHIFT));
    }

    const int responseBytes = sbufBytesRemaining(txBuf);
    const int payloadBytes = sbufBytesRemaining(payloadBuf);
    const int bytesToCopy = MIN(responseBytes, payloadBytes);
    if (bytesToCopy > 0) {
        sbufWriteData(payloadBuf, sbufPtr(txBuf), bytesToCopy);
        sbufAdvance(txBuf, bytesToCopy);
    }

    responseFn(payloadOut, (uint8_t)(payloadBuf->ptr - payloadOut));

    if (sbufBytesRemaining(txBuf) > 0) {
        return true;
    }

    // Keep the endpoint busy until both generic response state and any
    // transport-owned transaction identity have been reset. An RX callback may
    // start a new request as soon as responsePending becomes false.
    context->responseHeaderSent = false;
    sbufInit(txBuf, context->responseBuffer, context->responseBuffer);
    if (completeFn) {
        completeFn();
    }
    context->responsePending = false;
    return false;
}

#endif
