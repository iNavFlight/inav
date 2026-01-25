#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.ice.reciprocating.Status.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_ice_reciprocating_Status_encode(struct uavcan_equipment_ice_reciprocating_Status* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_MAX_SIZE);
    _uavcan_equipment_ice_reciprocating_Status_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_equipment_ice_reciprocating_Status_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_ice_reciprocating_Status* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_ICE_RECIPROCATING_STATUS_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_ice_reciprocating_Status_decode(transfer, &bit_ofs, msg,
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
struct uavcan_equipment_ice_reciprocating_Status sample_uavcan_equipment_ice_reciprocating_Status_msg(void) {
    struct uavcan_equipment_ice_reciprocating_Status msg;

    msg.state = (uint8_t)random_bitlen_unsigned_val(2);
    msg.flags = (uint32_t)random_bitlen_unsigned_val(30);
    msg.engine_load_percent = (uint8_t)random_bitlen_unsigned_val(7);
    msg.engine_speed_rpm = (uint32_t)random_bitlen_unsigned_val(17);
    msg.spark_dwell_time_ms = random_float16_val();
    msg.atmospheric_pressure_kpa = random_float16_val();
    msg.intake_manifold_pressure_kpa = random_float16_val();
    msg.intake_manifold_temperature = random_float16_val();
    msg.coolant_temperature = random_float16_val();
    msg.oil_pressure = random_float16_val();
    msg.oil_temperature = random_float16_val();
    msg.fuel_pressure = random_float16_val();
    msg.fuel_consumption_rate_cm3pm = random_float_val();
    msg.estimated_consumed_fuel_volume_cm3 = random_float_val();
    msg.throttle_position_percent = (uint8_t)random_bitlen_unsigned_val(7);
    msg.ecu_index = (uint8_t)random_bitlen_unsigned_val(6);
    msg.spark_plug_usage = (uint8_t)random_bitlen_unsigned_val(3);
    msg.cylinder_status.len = (uint8_t)random_range_unsigned_val(0, 16);
    for (size_t i=0; i < msg.cylinder_status.len; i++) {
        msg.cylinder_status.data[i] = sample_uavcan_equipment_ice_reciprocating_CylinderStatus_msg();
    }
    return msg;
}
#endif
