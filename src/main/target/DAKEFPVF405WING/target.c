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
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM2,   CH2, PA1,  TIM_USE_OUTPUT_AUTO,   1, 0), // S1
    
    DEF_TIM(TIM3,   CH3, PB0,  TIM_USE_OUTPUT_AUTO,   1, 0), // S2
    DEF_TIM(TIM3,   CH4, PB1,  TIM_USE_OUTPUT_AUTO,   1, 0), // S3

    DEF_TIM(TIM12,  CH1,PB14, TIM_USE_SERVO,   1, 0), // S4 - TIM12 has no DMA on F405, servo-only
    DEF_TIM(TIM12,  CH2,PB15, TIM_USE_SERVO,   1, 0), // S5 - TIM12 has no DMA on F405, servo-only
    
    DEF_TIM(TIM8,   CH3, PC8,  TIM_USE_OUTPUT_AUTO,   1, 1), // S6
    DEF_TIM(TIM8,   CH4, PC9,  TIM_USE_OUTPUT_AUTO,   1, 0), // S7
    
    DEF_TIM(TIM1,   CH1, PA8,  TIM_USE_OUTPUT_AUTO,   0, 1), // S8 - DMA2 Stream1)
    DEF_TIM(TIM1,   CH2, PA9,  TIM_USE_OUTPUT_AUTO,   0, 1), // S9 - DMA2 Stream2
    DEF_TIM(TIM1,   CH3, PA10, TIM_USE_OUTPUT_AUTO,   0, 1), // S10 - Stream6
    

    DEF_TIM(TIM5,   CH1, PA0,  TIM_USE_LED,   1, 0), // 2812LED 
    DEF_TIM(TIM5,   CH3, PA2,  TIM_USE_ANY,   0, 0), // TX2  softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
