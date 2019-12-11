/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

#include "brainfpv/brainfpv_osd.h"
#include "brainfpv/ir_transponder.h"
#include "fpga_drv.h"


const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_PPM,      0, 0), // PPM In
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0), // S1
    DEF_TIM(TIM2,  CH4, PA3,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0), // S2
    DEF_TIM(TIM1,  CH3, PA10, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0), // S3
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0), // S4
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO | TIM_USE_MC_SERVO, 0, 0), // S5
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0), // S6
//    DEF_TIM(TIM11, CH1, PB9,  TIM_USE_ANY,      0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

bool brainfpv_settings_updated = true;
bool brainfpv_settings_updated_from_cms = false;

extern bfOsdConfig_t bfOsdConfigCms;

void brainFPVUpdateSettings(void) {
    const bfOsdConfig_t * bfOsdConfigUse;

    if (brainfpv_settings_updated_from_cms)
        bfOsdConfigUse = &bfOsdConfigCms;
    else
        bfOsdConfigUse = bfOsdConfig();

    if (!bfOsdConfigUse->invert){
        BRAINFPVFPGA_SetBwLevels(bfOsdConfigUse->black_level, bfOsdConfigUse->white_level);
    }
    else {
        BRAINFPVFPGA_SetBwLevels(bfOsdConfigUse->white_level, bfOsdConfigUse->black_level);
    }

    BRAINFPVFPGA_SetSyncThreshold(bfOsdConfigUse->sync_threshold);
    BRAINFPVFPGA_SetXOffset(bfOsdConfigUse->x_offset);
    BRAINFPVFPGA_SetXScale(bfOsdConfigUse->x_scale);
    BRAINFPVFPGA_Set3DConfig(bfOsdConfigUse->sbs_3d_enabled, bfOsdConfigUse->sbs_3d_right_eye_offset);

    if (bfOsdConfigUse->ir_system == 1) {
        uint8_t ir_data[6];
        ir_generate_ilap_packet(bfOsdConfigUse->ir_ilap_id, ir_data, 6);
        BRAINFPVFPGA_SetIRData(ir_data, 6);
        BRAINFPVFPGA_SetIRProtocol(BRAINFPVFPGA_IR_PROTOCOL_ILAP);
    }

    if (bfOsdConfigUse->ir_system == 2) {
        uint8_t ir_data[4];
        ir_generate_trackmate_packet(bfOsdConfigUse->ir_trackmate_id, ir_data, 6);
        BRAINFPVFPGA_SetIRData(ir_data, 4);
        BRAINFPVFPGA_SetIRProtocol(BRAINFPVFPGA_IR_PROTOCOL_TRACKMATE);
    }
    brainfpv_settings_updated_from_cms = false;
}

#define RADIX_LI_IDENT_ADDR 0x1FFF7800
#define RADIX_LI_MAGIC 0x52414C49
bool brainfpv_is_radixli(void)
{
    uint32_t * magic = (uint32_t *)0x1FFF7800;
    return (*magic == RADIX_LI_MAGIC);
}

