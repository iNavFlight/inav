#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/streambuf.h"
#include "msp/msp.h"

typedef void (*mspResponseFnPtr)(uint8_t *payload, const uint8_t payloadSize);
typedef void (*mspResponseCompleteFnPtr)(void);

/*
 * MSP-over-telemetry state belongs to one transport endpoint. The caller owns
 * the request/response storage so each transport can reserve only the space it
 * actually needs (for example SmartPort is much smaller than CRSF).
 */
typedef struct mspSharedContext_s {
    uint8_t *requestBuffer;
    uint16_t requestBufferSize;
    uint8_t *responseBuffer;
    uint16_t responseBufferSize;
    mspPacket_t requestPacket;
    mspPacket_t responsePacket;
    uint8_t requestVersion;
    uint8_t lastRxSeq;
    uint8_t txSeq;
    volatile bool receivingRequest;
    volatile bool requestPending;
    volatile bool responsePending;
    bool responseHeaderSent;
} mspSharedContext_t;

void initSharedMsp(mspSharedContext_t *context,
    uint8_t *requestBuffer, uint16_t requestBufferSize,
    uint8_t *responseBuffer, uint16_t responseBufferSize);
void resetSharedMsp(mspSharedContext_t *context);

// receiveMspFrame() only assembles/parses the transport request. It is safe
// for an RX callback because it never dispatches an MSP command. A completed
// request is executed later by processMspRequest() in normal task context.
bool receiveMspFrame(mspSharedContext_t *context, const uint8_t *frameStart, int payloadLength);
bool processMspRequest(mspSharedContext_t *context, mspResponseCompleteFnPtr completeFn);

// Convenience path for transports already running in task context.
bool handleMspFrame(mspSharedContext_t *context, const uint8_t *frameStart, int payloadLength);
bool sendMspReply(mspSharedContext_t *context, uint8_t payloadSize, mspResponseFnPtr responseFn, mspResponseCompleteFnPtr completeFn);
bool sharedMspRequestPending(const mspSharedContext_t *context);
bool sharedMspReplyPending(const mspSharedContext_t *context);
bool sharedMspEndpointBusy(const mspSharedContext_t *context);
