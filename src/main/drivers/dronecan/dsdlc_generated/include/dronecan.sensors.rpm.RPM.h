#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define DRONECAN_SENSORS_RPM_RPM_MAX_SIZE 7
#define DRONECAN_SENSORS_RPM_RPM_SIGNATURE (0x140707C09274F6E7ULL)
#define DRONECAN_SENSORS_RPM_RPM_ID 1045

#define DRONECAN_SENSORS_RPM_RPM_FLAGS_UNHEALTHY 1

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class dronecan_sensors_rpm_RPM_cxx_iface;
#endif

struct dronecan_sensors_rpm_RPM {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = dronecan_sensors_rpm_RPM_cxx_iface;
#endif
    uint8_t sensor_id;
    uint16_t flags;
    float rpm;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t dronecan_sensors_rpm_RPM_encode(struct dronecan_sensors_rpm_RPM* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool dronecan_sensors_rpm_RPM_decode(const CanardRxTransfer* transfer, struct dronecan_sensors_rpm_RPM* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _dronecan_sensors_rpm_RPM_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_rpm_RPM* msg, bool tao);
static inline bool _dronecan_sensors_rpm_RPM_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_rpm_RPM* msg, bool tao);
void _dronecan_sensors_rpm_RPM_encode(uint8_t* buffer, uint32_t* bit_ofs, struct dronecan_sensors_rpm_RPM* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->sensor_id);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->flags);
    *bit_ofs += 16;
    canardEncodeScalar(buffer, *bit_ofs, 32, &msg->rpm);
    *bit_ofs += 32;
}

/*
 decode dronecan_sensors_rpm_RPM, return true on failure, false on success
*/
bool _dronecan_sensors_rpm_RPM_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct dronecan_sensors_rpm_RPM* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->sensor_id);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->flags);
    *bit_ofs += 16;

    canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->rpm);
    *bit_ofs += 32;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct dronecan_sensors_rpm_RPM sample_dronecan_sensors_rpm_RPM_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(dronecan_sensors_rpm_RPM, DRONECAN_SENSORS_RPM_RPM_ID, DRONECAN_SENSORS_RPM_RPM_SIGNATURE, DRONECAN_SENSORS_RPM_RPM_MAX_SIZE);
#endif
#endif
