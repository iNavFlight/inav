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
 //Target code By BorisB and Hector "Hectech FPV" Hind

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"

#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4,  CH2,  PB7, TIM_USE_PPM,                          0), // PPM  DMA(1,4)

    // Motors 1-4
    DEF_TIM(TIM16, CH1,  PA6, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0), // PWM1 UP(1,6)
    DEF_TIM(TIM8,  CH1N, PA7, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0), // PWM2 UP(2,1)
    DEF_TIM(TIM8,  CH2,  PB8, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM3 UP(2,1)
    DEF_TIM(TIM17, CH1,  PB9, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM4 UP(1,7)

    // Motors 5-6 or SoftSerial
    DEF_TIM(TIM3,  CH3,  PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM5
    DEF_TIM(TIM3,  CH4,  PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM6

    // Motors 7-8 or UART2
    DEF_TIM(TIM2,  CH4,  PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM7/UART2_RX
    DEF_TIM(TIM2,  CH3,  PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0), // PWM8/UART2_TX

    DEF_TIM(TIM1,  CH1,  PA8, TIM_USE_LED,                          0), // LED  DMA(1,2)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
