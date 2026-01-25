#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_COARSEORIENTATION_MAX_SIZE 2
#define UAVCAN_COARSEORIENTATION_SIGNATURE (0x271BA10B0DAC9E52ULL)

#define UAVCAN_COARSEORIENTATION_ANGLE_MULTIPLIER 4.7746482927568605


struct uavcan_CoarseOrientation {
    int8_t fixed_axis_roll_pitch_yaw[3];
    bool orientation_defined;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_CoarseOrientation_encode(struct uavcan_CoarseOrientation* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_CoarseOrientation_decode(const CanardRxTransfer* transfer, struct uavcan_CoarseOrientation* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_CoarseOrientation_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_CoarseOrientation* msg, bool tao);
static inline bool _uavcan_CoarseOrientation_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_CoarseOrientation* msg, bool tao);
void _uavcan_CoarseOrientation_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_CoarseOrientation* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &msg->fixed_axis_roll_pitch_yaw[i]);
        *bit_ofs += 5;
    }
    canardEncodeScalar(buffer, *bit_ofs, 1, &msg->orientation_defined);
    *bit_ofs += 1;
}

/*
 decode uavcan_CoarseOrientation, return true on failure, false on success
*/
bool _uavcan_CoarseOrientation_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_CoarseOrientation* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 5, true, &msg->fixed_axis_roll_pitch_yaw[i]);
        *bit_ofs += 5;
    }

    canardDecodeScalar(transfer, *bit_ofs, 1, false, &msg->orientation_defined);
    *bit_ofs += 1;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_CoarseOrientation sample_uavcan_CoarseOrientation_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
