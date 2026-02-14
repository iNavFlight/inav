#define CANARD_DSDLC_INTERNAL
#include <dronecan.protocol.CanStats.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t dronecan_protocol_CanStats_encode(struct dronecan_protocol_CanStats* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, DRONECAN_PROTOCOL_CANSTATS_MAX_SIZE);
    _dronecan_protocol_CanStats_encode(buffer, &bit_ofs, msg, 
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
bool dronecan_protocol_CanStats_decode(const CanardRxTransfer* transfer, struct dronecan_protocol_CanStats* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > DRONECAN_PROTOCOL_CANSTATS_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_dronecan_protocol_CanStats_decode(transfer, &bit_ofs, msg,
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
struct dronecan_protocol_CanStats sample_dronecan_protocol_CanStats_msg(void) {
    struct dronecan_protocol_CanStats msg;

    msg.interface = (uint8_t)random_bitlen_unsigned_val(8);
    msg.tx_requests = (uint32_t)random_bitlen_unsigned_val(32);
    msg.tx_rejected = (uint16_t)random_bitlen_unsigned_val(16);
    msg.tx_overflow = (uint16_t)random_bitlen_unsigned_val(16);
    msg.tx_success = (uint16_t)random_bitlen_unsigned_val(16);
    msg.tx_timedout = (uint16_t)random_bitlen_unsigned_val(16);
    msg.tx_abort = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_received = (uint32_t)random_bitlen_unsigned_val(32);
    msg.rx_overflow = (uint16_t)random_bitlen_unsigned_val(16);
    msg.rx_errors = (uint16_t)random_bitlen_unsigned_val(16);
    msg.busoff_errors = (uint16_t)random_bitlen_unsigned_val(16);
    return msg;
}
#endif
