#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_REQUEST_MAX_SIZE 205
#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_REQUEST_SIGNATURE (0x8C46E8AB568BDA79ULL)
#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_REQUEST_ID 46

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_GetDirectoryEntryInfo_cxx_iface;
#endif

struct uavcan_protocol_file_GetDirectoryEntryInfoRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_GetDirectoryEntryInfo_cxx_iface;
#endif
    uint32_t entry_index;
    struct uavcan_protocol_file_Path directory_path;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_GetDirectoryEntryInfoRequest_encode(struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_GetDirectoryEntryInfoRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_GetDirectoryEntryInfoRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg, bool tao);
static inline bool _uavcan_protocol_file_GetDirectoryEntryInfoRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg, bool tao);
void _uavcan_protocol_file_GetDirectoryEntryInfoRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->entry_index);
    *bit_ofs += 32;
    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->directory_path, tao);
}

/*
 decode uavcan_protocol_file_GetDirectoryEntryInfoRequest, return true on failure, false on success
*/
bool _uavcan_protocol_file_GetDirectoryEntryInfoRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->entry_index);
    *bit_ofs += 32;

    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->directory_path, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_GetDirectoryEntryInfoRequest sample_uavcan_protocol_file_GetDirectoryEntryInfoRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
