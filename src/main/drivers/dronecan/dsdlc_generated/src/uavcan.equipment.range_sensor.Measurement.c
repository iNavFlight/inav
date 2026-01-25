#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.range_sensor.Measurement.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_range_sensor_Measurement_encode(struct uavcan_equipment_range_sensor_Measurement* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_RANGE_SENSOR_MEASUREMENT_MAX_SIZE);
    _uavcan_equipment_range_sensor_Measurement_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_equipment_range_sensor_Measurement_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_range_sensor_Measurement* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_RANGE_SENSOR_MEASUREMENT_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_range_sensor_Measurement_decode(transfer, &bit_ofs, msg,
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
struct uavcan_equipment_range_sensor_Measurement sample_uavcan_equipment_range_sensor_Measurement_msg(void) {
    struct uavcan_equipment_range_sensor_Measurement msg;

    msg.timestamp = sample_uavcan_Timestamp_msg();
    msg.sensor_id = (uint8_t)random_bitlen_unsigned_val(8);
    msg.beam_orientation_in_body_frame = sample_uavcan_CoarseOrientation_msg();
    msg.field_of_view = random_float16_val();
    msg.sensor_type = (uint8_t)random_bitlen_unsigned_val(5);
    msg.reading_type = (uint8_t)random_bitlen_unsigned_val(3);
    msg.range = random_float16_val();
    return msg;
}
#endif
