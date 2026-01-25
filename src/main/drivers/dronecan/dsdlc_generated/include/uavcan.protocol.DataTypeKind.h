#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_DATATYPEKIND_MAX_SIZE 1
#define UAVCAN_PROTOCOL_DATATYPEKIND_SIGNATURE (0x9420A73E008E5930ULL)

#define UAVCAN_PROTOCOL_DATATYPEKIND_SERVICE 0
#define UAVCAN_PROTOCOL_DATATYPEKIND_MESSAGE 1


struct uavcan_protocol_DataTypeKind {
    uint8_t value;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_DataTypeKind_encode(struct uavcan_protocol_DataTypeKind* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_DataTypeKind_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_DataTypeKind* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_DataTypeKind_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_DataTypeKind* msg, bool tao);
static inline bool _uavcan_protocol_DataTypeKind_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_DataTypeKind* msg, bool tao);
void _uavcan_protocol_DataTypeKind_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_DataTypeKind* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->value);
    *bit_ofs += 8;
}

/*
 decode uavcan_protocol_DataTypeKind, return true on failure, false on success
*/
bool _uavcan_protocol_DataTypeKind_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_DataTypeKind* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->value);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_DataTypeKind sample_uavcan_protocol_DataTypeKind_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
