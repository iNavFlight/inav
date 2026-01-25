#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_GETINFO_REQUEST_MAX_SIZE 201
#define UAVCAN_PROTOCOL_FILE_GETINFO_REQUEST_SIGNATURE (0x5004891EE8A27531ULL)
#define UAVCAN_PROTOCOL_FILE_GETINFO_REQUEST_ID 45

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_GetInfo_cxx_iface;
#endif

struct uavcan_protocol_file_GetInfoRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_GetInfo_cxx_iface;
#endif
    struct uavcan_protocol_file_Path path;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_GetInfoRequest_encode(struct uavcan_protocol_file_GetInfoRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_GetInfoRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_GetInfoRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_GetInfoRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetInfoRequest* msg, bool tao);
static inline bool _uavcan_protocol_file_GetInfoRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetInfoRequest* msg, bool tao);
void _uavcan_protocol_file_GetInfoRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetInfoRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->path, tao);
}

/*
 decode uavcan_protocol_file_GetInfoRequest, return true on failure, false on success
*/
bool _uavcan_protocol_file_GetInfoRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetInfoRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->path, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_GetInfoRequest sample_uavcan_protocol_file_GetInfoRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
