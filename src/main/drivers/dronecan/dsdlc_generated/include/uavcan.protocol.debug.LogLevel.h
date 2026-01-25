#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_MAX_SIZE 1
#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_SIGNATURE (0x711BF141AF572346ULL)

#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_DEBUG 0
#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_INFO 1
#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_WARNING 2
#define UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_ERROR 3


struct uavcan_protocol_debug_LogLevel {
    uint8_t value;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_debug_LogLevel_encode(struct uavcan_protocol_debug_LogLevel* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_debug_LogLevel_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_debug_LogLevel* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_debug_LogLevel_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogLevel* msg, bool tao);
static inline bool _uavcan_protocol_debug_LogLevel_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogLevel* msg, bool tao);
void _uavcan_protocol_debug_LogLevel_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogLevel* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 3, &msg->value);
    *bit_ofs += 3;
}

/*
 decode uavcan_protocol_debug_LogLevel, return true on failure, false on success
*/
bool _uavcan_protocol_debug_LogLevel_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_debug_LogLevel* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->value);
    *bit_ofs += 3;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_debug_LogLevel sample_uavcan_protocol_debug_LogLevel_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
