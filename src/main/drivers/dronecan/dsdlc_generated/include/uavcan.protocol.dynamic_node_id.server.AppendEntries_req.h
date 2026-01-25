#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.dynamic_node_id.server.Entry.h>


#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_APPENDENTRIES_REQUEST_MAX_SIZE 32
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_APPENDENTRIES_REQUEST_SIGNATURE (0x8032C7097B48A3CCULL)
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_APPENDENTRIES_REQUEST_ID 30

#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_APPENDENTRIES_REQUEST_DEFAULT_MIN_ELECTION_TIMEOUT_MS 2000
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_APPENDENTRIES_REQUEST_DEFAULT_MAX_ELECTION_TIMEOUT_MS 4000

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_dynamic_node_id_server_AppendEntries_cxx_iface;
#endif

struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_dynamic_node_id_server_AppendEntries_cxx_iface;
#endif
    uint32_t term;
    uint32_t prev_log_term;
    uint8_t prev_log_index;
    uint8_t leader_commit;
    struct { uint8_t len; struct uavcan_protocol_dynamic_node_id_server_Entry data[1]; }entries;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_encode(struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg, bool tao);
static inline bool _uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg, bool tao);
void _uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->term);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->prev_log_term);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->prev_log_index);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->leader_commit);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t entries_len = msg->entries.len > 1 ? 1 : msg->entries.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 1, &entries_len);
        *bit_ofs += 1;
    }
    for (size_t i=0; i < entries_len; i++) {
        _uavcan_protocol_dynamic_node_id_server_Entry_encode(buffer, bit_ofs, &msg->entries.data[i], false);
    }
}

/*
 decode uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest, return true on failure, false on success
*/
bool _uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->term);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->prev_log_term);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->prev_log_index);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->leader_commit);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 1, false, &msg->entries.len);
        *bit_ofs += 1;
    }


    if (tao) {
        msg->entries.len = 0;
        size_t max_len = 1;
        uint32_t max_bits = (transfer->payload_len*8)-7; // TAO elements must be >= 8 bits
        while (max_bits > *bit_ofs) {
            if (!max_len-- || _uavcan_protocol_dynamic_node_id_server_Entry_decode(transfer, bit_ofs, &msg->entries.data[msg->entries.len], false)) {return true;}
            msg->entries.len++;
        }
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (msg->entries.len > 1) {
            return true; /* invalid value */
        }
#pragma GCC diagnostic pop
        for (size_t i=0; i < msg->entries.len; i++) {
            if (_uavcan_protocol_dynamic_node_id_server_Entry_decode(transfer, bit_ofs, &msg->entries.data[i], false)) {return true;}
        }
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest sample_uavcan_protocol_dynamic_node_id_server_AppendEntriesRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
