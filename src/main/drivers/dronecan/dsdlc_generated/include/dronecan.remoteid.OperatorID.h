#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_REMOTEID_OPERATORID_MAX_SIZE 43
#define DRONECAN_REMOTEID_OPERATORID_SIGNATURE (0x581E7FC7F03AF935ULL)
#define DRONECAN_REMOTEID_OPERATORID_ID 20034

#define DRONECAN_REMOTEID_OPERATORID_ODID_OPERATOR_ID_TYPE_CAA 0

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_remoteid_OperatorID_cxx_iface;
#endif

struct dronecan_remoteid_OperatorID {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_remoteid_OperatorID_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[20]; }id_or_mac;
    uint8_t operator_id_type;
    struct { uint8_t len; uint8_t data[20]; }operator_id;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_remoteid_OperatorID_encode(struct dronecan_remoteid_OperatorID* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_remoteid_OperatorID_decode(const CanardRxTransfer* transfer, struct dronecan_remoteid_OperatorID* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_remoteid_OperatorID_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_OperatorID* msg, bool tao);
static inline bool _dronecan_remoteid_OperatorID_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_OperatorID* msg, bool tao);
void _dronecan_remoteid_OperatorID_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_remoteid_OperatorID* msg, bool tao) {
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
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->operator_id_type);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t operator_id_len = msg->operator_id.len > 20 ? 20 : msg->operator_id.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &operator_id_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < operator_id_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->operator_id.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode dronecan_remoteid_OperatorID, return true on failure, false on success
*/
bool _dronecan_remoteid_OperatorID_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_remoteid_OperatorID* msg, bool tao) {
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

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->operator_id_type);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->operator_id.len);
        *bit_ofs += 5;
    } else {
        msg->operator_id.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->operator_id.len > 20) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->operator_id.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->operator_id.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_remoteid_OperatorID sample_dronecan_remoteid_OperatorID_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_remoteid_OperatorID, DRONECAN_REMOTEID_OPERATORID_ID, DRONECAN_REMOTEID_OPERATORID_SIGNATURE, DRONECAN_REMOTEID_OPERATORID_MAX_SIZE);
#endif
#endif
