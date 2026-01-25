#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_CANIFACESTATS_MAX_SIZE 18
#define UAVCAN_PROTOCOL_CANIFACESTATS_SIGNATURE (0x13B106F0C44CA350ULL)


struct uavcan_protocol_CANIfaceStats {
    uint64_t frames_tx;
    uint64_t frames_rx;
    uint64_t errors;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_CANIfaceStats_encode(struct uavcan_protocol_CANIfaceStats* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_CANIfaceStats_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_CANIfaceStats* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_CANIfaceStats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_CANIfaceStats* msg, bool tao);
static inline bool _uavcan_protocol_CANIfaceStats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_CANIfaceStats* msg, bool tao);
void _uavcan_protocol_CANIfaceStats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_CANIfaceStats* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 48, &msg->frames_tx);
    *bit_ofs += 48;
    canardEncodeScalar(buffer, *bit_ofs, 48, &msg->frames_rx);
    *bit_ofs += 48;
    canardEncodeScalar(buffer, *bit_ofs, 48, &msg->errors);
    *bit_ofs += 48;
}

/*
 decode uavcan_protocol_CANIfaceStats, return true on failure, false on success
*/
bool _uavcan_protocol_CANIfaceStats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_CANIfaceStats* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 48, false, &msg->frames_tx);
    *bit_ofs += 48;

    canardDecodeScalar(transfer, *bit_ofs, 48, false, &msg->frames_rx);
    *bit_ofs += 48;

    canardDecodeScalar(transfer, *bit_ofs, 48, false, &msg->errors);
    *bit_ofs += 48;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_CANIfaceStats sample_uavcan_protocol_CANIfaceStats_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
