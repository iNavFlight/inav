#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_CYLINDERSTATUS_MAX_SIZE 10
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_CYLINDERSTATUS_SIGNATURE (0xD68AC83A89D5B36BULL)


struct uavcan_equipment_ice_reciprocating_CylinderStatus {
    float ignition_timing_deg;
    float injection_time_ms;
    float cylinder_head_temperature;
    float exhaust_gas_temperature;
    float lambda_coefficient;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_ice_reciprocating_CylinderStatus_encode(struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_ice_reciprocating_CylinderStatus_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_ice_reciprocating_CylinderStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg, bool tao);
static inline bool _uavcan_equipment_ice_reciprocating_CylinderStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg, bool tao);
void _uavcan_equipment_ice_reciprocating_CylinderStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->ignition_timing_deg);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->injection_time_ms);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->cylinder_head_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->exhaust_gas_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->lambda_coefficient);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
}

/*
 decode uavcan_equipment_ice_reciprocating_CylinderStatus, return true on failure, false on success
*/
bool _uavcan_equipment_ice_reciprocating_CylinderStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_CylinderStatus* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->ignition_timing_deg = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->injection_time_ms = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->cylinder_head_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->exhaust_gas_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->lambda_coefficient = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_ice_reciprocating_CylinderStatus sample_uavcan_equipment_ice_reciprocating_CylinderStatus_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
