#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_PANIC_MAX_SIZE 8
#define UAVCAN_PROTOCOL_PANIC_SIGNATURE (0x8B79B4101811C1D7ULL)
#define UAVCAN_PROTOCOL_PANIC_ID 5

#define UAVCAN_PROTOCOL_PANIC_MIN_MESSAGES 3
#define UAVCAN_PROTOCOL_PANIC_MAX_INTERVAL_MS 500

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_Panic_cxx_iface;
#endif

struct uavcan_protocol_Panic {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_Panic_cxx_iface;
#endif
    struct { uint8_t len; uint8_t data[7]; }reason_text;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_Panic_encode(struct uavcan_protocol_Panic* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_Panic_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_Panic* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_Panic_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_Panic* msg, bool tao);
static inline bool _uavcan_protocol_Panic_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_Panic* msg, bool tao);
void _uavcan_protocol_Panic_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_Panic* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t reason_text_len = msg->reason_text.len > 7 ? 7 : msg->reason_text.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 3, &reason_text_len);
        *bit_ofs += 3;
    }
    for (size_t i=0; i < reason_text_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->reason_text.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_Panic, return true on failure, false on success
*/
bool _uavcan_protocol_Panic_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_Panic* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->reason_text.len);
        *bit_ofs += 3;
    } else {
        msg->reason_text.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->reason_text.len > 7) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->reason_text.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->reason_text.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_Panic sample_uavcan_protocol_Panic_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_protocol_Panic, UAVCAN_PROTOCOL_PANIC_ID, UAVCAN_PROTOCOL_PANIC_SIGNATURE, UAVCAN_PROTOCOL_PANIC_MAX_SIZE);
#endif
#endif
