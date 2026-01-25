#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_MAX_SIZE 129
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_SIGNATURE (0xB7D725DF72724126ULL)
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ID 40

#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_OK 0
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_INVALID_MODE 1
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_IN_PROGRESS 2
#define UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_UNKNOWN 255

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_BeginFirmwareUpdate_cxx_iface;
#endif

struct uavcan_protocol_file_BeginFirmwareUpdateResponse {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_BeginFirmwareUpdate_cxx_iface;
#endif
    uint8_t error;
    struct { uint8_t len; uint8_t data[127]; }optional_error_message;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_BeginFirmwareUpdateResponse_encode(struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_BeginFirmwareUpdateResponse_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_BeginFirmwareUpdateResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg, bool tao);
static inline bool _uavcan_protocol_file_BeginFirmwareUpdateResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg, bool tao);
void _uavcan_protocol_file_BeginFirmwareUpdateResponse_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->error);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t optional_error_message_len = msg->optional_error_message.len > 127 ? 127 : msg->optional_error_message.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 7, &optional_error_message_len);
        *bit_ofs += 7;
    }
    for (size_t i=0; i < optional_error_message_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->optional_error_message.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_file_BeginFirmwareUpdateResponse, return true on failure, false on success
*/
bool _uavcan_protocol_file_BeginFirmwareUpdateResponse_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_BeginFirmwareUpdateResponse* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->error);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->optional_error_message.len);
        *bit_ofs += 7;
    } else {
        msg->optional_error_message.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->optional_error_message.len > 127) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->optional_error_message.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->optional_error_message.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_BeginFirmwareUpdateResponse sample_uavcan_protocol_file_BeginFirmwareUpdateResponse_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
