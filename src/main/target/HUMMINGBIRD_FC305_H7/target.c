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

#include <platform.h>

#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5,  CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 0),   // Motor 1
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 1),   // Motor 2
    DEF_TIM(TIM5,  CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 2),   // Motor 3
    DEF_TIM(TIM5,  CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 3),   // Motor 4
    DEF_TIM(TIM4,  CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 4),   // Motor 5
    DEF_TIM(TIM4,  CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 5),   // Motor 6
    DEF_TIM(TIM4,  CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 6),   // Motor 7
    DEF_TIM(TIM4,  CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 7),   // Motor 8

    // TIM12 has no DMA request on STM32H743; these are PWM-only servo outputs.
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0),   // Servo 1
    DEF_TIM(TIM12, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0),   // Servo 2

    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_BEEPER,      0, 0),   // Beeper
    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_LED,         0, 13),  // LED strip

    // INAV does not currently support the Betaflight GYRO CLKIN feature.
    // DEF_TIM(TIM1, CH1, PE9, TIM_USE_ANY, 0, 0),             // GYRO_CLKIN
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
