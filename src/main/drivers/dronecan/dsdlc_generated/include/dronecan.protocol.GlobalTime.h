#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.Timestamp.h>


#define DRONECAN_PROTOCOL_GLOBALTIME_MAX_SIZE 7
#define DRONECAN_PROTOCOL_GLOBALTIME_SIGNATURE (0xA55177448A490F33ULL)
#define DRONECAN_PROTOCOL_GLOBALTIME_ID 344

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_protocol_GlobalTime_cxx_iface;
#endif

struct dronecan_protocol_GlobalTime {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_protocol_GlobalTime_cxx_iface;
#endif
    struct uavcan_Timestamp timestamp;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_protocol_GlobalTime_encode(struct dronecan_protocol_GlobalTime* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_protocol_GlobalTime_decode(const CanardRxTransfer* transfer, struct dronecan_protocol_GlobalTime* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_protocol_GlobalTime_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_GlobalTime* msg, bool tao);
static inline bool _dronecan_protocol_GlobalTime_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_GlobalTime* msg, bool tao);
void _dronecan_protocol_GlobalTime_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_GlobalTime* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->timestamp, tao);
}

/*
 decode dronecan_protocol_GlobalTime, return true on failure, false on success
*/
bool _dronecan_protocol_GlobalTime_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_GlobalTime* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->timestamp, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_protocol_GlobalTime sample_dronecan_protocol_GlobalTime_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_protocol_GlobalTime, DRONECAN_PROTOCOL_GLOBALTIME_ID, DRONECAN_PROTOCOL_GLOBALTIME_SIGNATURE, DRONECAN_PROTOCOL_GLOBALTIME_MAX_SIZE);
#endif
#endif
