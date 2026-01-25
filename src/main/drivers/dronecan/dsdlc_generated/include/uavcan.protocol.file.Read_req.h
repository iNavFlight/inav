#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_READ_REQUEST_MAX_SIZE 206
#define UAVCAN_PROTOCOL_FILE_READ_REQUEST_SIGNATURE (0x8DCDCA939F33F678ULL)
#define UAVCAN_PROTOCOL_FILE_READ_REQUEST_ID 48

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_Read_cxx_iface;
#endif

struct uavcan_protocol_file_ReadRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_Read_cxx_iface;
#endif
    uint64_t offset;
    struct uavcan_protocol_file_Path path;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_ReadRequest_encode(struct uavcan_protocol_file_ReadRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_ReadRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_ReadRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_ReadRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_ReadRequest* msg, bool tao);
static inline bool _uavcan_protocol_file_ReadRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_ReadRequest* msg, bool tao);
void _uavcan_protocol_file_ReadRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_ReadRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 40, &msg->offset);
    *bit_ofs += 40;
    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->path, tao);
}

/*
 decode uavcan_protocol_file_ReadRequest, return true on failure, false on success
*/
bool _uavcan_protocol_file_ReadRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_ReadRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 40, false, &msg->offset);
    *bit_ofs += 40;

    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->path, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_ReadRequest sample_uavcan_protocol_file_ReadRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
