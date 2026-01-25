#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_REMOTEID_BASICID_MAX_SIZE 44
#define DRONECAN_REMOTEID_BASICID_SIGNATURE (0x5B1C624A8E4FC533ULL)
#define DRONECAN_REMOTEID_BASICID_ID 20030

#define DRONECAN_REMOTEID_BASICID_ODID_ID_TYPE_NONE 0
#define DRONECAN_REMOTEID_BASICID_ODID_ID_TYPE_SERIAL_NUMBER 1
#define DRONECAN_REMOTEID_BASICID_ODID_ID_TYPE_CAA_REGISTRATION_ID 2
#define DRONECAN_REMOTEID_BASICID_ODID_ID_TYPE_UTM_ASSIGNED_UUID 3
#define DRONECAN_REMOTEID_BASICID_ODID_ID_TYPE_SPECIFIC_SESSION_ID 4
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_NONE 0
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_AEROPLANE 1
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_HELICOPTER_OR_MULTIROTOR 2
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_GYROPLANE 3
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_HYBRID_LIFT 4
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_ORNITHOPTER 5
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_GLIDER 6
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_KITE 7
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_FREE_BALLOON 8
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_CAPTIVE_BALLOON 9
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_AIRSHIP 10
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_FREE_FALL_PARACHUTE 11
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_ROCKET 12
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_TETHERED_POWERED_AIRCRAFT 13
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_GROUND_OBSTACLE 14
#define DRONECAN_REMOTEID_BASICID_ODID_UA_TYPE_OTHER 15

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_remoteid_BasicID_cxx_iface;
#endif

struct dronecan_remoteid_BasicID {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_remoteid_BasicID_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[20]; }id_or_mac;
    uint8_t id_type;
    uint8_t ua_type;
    struct { uint8_t len; uint8_t data[20]; }uas_id;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_remoteid_BasicID_encode(struct dronecan_remoteid_BasicID* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_remoteid_BasicID_decode(const CanardRxTransfer* transfer, struct dronecan_remoteid_BasicID* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_remoteid_BasicID_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_BasicID* msg, bool tao);
static inline bool _dronecan_remoteid_BasicID_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_BasicID* msg, bool tao);
void _dronecan_remoteid_BasicID_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_BasicID* msg, bool tao) {
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
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->id_type);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->ua_type);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t uas_id_len = msg->uas_id.len > 20 ? 20 : msg->uas_id.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &uas_id_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < uas_id_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->uas_id.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode dronecan_remoteid_BasicID, return true on failure, false on success
*/
bool _dronecan_remoteid_BasicID_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_BasicID* msg, bool tao) {
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

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->id_type);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->ua_type);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->uas_id.len);
        *bit_ofs += 5;
    } else {
        msg->uas_id.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->uas_id.len > 20) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->uas_id.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->uas_id.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_remoteid_BasicID sample_dronecan_remoteid_BasicID_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_remoteid_BasicID, DRONECAN_REMOTEID_BASICID_ID, DRONECAN_REMOTEID_BASICID_SIGNATURE, DRONECAN_REMOTEID_BASICID_MAX_SIZE);
#endif
#endif
