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

void impl_timerNVICConfigure(uint8_t irq, int irqPriority)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_BASE(irqPriority);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_PRIORITY_SUB(irqPriority);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void impl_timerConfigBase(TIM_TypeDef *tim, uint16_t period, uint8_t mhz)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = (period - 1) & 0xffff; // AKA TIMx_ARR
    TIM_TimeBaseStructure.TIM_Prescaler = timerGetPrescalerByDesiredMhz(tim, mhz);
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
}

void impl_enableTimer(TIM_TypeDef * tim)
{
    TIM_Cmd(tim, ENABLE);
}

void impl_timerPWMStart(TIM_TypeDef * tim, unsigned channel, bool isNChannel)
{
    UNUSED(channel);
    UNUSED(isNChannel);
    TIM_CtrlPWMOutputs(tim, ENABLE);
}

void impl_timerEnableIT(TIM_TypeDef * tim, uint32_t interrupt)
{
    TIM_ITConfig(tim, interrupt, ENABLE);
}

void impl_timerDisableIT(TIM_TypeDef * tim, uint32_t interrupt)
{
    TIM_ITConfig(tim, interrupt, DISABLE);
}

void impl_timerClearFlag(TIM_TypeDef * tim, uint32_t flag)
{
    TIM_ClearFlag(tim, flag);
}

// calculate input filter constant
static unsigned getFilter(unsigned ticks)
{
    static const unsigned ftab[16] = {
        1*1,                 // fDTS !
        1*2, 1*4, 1*8,       // fCK_INT
        2*6, 2*8,            // fDTS/2
        4*6, 4*8,
        8*6, 8*8,
        16*5, 16*6, 16*8,
        32*5, 32*6, 32*8
    };
    for (unsigned i = 1; i < ARRAYLEN(ftab); i++)
        if (ftab[i] > ticks)
            return i - 1;
    return 0x0f;
}

void impl_timerChConfigIC(const timerHardware_t *timHw, bool polarityRising, unsigned inputFilterTicks)
{
    TIM_ICInitTypeDef TIM_ICInitStructure;

    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = timHw->channel;
    TIM_ICInitStructure.TIM_ICPolarity = polarityRising ? TIM_ICPolarity_Rising : TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = getFilter(inputFilterTicks);

    TIM_ICInit(timHw->tim, &TIM_ICInitStructure);
}

void impl_timerCaptureCompareHandler(TIM_TypeDef *tim, timerConfig_t *timerConfig)
{
    unsigned tim_status = tim->SR & tim->DIER;

    while (tim_status) {
        // flags will be cleared by reading CCR in dual capture, make sure we call handler correctly
        // currrent order is highest bit first. Code should not rely on specific order (it will introduce race conditions anyway)
        unsigned bit = __builtin_clz(tim_status);
        unsigned mask = ~(0x80000000 >> bit);
        tim->SR = mask;
        tim_status &= mask;

        if (timerConfig) {
            switch (bit) {
                case __builtin_clz(TIM_IT_Update): {
                    const uint16_t capture = tim->ARR;
                    timerOvrHandlerRec_t *cb = timerConfig->overflowCallbackActive;
                    while (cb) {
                        cb->fn(cb, capture);
                        cb = cb->next;
                    }
                    break;
                }
                case __builtin_clz(TIM_IT_CC1):
                    timerConfig->edgeCallback[0]->fn(timerConfig->edgeCallback[0], tim->CCR1);
                    break;
                case __builtin_clz(TIM_IT_CC2):
                    timerConfig->edgeCallback[1]->fn(timerConfig->edgeCallback[1], tim->CCR2);
                    break;
                case __builtin_clz(TIM_IT_CC3):
                    timerConfig->edgeCallback[2]->fn(timerConfig->edgeCallback[2], tim->CCR3);
                    break;
                case __builtin_clz(TIM_IT_CC4):
                    timerConfig->edgeCallback[3]->fn(timerConfig->edgeCallback[3], tim->CCR4);
                    break;
            }
        }
        else {
            // timerConfig == NULL
            volatile uint32_t tmp;

            switch (bit) {
                case __builtin_clz(TIM_IT_Update):
                    tmp = tim->ARR;
                    break;
                case __builtin_clz(TIM_IT_CC1):
                    tmp = tim->CCR1;
                    break;
                case __builtin_clz(TIM_IT_CC2):
                    tmp = tim->CCR2;
                    break;
                case __builtin_clz(TIM_IT_CC3):
                    tmp = tim->CCR3;
                    break;
                case __builtin_clz(TIM_IT_CC4):
                    tmp = tim->CCR4;
                    break;
            }

            (void)tmp;
        }
    }
}

void impl_timerPWMConfigChannel(TIM_TypeDef * tim, uint8_t channel, bool isNChannel, bool inverted, uint16_t value)
{
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = value;

    if (isNChannel) {
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
        TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
        TIM_OCInitStructure.TIM_OCNPolarity = inverted ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    } else {
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
        TIM_OCInitStructure.TIM_OCPolarity = inverted ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    }

    switch (channel) {
        case TIM_Channel_1:
            TIM_OC1Init(tim, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_2:
            TIM_OC2Init(tim, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_3:
            TIM_OC3Init(tim, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_4:
            TIM_OC4Init(tim, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(tim, TIM_OCPreload_Enable);
            break;
    }
}

uint16_t impl_timerDmaSource(uint8_t channel)
{
    switch (channel) {
    case TIM_Channel_1:
        return TIM_DMA_CC1;
    case TIM_Channel_2:
        return TIM_DMA_CC2;
    case TIM_Channel_3:
        return TIM_DMA_CC3;
    case TIM_Channel_4:
        return TIM_DMA_CC4;
    }
    return 0;
}

volatile timCCR_t * impl_timerCCR(TIM_TypeDef *tim, uint8_t channel)
{
    switch (channel) {
        case TIM_Channel_1:
            return &tim->CCR1;
            break;
        case TIM_Channel_2:
            return &tim->CCR2;
            break;
        case TIM_Channel_3:
            return &tim->CCR3;
            break;
        case TIM_Channel_4:
            return &tim->CCR4;
            break;
    }
    return NULL;
}