#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.camera_gimbal.Mode.h>


#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_MAX_SIZE 10
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_SIGNATURE (0x4AF6E57B2B2BE29CULL)
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_ID 1040

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_camera_gimbal_AngularCommand_cxx_iface;
#endif

struct uavcan_equipment_camera_gimbal_AngularCommand {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_camera_gimbal_AngularCommand_cxx_iface;
#endif
    uint8_t gimbal_id;
    struct uavcan_equipment_camera_gimbal_Mode mode;
    float quaternion_xyzw[4];
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_camera_gimbal_AngularCommand_encode(struct uavcan_equipment_camera_gimbal_AngularCommand* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_camera_gimbal_AngularCommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_camera_gimbal_AngularCommand* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_camera_gimbal_AngularCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_AngularCommand* msg, bool tao);
static inline bool _uavcan_equipment_camera_gimbal_AngularCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_AngularCommand* msg, bool tao);
void _uavcan_equipment_camera_gimbal_AngularCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_AngularCommand* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->gimbal_id);
    *bit_ofs += 8;
    _uavcan_equipment_camera_gimbal_Mode_encode(buffer, bit_ofs, &msg->mode, false);
    for (size_t i=0; i < 4; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->quaternion_xyzw[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_equipment_camera_gimbal_AngularCommand, return true on failure, false on success
*/
bool _uavcan_equipment_camera_gimbal_AngularCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_AngularCommand* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->gimbal_id);
    *bit_ofs += 8;

    if (_uavcan_equipment_camera_gimbal_Mode_decode(transfer, bit_ofs, &msg->mode, false)) {return true;}

    for (size_t i=0; i < 4; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->quaternion_xyzw[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_camera_gimbal_AngularCommand sample_uavcan_equipment_camera_gimbal_AngularCommand_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_camera_gimbal_AngularCommand, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_ID, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_SIGNATURE, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_ANGULARCOMMAND_MAX_SIZE);
#endif
#endif
