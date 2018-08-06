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
    { TIM12, IO_TAG(PB14), TIM_Channel_1, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM12, TIM_USE_PPM }, // PPM
    { TIM5,  IO_TAG(PA2),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // S1_OUT
    { TIM2,  IO_TAG(PA3),  TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S2_OUT
    { TIM1,  IO_TAG(PA10), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S3_OUT
    { TIM2,  IO_TAG(PA15), TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S4_OUT
    { TIM8,  IO_TAG(PC8),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO | TIM_USE_MC_SERVO }, // S5_OUT
    { TIM3,  IO_TAG(PB0),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S6_OUT
//    { TIM11, IO_TAG(PB9),  TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM11, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // CC
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
