#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_PROTOCOL_CANSTATS_MAX_SIZE 25
#define DRONECAN_PROTOCOL_CANSTATS_SIGNATURE (0xCE080CAE3CA33C75ULL)
#define DRONECAN_PROTOCOL_CANSTATS_ID 343

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_protocol_CanStats_cxx_iface;
#endif

struct dronecan_protocol_CanStats {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_protocol_CanStats_cxx_iface;
#endif
    uint8_t interface;
    uint32_t tx_requests;
    uint16_t tx_rejected;
    uint16_t tx_overflow;
    uint16_t tx_success;
    uint16_t tx_timedout;
    uint16_t tx_abort;
    uint32_t rx_received;
    uint16_t rx_overflow;
    uint16_t rx_errors;
    uint16_t busoff_errors;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_protocol_CanStats_encode(struct dronecan_protocol_CanStats* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_protocol_CanStats_decode(const CanardRxTransfer* transfer, struct dronecan_protocol_CanStats* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_protocol_CanStats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_CanStats* msg, bool tao);
static inline bool _dronecan_protocol_CanStats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_CanStats* msg, bool tao);
void _dronecan_protocol_CanStats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_CanStats* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->interface);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->tx_requests);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_rejected);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_overflow);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_success);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_timedout);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_abort);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->rx_received);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_overflow);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_errors);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->busoff_errors);
    *bit_ofs += 16;
}

/*
 decode dronecan_protocol_CanStats, return true on failure, false on success
*/
bool _dronecan_protocol_CanStats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_CanStats* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->interface);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->tx_requests);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_rejected);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_overflow);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_success);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_timedout);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_abort);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->rx_received);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_overflow);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_errors);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->busoff_errors);
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_protocol_CanStats sample_dronecan_protocol_CanStats_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_protocol_CanStats, DRONECAN_PROTOCOL_CANSTATS_ID, DRONECAN_PROTOCOL_CANSTATS_SIGNATURE, DRONECAN_PROTOCOL_CANSTATS_MAX_SIZE);
#endif
#endif
