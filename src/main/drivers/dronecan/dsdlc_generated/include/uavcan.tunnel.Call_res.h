#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_TUNNEL_CALL_RESPONSE_MAX_SIZE 61
#define UAVCAN_TUNNEL_CALL_RESPONSE_SIGNATURE (0xDB11EDC510502658ULL)
#define UAVCAN_TUNNEL_CALL_RESPONSE_ID 63

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_tunnel_Call_cxx_iface;
#endif

struct uavcan_tunnel_CallResponse {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_tunnel_Call_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[60]; }buffer;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_tunnel_CallResponse_encode(struct uavcan_tunnel_CallResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_tunnel_CallResponse_decode(const CanardRxTransfer* transfer, struct uavcan_tunnel_CallResponse* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_tunnel_CallResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_tunnel_CallResponse* msg, bool tao);
static inline bool _uavcan_tunnel_CallResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_tunnel_CallResponse* msg, bool tao);
void _uavcan_tunnel_CallResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_tunnel_CallResponse* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

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
 decode uavcan_tunnel_CallResponse, return true on failure, false on success
*/
bool _uavcan_tunnel_CallResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_tunnel_CallResponse* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
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
struct uavcan_tunnel_CallResponse sample_uavcan_tunnel_CallResponse_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
