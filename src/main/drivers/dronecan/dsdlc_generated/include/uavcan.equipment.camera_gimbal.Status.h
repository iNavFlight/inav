#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.camera_gimbal.Mode.h>


#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_MAX_SIZE 29
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_SIGNATURE (0xB9F127865BE0D61EULL)
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_ID 1044

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_camera_gimbal_Status_cxx_iface;
#endif

struct uavcan_equipment_camera_gimbal_Status {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_camera_gimbal_Status_cxx_iface;
#endif
    uint8_t gimbal_id;
    struct uavcan_equipment_camera_gimbal_Mode mode;
    float camera_orientation_in_body_frame_xyzw[4];
    struct { uint8_t len; float data[9]; }camera_orientation_in_body_frame_covariance;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_camera_gimbal_Status_encode(struct uavcan_equipment_camera_gimbal_Status* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_camera_gimbal_Status_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_camera_gimbal_Status* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_camera_gimbal_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Status* msg, bool tao);
static inline bool _uavcan_equipment_camera_gimbal_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Status* msg, bool tao);
void _uavcan_equipment_camera_gimbal_Status_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Status* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->gimbal_id);
    *bit_ofs += 8;
    _uavcan_equipment_camera_gimbal_Mode_encode(buffer, bit_ofs, &msg->mode, false);
    for (size_t i=0; i < 4; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->camera_orientation_in_body_frame_xyzw[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t camera_orientation_in_body_frame_covariance_len = msg->camera_orientation_in_body_frame_covariance.len > 9 ? 9 : msg->camera_orientation_in_body_frame_covariance.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 4, &camera_orientation_in_body_frame_covariance_len);
        *bit_ofs += 4;
    }
    for (size_t i=0; i < camera_orientation_in_body_frame_covariance_len; i++) {
        {
            uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->camera_orientation_in_body_frame_covariance.data[i]);
            canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
        }
        *bit_ofs += 16;
    }
}

/*
 decode uavcan_equipment_camera_gimbal_Status, return true on failure, false on success
*/
bool _uavcan_equipment_camera_gimbal_Status_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_Status* msg, bool tao) {
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
            msg->camera_orientation_in_body_frame_xyzw[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->camera_orientation_in_body_frame_covariance.len);
        *bit_ofs += 4;
    } else {
        msg->camera_orientation_in_body_frame_covariance.len = ((transfer->payload_len*8)-*bit_ofs)/16;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->camera_orientation_in_body_frame_covariance.len > 9) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->camera_orientation_in_body_frame_covariance.len; i++) {
        {
            uint16_t float16_val;
            canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
            msg->camera_orientation_in_body_frame_covariance.data[i] = canardConvertFloat16ToNativeFloat(float16_val);
        }
        *bit_ofs += 16;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_camera_gimbal_Status sample_uavcan_equipment_camera_gimbal_Status_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_camera_gimbal_Status, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_ID, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_SIGNATURE, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_STATUS_MAX_SIZE);
#endif
#endif
