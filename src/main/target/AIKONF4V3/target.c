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

#include "platform.h"

#include "drivers/io.h"
#include "drivers/dma.h"
#include "drivers/timer.h"
#include "drivers/timer_def.h"

timerHardware_t timerHardware[] = {
    // Motors 1-4 on TIM8
    DEF_TIM(TIM8, CH1, PC6,  TIM_USE_OUTPUT_AUTO, 0, 1),  // M1 - DMA2_ST2
    DEF_TIM(TIM8, CH2, PC7,  TIM_USE_OUTPUT_AUTO, 0, 1),  // M2 - DMA2_ST3
    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 1),  // M3 - DMA2_ST4
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M4 - DMA2_ST7

    // Motors 5-6 on TIM3 (DMA enabled for DSHOT support)
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M5 - DMA1_ST7
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M6 - DMA1_ST2

    // LED Strip on TIM4
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_LED, 0, 0),          // LED - DMA1_ST0
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
