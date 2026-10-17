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
#include <stdint.h>

#include "platform.h"

#include "build/atomic.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"
#include "at32f435_437.h"
 
    const timerDef_t timerDefinitions[HARDWARE_TIMER_DEFINITION_COUNT] = {
    #if defined(TMR1)
        [TIMER_INDEX(1)] = { .tim = TMR1,  .rcc = RCC_APB2(TMR1),  .irq = TMR1_CH_IRQn, .secondIrq = TMR1_OVF_TMR10_IRQn },
    #endif
 
    #if defined(TMR2)
        [TIMER_INDEX(2)] = { .tim = TMR2,  .rcc = RCC_APB1(TMR2),  .irq = TMR2_GLOBAL_IRQn},
    #endif

    #if defined(TMR3)
        [TIMER_INDEX(3)] = { .tim = TMR3,  .rcc = RCC_APB1(TMR3),  .irq = TMR3_GLOBAL_IRQn},
    #endif

    #if defined(TMR4)
        [TIMER_INDEX(4)] = { .tim = TMR4,  .rcc = RCC_APB1(TMR4),  .irq = TMR4_GLOBAL_IRQn},
    #endif

    #if defined(TMR5)
        [TIMER_INDEX(5)] = { .tim = TMR5,  .rcc = RCC_APB1(TMR5),  .irq = TMR5_GLOBAL_IRQn},
    #endif

    #if defined(TMR6)
        [TIMER_INDEX(6)] = { .tim = TMR6,  .rcc = RCC_APB1(TMR6),  .irq = 0},
    #endif

    #if defined(TMR7)
        [TIMER_INDEX(7)] = { .tim = TMR7,  .rcc = RCC_APB1(TMR7),  .irq = 0},
    #endif

    #if defined(TMR8)   
        [TIMER_INDEX(8)] = { .tim = TMR8,  .rcc = RCC_APB2(TMR8),  .irq = TMR8_CH_IRQn, .secondIrq = TMR8_OVF_TMR13_IRQn },
    #endif

    #if defined(TMR9)
        [TIMER_INDEX(9)] = { .tim = TMR9,  .rcc = RCC_APB2(TMR9),  .irq = TMR1_BRK_TMR9_IRQn},
    #endif

    #if defined(TMR10)
        [TIMER_INDEX(10)] = { .tim = TMR10, .rcc = RCC_APB2(TMR10), .irq = TMR1_OVF_TMR10_IRQn},
    #endif

    #if defined(TMR11)
        [TIMER_INDEX(11)] = { .tim = TMR11, .rcc = RCC_APB2(TMR11), .irq = TMR1_TRG_HALL_TMR11_IRQn},
    #endif

    #if defined(TMR12) 
        [TIMER_INDEX(12)] = { .tim = TMR12, .rcc = RCC_APB1(TMR12), .irq = TMR8_BRK_TMR12_IRQn},
    #endif

    #if defined(TMR13)
        [TIMER_INDEX(13)] = { .tim = TMR13, .rcc = RCC_APB1(TMR13), .irq = TMR8_OVF_TMR13_IRQn},
    #endif

    #if defined(TMR14)
        [TIMER_INDEX(14)] = { .tim = TMR14, .rcc = RCC_APB1(TMR14), .irq = TMR8_TRG_HALL_TMR14_IRQn},
    #endif

    #if defined(TMR20)
         [TIMER_INDEX(20)] = { .tim = TMR20, .rcc = RCC_APB2(TMR20), .irq = TMR20_CH_IRQn},
     #endif
    }; 

// The timer table must cover every slot of timerCtx[] - see TIMER_INDEX() in timer.h
STATIC_ASSERT(TIMER_INDEX(20) == HARDWARE_TIMER_DEFINITION_COUNT - 1, timer_definition_table_size_mismatch);

uint32_t timerClock(tmr_type *tim)
{
    UNUSED(tim);
    return SystemCoreClock;
}

// Timer IRQ handlers
_TIM_IRQ_HANDLER(TMR1_CH_IRQHandler, 1);
_TIM_IRQ_HANDLER2(TMR1_BRK_TMR9_IRQHandler, 1, 9);
_TIM_IRQ_HANDLER2(TMR1_OVF_TMR10_IRQHandler, 1, 10);
_TIM_IRQ_HANDLER2(TMR1_TRG_HALL_TMR11_IRQHandler, 1, 11);
_TIM_IRQ_HANDLER(TMR2_GLOBAL_IRQHandler, 2);
_TIM_IRQ_HANDLER(TMR3_GLOBAL_IRQHandler, 3);
_TIM_IRQ_HANDLER(TMR4_GLOBAL_IRQHandler, 4);
_TIM_IRQ_HANDLER(TMR5_GLOBAL_IRQHandler, 5);
_TIM_IRQ_HANDLER(TMR8_CH_IRQHandler, 8);
//_TIM_IRQ_HANDLER(TIM8_UP_IRQHandler, 8);  
_TIM_IRQ_HANDLER2(TMR8_BRK_TMR12_IRQHandler, 8, 12);
_TIM_IRQ_HANDLER2(TMR8_OVF_TMR13_IRQHandler, 8, 13);
_TIM_IRQ_HANDLER2(TMR8_TRG_HALL_TMR14_IRQHandler, 8, 14);
_TIM_IRQ_HANDLER(TMR20_CH_IRQHandler, 20);
