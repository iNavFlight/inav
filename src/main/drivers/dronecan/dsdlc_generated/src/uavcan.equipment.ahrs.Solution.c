#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.ahrs.Solution.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_ahrs_Solution_encode(struct uavcan_equipment_ahrs_Solution* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_AHRS_SOLUTION_MAX_SIZE);
    _uavcan_equipment_ahrs_Solution_encode(buffer, &bit_ofs, msg, 
#if CANARD_ENABLE_TAO_OPTION
    tao
#else
    true
#endif
    );
    return ((bit_ofs+7)/8);
}

/*
  return true if the decode is invalid
 */
bool uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ahrs_Solution* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_AHRS_SOLUTION_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_ahrs_Solution_decode(transfer, &bit_ofs, msg,
#if CANARD_ENABLE_TAO_OPTION
    transfer->tao
#else
    true
#endif
    )) {
        return true; /* invalid payload */
    }

    const uint32_t byte_len = (bit_ofs+7U)/8U;
#if CANARD_ENABLE_TAO_OPTION
    // if this could be CANFD then the dlc could indicating more bytes than
    // we actually have
    if (!transfer->tao) {
        return byte_len > transfer->payload_len;
    }
#endif
    return byte_len != transfer->payload_len;
}

#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_ahrs_Solution sample_uavcan_equipment_ahrs_Solution_msg(void) {
    struct uavcan_equipment_ahrs_Solution msg;

    msg.timestamp = sample_uavcan_Timestamp_msg();
    for (size_t i=0; i < 4; i++) {
        msg.orientation_xyzw[i] = random_float16_val();
    }
    msg.orientation_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.orientation_covariance.len; i++) {
        msg.orientation_covariance.data[i] = random_float16_val();
    }
    for (size_t i=0; i < 3; i++) {
        msg.angular_velocity[i] = random_float16_val();
    }
    msg.angular_velocity_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.angular_velocity_covariance.len; i++) {
        msg.angular_velocity_covariance.data[i] = random_float16_val();
    }
    for (size_t i=0; i < 3; i++) {
        msg.linear_acceleration[i] = random_float16_val();
    }
    msg.linear_acceleration_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.linear_acceleration_covariance.len; i++) {
        msg.linear_acceleration_covariance.data[i] = random_float16_val();
    }
    return msg;
}
#endif
