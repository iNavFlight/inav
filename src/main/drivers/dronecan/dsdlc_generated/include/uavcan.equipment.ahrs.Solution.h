#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.Timestamp.h>


#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_MAX_SIZE 84
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_SIGNATURE (0x72A63A3C6F41FA9BULL)
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ID 1000

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_ahrs_Solution_cxx_iface;
#endif

struct uavcan_equipment_ahrs_Solution {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_ahrs_Solution_cxx_iface;
#endif
    struct uavcan_Timestamp timestamp;
    float orientation_xyzw[4];
    struct { uint8_t len; float data[9]; }orientation_covariance;
    float angular_velocity[3];
    struct { uint8_t len; float data[9]; }angular_velocity_covariance;
    float linear_acceleration[3];
    struct { uint8_t len; float data[9]; }linear_acceleration_covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_ahrs_Solution_encode(struct uavcan_equipment_ahrs_Solution* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ahrs_Solution* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_ahrs_Solution_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ahrs_Solution* msg, bool tao);
static inline bool _uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ahrs_Solution* msg, bool tao);
void _uavcan_equipment_ahrs_Solution_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ahrs_Solution* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->timestamp, false);
    for (size_t i=0; i < 4; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->orientation_xyzw[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t orientation_covariance_len = msg->orientation_covariance.len > 9 ? 9 : msg->orientation_covariance.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 4, &orientation_covariance_len);
    *bit_ofs += 4;
    for (size_t i=0; i < orientation_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->orientation_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->angular_velocity[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t angular_velocity_covariance_len = msg->angular_velocity_covariance.len > 9 ? 9 : msg->angular_velocity_covariance.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 4, &angular_velocity_covariance_len);
    *bit_ofs += 4;
    for (size_t i=0; i < angular_velocity_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->angular_velocity_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->linear_acceleration[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t linear_acceleration_covariance_len = msg->linear_acceleration_covariance.len > 9 ? 9 : msg->linear_acceleration_covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 4, &linear_acceleration_covariance_len);
        *bit_ofs += 4;
    }
    for (size_t i=0; i < linear_acceleration_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->linear_acceleration_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_equipment_ahrs_Solution, return true on failure, false on success
*/
bool _uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ahrs_Solution* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->timestamp, false)) {return true;}

    for (size_t i=0; i < 4; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->orientation_xyzw[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    *bit_ofs += 4;

    canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->orientation_covariance.len);
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->orientation_covariance.len > 9) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->orientation_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->orientation_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->angular_velocity[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    *bit_ofs += 4;

    canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->angular_velocity_covariance.len);
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->angular_velocity_covariance.len > 9) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->angular_velocity_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->angular_velocity_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->linear_acceleration[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->linear_acceleration_covariance.len);
        *bit_ofs += 4;
    } else {
        msg->linear_acceleration_covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->linear_acceleration_covariance.len > 9) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->linear_acceleration_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->linear_acceleration_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_ahrs_Solution sample_uavcan_equipment_ahrs_Solution_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_ahrs_Solution, UAVCAN_EQUIPMENT_AHRS_SOLUTION_ID, UAVCAN_EQUIPMENT_AHRS_SOLUTION_SIGNATURE, UAVCAN_EQUIPMENT_AHRS_SOLUTION_MAX_SIZE);
#endif
#endif
