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
    // PWM1 PPM Pad
    DEF_TIM(TIM3,  CH1, PB4, TIM_USE_PPM,                     0), // PPM - PB4

    // PB5 / TIM3 CH2 is connected to USBPresent

    // PWM2-PWM5
    DEF_TIM(TIM8,  CH2, PB8, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR,    0), // PWM1 - PB8
    DEF_TIM(TIM4,  CH4, PB9, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,                       0), // PWM2 - PB9
    DEF_TIM(TIM15, CH2, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                       0), // PWM3 - PA3
    DEF_TIM(TIM15, CH1, PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                       0), // PWM4 - PA2


    // For iNav, PWM6&7 (PWM pins 5&6) are shared with UART3
    DEF_TIM(TIM2, CH3, PB10, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                       0), // PWM4 - PA2
    DEF_TIM(TIM2, CH4, PB11, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,                       0), // PWM4 - PA2

    DEF_TIM(TIM1,  CH1, PA8, TIM_USE_LED, 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
