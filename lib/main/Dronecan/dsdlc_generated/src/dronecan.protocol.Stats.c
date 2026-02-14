#define CANARD_DSDLC_INTERNAL
#include <dronecan.protocol.Stats.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t dronecan_protocol_Stats_encode(struct dronecan_protocol_Stats* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, DRONECAN_PROTOCOL_STATS_MAX_SIZE);
    _dronecan_protocol_Stats_encode(buffer, &bit_ofs, msg, 
#if CANARD_ENABLE_TAO_OPTION
    tao
#else
    true
#endif
    );
    return ((bit_ofs+7)/8);
}

/*
  return true if the decode is invalid
 */
bool dronecan_protocol_Stats_decode(const CanardRxTransfer* transfer, struct dronecan_protocol_Stats* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > DRONECAN_PROTOCOL_STATS_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_dronecan_protocol_Stats_decode(transfer, &bit_ofs, msg,
#if CANARD_ENABLE_TAO_OPTION
    transfer->tao
#else
    true
#endif
    )) {
        return true; /* invalid payload */
    }

    const uint32_t byte_len = (bit_ofs+7U)/8U;
#if CANARD_ENABLE_TAO_OPTION
    // if this could be CANFD then the dlc could indicating more bytes than
    // we actually have
    if (!transfer->tao) {
        return byte_len > transfer->payload_len;
    }
#endif
    return byte_len != transfer->payload_len;
}

#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_protocol_Stats sample_dronecan_protocol_Stats_msg(void) {
    struct dronecan_protocol_Stats msg;

    msg.tx_frames = (uint32_t)random_bitlen_unsigned_val(32);
    msg.tx_errors = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_frames = (uint32_t)random_bitlen_unsigned_val(32);
    msg.rx_error_oom = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_error_internal = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_error_missed_start = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_error_wrong_toggle = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_error_short_frame = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_error_bad_crc = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_ignored_wrong_address = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_ignored_not_wanted = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_ignored_unexpected_tid = (uint16_t)random_bitlen_unsigned_val(16);
    return msg;
}
#endif
