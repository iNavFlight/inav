#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_ENTRY_MAX_SIZE 21
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_ENTRY_SIGNATURE (0x7FAA779D64FA75C2ULL)


struct uavcan_protocol_dynamic_node_id_server_Entry {
    uint32_t term;
    uint8_t unique_id[16];
    uint8_t node_id;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_dynamic_node_id_server_Entry_encode(struct uavcan_protocol_dynamic_node_id_server_Entry* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_dynamic_node_id_server_Entry_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_dynamic_node_id_server_Entry* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_dynamic_node_id_server_Entry_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Entry* msg, bool tao);
static inline bool _uavcan_protocol_dynamic_node_id_server_Entry_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Entry* msg, bool tao);
void _uavcan_protocol_dynamic_node_id_server_Entry_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Entry* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->term);
    *bit_ofs += 32;
    for (size_t i=0; i < 16; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->unique_id[i]);
        *bit_ofs += 8;
    }
    *bit_ofs += 1;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->node_id);
    *bit_ofs += 7;
}

/*
 decode uavcan_protocol_dynamic_node_id_server_Entry, return true on failure, false on success
*/
bool _uavcan_protocol_dynamic_node_id_server_Entry_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Entry* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->term);
    *bit_ofs += 32;

    for (size_t i=0; i < 16; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->unique_id[i]);
        *bit_ofs += 8;
    }

    *bit_ofs += 1;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->node_id);
    *bit_ofs += 7;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_dynamic_node_id_server_Entry sample_uavcan_protocol_dynamic_node_id_server_Entry_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
