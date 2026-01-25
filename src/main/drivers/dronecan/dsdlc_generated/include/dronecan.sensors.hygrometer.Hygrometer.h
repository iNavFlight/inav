#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_SENSORS_HYGROMETER_HYGROMETER_MAX_SIZE 5
#define DRONECAN_SENSORS_HYGROMETER_HYGROMETER_SIGNATURE (0xCEB308892BF163E8ULL)
#define DRONECAN_SENSORS_HYGROMETER_HYGROMETER_ID 1032

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_sensors_hygrometer_Hygrometer_cxx_iface;
#endif

struct dronecan_sensors_hygrometer_Hygrometer {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_sensors_hygrometer_Hygrometer_cxx_iface;
#endif
    float temperature;
    float humidity;
    uint8_t id;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_sensors_hygrometer_Hygrometer_encode(struct dronecan_sensors_hygrometer_Hygrometer* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_sensors_hygrometer_Hygrometer_decode(const CanardRxTransfer* transfer, struct dronecan_sensors_hygrometer_Hygrometer* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_sensors_hygrometer_Hygrometer_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_hygrometer_Hygrometer* msg, bool tao);
static inline bool _dronecan_sensors_hygrometer_Hygrometer_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_hygrometer_Hygrometer* msg, bool tao);
void _dronecan_sensors_hygrometer_Hygrometer_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_hygrometer_Hygrometer* msg, bool tao) {
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
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->humidity);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->id);
    *bit_ofs += 8;
}

/*
 decode dronecan_sensors_hygrometer_Hygrometer, return true on failure, false on success
*/
bool _dronecan_sensors_hygrometer_Hygrometer_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_hygrometer_Hygrometer* msg, bool tao) {
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
        msg->humidity = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->id);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_sensors_hygrometer_Hygrometer sample_dronecan_sensors_hygrometer_Hygrometer_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_sensors_hygrometer_Hygrometer, DRONECAN_SENSORS_HYGROMETER_HYGROMETER_ID, DRONECAN_SENSORS_HYGROMETER_HYGROMETER_SIGNATURE, DRONECAN_SENSORS_HYGROMETER_HYGROMETER_MAX_SIZE);
#endif
#endif
