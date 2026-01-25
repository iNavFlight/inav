#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.camera_gimbal.Mode.h>


#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_MAX_SIZE 13
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_SIGNATURE (0x9371428A92F01FD6ULL)
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_ID 1041

#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_HEIGHT_REFERENCE_ELLIPSOID 0
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_HEIGHT_REFERENCE_MEAN_SEA_LEVEL 1

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_camera_gimbal_GEOPOICommand_cxx_iface;
#endif

struct uavcan_equipment_camera_gimbal_GEOPOICommand {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_camera_gimbal_GEOPOICommand_cxx_iface;
#endif
    uint8_t gimbal_id;
    struct uavcan_equipment_camera_gimbal_Mode mode;
    int32_t longitude_deg_1e7;
    int32_t latitude_deg_1e7;
    int32_t height_cm;
    uint8_t height_reference;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_camera_gimbal_GEOPOICommand_encode(struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_camera_gimbal_GEOPOICommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_camera_gimbal_GEOPOICommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg, bool tao);
static inline bool _uavcan_equipment_camera_gimbal_GEOPOICommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg, bool tao);
void _uavcan_equipment_camera_gimbal_GEOPOICommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->gimbal_id);
    *bit_ofs += 8;
    _uavcan_equipment_camera_gimbal_Mode_encode(buffer, bit_ofs, &msg->mode, false);
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->longitude_deg_1e7);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->latitude_deg_1e7);
    *bit_ofs += 32;
    canardEncodeScalar(buffer, *bit_ofs, 22, &msg->height_cm);
    *bit_ofs += 22;
    canardEncodeScalar(buffer, *bit_ofs, 2, &msg->height_reference);
    *bit_ofs += 2;
}

/*
 decode uavcan_equipment_camera_gimbal_GEOPOICommand, return true on failure, false on success
*/
bool _uavcan_equipment_camera_gimbal_GEOPOICommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_camera_gimbal_GEOPOICommand* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->gimbal_id);
    *bit_ofs += 8;

    if (_uavcan_equipment_camera_gimbal_Mode_decode(transfer, bit_ofs, &msg->mode, false)) {return true;}

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->longitude_deg_1e7);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->latitude_deg_1e7);
    *bit_ofs += 32;

    canardDecodeScalar(transfer, *bit_ofs, 22, true, &msg->height_cm);
    *bit_ofs += 22;

    canardDecodeScalar(transfer, *bit_ofs, 2, false, &msg->height_reference);
    *bit_ofs += 2;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_camera_gimbal_GEOPOICommand sample_uavcan_equipment_camera_gimbal_GEOPOICommand_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_camera_gimbal_GEOPOICommand, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_ID, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_SIGNATURE, UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_MAX_SIZE);
#endif
#endif
