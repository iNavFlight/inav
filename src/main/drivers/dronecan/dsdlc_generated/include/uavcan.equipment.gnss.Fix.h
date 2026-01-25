#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.Timestamp.h>


#define UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE 79
#define UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE (0x54C1572B9E07F297ULL)
#define UAVCAN_EQUIPMENT_GNSS_FIX_ID 1060

#define UAVCAN_EQUIPMENT_GNSS_FIX_GNSS_TIME_STANDARD_NONE 0
#define UAVCAN_EQUIPMENT_GNSS_FIX_GNSS_TIME_STANDARD_TAI 1
#define UAVCAN_EQUIPMENT_GNSS_FIX_GNSS_TIME_STANDARD_UTC 2
#define UAVCAN_EQUIPMENT_GNSS_FIX_GNSS_TIME_STANDARD_GPS 3
#define UAVCAN_EQUIPMENT_GNSS_FIX_NUM_LEAP_SECONDS_UNKNOWN 0
#define UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_NO_FIX 0
#define UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_TIME_ONLY 1
#define UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_2D_FIX 2
#define UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_3D_FIX 3

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_gnss_Fix_cxx_iface;
#endif

struct uavcan_equipment_gnss_Fix {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_gnss_Fix_cxx_iface;
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
    float pdop;
    struct { uint8_t len; float data[9]; }position_covariance;
    struct { uint8_t len; float data[9]; }velocity_covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_gnss_Fix_encode(struct uavcan_equipment_gnss_Fix* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_gnss_Fix_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_Fix* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_gnss_Fix_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix* msg, bool tao);
static inline bool _uavcan_equipment_gnss_Fix_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix* msg, bool tao);
void _uavcan_equipment_gnss_Fix_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->timestamp, false);
    _uavcan_Timestamp_encode(buffer, bit_ofs, &msg->gnss_timestamp, false);
    canardEncodeScalar(buffer, *bit_ofs, 3, &msg->gnss_time_standard);
    *bit_ofs += 3;
    *bit_ofs += 5;
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
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->ned_velocity[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
    canardEncodeScalar(buffer, *bit_ofs, 6, &msg->sats_used);
    *bit_ofs += 6;
    canardEncodeScalar(buffer, *bit_ofs, 2, &msg->status);
    *bit_ofs += 2;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->pdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t position_covariance_len = msg->position_covariance.len > 9 ? 9 : msg->position_covariance.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 4, &position_covariance_len);
    *bit_ofs += 4;
    for (size_t i=0; i < position_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->position_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t velocity_covariance_len = msg->velocity_covariance.len > 9 ? 9 : msg->velocity_covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 4, &velocity_covariance_len);
        *bit_ofs += 4;
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
 decode uavcan_equipment_gnss_Fix, return true on failure, false on success
*/
bool _uavcan_equipment_gnss_Fix_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Fix* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->timestamp, false)) {return true;}

    if (_uavcan_Timestamp_decode(transfer, bit_ofs, &msg->gnss_timestamp, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->gnss_time_standard);
    *bit_ofs += 3;

    *bit_ofs += 5;

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
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->ned_velocity[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->sats_used);
    *bit_ofs += 6;

    canardDecodeScalar(transfer, *bit_ofs, 2, false, &msg->status);
    *bit_ofs += 2;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->pdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    *bit_ofs += 4;

    canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->position_covariance.len);
    *bit_ofs += 4;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->position_covariance.len > 9) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->position_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->position_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->velocity_covariance.len);
        *bit_ofs += 4;
    } else {
        msg->velocity_covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->velocity_covariance.len > 9) {
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
struct uavcan_equipment_gnss_Fix sample_uavcan_equipment_gnss_Fix_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_gnss_Fix, UAVCAN_EQUIPMENT_GNSS_FIX_ID, UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE, UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE);
#endif
#endif
