#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.debug.LogLevel.h>


#define UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE 123
#define UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SIGNATURE (0xD654A48E0C049D75ULL)
#define UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_ID 16383

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_debug_LogMessage_cxx_iface;
#endif

struct uavcan_protocol_debug_LogMessage {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_debug_LogMessage_cxx_iface;
#endif
    struct uavcan_protocol_debug_LogLevel level;
    struct { uint8_t len; uint8_t data[31]; }source;
    struct { uint8_t len; uint8_t data[90]; }text;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_debug_LogMessage_encode(struct uavcan_protocol_debug_LogMessage* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_debug_LogMessage_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_debug_LogMessage* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_debug_LogMessage_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogMessage* msg, bool tao);
static inline bool _uavcan_protocol_debug_LogMessage_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogMessage* msg, bool tao);
void _uavcan_protocol_debug_LogMessage_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogMessage* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_protocol_debug_LogLevel_encode(buffer, bit_ofs, &msg->level, false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t source_len = msg->source.len > 31 ? 31 : msg->source.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 5, &source_len);
    *bit_ofs += 5;
    for (size_t i=0; i < source_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->source.data[i]);
        *bit_ofs += 8;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t text_len = msg->text.len > 90 ? 90 : msg->text.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 7, &text_len);
        *bit_ofs += 7;
    }
    for (size_t i=0; i < text_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->text.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_debug_LogMessage, return true on failure, false on success
*/
bool _uavcan_protocol_debug_LogMessage_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogMessage* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_protocol_debug_LogLevel_decode(transfer, bit_ofs, &msg->level, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->source.len);
    *bit_ofs += 5;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->source.len > 31) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->source.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->source.data[i]);
        *bit_ofs += 8;
    }

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->text.len);
        *bit_ofs += 7;
    } else {
        msg->text.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->text.len > 90) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->text.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->text.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_debug_LogMessage sample_uavcan_protocol_debug_LogMessage_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_protocol_debug_LogMessage, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_ID, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SIGNATURE, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE);
#endif
#endif
