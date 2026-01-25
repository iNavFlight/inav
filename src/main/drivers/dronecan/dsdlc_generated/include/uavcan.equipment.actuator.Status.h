#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ACTUATOR_STATUS_MAX_SIZE 8
#define UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE (0x5E9BBA44FAF1EA04ULL)
#define UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID 1011

#define UAVCAN_EQUIPMENT_ACTUATOR_STATUS_POWER_RATING_PCT_UNKNOWN 127

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_actuator_Status_cxx_iface;
#endif

struct uavcan_equipment_actuator_Status {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_actuator_Status_cxx_iface;
#endif
    uint8_t actuator_id;
    float position;
    float force;
    float speed;
    uint8_t power_rating_pct;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_actuator_Status_encode(struct uavcan_equipment_actuator_Status* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_actuator_Status_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_actuator_Status* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_actuator_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Status* msg, bool tao);
static inline bool _uavcan_equipment_actuator_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Status* msg, bool tao);
void _uavcan_equipment_actuator_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Status* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->actuator_id);
    *bit_ofs += 8;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->position);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->force);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->speed);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
    *bit_ofs += 1;
    canardEncodeScalar(buffer, *bit_ofs, 7, &msg->power_rating_pct);
    *bit_ofs += 7;
}

/*
 decode uavcan_equipment_actuator_Status, return true on failure, false on success
*/
bool _uavcan_equipment_actuator_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Status* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->actuator_id);
    *bit_ofs += 8;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->position = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->force = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->speed = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    *bit_ofs += 1;

    canardDecodeScalar(transfer, *bit_ofs, 7, false, &msg->power_rating_pct);
    *bit_ofs += 7;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_actuator_Status sample_uavcan_equipment_actuator_Status_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_actuator_Status, UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID, UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE, UAVCAN_EQUIPMENT_ACTUATOR_STATUS_MAX_SIZE);
#endif
#endif
