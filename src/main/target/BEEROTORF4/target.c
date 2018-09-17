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

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM9, CH2,  PA3, TIM_USE_PPM,                                               0, 0), // PPM IN

    DEF_TIM(TIM1, CH2N, PB0, TIM_USE_MC_MOTOR |                    TIM_USE_FW_MOTOR,    0, 0), // M1 - DMAR: DMA2_ST5
    DEF_TIM(TIM1, CH3N, PB1, TIM_USE_MC_MOTOR |                    TIM_USE_FW_MOTOR,    0, 0), // M2 -
    DEF_TIM(TIM2, CH2,  PA1, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,    0, 0), // M3 - DMAR: DMA1_ST7
    DEF_TIM(TIM2, CH1,  PA0, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,    0, 0), // M4 -
    DEF_TIM(TIM8, CH1,  PC6, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0), // M5 - DMAR: DMA2_ST1
    DEF_TIM(TIM8, CH2,  PC7, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0), // M6 -
    DEF_TIM(TIM3, CH2,  PB5, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0), // M7 - DMAR: DMA1_ST2
    DEF_TIM(TIM4, CH4,  PB9, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0), // M8 - DMAR: DMA1_ST6

    DEF_TIM(TIM4, CH3,  PB8, TIM_USE_LED,                                               0, 0), // LED_STRIP / TRANSPONDER - DMA1_ST7
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
