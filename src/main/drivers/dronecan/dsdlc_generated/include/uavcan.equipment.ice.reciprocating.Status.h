#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.ice.reciprocating.CylinderStatus.h>


#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_MAX_SIZE 196
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SIGNATURE (0xD38AA3EE75537EC6ULL)
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_ID 1120

#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_STATE_STOPPED 0
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_STATE_STARTING 1
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_STATE_RUNNING 2
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_STATE_FAULT 3
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_GENERAL_ERROR 1
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_CRANKSHAFT_SENSOR_ERROR_SUPPORTED 2
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_CRANKSHAFT_SENSOR_ERROR 4
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_TEMPERATURE_SUPPORTED 8
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_TEMPERATURE_BELOW_NOMINAL 16
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_TEMPERATURE_ABOVE_NOMINAL 32
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_TEMPERATURE_OVERHEATING 64
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_TEMPERATURE_EGT_ABOVE_NOMINAL 128
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_FUEL_PRESSURE_SUPPORTED 256
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_FUEL_PRESSURE_BELOW_NOMINAL 512
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_FUEL_PRESSURE_ABOVE_NOMINAL 1024
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_DETONATION_SUPPORTED 2048
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_DETONATION_OBSERVED 4096
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_MISFIRE_SUPPORTED 8192
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_MISFIRE_OBSERVED 16384
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_OIL_PRESSURE_SUPPORTED 32768
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_OIL_PRESSURE_BELOW_NOMINAL 65536
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_OIL_PRESSURE_ABOVE_NOMINAL 131072
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_DEBRIS_SUPPORTED 262144
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_FLAG_DEBRIS_DETECTED 524288
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SPARK_PLUG_SINGLE 0
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SPARK_PLUG_FIRST_ACTIVE 1
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SPARK_PLUG_SECOND_ACTIVE 2
#define UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SPARK_PLUG_BOTH_ACTIVE 3

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_ice_reciprocating_Status_cxx_iface;
#endif

struct uavcan_equipment_ice_reciprocating_Status {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_ice_reciprocating_Status_cxx_iface;
#endif
    uint8_t state;
    uint32_t flags;
    uint8_t engine_load_percent;
    uint32_t engine_speed_rpm;
    float spark_dwell_time_ms;
    float atmospheric_pressure_kpa;
    float intake_manifold_pressure_kpa;
    float intake_manifold_temperature;
    float coolant_temperature;
    float oil_pressure;
    float oil_temperature;
    float fuel_pressure;
    float fuel_consumption_rate_cm3pm;
    float estimated_consumed_fuel_volume_cm3;
    uint8_t throttle_position_percent;
    uint8_t ecu_index;
    uint8_t spark_plug_usage;
    struct { uint8_t len; struct uavcan_equipment_ice_reciprocating_CylinderStatus data[16]; }cylinder_status;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_ice_reciprocating_Status_encode(struct uavcan_equipment_ice_reciprocating_Status* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_ice_reciprocating_Status_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ice_reciprocating_Status* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_ice_reciprocating_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_Status* msg, bool tao);
static inline bool _uavcan_equipment_ice_reciprocating_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_Status* msg, bool tao);
void _uavcan_equipment_ice_reciprocating_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_Status* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 2, &msg->state);
    *bit_ofs += 2;
    canardEncodeScalar(buffer, *bit_ofs, 30, &msg->flags);
    *bit_ofs += 30;
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->engine_load_percent);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 17, &msg->engine_speed_rpm);
    *bit_ofs += 17;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->spark_dwell_time_ms);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->atmospheric_pressure_kpa);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->intake_manifold_pressure_kpa);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->intake_manifold_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->coolant_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->oil_pressure);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->oil_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->fuel_pressure);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->fuel_consumption_rate_cm3pm);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->estimated_consumed_fuel_volume_cm3);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->throttle_position_percent);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 6, &msg->ecu_index);
    *bit_ofs += 6;
    canardEncodeScalar(buffer, *bit_ofs, 3, &msg->spark_plug_usage);
    *bit_ofs += 3;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t cylinder_status_len = msg->cylinder_status.len > 16 ? 16 : msg->cylinder_status.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &cylinder_status_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < cylinder_status_len; i++) {
        _uavcan_equipment_ice_reciprocating_CylinderStatus_encode(buffer, bit_ofs, &msg->cylinder_status.data[i], false);
    }
}

/*
 decode uavcan_equipment_ice_reciprocating_Status, return true on failure, false on success
*/
bool _uavcan_equipment_ice_reciprocating_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_ice_reciprocating_Status* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 2, false, &msg->state);
    *bit_ofs += 2;

    canardDecodeScalar(transfer, *bit_ofs, 30, false, &msg->flags);
    *bit_ofs += 30;

    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->engine_load_percent);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 17, false, &msg->engine_speed_rpm);
    *bit_ofs += 17;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->spark_dwell_time_ms = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->atmospheric_pressure_kpa = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->intake_manifold_pressure_kpa = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->intake_manifold_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->coolant_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->oil_pressure = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->oil_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->fuel_pressure = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->fuel_consumption_rate_cm3pm);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->estimated_consumed_fuel_volume_cm3);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->throttle_position_percent);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 6, false, &msg->ecu_index);
    *bit_ofs += 6;

    canardDecodeScalar(transfer, *bit_ofs, 3, false, &msg->spark_plug_usage);
    *bit_ofs += 3;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->cylinder_status.len);
        *bit_ofs += 5;
    }


    if (tao) {
        msg->cylinder_status.len = 0;
        size_t max_len = 16;
        uint32_t max_bits = (transfer->payload_len*8)-7; // TAO elements must be >= 8 bits
        while (max_bits > *bit_ofs) {
            if (!max_len-- || _uavcan_equipment_ice_reciprocating_CylinderStatus_decode(transfer, bit_ofs, &msg->cylinder_status.data[msg->cylinder_status.len], false)) {return true;}
            msg->cylinder_status.len++;
        }
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (msg->cylinder_status.len > 16) {
            return true; /* invalid value */
        }
#pragma GCC diagnostic pop
        for (size_t i=0; i < msg->cylinder_status.len; i++) {
            if (_uavcan_equipment_ice_reciprocating_CylinderStatus_decode(transfer, bit_ofs, &msg->cylinder_status.data[i], false)) {return true;}
        }
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_ice_reciprocating_Status sample_uavcan_equipment_ice_reciprocating_Status_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_ice_reciprocating_Status, UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_ID, UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_SIGNATURE, UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_MAX_SIZE);
#endif
#endif
