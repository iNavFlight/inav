#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.tunnel.Protocol.h>


#define UAVCAN_TUNNEL_BROADCAST_MAX_SIZE 63
#define UAVCAN_TUNNEL_BROADCAST_SIGNATURE (0x5AA2D4D9CF4B1E85ULL)
#define UAVCAN_TUNNEL_BROADCAST_ID 2010

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_tunnel_Broadcast_cxx_iface;
#endif

struct uavcan_tunnel_Broadcast {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_tunnel_Broadcast_cxx_iface;
#endif
    struct uavcan_tunnel_Protocol protocol;
    uint8_t channel_id;
    struct { uint8_t len; uint8_t data[60]; }buffer;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_tunnel_Broadcast_encode(struct uavcan_tunnel_Broadcast* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_tunnel_Broadcast_decode(const CanardRxTransfer* transfer, struct uavcan_tunnel_Broadcast* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_tunnel_Broadcast_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_tunnel_Broadcast* msg, bool tao);
static inline bool _uavcan_tunnel_Broadcast_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_tunnel_Broadcast* msg, bool tao);
void _uavcan_tunnel_Broadcast_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_tunnel_Broadcast* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_tunnel_Protocol_encode(buffer, bit_ofs, &msg->protocol, false);
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->channel_id);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t buffer_len = msg->buffer.len > 60 ? 60 : msg->buffer.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 6, &buffer_len);
        *bit_ofs += 6;
    }
    for (size_t i=0; i < buffer_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->buffer.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_tunnel_Broadcast, return true on failure, false on success
*/
bool _uavcan_tunnel_Broadcast_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_tunnel_Broadcast* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_tunnel_Protocol_decode(transfer, bit_ofs, &msg->protocol, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->channel_id);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->buffer.len);
        *bit_ofs += 6;
    } else {
        msg->buffer.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->buffer.len > 60) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->buffer.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->buffer.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_tunnel_Broadcast sample_uavcan_tunnel_Broadcast_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_tunnel_Broadcast, UAVCAN_TUNNEL_BROADCAST_ID, UAVCAN_TUNNEL_BROADCAST_SIGNATURE, UAVCAN_TUNNEL_BROADCAST_MAX_SIZE);
#endif
#endif
