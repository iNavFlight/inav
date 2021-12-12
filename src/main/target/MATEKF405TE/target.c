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

#include <stdbool.h>
#include <platform.h>
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"
#include "drivers/timer_def_stm32f4xx.h"

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR,   0, 0), // S1 D(2,7,7)
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR,   0, 0), // S2 D(2,2,0)
    DEF_TIM(TIM1,  CH3N, PB15, TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 1), // S3 D(2,6,6)
    DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 1), // S4 D(2,1,6)

    DEF_TIM(TIM2,  CH4,  PB11, TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 0), // S5 D(1,7,3)
    DEF_TIM(TIM2,  CH3,  PB10, TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 0), // S6 D(1,1,3)
    DEF_TIM(TIM2,  CH2,  PB3,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 0), // S7 D(1,6,3)
    DEF_TIM(TIM2,  CH1,  PA15, TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO,   0, 0), // S8 D(1,5,3)

    DEF_TIM(TIM12, CH1,  PB14, TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO,   0, 0), // S9  DMA NONE
    DEF_TIM(TIM13, CH1,  PA6,  TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO,   0, 0), // S10 DMA NONE
    DEF_TIM(TIM4,  CH1,  PB6,  TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO,   0, 0), // S11 D(1,0,2)

    DEF_TIM(TIM3,  CH4,  PB1,  TIM_USE_LED,    0, 0), // 2812LED  D(1,2,5)
    DEF_TIM(TIM11, CH1,  PB9,  TIM_USE_BEEPER, 0, 0), // BEEPER PWM
    
    DEF_TIM(TIM9,  CH2,  PA3,  TIM_USE_PPM,    0, 0), //RX2
    DEF_TIM(TIM5,  CH3,  PA2,  TIM_USE_ANY,    0, 0), //TX2  softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
