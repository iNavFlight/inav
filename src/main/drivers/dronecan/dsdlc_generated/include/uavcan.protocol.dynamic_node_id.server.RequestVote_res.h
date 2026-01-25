#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_REQUESTVOTE_RESPONSE_MAX_SIZE 5
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_REQUESTVOTE_RESPONSE_SIGNATURE (0xCDDE07BB89A56356ULL)
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_REQUESTVOTE_RESPONSE_ID 31

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_dynamic_node_id_server_RequestVote_cxx_iface;
#endif

struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_dynamic_node_id_server_RequestVote_cxx_iface;
#endif
    uint32_t term;
    bool vote_granted;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_encode(struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, bool tao);
static inline bool _uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, bool tao);
void _uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->term);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 1, &msg->vote_granted);
    *bit_ofs += 1;
}

/*
 decode uavcan_protocol_dynamic_node_id_server_RequestVoteResponse, return true on failure, false on success
*/
bool _uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->term);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 1, false, &msg->vote_granted);
    *bit_ofs += 1;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_dynamic_node_id_server_RequestVoteResponse sample_uavcan_protocol_dynamic_node_id_server_RequestVoteResponse_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
