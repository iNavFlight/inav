#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.EntryType.h>
#include <uavcan.protocol.file.Error.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_RESPONSE_MAX_SIZE 204
#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_RESPONSE_SIGNATURE (0x8C46E8AB568BDA79ULL)
#define UAVCAN_PROTOCOL_FILE_GETDIRECTORYENTRYINFO_RESPONSE_ID 46

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_GetDirectoryEntryInfo_cxx_iface;
#endif

struct uavcan_protocol_file_GetDirectoryEntryInfoResponse {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_GetDirectoryEntryInfo_cxx_iface;
#endif
    struct uavcan_protocol_file_Error error;
    struct uavcan_protocol_file_EntryType entry_type;
    struct uavcan_protocol_file_Path entry_full_path;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_GetDirectoryEntryInfoResponse_encode(struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_GetDirectoryEntryInfoResponse_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_GetDirectoryEntryInfoResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg, bool tao);
static inline bool _uavcan_protocol_file_GetDirectoryEntryInfoResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg, bool tao);
void _uavcan_protocol_file_GetDirectoryEntryInfoResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    _uavcan_protocol_file_Error_encode(buffer, bit_ofs, &msg->error, false);
    _uavcan_protocol_file_EntryType_encode(buffer, bit_ofs, &msg->entry_type, false);
    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->entry_full_path, tao);
}

/*
 decode uavcan_protocol_file_GetDirectoryEntryInfoResponse, return true on failure, false on success
*/
bool _uavcan_protocol_file_GetDirectoryEntryInfoResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_GetDirectoryEntryInfoResponse* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (_uavcan_protocol_file_Error_decode(transfer, bit_ofs, &msg->error, false)) {return true;}

    if (_uavcan_protocol_file_EntryType_decode(transfer, bit_ofs, &msg->entry_type, false)) {return true;}

    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->entry_full_path, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_GetDirectoryEntryInfoResponse sample_uavcan_protocol_file_GetDirectoryEntryInfoResponse_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
