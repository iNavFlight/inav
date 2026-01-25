#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_REQUEST_MAX_SIZE 202
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_REQUEST_SIGNATURE (0xB7D725DF72724126ULL)
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_REQUEST_ID 40

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_BeginFirmwareUpdate_cxx_iface;
#endif

struct uavcan_protocol_file_BeginFirmwareUpdateRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_BeginFirmwareUpdate_cxx_iface;
#endif
    uint8_t source_node_id;
    struct uavcan_protocol_file_Path image_file_remote_path;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_BeginFirmwareUpdateRequest_encode(struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_BeginFirmwareUpdateRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_BeginFirmwareUpdateRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg, bool tao);
static inline bool _uavcan_protocol_file_BeginFirmwareUpdateRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg, bool tao);
void _uavcan_protocol_file_BeginFirmwareUpdateRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->source_node_id);
    *bit_ofs += 8;
    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->image_file_remote_path, tao);
}

/*
 decode uavcan_protocol_file_BeginFirmwareUpdateRequest, return true on failure, false on success
*/
bool _uavcan_protocol_file_BeginFirmwareUpdateRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->source_node_id);
    *bit_ofs += 8;

    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->image_file_remote_path, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_BeginFirmwareUpdateRequest sample_uavcan_protocol_file_BeginFirmwareUpdateRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
