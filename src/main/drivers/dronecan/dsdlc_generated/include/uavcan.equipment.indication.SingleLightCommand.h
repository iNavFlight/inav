#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.equipment.indication.RGB565.h>


#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_MAX_SIZE 3
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_SIGNATURE (0xE894B8B589807007ULL)

#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_ANTI_COLLISION 246
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_RIGHT_OF_WAY 247
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_STROBE 248
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_WING 249
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_LOGO 250
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_TAXI 251
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_TURN_OFF 252
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_TAKE_OFF 253
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_LANDING 254
#define UAVCAN_EQUIPMENT_INDICATION_SINGLELIGHTCOMMAND_LIGHT_ID_FORMATION 255


struct uavcan_equipment_indication_SingleLightCommand {
    uint8_t light_id;
    struct uavcan_equipment_indication_RGB565 color;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_equipment_indication_SingleLightCommand_encode(struct uavcan_equipment_indication_SingleLightCommand* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_equipment_indication_SingleLightCommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_indication_SingleLightCommand* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_equipment_indication_SingleLightCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_indication_SingleLightCommand* msg, bool tao);
static inline bool _uavcan_equipment_indication_SingleLightCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_indication_SingleLightCommand* msg, bool tao);
void _uavcan_equipment_indication_SingleLightCommand_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_indication_SingleLightCommand* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->light_id);
    *bit_ofs += 8;
    _uavcan_equipment_indication_RGB565_encode(buffer, bit_ofs, &msg->color, tao);
}

/*
 decode uavcan_equipment_indication_SingleLightCommand, return true on failure, false on success
*/
bool _uavcan_equipment_indication_SingleLightCommand_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_indication_SingleLightCommand* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->light_id);
    *bit_ofs += 8;

    if (_uavcan_equipment_indication_RGB565_decode(transfer, bit_ofs, &msg->color, tao)) {return true;}

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_indication_SingleLightCommand sample_uavcan_equipment_indication_SingleLightCommand_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
