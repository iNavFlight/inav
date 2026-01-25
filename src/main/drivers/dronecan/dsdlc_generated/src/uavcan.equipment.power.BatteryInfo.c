#define CANARD_DSDLC_INTERNAL
#include <uavcan.equipment.power.BatteryInfo.h>
#include <string.h>

#ifdef CANARD_DSDLC_TEST_BUILD
#include <test_helpers.h>
#endif

uint32_t uavcan_equipment_power_BatteryInfo_encode(struct uavcan_equipment_power_BatteryInfo* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
    uint32_t bit_ofs = 0;
    memset(buffer, 0, UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE);
    _uavcan_equipment_power_BatteryInfo_encode(buffer, &bit_ofs, msg, 
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
bool uavcan_equipment_power_BatteryInfo_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_power_BatteryInfo* msg) {
#if CANARD_ENABLE_TAO_OPTION
    if (transfer->tao && (transfer->payload_len > UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE)) {
        return true; /* invalid payload length */
    }
#endif
    uint32_t bit_ofs = 0;
    if (_uavcan_equipment_power_BatteryInfo_decode(transfer, &bit_ofs, msg,
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
struct uavcan_equipment_power_BatteryInfo sample_uavcan_equipment_power_BatteryInfo_msg(void) {
    struct uavcan_equipment_power_BatteryInfo msg;

    msg.temperature = random_float16_val();
    msg.voltage = random_float16_val();
    msg.current = random_float16_val();
    msg.average_power_10sec = random_float16_val();
    msg.remaining_capacity_wh = random_float16_val();
    msg.full_charge_capacity_wh = random_float16_val();
    msg.hours_to_full_charge = random_float16_val();
    msg.status_flags = (uint16_t)random_bitlen_unsigned_val(11);
    msg.state_of_health_pct = (uint8_t)random_bitlen_unsigned_val(7);
    msg.state_of_charge_pct = (uint8_t)random_bitlen_unsigned_val(7);
    msg.state_of_charge_pct_stdev = (uint8_t)random_bitlen_unsigned_val(7);
    msg.battery_id = (uint8_t)random_bitlen_unsigned_val(8);
    msg.model_instance_id = (uint32_t)random_bitlen_unsigned_val(32);
    msg.model_name.len = (uint8_t)random_range_unsigned_val(0, 31);
    for (size_t i=0; i < msg.model_name.len; i++) {
        msg.model_name.data[i] = (uint8_t)random_bitlen_unsigned_val(8);
    }
    return msg;
}
#endif
