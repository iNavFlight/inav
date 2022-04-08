/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "platform.h"
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH4, PB1,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),  // S1 DMA1_S2_CH5
    DEF_TIM(TIM3, CH3, PB0,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),  // S2 DMA1_S7_CH5

    DEF_TIM(TIM8, CH3, PC8,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 1),  // S3 DMA2_S4_CH7
    DEF_TIM(TIM8, CH4, PC9,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S4 DMA2_S7_CH7
    DEF_TIM(TIM4, CH1, PB6,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S5 DMA1_S0_CH2
    DEF_TIM(TIM4, CH2, PB7,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S6 DMA1_S3_CH2

    DEF_TIM(TIM2, CH3, PB10,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S7 DMA1_S1_CH3
    DEF_TIM(TIM2, CH4, PB11,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 1),  // S8 DMA1_S6_CH3
    

    DEF_TIM(TIM1, CH2N, PB0,  TIM_USE_LED,                         0, 0),  // WS2812B DMA2_S6_CH0
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
