#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.file.Path.h>


#define UAVCAN_PROTOCOL_FILE_WRITE_REQUEST_MAX_SIZE 399
#define UAVCAN_PROTOCOL_FILE_WRITE_REQUEST_SIGNATURE (0x515AA1DC77E58429ULL)
#define UAVCAN_PROTOCOL_FILE_WRITE_REQUEST_ID 49

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_file_Write_cxx_iface;
#endif

struct uavcan_protocol_file_WriteRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_file_Write_cxx_iface;
#endif
    uint64_t offset;
    struct uavcan_protocol_file_Path path;
    struct { uint8_t len; uint8_t data[192]; }data;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_file_WriteRequest_encode(struct uavcan_protocol_file_WriteRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_file_WriteRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_file_WriteRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_file_WriteRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_WriteRequest* msg, bool tao);
static inline bool _uavcan_protocol_file_WriteRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_WriteRequest* msg, bool tao);
void _uavcan_protocol_file_WriteRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_file_WriteRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 40, &msg->offset);
    *bit_ofs += 40;
    _uavcan_protocol_file_Path_encode(buffer, bit_ofs, &msg->path, false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t data_len = msg->data.len > 192 ? 192 : msg->data.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &data_len);
        *bit_ofs += 8;
    }
    for (size_t i=0; i < data_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->data.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_protocol_file_WriteRequest, return true on failure, false on success
*/
bool _uavcan_protocol_file_WriteRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_file_WriteRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 40, false, &msg->offset);
    *bit_ofs += 40;

    if (_uavcan_protocol_file_Path_decode(transfer, bit_ofs, &msg->path, false)) {return true;}

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->data.len);
        *bit_ofs += 8;
    } else {
        msg->data.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->data.len > 192) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->data.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->data.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_file_WriteRequest sample_uavcan_protocol_file_WriteRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
