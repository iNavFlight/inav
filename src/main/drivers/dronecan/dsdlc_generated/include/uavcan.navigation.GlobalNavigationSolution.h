#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.Timestamp.h>


#define UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_MAX_SIZE 233
#define UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_SIGNATURE (0x463B10CCCBE51C3DULL)
#define UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_ID 2000

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_navigation_GlobalNavigationSolution_cxx_iface;
#endif

struct uavcan_navigation_GlobalNavigationSolution {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_navigation_GlobalNavigationSolution_cxx_iface;
#endif
    struct uavcan_Timestamp timestamp;
    double longitude;
    double latitude;
    float height_ellipsoid;
    float height_msl;
    float height_agl;
    float height_baro;
    float qnh_hpa;
    float orientation_xyzw[4];
    struct { uint8_t len; float data[36]; }pose_covariance;
    float linear_velocity_body[3];
    float angular_velocity_body[3];
    float linear_acceleration_body[3];
    struct { uint8_t len; float data[36]; }velocity_covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_navigation_GlobalNavigationSolution_encode(struct uavcan_navigation_GlobalNavigationSolution* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_navigation_GlobalNavigationSolution_decode(const CanardRxTransfer* transfer, struct uavcan_navigation_GlobalNavigationSolution* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_navigation_GlobalNavigationSolution_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_navigation_GlobalNavigationSolution* msg, bool tao);
static inline bool _uavcan_navigation_GlobalNavigationSolution_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_navigation_GlobalNavigationSolution* msg, bool tao);
void _uavcan_navigation_GlobalNavigationSolution_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_navigation_GlobalNavigationSolution* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->timestamp, false);
    canardEncodeScalar(buffer, *bit_ofs, 64, &msg->longitude);
    *bit_ofs += 64;
    canardEncodeScalar(buffer, *bit_ofs, 64, &msg->latitude);
    *bit_ofs += 64;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->height_ellipsoid);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->height_msl);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->height_agl);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->height_baro);
    *bit_ofs += 32;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->qnh_hpa);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    for (size_t i=0; i < 4; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->orientation_xyzw[i]);
        *bit_ofs += 32;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t pose_covariance_len = msg->pose_covariance.len > 36 ? 36 : msg->pose_covariance.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 6, &pose_covariance_len);
    *bit_ofs += 6;
    for (size_t i=0; i < pose_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->pose_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->linear_velocity_body[i]);
        *bit_ofs += 32;
    }
    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->angular_velocity_body[i]);
        *bit_ofs += 32;
    }
    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->linear_acceleration_body[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t velocity_covariance_len = msg->velocity_covariance.len > 36 ? 36 : msg->velocity_covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 6, &velocity_covariance_len);
        *bit_ofs += 6;
    }
    for (size_t i=0; i < velocity_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->velocity_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_navigation_GlobalNavigationSolution, return true on failure, false on success
*/
bool _uavcan_navigation_GlobalNavigationSolution_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_navigation_GlobalNavigationSolution* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->timestamp, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 64, true, &msg->longitude);
    *bit_ofs += 64;

    canardDecodeScalar(transfer, *bit_ofs, 64, true, &msg->latitude);
    *bit_ofs += 64;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->height_ellipsoid);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->height_msl);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->height_agl);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->height_baro);
    *bit_ofs += 32;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->qnh_hpa = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    for (size_t i=0; i < 4; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->orientation_xyzw[i]);
        *bit_ofs += 32;
    }

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->pose_covariance.len);
    *bit_ofs += 6;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->pose_covariance.len > 36) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->pose_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->pose_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->linear_velocity_body[i]);
        *bit_ofs += 32;
    }

    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->angular_velocity_body[i]);
        *bit_ofs += 32;
    }

    for (size_t i=0; i < 3; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->linear_acceleration_body[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->velocity_covariance.len);
        *bit_ofs += 6;
    } else {
        msg->velocity_covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->velocity_covariance.len > 36) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->velocity_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->velocity_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_navigation_GlobalNavigationSolution sample_uavcan_navigation_GlobalNavigationSolution_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_navigation_GlobalNavigationSolution, UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_ID, UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_SIGNATURE, UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_MAX_SIZE);
#endif
#endif
