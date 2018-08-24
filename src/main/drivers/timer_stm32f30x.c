/*
  modified version of StdPeriph function is located here.
  TODO - what license does apply here?
  original file was lincesed under MCD-ST Liberty SW License Agreement V2
  http://www.st.com/software_license_agreement_liberty_v2
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

#include "stm32f30x.h"

const timerDef_t timerDefinitions[HARDWARE_TIMER_DEFINITION_COUNT] = {
    [0] = { .tim = TIM1,  .rcc = RCC_APB2(TIM1),  .irq = TIM1_CC_IRQn, .secondIrq = TIM1_UP_TIM16_IRQn },
    [1] = { .tim = TIM2,  .rcc = RCC_APB1(TIM2),  .irq = TIM2_IRQn },
    [2] = { .tim = TIM3,  .rcc = RCC_APB1(TIM3),  .irq = TIM3_IRQn },
    [3] = { .tim = TIM4,  .rcc = RCC_APB1(TIM4),  .irq = TIM4_IRQn },

    [5] = { .tim = TIM6,  .rcc = RCC_APB1(TIM6),  .irq = 0 },
    [6] = { .tim = TIM7,  .rcc = RCC_APB1(TIM7),  .irq = 0 },
    [7] = { .tim = TIM8,  .rcc = RCC_APB2(TIM8),  .irq = TIM8_CC_IRQn },

    [14] = { .tim = TIM15, .rcc = RCC_APB2(TIM15), .irq = TIM1_BRK_TIM15_IRQn },
    [15] = { .tim = TIM16, .rcc = RCC_APB2(TIM16), .irq = TIM1_UP_TIM16_IRQn },
    [16] = { .tim = TIM17, .rcc = RCC_APB2(TIM17), .irq = TIM1_TRG_COM_TIM17_IRQn },
};

uint8_t timerClockDivisor(TIM_TypeDef *tim)
{
    UNUSED(tim);
    return 1;
}

_TIM_IRQ_HANDLER(TIM1_CC_IRQHandler, 1);
_TIM_IRQ_HANDLER2(TIM1_UP_TIM16_IRQHandler, 1, 16);
_TIM_IRQ_HANDLER(TIM2_IRQHandler, 2);
_TIM_IRQ_HANDLER(TIM3_IRQHandler, 3);
_TIM_IRQ_HANDLER(TIM4_IRQHandler, 4);
_TIM_IRQ_HANDLER(TIM8_CC_IRQHandler, 8);
_TIM_IRQ_HANDLER(TIM8_UP_IRQHandler, 8);
_TIM_IRQ_HANDLER(TIM1_BRK_TIM15_IRQHandler, 15);
_TIM_IRQ_HANDLER(TIM1_TRG_COM_TIM17_IRQHandler, 17);
