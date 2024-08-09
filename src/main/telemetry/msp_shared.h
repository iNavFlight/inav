#pragma once

#define MSP_TLM_INBUF_SIZE 128
#define MSP_TLM_OUTBUF_SIZE 128

// type of function to send MSP response chunk over telemetry.
typedef void (*mspResponseFnPtr)(uint8_t *payload, const uint8_t payloadSize);

void initSharedMsp(void);

// receives telemetry payload with msp and handles it.
bool handleMspFrame(uint8_t *const payload, uint8_t const payloadLength, uint8_t *const skipsBeforeResponse);

// sends MSP reply from previously handled msp-request over telemetry
bool sendMspReply(const uint8_t payloadSize_max, mspResponseFnPtr responseFn);