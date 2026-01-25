#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_MAX_SIZE 13
#define DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_SIGNATURE (0x3053EBE3D750286FULL)
#define DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_ID 1043

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_cxx_iface;
#endif

struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_cxx_iface;
#endif
    uint8_t sensor_id;
    float magnetic_field_ga[3];
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_encode(struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_decode(const CanardRxTransfer* transfer, struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg, bool tao);
static inline bool _dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg, bool tao);
void _dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->sensor_id);
    *bit_ofs += 8;
    for (size_t i=0; i < 3; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 32, &msg->magnetic_field_ga[i]);
        *bit_ofs += 32;
    }
}

/*
 decode dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes, return true on failure, false on success
*/
bool _dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->sensor_id);
    *bit_ofs += 8;

    for (size_t i=0; i < 3; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->magnetic_field_ga[i]);
        *bit_ofs += 32;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes sample_dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_sensors_magnetometer_MagneticFieldStrengthHiRes, DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_ID, DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_SIGNATURE, DRONECAN_SENSORS_MAGNETOMETER_MAGNETICFIELDSTRENGTHHIRES_MAX_SIZE);
#endif
#endif
