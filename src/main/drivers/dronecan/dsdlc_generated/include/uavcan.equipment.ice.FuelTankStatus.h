#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_MAX_SIZE 13
#define UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_SIGNATURE (0x286B4A387BA84BC4ULL)
#define UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_ID 1129

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_ice_FuelTankStatus_cxx_iface;
#endif

struct uavcan_equipment_ice_FuelTankStatus {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_ice_FuelTankStatus_cxx_iface;
#endif
    uint8_t available_fuel_volume_percent;
    float available_fuel_volume_cm3;
    float fuel_consumption_rate_cm3pm;
    float fuel_temperature;
    uint8_t fuel_tank_id;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_ice_FuelTankStatus_encode(struct uavcan_equipment_ice_FuelTankStatus* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_ice_FuelTankStatus_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ice_FuelTankStatus* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_ice_FuelTankStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_FuelTankStatus* msg, bool tao);
static inline bool _uavcan_equipment_ice_FuelTankStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_FuelTankStatus* msg, bool tao);
void _uavcan_equipment_ice_FuelTankStatus_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_FuelTankStatus* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    *bit_ofs += 9;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->available_fuel_volume_percent);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->available_fuel_volume_cm3);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->fuel_consumption_rate_cm3pm);
    *bit_ofs += 32;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->fuel_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->fuel_tank_id);
    *bit_ofs += 8;
}

/*
 decode uavcan_equipment_ice_FuelTankStatus, return true on failure, false on success
*/
bool _uavcan_equipment_ice_FuelTankStatus_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_FuelTankStatus* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    *bit_ofs += 9;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->available_fuel_volume_percent);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->available_fuel_volume_cm3);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->fuel_consumption_rate_cm3pm);
    *bit_ofs += 32;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->fuel_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->fuel_tank_id);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_ice_FuelTankStatus sample_uavcan_equipment_ice_FuelTankStatus_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_ice_FuelTankStatus, UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_ID, UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_SIGNATURE, UAVCAN_EQUIPMENT_ICE_FUELTANKSTATUS_MAX_SIZE);
#endif
#endif
