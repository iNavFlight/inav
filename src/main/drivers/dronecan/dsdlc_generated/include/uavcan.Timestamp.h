#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_TIMESTAMP_MAX_SIZE 7
#define UAVCAN_TIMESTAMP_SIGNATURE (0x5BD0B5C81087E0DULL)

#define UAVCAN_TIMESTAMP_UNKNOWN 0


struct uavcan_Timestamp {
    uint64_t usec;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_Timestamp_encode(struct uavcan_Timestamp* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_Timestamp_decode(const CanardRxTransfer* transfer, struct uavcan_Timestamp* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_Timestamp_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_Timestamp* msg, bool tao);
static inline bool _uavcan_Timestamp_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_Timestamp* msg, bool tao);
void _uavcan_Timestamp_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_Timestamp* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 56, &msg->usec);
    *bit_ofs += 56;
}

/*
 decode uavcan_Timestamp, return true on failure, false on success
*/
bool _uavcan_Timestamp_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_Timestamp* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 56, false, &msg->usec);
    *bit_ofs += 56;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_Timestamp sample_uavcan_Timestamp_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
