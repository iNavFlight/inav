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

#pragma once

#include "drivers/dma.h"

// Macros expand to keep DMA descriptor table compatible with Betaflight
#define DEF_TIM_DMAMAP(variant, timch) CONCAT(DEF_TIM_DMAMAP__, PP_CALL(CONCAT(DEF_TIM_DMAMAP_VARIANT__, variant), CONCAT(DEF_TIM_DMA__, DEF_TIM_TCH2BTCH(timch)), DMA_VARIANT_MISSING, DMA_VARIANT_MISSING))
#define DEF_TIM_DMAMAP_VARIANT__0(_0, ...)         _0
#define DEF_TIM_DMAMAP_VARIANT__1(_0, _1, ...)     _1
#define DEF_TIM_DMAMAP_VARIANT__2(_0, _1, _2, ...) _2

// Timer channel indexes
#define DEF_TIM_CHNL_CH1    0
#define DEF_TIM_CHNL_CH1N   0
#define DEF_TIM_CHNL_CH2    1
#define DEF_TIM_CHNL_CH2N   1
#define DEF_TIM_CHNL_CH3    2
#define DEF_TIM_CHNL_CH3N   2
#define DEF_TIM_CHNL_CH4    3
#define DEF_TIM_CHNL_CH4N   3

// map to base channel (strip N from channel); works only when channel N exists
#define DEF_TIM_TCH2BTCH(timch) CONCAT(BTCH_, timch)
#define BTCH_TIM1_CH1N BTCH_TIM1_CH1
#define BTCH_TIM1_CH2N BTCH_TIM1_CH2
#define BTCH_TIM1_CH3N BTCH_TIM1_CH3

#define BTCH_TIM8_CH1N BTCH_TIM8_CH1
#define BTCH_TIM8_CH2N BTCH_TIM8_CH2
#define BTCH_TIM8_CH3N BTCH_TIM8_CH3

#define BTCH_TIM20_CH1N BTCH_TIM20_CH1
#define BTCH_TIM20_CH2N BTCH_TIM20_CH2
#define BTCH_TIM20_CH3N BTCH_TIM20_CH3

#define BTCH_TIM15_CH1N BTCH_TIM15_CH1
#define BTCH_TIM16_CH1N BTCH_TIM16_CH1

// Default output flags
#define DEF_TIM_OUTPUT(ch)                      DEF_TIM_OUTPUT__ ## ch
#define DEF_TIM_OUTPUT__CH1                     (TIMER_OUTPUT_NONE)
#define DEF_TIM_OUTPUT__CH2                     (TIMER_OUTPUT_NONE)
#define DEF_TIM_OUTPUT__CH3                     (TIMER_OUTPUT_NONE)
#define DEF_TIM_OUTPUT__CH4                     (TIMER_OUTPUT_NONE)
#define DEF_TIM_OUTPUT__CH1N                    (TIMER_OUTPUT_N_CHANNEL)
#define DEF_TIM_OUTPUT__CH2N                    (TIMER_OUTPUT_N_CHANNEL)
#define DEF_TIM_OUTPUT__CH3N                    (TIMER_OUTPUT_N_CHANNEL)
#define DEF_TIM_OUTPUT__CH4N                    (TIMER_OUTPUT_N_CHANNEL)

#if defined(STM32F3)
    #include "timer_def_stm32f3xx.h"
#elif defined(STM32F4)
    #include "timer_def_stm32f4xx.h"
#elif defined(STM32F7)
    #include "timer_def_stm32f7xx.h"
#else
    #error "Unknown CPU defined"
#endif

