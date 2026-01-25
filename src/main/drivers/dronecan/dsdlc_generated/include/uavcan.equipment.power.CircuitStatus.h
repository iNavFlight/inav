#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_MAX_SIZE 7
#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_SIGNATURE (0x8313D33D0DDDA115ULL)
#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ID 1091

#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ERROR_FLAG_OVERVOLTAGE 1
#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ERROR_FLAG_UNDERVOLTAGE 2
#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ERROR_FLAG_OVERCURRENT 4
#define UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ERROR_FLAG_UNDERCURRENT 8

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_power_CircuitStatus_cxx_iface;
#endif

struct uavcan_equipment_power_CircuitStatus {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_power_CircuitStatus_cxx_iface;
#endif
    uint16_t circuit_id;
    float voltage;
    float current;
    uint8_t error_flags;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_power_CircuitStatus_encode(struct uavcan_equipment_power_CircuitStatus* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_power_CircuitStatus_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_power_CircuitStatus* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_power_CircuitStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_power_CircuitStatus* msg, bool tao);
static inline bool _uavcan_equipment_power_CircuitStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_power_CircuitStatus* msg, bool tao);
void _uavcan_equipment_power_CircuitStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_power_CircuitStatus* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->circuit_id);
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->voltage);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->current);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->error_flags);
    *bit_ofs += 8;
}

/*
 decode uavcan_equipment_power_CircuitStatus, return true on failure, false on success
*/
bool _uavcan_equipment_power_CircuitStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_power_CircuitStatus* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->circuit_id);
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->voltage = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->current = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->error_flags);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_power_CircuitStatus sample_uavcan_equipment_power_CircuitStatus_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_power_CircuitStatus, UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ID, UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_SIGNATURE, UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_MAX_SIZE);
#endif
#endif
