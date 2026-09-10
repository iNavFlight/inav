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

timerHardware_t timerHardware[] = {
    // DEF_TIM(TIM5,   CH1, PA0,  TIM_USE_PWM | TIM_USE_PPM,   0, 0 ), // PPM IN
    // DEF_TIM(TIM5,   CH2, PA1,  TIM_USE_PWM,                 0, 0 ),
    // DEF_TIM(TIM3,   CH3, PB0,  TIM_USE_PWM,                 0, 0 ),
    // DEF_TIM(TIM2,   CH3, PB10, TIM_USE_PWM,                 0, 0 ),
    // DEF_TIM(TIM2,   CH4, PB11, TIM_USE_PWM,                 0, 0 ),
    // DEF_TIM(TIM1,   CH1, PA8,  TIM_USE_PWM,                 0, 0 ),

    DEF_TIM(TIM1,  CH2N, PB14,  TIM_USE_OUTPUT_AUTO,    0, 0 ), // was TIM12_CH1, which has no DMA request line on F405 (no DSHOT); TIM1_CH2N dmavar 0 is DMA2 Stream6. PB15 below cannot also be moved: TIM1_CH3 has only DMA2 Stream6 as well, and TIM1_CH2's other option (DMA2 Stream2) is held by TIM8_CH1/PC6
    DEF_TIM(TIM12,  CH2, PB15,  TIM_USE_OUTPUT_AUTO,    0, 0 ),
    DEF_TIM(TIM8,   CH1, PC6,   TIM_USE_OUTPUT_AUTO,    0, 1 ),
    DEF_TIM(TIM8,   CH2, PC7,   TIM_USE_OUTPUT_AUTO,    0, 1 ),
    DEF_TIM(TIM8,   CH3, PC8,   TIM_USE_OUTPUT_AUTO,    0, 1 ),
    DEF_TIM(TIM8,   CH4, PC9,   TIM_USE_OUTPUT_AUTO,    0, 0 ),

    DEF_TIM(TIM3,   CH4, PB1,   TIM_USE_LED,    0, 0 ),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
