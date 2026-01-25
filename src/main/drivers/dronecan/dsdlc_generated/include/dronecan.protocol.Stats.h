#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_PROTOCOL_STATS_MAX_SIZE 28
#define DRONECAN_PROTOCOL_STATS_SIGNATURE (0x763AE3B8A986F8D1ULL)
#define DRONECAN_PROTOCOL_STATS_ID 342

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_protocol_Stats_cxx_iface;
#endif

struct dronecan_protocol_Stats {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_protocol_Stats_cxx_iface;
#endif
    uint32_t tx_frames;
    uint16_t tx_errors;
    uint32_t rx_frames;
    uint16_t rx_error_oom;
    uint16_t rx_error_internal;
    uint16_t rx_error_missed_start;
    uint16_t rx_error_wrong_toggle;
    uint16_t rx_error_short_frame;
    uint16_t rx_error_bad_crc;
    uint16_t rx_ignored_wrong_address;
    uint16_t rx_ignored_not_wanted;
    uint16_t rx_ignored_unexpected_tid;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_protocol_Stats_encode(struct dronecan_protocol_Stats* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_protocol_Stats_decode(const CanardRxTransfer* transfer, struct dronecan_protocol_Stats* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_protocol_Stats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_Stats* msg, bool tao);
static inline bool _dronecan_protocol_Stats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_Stats* msg, bool tao);
void _dronecan_protocol_Stats_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_protocol_Stats* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->tx_frames);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->tx_errors);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->rx_frames);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_oom);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_internal);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_missed_start);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_wrong_toggle);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_short_frame);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_error_bad_crc);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_ignored_wrong_address);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_ignored_not_wanted);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->rx_ignored_unexpected_tid);
    *bit_ofs += 16;
}

/*
 decode dronecan_protocol_Stats, return true on failure, false on success
*/
bool _dronecan_protocol_Stats_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_protocol_Stats* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->tx_frames);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->tx_errors);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->rx_frames);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_oom);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_internal);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_missed_start);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_wrong_toggle);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_short_frame);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_error_bad_crc);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_ignored_wrong_address);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_ignored_not_wanted);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->rx_ignored_unexpected_tid);
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_protocol_Stats sample_dronecan_protocol_Stats_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_protocol_Stats, DRONECAN_PROTOCOL_STATS_ID, DRONECAN_PROTOCOL_STATS_SIGNATURE, DRONECAN_PROTOCOL_STATS_MAX_SIZE);
#endif
#endif
