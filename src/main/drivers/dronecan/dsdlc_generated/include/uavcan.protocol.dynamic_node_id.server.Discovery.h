#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_MAX_SIZE 7
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_SIGNATURE (0x821AE2F525F69F21ULL)
#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_ID 390

#define UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_BROADCASTING_PERIOD_MS 1000

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_dynamic_node_id_server_Discovery_cxx_iface;
#endif

struct uavcan_protocol_dynamic_node_id_server_Discovery {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_dynamic_node_id_server_Discovery_cxx_iface;
#endif
    uint8_t configured_cluster_size;
    struct { uint8_t len; uint8_t data[5]; }known_nodes;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_dynamic_node_id_server_Discovery_encode(struct uavcan_protocol_dynamic_node_id_server_Discovery* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_dynamic_node_id_server_Discovery_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_dynamic_node_id_server_Discovery* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_dynamic_node_id_server_Discovery_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Discovery* msg, bool tao);
static inline bool _uavcan_protocol_dynamic_node_id_server_Discovery_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Discovery* msg, bool tao);
void _uavcan_protocol_dynamic_node_id_server_Discovery_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Discovery* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->configured_cluster_size);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t known_nodes_len = msg->known_nodes.len > 5 ? 5 : msg->known_nodes.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 3, &known_nodes_len);
        *bit_ofs += 3;
    }
    for (size_t i=0; i < known_nodes_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->known_nodes.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_dynamic_node_id_server_Discovery, return true on failure, false on success
*/
bool _uavcan_protocol_dynamic_node_id_server_Discovery_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_dynamic_node_id_server_Discovery* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->configured_cluster_size);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->known_nodes.len);
        *bit_ofs += 3;
    } else {
        msg->known_nodes.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->known_nodes.len > 5) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->known_nodes.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->known_nodes.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_dynamic_node_id_server_Discovery sample_uavcan_protocol_dynamic_node_id_server_Discovery_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_protocol_dynamic_node_id_server_Discovery, UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_ID, UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_SIGNATURE, UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_SERVER_DISCOVERY_MAX_SIZE);
#endif
#endif
