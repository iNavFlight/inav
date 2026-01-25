#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_MAX_SIZE 1
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_SIGNATURE (0x9108C7785AEB69C4ULL)

#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_COMMAND_MODE_ANGULAR_VELOCITY 0
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_COMMAND_MODE_ORIENTATION_FIXED_FRAME 1
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_COMMAND_MODE_ORIENTATION_BODY_FRAME 2
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_MODE_COMMAND_MODE_GEO_POI 3


struct uavcan_equipment_camera_gimbal_Mode {
    uint8_t command_mode;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_camera_gimbal_Mode_encode(struct uavcan_equipment_camera_gimbal_Mode* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_camera_gimbal_Mode_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_camera_gimbal_Mode* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_camera_gimbal_Mode_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Mode* msg, bool tao);
static inline bool _uavcan_equipment_camera_gimbal_Mode_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Mode* msg, bool tao);
void _uavcan_equipment_camera_gimbal_Mode_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Mode* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->command_mode);
    *bit_ofs += 8;
}

/*
 decode uavcan_equipment_camera_gimbal_Mode, return true on failure, false on success
*/
bool _uavcan_equipment_camera_gimbal_Mode_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Mode* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->command_mode);
    *bit_ofs += 8;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_camera_gimbal_Mode sample_uavcan_equipment_camera_gimbal_Mode_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
