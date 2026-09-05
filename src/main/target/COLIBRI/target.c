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

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    // DEF_TIM(TIM1,  CH3, PA10, TIM_USE_PWM | TIM_USE_PPM, 0, 0), // S1_IN
    // DEF_TIM(TIM8,  CH1, PC6,  TIM_USE_PWM,               0, 0), // S2_IN
    // DEF_TIM(TIM8,  CH2, PC7,  TIM_USE_PWM,               0, 0), // S3_IN
    // DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_PWM,               0, 0), // S4_IN
    // DEF_TIM(TIM2,  CH1, PA15, TIM_USE_PWM,               0, 0), // S5_IN
    // DEF_TIM(TIM2,  CH2, PB3,  TIM_USE_PWM,               0, 0), // S6_IN
    // DEF_TIM(TIM5,  CH1, PA0,  TIM_USE_PWM,               0, 0), // S7_IN
    // DEF_TIM(TIM5,  CH2, PA1,  TIM_USE_PWM,               0, 0), // S8_IN

    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_OUTPUT_AUTO,   0, 0), // S1_OUT
    DEF_TIM(TIM3,  CH1, PB4,  TIM_USE_OUTPUT_AUTO,   0, 0), // S2_OUT
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO,   0, 0), // S3_OUT
    DEF_TIM(TIM1, CH3N, PB15, TIM_USE_OUTPUT_AUTO,   0, 1), // S4_OUT -- was TIM12_CH2, which has no DMA request line on F405 (no DSHOT); dmavar 1 is TIM1_CH3's dedicated DMA2 Stream6 line
    DEF_TIM(TIM3,  CH2, PB5,  TIM_USE_OUTPUT_AUTO,   0, 0), // S5_OUT
    DEF_TIM(TIM1, CH2N, PB14, TIM_USE_OUTPUT_AUTO,   0, 1), // S6_OUT -- was TIM12_CH1, which has no DMA request line on F405 (no DSHOT); dmavar 1 is TIM1_CH2's dedicated DMA2 Stream2 line. dmavar 0 is the combined TIM1 CH1-3 request, which would collide with S4_OUT now that both are live TIM1 channels
    DEF_TIM(TIM10, CH1, PB8,  TIM_USE_OUTPUT_AUTO,   0, 0), // S7_OUT
    DEF_TIM(TIM11, CH1, PB9,  TIM_USE_OUTPUT_AUTO,   0, 0), // S8_OUT

    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_LED,                                              0, 0)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
