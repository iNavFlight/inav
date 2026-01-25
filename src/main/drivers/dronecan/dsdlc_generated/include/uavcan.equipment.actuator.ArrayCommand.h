#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.actuator.Command.h>


#define UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_MAX_SIZE 61
#define UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE (0xD8A7486238EC3AF3ULL)
#define UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID 1010

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_actuator_ArrayCommand_cxx_iface;
#endif

struct uavcan_equipment_actuator_ArrayCommand {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_actuator_ArrayCommand_cxx_iface;
#endif
    struct { uint8_t len; struct uavcan_equipment_actuator_Command data[15]; }commands;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_actuator_ArrayCommand_encode(struct uavcan_equipment_actuator_ArrayCommand* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_actuator_ArrayCommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_actuator_ArrayCommand* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_actuator_ArrayCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_ArrayCommand* msg, bool tao);
static inline bool _uavcan_equipment_actuator_ArrayCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_ArrayCommand* msg, bool tao);
void _uavcan_equipment_actuator_ArrayCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_ArrayCommand* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t commands_len = msg->commands.len > 15 ? 15 : msg->commands.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 4, &commands_len);
        *bit_ofs += 4;
    }
    for (size_t i=0; i < commands_len; i++) {
        _uavcan_equipment_actuator_Command_encode(buffer, bit_ofs, &msg->commands.data[i], false);
    }
}

/*
 decode uavcan_equipment_actuator_ArrayCommand, return true on failure, false on success
*/
bool _uavcan_equipment_actuator_ArrayCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_ArrayCommand* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 4, false, &msg->commands.len);
        *bit_ofs += 4;
    }


    if (tao) {
        msg->commands.len = 0;
        size_t max_len = 15;
        uint32_t max_bits = (transfer->payload_len*8)-7; // TAO elements must be >= 8 bits
        while (max_bits > *bit_ofs) {
            if (!max_len-- || _uavcan_equipment_actuator_Command_decode(transfer, bit_ofs, &msg->commands.data[msg->commands.len], false)) {return true;}
            msg->commands.len++;
        }
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (msg->commands.len > 15) {
            return true; /* invalid value */
        }
#pragma GCC diagnostic pop
        for (size_t i=0; i < msg->commands.len; i++) {
            if (_uavcan_equipment_actuator_Command_decode(transfer, bit_ofs, &msg->commands.data[i], false)) {return true;}
        }
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_actuator_ArrayCommand sample_uavcan_equipment_actuator_ArrayCommand_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_actuator_ArrayCommand, UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID, UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE, UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_MAX_SIZE);
#endif
#endif
