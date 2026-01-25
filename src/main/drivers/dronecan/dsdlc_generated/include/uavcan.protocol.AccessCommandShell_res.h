#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_MAX_SIZE 263
#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_SIGNATURE (0x59276B5921C9246EULL)
#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_ID 6

#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_FLAG_RUNNING 1
#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_FLAG_SHELL_ERROR 2
#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_FLAG_HAS_PENDING_STDOUT 64
#define UAVCAN_PROTOCOL_ACCESSCOMMANDSHELL_RESPONSE_FLAG_HAS_PENDING_STDERR 128

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_AccessCommandShell_cxx_iface;
#endif

struct uavcan_protocol_AccessCommandShellResponse {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_AccessCommandShell_cxx_iface;
#endif
    int32_t last_exit_status;
    uint8_t flags;
    struct { uint16_t len; uint8_t data[256]; }output;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_AccessCommandShellResponse_encode(struct uavcan_protocol_AccessCommandShellResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_AccessCommandShellResponse_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_AccessCommandShellResponse* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_AccessCommandShellResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_AccessCommandShellResponse* msg, bool tao);
static inline bool _uavcan_protocol_AccessCommandShellResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_AccessCommandShellResponse* msg, bool tao);
void _uavcan_protocol_AccessCommandShellResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_AccessCommandShellResponse* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->last_exit_status);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->flags);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint16_t output_len = msg->output.len > 256 ? 256 : msg->output.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 9, &output_len);
        *bit_ofs += 9;
    }
    for (size_t i=0; i < output_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->output.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_AccessCommandShellResponse, return true on failure, false on success
*/
bool _uavcan_protocol_AccessCommandShellResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_AccessCommandShellResponse* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->last_exit_status);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->flags);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 9, false, &msg->output.len);
        *bit_ofs += 9;
    } else {
        msg->output.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->output.len > 256) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->output.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->output.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_AccessCommandShellResponse sample_uavcan_protocol_AccessCommandShellResponse_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
