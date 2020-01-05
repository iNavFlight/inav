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
#include <string.h>
#include <math.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/debug.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

extern uint32_t timerClock(TIM_TypeDef *tim);

const uint16_t lookupDMASourceTable[] = { TIM_DMA_CC1, TIM_DMA_CC2, TIM_DMA_CC3, TIM_DMA_CC4 };
const uint8_t lookupTIMChannelTable[] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

static const uint32_t lookupDMALLStreamTable[] = { LL_DMA_STREAM_0, LL_DMA_STREAM_1, LL_DMA_STREAM_2, LL_DMA_STREAM_3, LL_DMA_STREAM_4, LL_DMA_STREAM_5, LL_DMA_STREAM_6, LL_DMA_STREAM_7 };
static const uint32_t lookupDMALLChannelTable[] = { LL_DMA_CHANNEL_0, LL_DMA_CHANNEL_1, LL_DMA_CHANNEL_2, LL_DMA_CHANNEL_3, LL_DMA_CHANNEL_4, LL_DMA_CHANNEL_5, LL_DMA_CHANNEL_6, LL_DMA_CHANNEL_7 };

static TIM_HandleTypeDef timerHandle[HARDWARE_TIMER_DEFINITION_COUNT];

static TIM_HandleTypeDef * timerFindTimerHandle(TIM_TypeDef *tim)
{
    uint8_t timerIndex = lookupTimerIndex(tim);
    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return NULL;
    }

    return &timerHandle[timerIndex];
}

void impl_timerInitContext(timHardwareContext_t * timCtx)
{
    timCtx->timHandle = timerFindTimerHandle(timCtx->timDef->tim);
}

void impl_timerNVICConfigure(TCH_t * tch, int irqPriority)
{
    if (tch->timCtx->timDef->irq) {
        HAL_NVIC_SetPriority(tch->timCtx->timDef->irq, NVIC_PRIORITY_BASE(irqPriority), NVIC_PRIORITY_SUB(irqPriority));
        HAL_NVIC_EnableIRQ(tch->timCtx->timDef->irq);
    }

    if (tch->timCtx->timDef->secondIrq) {
        HAL_NVIC_SetPriority(tch->timCtx->timDef->secondIrq, NVIC_PRIORITY_BASE(irqPriority), NVIC_PRIORITY_SUB(irqPriority));
        HAL_NVIC_EnableIRQ(tch->timCtx->timDef->secondIrq);
    }
}

void impl_timerConfigBase(TCH_t * tch, uint16_t period, uint32_t hz)
{
    // Get and verify HAL TIM_Handle object 
    TIM_HandleTypeDef * timHandle = tch->timCtx->timHandle;
    TIM_TypeDef * timer = tch->timCtx->timDef->tim;

    if (timHandle->Instance == timer) {
        return;
    }

    timHandle->Instance = timer;
    timHandle->Init.Prescaler = lrintf((float)timerGetBaseClock(tch) / hz + 0.01f) - 1;
    timHandle->Init.Period = (period - 1) & 0xffff; // AKA TIMx_ARR
    timHandle->Init.RepetitionCounter = 0;
    timHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timHandle->Init.CounterMode = TIM_COUNTERMODE_UP;
    timHandle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    HAL_TIM_Base_Init(timHandle);
    if (timer == TIM1 || timer == TIM2 || timer == TIM3 || timer == TIM4 || timer == TIM5 || timer == TIM8 || timer == TIM9) {
        TIM_ClockConfigTypeDef sClockSourceConfig;
        memset(&sClockSourceConfig, 0, sizeof(sClockSourceConfig));
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(timHandle, &sClockSourceConfig) != HAL_OK) {
            return;
        }
    }

    if (timer == TIM1 || timer == TIM2 || timer == TIM3 || timer == TIM4 || timer == TIM5 || timer == TIM8) {
        TIM_MasterConfigTypeDef sMasterConfig;
        memset(&sMasterConfig, 0, sizeof(sMasterConfig));
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(timHandle, &sMasterConfig) != HAL_OK) {
            return;
        }
    }
}

