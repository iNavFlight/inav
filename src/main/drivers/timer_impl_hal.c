/*
  modified version of StdPeriph function is located here.
  TODO - what license does apply here?
  original file was lincesed under MCD-ST Liberty SW License Agreement V2
  http://www.st.com/software_license_agreement_liberty_v2
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

static TIM_HandleTypeDef timerHandle[HARDWARE_TIMER_DEFINITION_COUNT];

TIM_HandleTypeDef * timerFindTimerHandle(TIM_TypeDef *tim)
{
    uint8_t timerIndex = lookupTimerIndex(tim);
    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return NULL;
    }

    return &timerHandle[timerIndex];
}

void impl_timerNVICConfigure(uint8_t irq, int irqPriority)
{
    HAL_NVIC_SetPriority(irq, NVIC_PRIORITY_BASE(irqPriority), NVIC_PRIORITY_SUB(irqPriority));
    HAL_NVIC_EnableIRQ(irq);
}

void impl_timerConfigBase(TIM_TypeDef *tim, uint16_t period, uint8_t mhz)
{
    uint8_t timerIndex = lookupTimerIndex(tim);

    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return;
    }

    if (timerHandle[timerIndex].Instance == tim)
    {
        // already configured
        return;
    }

    timerHandle[timerIndex].Instance = tim;
    timerHandle[timerIndex].Init.Period = (period - 1) & 0xffff; // AKA TIMx_ARR
    timerHandle[timerIndex].Init.Prescaler = (timerClock(tim) / ((uint32_t)mhz * 1000000)) - 1;
    timerHandle[timerIndex].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timerHandle[timerIndex].Init.CounterMode = TIM_COUNTERMODE_UP;
    timerHandle[timerIndex].Init.RepetitionCounter = 0x0000;

    HAL_TIM_Base_Init(&timerHandle[timerIndex]);
    if (tim == TIM1 || tim == TIM2 || tim == TIM3 || tim == TIM4 || tim == TIM5 || tim == TIM8 || tim == TIM9)
    {
        TIM_ClockConfigTypeDef sClockSourceConfig;
        memset(&sClockSourceConfig, 0, sizeof(sClockSourceConfig));
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&timerHandle[timerIndex], &sClockSourceConfig) != HAL_OK) {
            return;
        }
    }
    if (tim == TIM1 || tim == TIM2 || tim == TIM3 || tim == TIM4 || tim == TIM5 || tim == TIM8 )
    {
        TIM_MasterConfigTypeDef sMasterConfig;
        memset(&sMasterConfig, 0, sizeof(sMasterConfig));
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&timerHandle[timerIndex], &sMasterConfig) != HAL_OK) {
            return;
        }
    }
}

void impl_enableTimer(TIM_TypeDef * tim)
{
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    HAL_TIM_Base_Start(Handle);
}

void impl_timerPWMStart(TIM_TypeDef * tim, unsigned channel, bool isNChannel)
{
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    if (isNChannel)
        HAL_TIMEx_PWMN_Start(Handle, channel);
    else
        HAL_TIM_PWM_Start(Handle, channel);
}

void impl_timerEnableIT(TIM_TypeDef * tim, uint32_t interrupt)
{
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    __HAL_TIM_ENABLE_IT(Handle, interrupt);
}

void impl_timerDisableIT(TIM_TypeDef * tim, uint32_t interrupt)
{
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    __HAL_TIM_DISABLE_IT(Handle, interrupt);
}

void impl_timerClearFlag(TIM_TypeDef * tim, uint32_t flag)
{
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    __HAL_TIM_CLEAR_FLAG(Handle, flag);
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
    TIM_HandleTypeDef * Handle = timerFindTimerHandle(timHw->tim);
    if (Handle == NULL) {
        return;
    }

    TIM_IC_InitTypeDef TIM_ICInitStructure;

    TIM_ICInitStructure.ICPolarity = polarityRising ? TIM_ICPOLARITY_RISING : TIM_ICPOLARITY_FALLING;
    TIM_ICInitStructure.ICSelection = TIM_ICSELECTION_DIRECTTI;
    TIM_ICInitStructure.ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.ICFilter = getFilter(inputFilterTicks);
    HAL_TIM_IC_ConfigChannel(Handle,&TIM_ICInitStructure, timHw->channel);
}

void impl_timerCaptureCompareHandler(TIM_TypeDef *tim, timerConfig_t * timerConfig)
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
                case __builtin_clz(TIM_IT_UPDATE): {
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
                case __builtin_clz(TIM_IT_UPDATE):
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
    UNUSED(isNChannel);

    TIM_HandleTypeDef * Handle = timerFindTimerHandle(tim);
    if (Handle == NULL) {
        return;
    }

    TIM_OC_InitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OCIdleState = TIM_OCIDLESTATE_SET;
    TIM_OCInitStructure.OCPolarity = inverted ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
    TIM_OCInitStructure.OCNIdleState = TIM_OCNIDLESTATE_SET;
    TIM_OCInitStructure.OCNPolarity = inverted ? TIM_OCNPOLARITY_LOW : TIM_OCNPOLARITY_HIGH;
    TIM_OCInitStructure.Pulse = value;
    TIM_OCInitStructure.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(Handle, &TIM_OCInitStructure, channel);
}

uint16_t impl_timerDmaSource(uint8_t channel)
{
    switch (channel) {
    case TIM_CHANNEL_1:
        return TIM_DMA_ID_CC1;
    case TIM_CHANNEL_2:
        return TIM_DMA_ID_CC2;
    case TIM_CHANNEL_3:
        return TIM_DMA_ID_CC3;
    case TIM_CHANNEL_4:
        return TIM_DMA_ID_CC4;
    }
    return 0;
}

volatile timCCR_t * impl_timerCCR(TIM_TypeDef *tim, uint8_t channel)
{
    switch (channel) {
        case TIM_CHANNEL_1:
            return &tim->CCR1;
            break;
        case TIM_CHANNEL_2:
            return &tim->CCR2;
            break;
        case TIM_CHANNEL_3:
            return &tim->CCR3;
            break;
        case TIM_CHANNEL_4:
            return &tim->CCR4;
            break;
    }
    return NULL;
}