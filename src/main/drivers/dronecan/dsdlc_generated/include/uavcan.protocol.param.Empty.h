#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_PARAM_EMPTY_MAX_SIZE 0
#define UAVCAN_PROTOCOL_PARAM_EMPTY_SIGNATURE (0x6C4D0E8EF37361DFULL)


struct uavcan_protocol_param_Empty {
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_param_Empty_encode(struct uavcan_protocol_param_Empty* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_param_Empty_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_param_Empty* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_param_Empty_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_Empty* msg, bool tao);
static inline bool _uavcan_protocol_param_Empty_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_Empty* msg, bool tao);
void _uavcan_protocol_param_Empty_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_Empty* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

}

/*
 decode uavcan_protocol_param_Empty, return true on failure, false on success
*/
bool _uavcan_protocol_param_Empty_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_Empty* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_param_Empty sample_uavcan_protocol_param_Empty_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
