#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_MAX_SIZE 3
#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_SIGNATURE (0xA1A036268B0C3455ULL)
#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_ID 1070

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_hardpoint_Command_cxx_iface;
#endif

struct uavcan_equipment_hardpoint_Command {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_hardpoint_Command_cxx_iface;
#endif
    uint8_t hardpoint_id;
    uint16_t command;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_hardpoint_Command_encode(struct uavcan_equipment_hardpoint_Command* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_hardpoint_Command_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_hardpoint_Command* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_hardpoint_Command_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_hardpoint_Command* msg, bool tao);
static inline bool _uavcan_equipment_hardpoint_Command_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_hardpoint_Command* msg, bool tao);
void _uavcan_equipment_hardpoint_Command_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_hardpoint_Command* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->hardpoint_id);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 16, &msg->command);
    *bit_ofs += 16;
}

/*
 decode uavcan_equipment_hardpoint_Command, return true on failure, false on success
*/
bool _uavcan_equipment_hardpoint_Command_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_hardpoint_Command* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->hardpoint_id);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 16, false, &msg->command);
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_hardpoint_Command sample_uavcan_equipment_hardpoint_Command_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_hardpoint_Command, UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_ID, UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_SIGNATURE, UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_MAX_SIZE);
#endif
#endif
