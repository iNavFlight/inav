#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_SIZE 36
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE (0x217F5C87D7EC951DULL)
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID 1030

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_equipment_esc_RawCommand_cxx_iface;
#endif

struct uavcan_equipment_esc_RawCommand {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_equipment_esc_RawCommand_cxx_iface;
#endif
    struct { uint8_t len; int16_t data[20]; }cmd;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_esc_RawCommand_encode(struct uavcan_equipment_esc_RawCommand* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_esc_RawCommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_esc_RawCommand* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_esc_RawCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_esc_RawCommand* msg, bool tao);
static inline bool _uavcan_equipment_esc_RawCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_esc_RawCommand* msg, bool tao);
void _uavcan_equipment_esc_RawCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_esc_RawCommand* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    const uint8_t cmd_len = msg->cmd.len > 20 ? 20 : msg->cmd.len;
#pragma GCC diagnostic pop
    if (!tao) {
        canardEncodeScalar(buffer, *bit_ofs, 5, &cmd_len);
        *bit_ofs += 5;
    }
    for (size_t i=0; i < cmd_len; i++) {
        canardEncodeScalar(buffer, *bit_ofs, 14, &msg->cmd.data[i]);
        *bit_ofs += 14;
    }
}

/*
 decode uavcan_equipment_esc_RawCommand, return true on failure, false on success
*/
bool _uavcan_equipment_esc_RawCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_esc_RawCommand* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    if (!tao) {
        canardDecodeScalar(transfer, *bit_ofs, 5, false, &msg->cmd.len);
        *bit_ofs += 5;
    } else {
        msg->cmd.len = ((transfer->payload_len*8)-*bit_ofs)/14;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (msg->cmd.len > 20) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    for (size_t i=0; i < msg->cmd.len; i++) {
        canardDecodeScalar(transfer, *bit_ofs, 14, true, &msg->cmd.data[i]);
        *bit_ofs += 14;
    }

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_esc_RawCommand sample_uavcan_equipment_esc_RawCommand_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
BROADCAST_MESSAGE_CXX_IFACE(uavcan_equipment_esc_RawCommand, UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID, UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE, UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_SIZE);
#endif
#endif
