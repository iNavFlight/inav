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
    // PPM Pad
#ifdef SPRACINGF3MINI_MKII_REVA
    DEF_TIM(TIM3,  CH2, PB5,  TIM_USE_PPM,                       0), // PPM - PB5
    // PB4 / TIM3 CH1 is connected to USBPresent
#else
    DEF_TIM(TIM3,  CH1, PB4,  TIM_USE_PPM,                       0), // PPM - PB4
    // PB5 / TIM3 CH2 is connected to USBPresent
#endif

    DEF_TIM(TIM16, CH1, PA6,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,                     0), // PWM1 - PA6
    DEF_TIM(TIM17, CH1, PA7,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,                     0), // PWM2 - PA7
    DEF_TIM(TIM4,  CH3, PB8,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                     0), // PWM3 - PB8
    DEF_TIM(TIM4,  CH4, PB9,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                     0), // PWM4 - PB9
    DEF_TIM(TIM15, CH1, PA2,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM5 - PA2
    DEF_TIM(TIM15, CH2, PA3,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM6 - PA3
    DEF_TIM(TIM2,  CH1, PA0,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM7 - PA0
    DEF_TIM(TIM2,  CH2, PA1,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM8 - PA1

    // UART3 RX/TX
    DEF_TIM(TIM2,  CH3, PB10, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM9  - PB10 - TIM2_CH3 / UART3_TX (AF7)
    DEF_TIM(TIM2,  CH4, PB11, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0), // PWM10 - PB11 - TIM2_CH4 / UART3_RX (AF7)

    // LED Strip Pad
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED,                                             0), // LED_STRIP / TRANSPONDER
};


const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
