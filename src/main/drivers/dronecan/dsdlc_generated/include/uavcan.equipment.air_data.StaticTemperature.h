#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_MAX_SIZE 4
#define UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_SIGNATURE (0x49272A6477D96271ULL)
#define UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_ID 1029

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_air_data_StaticTemperature_cxx_iface;
#endif

struct uavcan_equipment_air_data_StaticTemperature {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_air_data_StaticTemperature_cxx_iface;
#endif
    float static_temperature;
    float static_temperature_variance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_air_data_StaticTemperature_encode(struct uavcan_equipment_air_data_StaticTemperature* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_air_data_StaticTemperature_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_air_data_StaticTemperature* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_air_data_StaticTemperature_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_StaticTemperature* msg, bool tao);
static inline bool _uavcan_equipment_air_data_StaticTemperature_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_StaticTemperature* msg, bool tao);
void _uavcan_equipment_air_data_StaticTemperature_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_StaticTemperature* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->static_temperature);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->static_temperature_variance);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
}

/*
 decode uavcan_equipment_air_data_StaticTemperature, return true on failure, false on success
*/
bool _uavcan_equipment_air_data_StaticTemperature_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_StaticTemperature* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->static_temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->static_temperature_variance = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_air_data_StaticTemperature sample_uavcan_equipment_air_data_StaticTemperature_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_air_data_StaticTemperature, UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_ID, UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_SIGNATURE, UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_MAX_SIZE);
#endif
#endif
