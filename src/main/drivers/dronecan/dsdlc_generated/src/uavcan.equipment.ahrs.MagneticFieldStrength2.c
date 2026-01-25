#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.ahrs.MagneticFieldStrength2.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_ahrs_MagneticFieldStrength2_encode(struct uavcan_equipment_ahrs_MagneticFieldStrength2* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_AHRS_MAGNETICFIELDSTRENGTH2_MAX_SIZE);
    _uavcan_equipment_ahrs_MagneticFieldStrength2_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_equipment_ahrs_MagneticFieldStrength2_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ahrs_MagneticFieldStrength2* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_AHRS_MAGNETICFIELDSTRENGTH2_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_ahrs_MagneticFieldStrength2_decode(transfer, &bit_ofs, msg,
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
struct uavcan_equipment_ahrs_MagneticFieldStrength2 sample_uavcan_equipment_ahrs_MagneticFieldStrength2_msg(void) {
    struct uavcan_equipment_ahrs_MagneticFieldStrength2 msg;

    msg.sensor_id = (uint8_t)random_bitlen_unsigned_val(8);
    for (size_t i=0; i < 3; i++) {
        msg.magnetic_field_ga[i] = random_float16_val();
    }
    msg.magnetic_field_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.magnetic_field_covariance.len; i++) {
        msg.magnetic_field_covariance.data[i] = random_float16_val();
    }
    return msg;
}
#endif
