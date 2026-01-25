#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_GNSS_ECEFPOSITIONVELOCITY_MAX_SIZE 99
#define UAVCAN_EQUIPMENT_GNSS_ECEFPOSITIONVELOCITY_SIGNATURE (0x24A5DA4ABEE3A248ULL)


struct uavcan_equipment_gnss_ECEFPositionVelocity {
    float velocity_xyz[3];
    int64_t position_xyz_mm[3];
    struct { uint8_t len; float data[36]; }covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_gnss_ECEFPositionVelocity_encode(struct uavcan_equipment_gnss_ECEFPositionVelocity* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_gnss_ECEFPositionVelocity_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_ECEFPositionVelocity* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_gnss_ECEFPositionVelocity_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_ECEFPositionVelocity* msg, bool tao);
static inline bool _uavcan_equipment_gnss_ECEFPositionVelocity_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_ECEFPositionVelocity* msg, bool tao);
void _uavcan_equipment_gnss_ECEFPositionVelocity_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_ECEFPositionVelocity* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->velocity_xyz[i]);
        *bit_ofs += 32;
    }
    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 36, &msg->position_xyz_mm[i]);
        *bit_ofs += 36;
    }
    *bit_ofs += 6;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t covariance_len = msg->covariance.len > 36 ? 36 : msg->covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 6, &covariance_len);
        *bit_ofs += 6;
    }
    for (size_t i=0; i < covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_equipment_gnss_ECEFPositionVelocity, return true on failure, false on success
*/
bool _uavcan_equipment_gnss_ECEFPositionVelocity_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_ECEFPositionVelocity* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->velocity_xyz[i]);
        *bit_ofs += 32;
    }

    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 36, true, &msg->position_xyz_mm[i]);
        *bit_ofs += 36;
    }

    *bit_ofs += 6;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->covariance.len);
        *bit_ofs += 6;
    } else {
        msg->covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->covariance.len > 36) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_gnss_ECEFPositionVelocity sample_uavcan_equipment_gnss_ECEFPositionVelocity_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
