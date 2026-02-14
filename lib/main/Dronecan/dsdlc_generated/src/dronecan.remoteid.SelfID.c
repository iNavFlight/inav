#define CANARD_DSDLC_INTERNAL
#include <dronecan.remoteid.SelfID.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t dronecan_remoteid_SelfID_encode(struct dronecan_remoteid_SelfID* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, DRONECAN_REMOTEID_SELFID_MAX_SIZE);
    _dronecan_remoteid_SelfID_encode(buffer, &bit_ofs, msg, 
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
bool dronecan_remoteid_SelfID_decode(const CanardRxTransfer* transfer, struct dronecan_remoteid_SelfID* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > DRONECAN_REMOTEID_SELFID_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_dronecan_remoteid_SelfID_decode(transfer, &bit_ofs, msg,
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
struct dronecan_remoteid_SelfID sample_dronecan_remoteid_SelfID_msg(void) {
    struct dronecan_remoteid_SelfID msg;

    msg.id_or_mac.len = (uint8_t)random_range_unsigned_val(0, 20);
    for (size_t i=0; i < msg.id_or_mac.len; i++) {
        msg.id_or_mac.data[i] = (uint8_t)random_bitlen_unsigned_val(8);
    }
    msg.description_type = (uint8_t)random_bitlen_unsigned_val(8);
    msg.description.len = (uint8_t)random_range_unsigned_val(0, 23);
    for (size_t i=0; i < msg.description.len; i++) {
        msg.description.data[i] = (uint8_t)random_bitlen_unsigned_val(8);
    }
    return msg;
}
#endif
