#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_MAX_SIZE 4
#define UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_SIGNATURE (0xA1892D72AB8945FULL)
#define UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_ID 1021

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_air_data_IndicatedAirspeed_cxx_iface;
#endif

struct uavcan_equipment_air_data_IndicatedAirspeed {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_air_data_IndicatedAirspeed_cxx_iface;
#endif
    float indicated_airspeed;
    float indicated_airspeed_variance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_air_data_IndicatedAirspeed_encode(struct uavcan_equipment_air_data_IndicatedAirspeed* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_air_data_IndicatedAirspeed_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_air_data_IndicatedAirspeed* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_air_data_IndicatedAirspeed_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_IndicatedAirspeed* msg, bool tao);
static inline bool _uavcan_equipment_air_data_IndicatedAirspeed_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_IndicatedAirspeed* msg, bool tao);
void _uavcan_equipment_air_data_IndicatedAirspeed_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_IndicatedAirspeed* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->indicated_airspeed);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->indicated_airspeed_variance);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
}

/*
 decode uavcan_equipment_air_data_IndicatedAirspeed, return true on failure, false on success
*/
bool _uavcan_equipment_air_data_IndicatedAirspeed_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_air_data_IndicatedAirspeed* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->indicated_airspeed = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->indicated_airspeed_variance = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_air_data_IndicatedAirspeed sample_uavcan_equipment_air_data_IndicatedAirspeed_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_air_data_IndicatedAirspeed, UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_ID, UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_SIGNATURE, UAVCAN_EQUIPMENT_AIR_DATA_INDICATEDAIRSPEED_MAX_SIZE);
#endif
#endif
