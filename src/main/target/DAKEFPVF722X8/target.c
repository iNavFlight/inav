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

#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include <platform.h>
#include <stdbool.h>

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_OUTPUT_AUTO, 0, 0),  // S1  D(1, 4, 6)
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_OUTPUT_AUTO, 0, 0),  // S2  D(1, 0, 6)
    DEF_TIM(TIM5, CH4, PA3, TIM_USE_OUTPUT_AUTO, 0, 0),  // S3  D(1, 1, 6),D(1, 3, 6)
    DEF_TIM(TIM1, CH1, PA8, TIM_USE_OUTPUT_AUTO, 0, 1),  // S4  D(2, 6, 0), D(2, 1, 6), D(2, 3, 6)
    DEF_TIM(TIM1, CH2, PA9, TIM_USE_OUTPUT_AUTO, 0, 1),  // S5  D(2, 6, 0), D(2, 2, 6)
    DEF_TIM(TIM1, CH3, PA10,TIM_USE_OUTPUT_AUTO, 0, 1),  // S6  D(2, 6, 0), D(2, 6, 6)
    DEF_TIM(TIM8, CH3, PC8, TIM_USE_OUTPUT_AUTO, 0, 0),  // S7  D(2, 4, 7), D(2, 2, 0)
    DEF_TIM(TIM8, CH4, PC9, TIM_USE_OUTPUT_AUTO, 0, 0),  // S8  D(2, 7, 7)
    DEF_TIM(TIM2, CH1, PA0, TIM_USE_LED, 0, 0), // LEDStrip     D(1, 5, 3)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
