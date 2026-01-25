#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.param.Value.h>


#define UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_MAX_SIZE 224
#define UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_SIGNATURE (0xA7B622F939D1A4D5ULL)
#define UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_ID 11

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_param_GetSet_cxx_iface;
#endif

struct uavcan_protocol_param_GetSetRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_param_GetSet_cxx_iface;
#endif
    uint16_t index;
    struct uavcan_protocol_param_Value value;
    struct { uint8_t len; uint8_t data[92]; }name;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_param_GetSetRequest_encode(struct uavcan_protocol_param_GetSetRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_param_GetSetRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_param_GetSetRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_param_GetSetRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_GetSetRequest* msg, bool tao);
static inline bool _uavcan_protocol_param_GetSetRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_GetSetRequest* msg, bool tao);
void _uavcan_protocol_param_GetSetRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_GetSetRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 13, &msg->index);
    *bit_ofs += 13;
    _uavcan_protocol_param_Value_encode(buffer, bit_ofs, &msg->value, false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t name_len = msg->name.len > 92 ? 92 : msg->name.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 7, &name_len);
        *bit_ofs += 7;
    }
    for (size_t i=0; i < name_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->name.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_param_GetSetRequest, return true on failure, false on success
*/
bool _uavcan_protocol_param_GetSetRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_GetSetRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 13, false, &msg->index);
    *bit_ofs += 13;

    if (_uavcan_protocol_param_Value_decode(transfer, bit_ofs, &msg->value, false)) {return true;}

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->name.len);
        *bit_ofs += 7;
    } else {
        msg->name.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->name.len > 92) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->name.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->name.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_param_GetSetRequest sample_uavcan_protocol_param_GetSetRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
