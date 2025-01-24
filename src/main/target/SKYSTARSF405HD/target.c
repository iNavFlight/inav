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
    // DEF_TIM(TIM1, CH1, PA8,  TIM_USE_PPM,   0, 0),

    DEF_TIM(TIM4, CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0, 0),
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_OUTPUT_AUTO, 0, 0),
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0),
    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_OUTPUT_AUTO, 0, 0),
#if defined(SKYSTARSF405HD2) || defined(SKYSTARSF405AIO)
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),
#endif

    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_LED, 0, 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
