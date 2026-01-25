#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE 55
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE (0x249C26548A711966ULL)
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID 1092

#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_IN_USE 1
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_CHARGING 2
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_CHARGED 4
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_TEMP_HOT 8
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_TEMP_COLD 16
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_OVERLOAD 32
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_BAD_BATTERY 64
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_NEED_SERVICE 128
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_BMS_ERROR 256
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_RESERVED_A 512
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_RESERVED_B 1024
#define UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATE_OF_HEALTH_UNKNOWN 127

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_power_BatteryInfo_cxx_iface;
#endif

struct uavcan_equipment_power_BatteryInfo {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_power_BatteryInfo_cxx_iface;
#endif
    float temperature;
    float voltage;
    float current;
    float average_power_10sec;
    float remaining_capacity_wh;
    float full_charge_capacity_wh;
    float hours_to_full_charge;
    uint16_t status_flags;
    uint8_t state_of_health_pct;
    uint8_t state_of_charge_pct;
    uint8_t state_of_charge_pct_stdev;
    uint8_t battery_id;
    uint32_t model_instance_id;
    struct { uint8_t len; uint8_t data[31]; }model_name;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_power_BatteryInfo_encode(struct uavcan_equipment_power_BatteryInfo* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_power_BatteryInfo_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_power_BatteryInfo* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_power_BatteryInfo_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_power_BatteryInfo* msg, bool tao);
static inline bool _uavcan_equipment_power_BatteryInfo_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_power_BatteryInfo* msg, bool tao);
void _uavcan_equipment_power_BatteryInfo_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_power_BatteryInfo* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
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
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->average_power_10sec);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->remaining_capacity_wh);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->full_charge_capacity_wh);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->hours_to_full_charge);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 11, &msg->status_flags);
    *bit_ofs += 11;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->state_of_health_pct);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->state_of_charge_pct);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->state_of_charge_pct_stdev);
    *bit_ofs += 7;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->battery_id);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->model_instance_id);
    *bit_ofs += 32;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t model_name_len = msg->model_name.len > 31 ? 31 : msg->model_name.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &model_name_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < model_name_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 8, &msg->model_name.data[i]);
        *bit_ofs += 8;
    }
}

/*
 decode uavcan_equipment_power_BatteryInfo, return true on failure, false on success
*/
bool _uavcan_equipment_power_BatteryInfo_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_power_BatteryInfo* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
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

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->average_power_10sec = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->remaining_capacity_wh = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->full_charge_capacity_wh = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->hours_to_full_charge = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 11, false, &msg->status_flags);
    *bit_ofs += 11;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->state_of_health_pct);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->state_of_charge_pct);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->state_of_charge_pct_stdev);
    *bit_ofs += 7;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->battery_id);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, false, &msg->model_instance_id);
    *bit_ofs += 32;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->model_name.len);
        *bit_ofs += 5;
    } else {
        msg->model_name.len = ((transfer->payload_len*8)-*bit_ofs)/8;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->model_name.len > 31) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->model_name.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->model_name.data[i]);
        *bit_ofs += 8;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_power_BatteryInfo sample_uavcan_equipment_power_BatteryInfo_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_power_BatteryInfo, UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID, UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE, UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE);
#endif
#endif
