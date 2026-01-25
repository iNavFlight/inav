#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_REMOTEID_SYSTEM_MAX_SIZE 53
#define DRONECAN_REMOTEID_SYSTEM_SIGNATURE (0x9AC872F49BF32437ULL)
#define DRONECAN_REMOTEID_SYSTEM_ID 20033

#define DRONECAN_REMOTEID_SYSTEM_ODID_OPERATOR_LOCATION_TYPE_TAKEOFF 0
#define DRONECAN_REMOTEID_SYSTEM_ODID_OPERATOR_LOCATION_TYPE_LIVE_GNSS 1
#define DRONECAN_REMOTEID_SYSTEM_ODID_OPERATOR_LOCATION_TYPE_FIXED 2
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASSIFICATION_TYPE_EU 0
#define DRONECAN_REMOTEID_SYSTEM_ODID_CATEGORY_EU_UNDECLARED 0
#define DRONECAN_REMOTEID_SYSTEM_ODID_CATEGORY_EU_OPEN 1
#define DRONECAN_REMOTEID_SYSTEM_ODID_CATEGORY_EU_SPECIFIC 2
#define DRONECAN_REMOTEID_SYSTEM_ODID_CATEGORY_EU_CERTIFIED 3
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_UNDECLARED 0
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_0 1
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_1 2
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_2 3
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_3 4
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_4 5
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_5 6
#define DRONECAN_REMOTEID_SYSTEM_ODID_CLASS_EU_CLASS_6 7

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_remoteid_System_cxx_iface;
#endif

struct dronecan_remoteid_System {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_remoteid_System_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[20]; }id_or_mac;
    uint8_t operator_location_type;
    uint8_t classification_type;
    int32_t operator_latitude;
    int32_t operator_longitude;
    uint16_t area_count;
    uint16_t area_radius;
    float area_ceiling;
    float area_floor;
    uint8_t category_eu;
    uint8_t class_eu;
    float operator_altitude_geo;
    uint32_t timestamp;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_remoteid_System_encode(struct dronecan_remoteid_System* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_remoteid_System_decode(const CanardRxTransfer* transfer, struct dronecan_remoteid_System* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_remoteid_System_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_System* msg, bool tao);
static inline bool _dronecan_remoteid_System_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_System* msg, bool tao);
void _dronecan_remoteid_System_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_System* msg, bool tao) {
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
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->operator_location_type);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->classification_type);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->operator_latitude);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->operator_longitude);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->area_count);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->area_radius);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->area_ceiling);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->area_floor);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->category_eu);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->class_eu);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->operator_altitude_geo);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->timestamp);
    *bit_ofs += 32;
}

/*
 decode dronecan_remoteid_System, return true on failure, false on success
*/
bool _dronecan_remoteid_System_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_System* msg, bool tao) {
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

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->operator_location_type);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->classification_type);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->operator_latitude);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->operator_longitude);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->area_count);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->area_radius);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->area_ceiling);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->area_floor);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->category_eu);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->class_eu);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->operator_altitude_geo);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->timestamp);
    *bit_ofs += 32;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_remoteid_System sample_dronecan_remoteid_System_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_remoteid_System, DRONECAN_REMOTEID_SYSTEM_ID, DRONECAN_REMOTEID_SYSTEM_SIGNATURE, DRONECAN_REMOTEID_SYSTEM_MAX_SIZE);
#endif
#endif
