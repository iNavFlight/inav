#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.Timestamp.h>
#include <uavcan.equipment.gnss.ECEFPositionVelocity.h>


#define UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE 222
#define UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE (0xCA41E7000F37435FULL)
#define UAVCAN_EQUIPMENT_GNSS_FIX2_ID 1063

#define UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_NONE 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_TAI 1
#define UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_UTC 2
#define UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_GPS 3
#define UAVCAN_EQUIPMENT_GNSS_FIX2_NUM_LEAP_SECONDS_UNKNOWN 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_NO_FIX 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_TIME_ONLY 1
#define UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_2D_FIX 2
#define UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_3D_FIX 3
#define UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_SINGLE 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_DGPS 1
#define UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_RTK 2
#define UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_PPP 3
#define UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_DGPS_OTHER 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_DGPS_SBAS 1
#define UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_RTK_FLOAT 0
#define UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_RTK_FIXED 1

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_gnss_Fix2_cxx_iface;
#endif

struct uavcan_equipment_gnss_Fix2 {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_gnss_Fix2_cxx_iface;
#endif
    struct uavcan_Timestamp timestamp;
    struct uavcan_Timestamp gnss_timestamp;
    uint8_t gnss_time_standard;
    uint8_t num_leap_seconds;
    int64_t longitude_deg_1e8;
    int64_t latitude_deg_1e8;
    int32_t height_ellipsoid_mm;
    int32_t height_msl_mm;
    float ned_velocity[3];
    uint8_t sats_used;
    uint8_t status;
    uint8_t mode;
    uint8_t sub_mode;
    struct { uint8_t len; float data[36]; }covariance;
    float pdop;
    struct { uint8_t len; struct uavcan_equipment_gnss_ECEFPositionVelocity data[1]; }ecef_position_velocity;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_gnss_Fix2_encode(struct uavcan_equipment_gnss_Fix2* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_gnss_Fix2_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_Fix2* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_gnss_Fix2_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix2* msg, bool tao);
static inline bool _uavcan_equipment_gnss_Fix2_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix2* msg, bool tao);
void _uavcan_equipment_gnss_Fix2_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix2* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->timestamp, false);
    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->gnss_timestamp, false);
    canardEncodeScalar(buffer, *bit_ofs, 3, &msg->gnss_time_standard);
    *bit_ofs += 3;
    *bit_ofs += 13;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->num_leap_seconds);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 37, &msg->longitude_deg_1e8);
    *bit_ofs += 37;
    canardEncodeScalar(buffer, *bit_ofs, 37, &msg->latitude_deg_1e8);
    *bit_ofs += 37;
    canardEncodeScalar(buffer, *bit_ofs, 27, &msg->height_ellipsoid_mm);
    *bit_ofs += 27;
    canardEncodeScalar(buffer, *bit_ofs, 27, &msg->height_msl_mm);
    *bit_ofs += 27;
    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->ned_velocity[i]);
        *bit_ofs += 32;
    }
    canardEncodeScalar(buffer, *bit_ofs, 6, &msg->sats_used);
    *bit_ofs += 6;
    canardEncodeScalar(buffer, *bit_ofs, 2, &msg->status);
    *bit_ofs += 2;
    canardEncodeScalar(buffer, *bit_ofs, 4, &msg->mode);
    *bit_ofs += 4;
    canardEncodeScalar(buffer, *bit_ofs, 6, &msg->sub_mode);
    *bit_ofs += 6;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t covariance_len = msg->covariance.len > 36 ? 36 : msg->covariance.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 6, &covariance_len);
    *bit_ofs += 6;
    for (size_t i=0; i < covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->pdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t ecef_position_velocity_len = msg->ecef_position_velocity.len > 1 ? 1 : msg->ecef_position_velocity.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 1, &ecef_position_velocity_len);
        *bit_ofs += 1;
    }
    for (size_t i=0; i < ecef_position_velocity_len; i++) {
        _uavcan_equipment_gnss_ECEFPositionVelocity_encode(buffer, bit_ofs, &msg->ecef_position_velocity.data[i], false);
    }
}

/*
 decode uavcan_equipment_gnss_Fix2, return true on failure, false on success
*/
bool _uavcan_equipment_gnss_Fix2_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix2* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->timestamp, false)) {return true;}

    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->gnss_timestamp, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->gnss_time_standard);
    *bit_ofs += 3;

    *bit_ofs += 13;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->num_leap_seconds);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 37, true, &msg->longitude_deg_1e8);
    *bit_ofs += 37;

    canardDecodeScalar(transfer, *bit_ofs, 37, true, &msg->latitude_deg_1e8);
    *bit_ofs += 37;

    canardDecodeScalar(transfer, *bit_ofs, 27, true, &msg->height_ellipsoid_mm);
    *bit_ofs += 27;

    canardDecodeScalar(transfer, *bit_ofs, 27, true, &msg->height_msl_mm);
    *bit_ofs += 27;

    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->ned_velocity[i]);
        *bit_ofs += 32;
    }

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->sats_used);
    *bit_ofs += 6;

    canardDecodeScalar(transfer, *bit_ofs, 2, false, &msg->status);
    *bit_ofs += 2;

    canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->mode);
    *bit_ofs += 4;

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->sub_mode);
    *bit_ofs += 6;

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->covariance.len);
    *bit_ofs += 6;
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

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->pdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 1, false, &msg->ecef_position_velocity.len);
        *bit_ofs += 1;
    }


    if (tao) {
        msg->ecef_position_velocity.len = 0;
        size_t max_len = 1;
        uint32_t max_bits = (transfer->payload_len*8)-7; // TAO elements must be >= 8 bits
        while (max_bits > *bit_ofs) {
            if (!max_len-- || _uavcan_equipment_gnss_ECEFPositionVelocity_decode(transfer, bit_ofs, &msg->ecef_position_velocity.data[msg->ecef_position_velocity.len], false)) {return true;}
            msg->ecef_position_velocity.len++;
        }
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (msg->ecef_position_velocity.len > 1) {
            return true; /* invalid value */
        }
#pragma GCC diagnostic pop
        for (size_t i=0; i < msg->ecef_position_velocity.len; i++) {
            if (_uavcan_equipment_gnss_ECEFPositionVelocity_decode(transfer, bit_ofs, &msg->ecef_position_velocity.data[i], false)) {return true;}
        }
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_gnss_Fix2 sample_uavcan_equipment_gnss_Fix2_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_gnss_Fix2, UAVCAN_EQUIPMENT_GNSS_FIX2_ID, UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE, UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE);
#endif
#endif
