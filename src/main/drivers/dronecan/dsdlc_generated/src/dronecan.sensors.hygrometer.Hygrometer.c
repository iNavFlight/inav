#define CANARD_DSDLC_INTERNAL
#include <dronecan.sensors.hygrometer.Hygrometer.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t dronecan_sensors_hygrometer_Hygrometer_encode(struct dronecan_sensors_hygrometer_Hygrometer* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, DRONECAN_SENSORS_HYGROMETER_HYGROMETER_MAX_SIZE);
    _dronecan_sensors_hygrometer_Hygrometer_encode(buffer, &bit_ofs, msg, 
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
bool dronecan_sensors_hygrometer_Hygrometer_decode(const CanardRxTransfer* transfer, struct dronecan_sensors_hygrometer_Hygrometer* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > DRONECAN_SENSORS_HYGROMETER_HYGROMETER_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_dronecan_sensors_hygrometer_Hygrometer_decode(transfer, &bit_ofs, msg,
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
struct dronecan_sensors_hygrometer_Hygrometer sample_dronecan_sensors_hygrometer_Hygrometer_msg(void) {
    struct dronecan_sensors_hygrometer_Hygrometer msg;

    msg.temperature = random_float16_val();
    msg.humidity = random_float16_val();
    msg.id = (uint8_t)random_bitlen_unsigned_val(8);
    return msg;
}
#endif
