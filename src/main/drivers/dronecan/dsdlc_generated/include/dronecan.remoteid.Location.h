#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_REMOTEID_LOCATION_MAX_SIZE 58
#define DRONECAN_REMOTEID_LOCATION_SIGNATURE (0xEAA3A2C5BCB14CAAULL)
#define DRONECAN_REMOTEID_LOCATION_ID 20031

#define DRONECAN_REMOTEID_LOCATION_ODID_STATUS_UNDECLARED 0
#define DRONECAN_REMOTEID_LOCATION_ODID_STATUS_GROUND 1
#define DRONECAN_REMOTEID_LOCATION_ODID_STATUS_AIRBORNE 2
#define DRONECAN_REMOTEID_LOCATION_ODID_STATUS_EMERGENCY 3
#define DRONECAN_REMOTEID_LOCATION_ODID_HEIGHT_REF_OVER_TAKEOFF 0
#define DRONECAN_REMOTEID_LOCATION_ODID_HEIGHT_REF_OVER_GROUND 1
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_UNKNOWN 0
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_10NM 1
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_4NM 2
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_2NM 3
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_1NM 4
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_0_5NM 5
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_0_3NM 6
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_0_1NM 7
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_0_05NM 8
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_30_METER 9
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_10_METER 10
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_3_METER 11
#define DRONECAN_REMOTEID_LOCATION_ODID_HOR_ACC_1_METER 12
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_UNKNOWN 0
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_150_METER 1
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_45_METER 2
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_25_METER 3
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_10_METER 4
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_3_METER 5
#define DRONECAN_REMOTEID_LOCATION_ODID_VER_ACC_1_METER 6
#define DRONECAN_REMOTEID_LOCATION_ODID_SPEED_ACC_UNKNOWN 0
#define DRONECAN_REMOTEID_LOCATION_ODID_SPEED_ACC_10_METERS_PER_SECOND 1
#define DRONECAN_REMOTEID_LOCATION_ODID_SPEED_ACC_3_METERS_PER_SECOND 2
#define DRONECAN_REMOTEID_LOCATION_ODID_SPEED_ACC_1_METERS_PER_SECOND 3
#define DRONECAN_REMOTEID_LOCATION_ODID_SPEED_ACC_0_3_METERS_PER_SECOND 4
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_1_SECOND 1
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_2_SECOND 2
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_3_SECOND 3
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_4_SECOND 4
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_5_SECOND 5
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_6_SECOND 6
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_7_SECOND 7
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_8_SECOND 8
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_0_9_SECOND 9
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_0_SECOND 10
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_1_SECOND 11
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_2_SECOND 12
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_3_SECOND 13
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_4_SECOND 14
#define DRONECAN_REMOTEID_LOCATION_ODID_TIME_ACC_1_5_SECOND 15

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_remoteid_Location_cxx_iface;
#endif

struct dronecan_remoteid_Location {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_remoteid_Location_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[20]; }id_or_mac;
    uint8_t status;
    uint16_t direction;
    uint16_t speed_horizontal;
    int16_t speed_vertical;
    int32_t latitude;
    int32_t longitude;
    float altitude_barometric;
    float altitude_geodetic;
    uint8_t height_reference;
    float height;
    uint8_t horizontal_accuracy;
    uint8_t vertical_accuracy;
    uint8_t barometer_accuracy;
    uint8_t speed_accuracy;
    float timestamp;
    uint8_t timestamp_accuracy;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_remoteid_Location_encode(struct dronecan_remoteid_Location* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_remoteid_Location_decode(const CanardRxTransfer* transfer, struct dronecan_remoteid_Location* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_remoteid_Location_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_Location* msg, bool tao);
static inline bool _dronecan_remoteid_Location_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_Location* msg, bool tao);
void _dronecan_remoteid_Location_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_Location* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t id_or_mac_len = msg->id_or_mac.len > 20 ? 20 : msg->id_or_mac.len;
#pragma GCC diagnostic pop
    canardEncodeScalar(buffer, *bit_ofs, 5, &id_or_mac_len);
    *bit_ofs += 5;
    for (size_t i=0; i < id_or_mac_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->id_or_mac.data[i]);
        *bit_ofs += 8;
    }
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->status);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->direction);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->speed_horizontal);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->speed_vertical);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->latitude);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->longitude);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->altitude_barometric);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->altitude_geodetic);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->height_reference);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->height);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->horizontal_accuracy);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->vertical_accuracy);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->barometer_accuracy);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->speed_accuracy);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->timestamp);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->timestamp_accuracy);
    *bit_ofs += 8;
}

/*
 decode dronecan_remoteid_Location, return true on failure, false on success
*/
bool _dronecan_remoteid_Location_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_Location* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->id_or_mac.len);
    *bit_ofs += 5;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->id_or_mac.len > 20) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->id_or_mac.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->id_or_mac.data[i]);
        *bit_ofs += 8;
    }

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->status);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->direction);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->speed_horizontal);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, true, &msg->speed_vertical);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->latitude);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->longitude);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->altitude_barometric);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->altitude_geodetic);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->height_reference);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->height);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->horizontal_accuracy);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->vertical_accuracy);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->barometer_accuracy);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->speed_accuracy);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->timestamp);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->timestamp_accuracy);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_remoteid_Location sample_dronecan_remoteid_Location_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_remoteid_Location, DRONECAN_REMOTEID_LOCATION_ID, DRONECAN_REMOTEID_LOCATION_SIGNATURE, DRONECAN_REMOTEID_LOCATION_MAX_SIZE);
#endif
#endif