void impl_timerPWMConfigChannel(TCH_t * tch, uint16_t value)
{
    const bool inverted = tch->timHw->output & TIMER_OUTPUT_INVERTED;

    TIM_OC_InitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OCIdleState = TIM_OCIDLESTATE_SET;
    TIM_OCInitStructure.OCPolarity = inverted ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
    TIM_OCInitStructure.OCNIdleState = TIM_OCNIDLESTATE_SET;
    TIM_OCInitStructure.OCNPolarity = inverted ? TIM_OCNPOLARITY_LOW : TIM_OCNPOLARITY_HIGH;
    TIM_OCInitStructure.Pulse = value;
    TIM_OCInitStructure.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(tch->timCtx->timHandle, &TIM_OCInitStructure, lookupTIMChannelTable[tch->timHw->channelIndex]);
}

volatile timCCR_t * impl_timerCCR(TCH_t * tch)
{
    switch (tch->timHw->channelIndex) {
        case 0:
            return &tch->timHw->tim->CCR1;
            break;
        case 1:
            return &tch->timHw->tim->CCR2;
            break;
        case 2:
            return &tch->timHw->tim->CCR3;
            break;
        case 3:
            return &tch->timHw->tim->CCR4;
            break;
    }
    return NULL;
}

void impl_enableTimer(TCH_t * tch)
{
    HAL_TIM_Base_Start(tch->timCtx->timHandle);
}

void impl_timerPWMStart(TCH_t * tch)
{
    if (tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        HAL_TIMEx_PWMN_Start(tch->timCtx->timHandle, lookupTIMChannelTable[tch->timHw->channelIndex]);
    }
    else {
        HAL_TIM_PWM_Start(tch->timCtx->timHandle, lookupTIMChannelTable[tch->timHw->channelIndex]);
    }
}

void impl_timerEnableIT(TCH_t * tch, uint32_t interrupt)
{
    __HAL_TIM_ENABLE_IT(tch->timCtx->timHandle, interrupt);
}

void impl_timerDisableIT(TCH_t * tch, uint32_t interrupt)
{
    __HAL_TIM_DISABLE_IT(tch->timCtx->timHandle, interrupt);
}

void impl_timerClearFlag(TCH_t * tch, uint32_t flag)
{
    __HAL_TIM_CLEAR_FLAG(tch->timCtx->timHandle, flag);
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

    for (unsigned i = 1; i < ARRAYLEN(ftab); i++) {
        if (ftab[i] > ticks) {
            return i - 1;
        }
    }

    return 0x0f;
}

void impl_timerChConfigIC(TCH_t * tch, bool polarityRising, unsigned inputFilterTicks)
{
    TIM_IC_InitTypeDef TIM_ICInitStructure;

    TIM_ICInitStructure.ICPolarity = polarityRising ? TIM_ICPOLARITY_RISING : TIM_ICPOLARITY_FALLING;
    TIM_ICInitStructure.ICSelection = TIM_ICSELECTION_DIRECTTI;
    TIM_ICInitStructure.ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.ICFilter = getFilter(inputFilterTicks);
    HAL_TIM_IC_ConfigChannel(tch->timCtx->timHandle, &TIM_ICInitStructure, lookupTIMChannelTable[tch->timHw->channelIndex]);
}

