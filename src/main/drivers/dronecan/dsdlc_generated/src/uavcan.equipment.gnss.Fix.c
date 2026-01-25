#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.gnss.Fix.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_gnss_Fix_encode(struct uavcan_equipment_gnss_Fix* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE);
    _uavcan_equipment_gnss_Fix_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_equipment_gnss_Fix_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_gnss_Fix* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_gnss_Fix_decode(transfer, &bit_ofs, msg,
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
struct uavcan_equipment_gnss_Fix sample_uavcan_equipment_gnss_Fix_msg(void) {
    struct uavcan_equipment_gnss_Fix msg;

    msg.timestamp = sample_uavcan_Timestamp_msg();
    msg.gnss_timestamp = sample_uavcan_Timestamp_msg();
    msg.gnss_time_standard = (uint8_t)random_bitlen_unsigned_val(3);
    msg.num_leap_seconds = (uint8_t)random_bitlen_unsigned_val(8);
    msg.longitude_deg_1e8 = (int64_t)random_bitlen_signed_val(37);
    msg.latitude_deg_1e8 = (int64_t)random_bitlen_signed_val(37);
    msg.height_ellipsoid_mm = (int32_t)random_bitlen_signed_val(27);
    msg.height_msl_mm = (int32_t)random_bitlen_signed_val(27);
    for (size_t i=0; i < 3; i++) {
        msg.ned_velocity[i] = random_float16_val();
    }
    msg.sats_used = (uint8_t)random_bitlen_unsigned_val(6);
    msg.status = (uint8_t)random_bitlen_unsigned_val(2);
    msg.pdop = random_float16_val();
    msg.position_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.position_covariance.len; i++) {
        msg.position_covariance.data[i] = random_float16_val();
    }
    msg.velocity_covariance.len = (uint8_t)random_range_unsigned_val(0, 9);
    for (size_t i=0; i < msg.velocity_covariance.len; i++) {
        msg.velocity_covariance.data[i] = random_float16_val();
    }
    return msg;
}
#endif
