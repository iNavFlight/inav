#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_GNSS_AUXILIARY_MAX_SIZE 16
#define UAVCAN_EQUIPMENT_GNSS_AUXILIARY_SIGNATURE (0x9BE8BDC4C3DBBFD2ULL)
#define UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID 1061

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_gnss_Auxiliary_cxx_iface;
#endif

struct uavcan_equipment_gnss_Auxiliary {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_gnss_Auxiliary_cxx_iface;
#endif
    float gdop;
    float pdop;
    float hdop;
    float vdop;
    float tdop;
    float ndop;
    float edop;
    uint8_t sats_visible;
    uint8_t sats_used;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_gnss_Auxiliary_encode(struct uavcan_equipment_gnss_Auxiliary* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_gnss_Auxiliary_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_Auxiliary* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_gnss_Auxiliary_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Auxiliary* msg, bool tao);
static inline bool _uavcan_equipment_gnss_Auxiliary_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Auxiliary* msg, bool tao);
void _uavcan_equipment_gnss_Auxiliary_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Auxiliary* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->gdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->pdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->hdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->vdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->tdop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->ndop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->edop);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->sats_visible);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 6, &msg->sats_used);
    *bit_ofs += 6;
}

/*
 decode uavcan_equipment_gnss_Auxiliary, return true on failure, false on success
*/
bool _uavcan_equipment_gnss_Auxiliary_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_gnss_Auxiliary* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->gdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->pdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->hdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->vdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->tdop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->ndop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->edop = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->sats_visible);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->sats_used);
    *bit_ofs += 6;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_gnss_Auxiliary sample_uavcan_equipment_gnss_Auxiliary_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_gnss_Auxiliary, UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID, UAVCAN_EQUIPMENT_GNSS_AUXILIARY_SIGNATURE, UAVCAN_EQUIPMENT_GNSS_AUXILIARY_MAX_SIZE);
#endif
#endif