void impl_timerCaptureCompareHandler(TIM_TypeDef *tim, timHardwareContext_t *timerCtx)
{
    unsigned tim_status = tim->SR & tim->DIER;

    while (tim_status) {
        // flags will be cleared by reading CCR in dual capture, make sure we call handler correctly
        // currrent order is highest bit first. Code should not rely on specific order (it will introduce race conditions anyway)
        unsigned bit = __builtin_clz(tim_status);
        unsigned mask = ~(0x80000000 >> bit);
        tim->SR = mask;
        tim_status &= mask;

        if (timerCtx) {
            switch (bit) {
                case __builtin_clz(TIM_IT_UPDATE): {
                    const uint16_t capture = tim->ARR;
                    if (timerCtx->ch[0].cb && timerCtx->ch[0].cb->callbackOvr) {
                        timerCtx->ch[0].cb->callbackOvr(&timerCtx->ch[0], capture);
                    }
                    if (timerCtx->ch[1].cb && timerCtx->ch[1].cb->callbackOvr) {
                        timerCtx->ch[1].cb->callbackOvr(&timerCtx->ch[1], capture);
                    }
                    if (timerCtx->ch[2].cb && timerCtx->ch[2].cb->callbackOvr) {
                        timerCtx->ch[2].cb->callbackOvr(&timerCtx->ch[2], capture);
                    }
                    if (timerCtx->ch[3].cb && timerCtx->ch[3].cb->callbackOvr) {
                        timerCtx->ch[3].cb->callbackOvr(&timerCtx->ch[3], capture);
                    }
                    break;
                }
                case __builtin_clz(TIM_IT_CC1):
                    timerCtx->ch[0].cb->callbackEdge(&timerCtx->ch[0], tim->CCR1);
                    break;
                case __builtin_clz(TIM_IT_CC2):
                    timerCtx->ch[1].cb->callbackEdge(&timerCtx->ch[1], tim->CCR2);
                    break;
                case __builtin_clz(TIM_IT_CC3):
                    timerCtx->ch[2].cb->callbackEdge(&timerCtx->ch[2], tim->CCR3);
                    break;
                case __builtin_clz(TIM_IT_CC4):
                    timerCtx->ch[3].cb->callbackEdge(&timerCtx->ch[3], tim->CCR4);
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

void impl_timerChCaptureCompareEnable(TCH_t * tch, bool enable)
{
    static const uint32_t lookupTIMLLChannelTable[] = { LL_TIM_CHANNEL_CH1, LL_TIM_CHANNEL_CH2, LL_TIM_CHANNEL_CH3, LL_TIM_CHANNEL_CH4 };

    if (enable) {
        LL_TIM_CC_EnableChannel(tch->timHw->tim, lookupTIMLLChannelTable[tch->timHw->channelIndex]);
    }
    else {
        LL_TIM_CC_DisableChannel(tch->timHw->tim, lookupTIMLLChannelTable[tch->timHw->channelIndex]);
    }
}

// HAL_LL additionan implementation for enabling multiple DMA channels in one operation
static inline void LL_TIM_EnableDMAReq_CCx(TIM_TypeDef * TIMx, uint16_t dmaSources)
{
    SET_BIT(TIMx->DIER, dmaSources & (TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3 | TIM_DMA_CC4));
}

static inline void LL_TIM_DisableDMAReq_CCx(TIM_TypeDef * TIMx, uint16_t dmaSources)
{
    CLEAR_BIT(TIMx->DIER, dmaSources & (TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3 | TIM_DMA_CC4));
}

static void impl_timerDMA_IRQHandler(DMA_t descriptor)
{
    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        TCH_t * tch = (TCH_t *)descriptor->userParam;

        // If it was ACTIVE - switch to IDLE
        if (tch->dmaState == TCH_DMA_ACTIVE) {
            tch->dmaState = TCH_DMA_IDLE;
        }

        LL_DMA_DisableStream(tch->dma->dma, lookupDMALLStreamTable[DMATAG_GET_STREAM(tch->timHw->dmaTag)]);
        LL_TIM_DisableDMAReq_CCx(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex]);

        DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
    }
}

bool impl_timerPWMConfigChannelDMA(TCH_t * tch, void * dmaBuffer, uint8_t dmaBufferElementSize, uint32_t dmaBufferElementCount)
{
    tch->dma = dmaGetByTag(tch->timHw->dmaTag);
    tch->dmaBuffer = dmaBuffer;
    if (tch->dma == NULL) {
        return false;
    }

    // If DMA is already in use - abort
    if (dmaGetOwner(tch->dma) != OWNER_FREE) {
        return false;
    }

    // We assume that timer channels are already initialized by calls to:
    //  timerConfigBase
    //  timerPWMConfigChannel
    const uint32_t streamLL = lookupDMALLStreamTable[DMATAG_GET_STREAM(tch->timHw->dmaTag)];
    const uint32_t channelLL = lookupDMALLChannelTable[DMATAG_GET_CHANNEL(tch->timHw->dmaTag)];

    LL_DMA_DeInit(tch->dma->dma, streamLL);

    LL_DMA_InitTypeDef init;
    LL_DMA_StructInit(&init);

    init.Channel = channelLL;
    init.PeriphOrM2MSrcAddress = (uint32_t)impl_timerCCR(tch);
    init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;

    switch (dmaBufferElementSize) {
        case 1:
            init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
            init.PeriphOrM2MSrcDataSize = LL_DMA_MDATAALIGN_BYTE;
            break;
        case 2:
            init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
            init.PeriphOrM2MSrcDataSize = LL_DMA_MDATAALIGN_HALFWORD;
            break;
        case 4:
            init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
            init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
            break;
        default:
            // Programmer error
            while(1) {

            }
    }

    init.MemoryOrM2MDstAddress = (uint32_t)dmaBuffer;
    init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    init.NbData = dmaBufferElementCount;
    init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    init.Mode = LL_DMA_MODE_NORMAL;
    init.Priority = LL_DMA_PRIORITY_HIGH;
    init.FIFOMode = LL_DMA_FIFOMODE_ENABLE;
    init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_FULL;
    init.MemBurst = LL_DMA_MBURST_SINGLE;
    init.PeriphBurst = LL_DMA_PBURST_SINGLE;

    dmaInit(tch->dma, OWNER_TIMER, 0);
    dmaSetHandler(tch->dma, impl_timerDMA_IRQHandler, NVIC_PRIO_WS2811_DMA, (uint32_t)tch);

    LL_DMA_Init(tch->dma->dma, streamLL, &init);

    // Start PWM generation
    if (tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        HAL_TIMEx_PWMN_Start(tch->timCtx->timHandle, lookupTIMChannelTable[tch->timHw->channelIndex]);
    }
    else {
        HAL_TIM_PWM_Start(tch->timCtx->timHandle, lookupTIMChannelTable[tch->timHw->channelIndex]);
    }

    return true;
}

void impl_timerPWMPrepareDMA(TCH_t * tch, uint32_t dmaBufferElementCount)
{
    const uint32_t streamLL = lookupDMALLStreamTable[DMATAG_GET_STREAM(tch->timHw->dmaTag)];
    DMA_TypeDef *dmaBase = tch->dma->dma;

    // Make sure we terminate any DMA transaction currently in progress
    // Clear the flag as well, so even if DMA transfer finishes while within ATOMIC_BLOCK
    // the resulting IRQ won't mess up the DMA state
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        LL_TIM_DisableDMAReq_CCx(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex]);
        LL_DMA_DisableStream(dmaBase, streamLL);
        DMA_CLEAR_FLAG(tch->dma, DMA_IT_TCIF);
    }

    LL_DMA_SetDataLength(dmaBase, streamLL, dmaBufferElementCount);
    LL_DMA_ConfigAddresses(dmaBase, streamLL, (uint32_t)tch->dmaBuffer, (uint32_t)impl_timerCCR(tch), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_EnableIT_TC(dmaBase, streamLL);
    LL_DMA_EnableStream(dmaBase, streamLL);
    tch->dmaState = TCH_DMA_READY;
}

void impl_timerPWMStartDMA(TCH_t * tch)
{
    uint16_t dmaSources = 0;
    timHardwareContext_t * timCtx = tch->timCtx;

    if (timCtx->ch[0].dmaState == TCH_DMA_READY) {
        timCtx->ch[0].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TIM_DMA_CC1;
    }

    if (timCtx->ch[1].dmaState == TCH_DMA_READY) {
        timCtx->ch[1].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TIM_DMA_CC2;
    }

    if (timCtx->ch[2].dmaState == TCH_DMA_READY) {
        timCtx->ch[2].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TIM_DMA_CC3;
    }

    if (timCtx->ch[3].dmaState == TCH_DMA_READY) {
        timCtx->ch[3].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TIM_DMA_CC4;
    }

    if (dmaSources) {
        LL_TIM_SetCounter(timCtx->timDef->tim, 0);
        LL_TIM_EnableDMAReq_CCx(timCtx->timDef->tim, dmaSources);
    }
}

void impl_timerPWMStopDMA(TCH_t * tch)
{
    (void)tch;
    // FIXME
}
