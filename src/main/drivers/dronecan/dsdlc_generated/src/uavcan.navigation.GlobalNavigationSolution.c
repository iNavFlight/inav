#define CANARD_DSDLC_INTERNAL
#include <uavcan.navigation.GlobalNavigationSolution.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_navigation_GlobalNavigationSolution_encode(struct uavcan_navigation_GlobalNavigationSolution* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_MAX_SIZE);
    _uavcan_navigation_GlobalNavigationSolution_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_navigation_GlobalNavigationSolution_decode(const CanardRxTransfer* transfer, struct uavcan_navigation_GlobalNavigationSolution* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_NAVIGATION_GLOBALNAVIGATIONSOLUTION_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_navigation_GlobalNavigationSolution_decode(transfer, &bit_ofs, msg,
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
struct uavcan_navigation_GlobalNavigationSolution sample_uavcan_navigation_GlobalNavigationSolution_msg(void) {
    struct uavcan_navigation_GlobalNavigationSolution msg;

    msg.timestamp = sample_uavcan_Timestamp_msg();
    msg.longitude = random_float_val();
    msg.latitude = random_float_val();
    msg.height_ellipsoid = random_float_val();
    msg.height_msl = random_float_val();
    msg.height_agl = random_float_val();
    msg.height_baro = random_float_val();
    msg.qnh_hpa = random_float16_val();
    for (size_t i=0; i < 4; i++) {
        msg.orientation_xyzw[i] = random_float_val();
    }
    msg.pose_covariance.len = (uint8_t)random_range_unsigned_val(0, 36);
    for (size_t i=0; i < msg.pose_covariance.len; i++) {
        msg.pose_covariance.data[i] = random_float16_val();
    }
    for (size_t i=0; i < 3; i++) {
        msg.linear_velocity_body[i] = random_float_val();
    }
    for (size_t i=0; i < 3; i++) {
        msg.angular_velocity_body[i] = random_float_val();
    }
    for (size_t i=0; i < 3; i++) {
        msg.linear_acceleration_body[i] = random_float16_val();
    }
    msg.velocity_covariance.len = (uint8_t)random_range_unsigned_val(0, 36);
    for (size_t i=0; i < msg.velocity_covariance.len; i++) {
        msg.velocity_covariance.data[i] = random_float16_val();
    }
    return msg;
}
#endif
