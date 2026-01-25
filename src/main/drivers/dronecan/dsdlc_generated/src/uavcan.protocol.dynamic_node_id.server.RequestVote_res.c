#define CANARD_DSDLC_INTERNAL
#include <uavcan.protocol.dynamic_node_id.server.RequestVote_res.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_encode(struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_REQUESTVOTE_RESPONSE_MAX_SIZE);
    _uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_REQUESTVOTE_RESPONSE_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_decode(transfer, &bit_ofs, msg,
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
struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse sample_uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_msg(void) {
    struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse msg;

    msg.term = (uint32_t)random_bitlen_unsigned_val(32);
    msg.vote_granted = (bool)random_bitlen_unsigned_val(1);
    return msg;
}
#endif
