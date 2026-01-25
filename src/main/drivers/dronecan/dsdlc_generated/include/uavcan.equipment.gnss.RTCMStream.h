#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_MAX_SIZE 130
#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_SIGNATURE (0x1F56030ECB171501ULL)
#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID 1062

#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_UNKNOWN 0
#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM2 2
#define UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM3 3

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_gnss_RTCMStream_cxx_iface;
#endif

struct uavcan_equipment_gnss_RTCMStream {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_gnss_RTCMStream_cxx_iface;
#endif
    uint8_t protocol_id;
    struct { uint8_t len; uint8_t data[128]; }data;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_gnss_RTCMStream_encode(struct uavcan_equipment_gnss_RTCMStream* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_gnss_RTCMStream_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_RTCMStream* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_gnss_RTCMStream_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_RTCMStream* msg, bool tao);
static inline bool _uavcan_equipment_gnss_RTCMStream_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_RTCMStream* msg, bool tao);
void _uavcan_equipment_gnss_RTCMStream_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_RTCMStream* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->protocol_id);
    *bit_ofs += 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t data_len = msg->data.len > 128 ? 128 : msg->data.len;
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
 decode uavcan_equipment_gnss_RTCMStream, return true on failure, false on success
*/
bool _uavcan_equipment_gnss_RTCMStream_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_RTCMStream* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->protocol_id);
    *bit_ofs += 8;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->data.len);
        *bit_ofs += 8;
    } else {
        msg->data.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->data.len > 128) {
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
struct uavcan_equipment_gnss_RTCMStream sample_uavcan_equipment_gnss_RTCMStream_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_gnss_RTCMStream, UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID, UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_SIGNATURE, UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_MAX_SIZE);
#endif
#endif
