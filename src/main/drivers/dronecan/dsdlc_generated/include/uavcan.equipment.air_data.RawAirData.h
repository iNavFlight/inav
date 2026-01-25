#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_MAX_SIZE 50
#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_SIGNATURE (0xC77DF38BA122F5DAULL)
#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_ID 1027

#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_FLAG_HEATER_AVAILABLE 1
#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_FLAG_HEATER_WORKING 2
#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_FLAG_HEATER_OVERCURRENT 4
#define UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_FLAG_HEATER_OPENCIRCUIT 8

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_air_data_RawAirData_cxx_iface;
#endif

struct uavcan_equipment_air_data_RawAirData {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_air_data_RawAirData_cxx_iface;
#endif
    uint8_t flags;
    float static_pressure;
    float differential_pressure;
    float static_pressure_sensor_temperature;
    float differential_pressure_sensor_temperature;
    float static_air_temperature;
    float pitot_temperature;
    struct { uint8_t len; float data[16]; }covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_air_data_RawAirData_encode(struct uavcan_equipment_air_data_RawAirData* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_air_data_RawAirData_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_air_data_RawAirData* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_air_data_RawAirData_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_RawAirData* msg, bool tao);
static inline bool _uavcan_equipment_air_data_RawAirData_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_RawAirData* msg, bool tao);
void _uavcan_equipment_air_data_RawAirData_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_RawAirData* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->flags);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->static_pressure);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->differential_pressure);
    *bit_ofs += 32;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->static_pressure_sensor_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->differential_pressure_sensor_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->static_air_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->pitot_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t covariance_len = msg->covariance.len > 16 ? 16 : msg->covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &covariance_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_equipment_air_data_RawAirData, return true on failure, false on success
*/
bool _uavcan_equipment_air_data_RawAirData_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_RawAirData* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->flags);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->static_pressure);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->differential_pressure);
    *bit_ofs += 32;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->static_pressure_sensor_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->differential_pressure_sensor_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->static_air_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->pitot_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->covariance.len);
        *bit_ofs += 5;
    } else {
        msg->covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->covariance.len > 16) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_air_data_RawAirData sample_uavcan_equipment_air_data_RawAirData_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_air_data_RawAirData, UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_ID, UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_SIGNATURE, UAVCAN_EQUIPMENT_AIR_DATA_RAWAIRDATA_MAX_SIZE);
#endif
#endif
