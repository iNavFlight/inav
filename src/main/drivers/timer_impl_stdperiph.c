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
#include <math.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/debug.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/dma.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

const uint16_t lookupDMASourceTable[4] = { TIM_DMA_CC1, TIM_DMA_CC2, TIM_DMA_CC3, TIM_DMA_CC4 };
const uint8_t lookupTIMChannelTable[4] = { TIM_Channel_1, TIM_Channel_2, TIM_Channel_3, TIM_Channel_4 };

void impl_timerInitContext(timHardwareContext_t * timCtx)
{
    (void)timCtx;   // NoOp
}

void impl_timerNVICConfigure(TCH_t * tch, int irqPriority)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_BASE(irqPriority);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_PRIORITY_SUB(irqPriority);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    if (tch->timCtx->timDef->irq) {
        NVIC_InitStructure.NVIC_IRQChannel = tch->timCtx->timDef->irq;
        NVIC_Init(&NVIC_InitStructure);
    }

    if (tch->timCtx->timDef->secondIrq) {
        NVIC_InitStructure.NVIC_IRQChannel = tch->timCtx->timDef->secondIrq;
        NVIC_Init(&NVIC_InitStructure);
    }
}

void impl_timerConfigBase(TCH_t * tch, uint16_t period, uint32_t hz)
{
    TIM_TypeDef * tim = tch->timCtx->timDef->tim;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = (period - 1) & 0xffff; // AKA TIMx_ARR
    TIM_TimeBaseStructure.TIM_Prescaler = lrintf((float)timerGetBaseClock(tch) / hz + 0.01f) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
}

void impl_enableTimer(TCH_t * tch)
{
    TIM_Cmd(tch->timHw->tim, ENABLE);
}

void impl_timerPWMStart(TCH_t * tch)
{
    TIM_CtrlPWMOutputs(tch->timHw->tim, ENABLE);
}

void impl_timerEnableIT(TCH_t * tch, uint32_t interrupt)
{
    TIM_ITConfig(tch->timHw->tim, interrupt, ENABLE);
}

void impl_timerDisableIT(TCH_t * tch, uint32_t interrupt)
{
    TIM_ITConfig(tch->timHw->tim, interrupt, DISABLE);
}

void impl_timerClearFlag(TCH_t * tch, uint32_t flag)
{
    TIM_ClearFlag(tch->timHw->tim, flag);
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
    TIM_ICInitTypeDef TIM_ICInitStructure;

    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = lookupTIMChannelTable[tch->timHw->channelIndex];
    TIM_ICInitStructure.TIM_ICPolarity = polarityRising ? TIM_ICPolarity_Rising : TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = getFilter(inputFilterTicks);

    TIM_ICInit(tch->timHw->tim, &TIM_ICInitStructure);
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
                case __builtin_clz(TIM_IT_Update): {
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

void impl_timerPWMConfigChannel(TCH_t * tch, uint16_t value)
{
    const bool inverted = tch->timHw->output & TIMER_OUTPUT_INVERTED;

    TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = value;

    if (tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
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

    switch (tch->timHw->channelIndex) {
        case 0:
            TIM_OC1Init(tch->timHw->tim, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 1:
            TIM_OC2Init(tch->timHw->tim, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 2:
            TIM_OC3Init(tch->timHw->tim, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 3:
            TIM_OC4Init(tch->timHw->tim, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(tch->timHw->tim, TIM_OCPreload_Enable);
            break;
    }
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

void impl_timerChCaptureCompareEnable(TCH_t * tch, bool enable)
{
    TIM_CCxCmd(tch->timHw->tim, lookupTIMChannelTable[tch->timHw->channelIndex], (enable ? TIM_CCx_Enable : TIM_CCx_Disable));
}

static void impl_timerDMA_IRQHandler(DMA_t descriptor)
{
    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        TCH_t * tch = (TCH_t *)descriptor->userParam;
        tch->dmaState = TCH_DMA_IDLE;

        DMA_Cmd(tch->dma->ref, DISABLE);
        TIM_DMACmd(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], DISABLE);

        DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
    }
}

bool impl_timerPWMConfigChannelDMA(TCH_t * tch, void * dmaBuffer, uint8_t dmaBufferElementSize, uint32_t dmaBufferElementCount)
{
    DMA_InitTypeDef DMA_InitStructure;
    TIM_TypeDef * timer = tch->timHw->tim;
    
    tch->dma = dmaGetByTag(tch->timHw->dmaTag);
    if (tch->dma == NULL) {
        return false;
    }

    // If DMA is already in use - abort
    if (tch->dma->owner != OWNER_FREE) {
        return false;
    }

    // We assume that timer channels are already initialized by calls to:
    //  timerConfigBase
    //  timerPWMConfigChannel

    TIM_CtrlPWMOutputs(timer, ENABLE);
    TIM_ARRPreloadConfig(timer, ENABLE);

    TIM_CCxCmd(timer, lookupTIMChannelTable[tch->timHw->channelIndex], TIM_CCx_Enable);
    TIM_Cmd(timer, ENABLE);

    dmaInit(tch->dma, OWNER_TIMER, 0);
    dmaSetHandler(tch->dma, impl_timerDMA_IRQHandler, NVIC_PRIO_WS2811_DMA, (uint32_t)tch);

    DMA_DeInit(tch->dma->ref);
    DMA_Cmd(tch->dma->ref, DISABLE);

    DMA_DeInit(tch->dma->ref);
    DMA_StructInit(&DMA_InitStructure);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)impl_timerCCR(tch);
    DMA_InitStructure.DMA_BufferSize = dmaBufferElementCount;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

    switch (dmaBufferElementSize) {
        case 1:
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
            break;
        case 2:
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
            break;
        case 4:
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
            break;
        default:
            // Programmer error
            while(1) {

            }
    }

#ifdef STM32F4
    DMA_InitStructure.DMA_Channel = dmaGetChannelByTag(tch->timHw->dmaTag);
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dmaBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
#else // F3
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dmaBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
#endif

    DMA_Init(tch->dma->ref, &DMA_InitStructure);
    DMA_ITConfig(tch->dma->ref, DMA_IT_TC, ENABLE);

    return true;
}

void impl_timerPWMPrepareDMA(TCH_t * tch, uint32_t dmaBufferElementCount)
{
    // Make sure we terminate any DMA transaction currently in progress
    // Clear the flag as well, so even if DMA transfer finishes while within ATOMIC_BLOCK
    // the resulting IRQ won't mess up the DMA state
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        DMA_Cmd(tch->dma->ref, DISABLE);
        TIM_DMACmd(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], DISABLE);
        DMA_CLEAR_FLAG(tch->dma, DMA_IT_TCIF);
    }

    DMA_SetCurrDataCounter(tch->dma->ref, dmaBufferElementCount);
    DMA_Cmd(tch->dma->ref, ENABLE);
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
        TIM_SetCounter(tch->timHw->tim, 0);
        TIM_DMACmd(tch->timHw->tim, dmaSources, ENABLE);
    }
}

void impl_timerPWMStopDMA(TCH_t * tch)
{
    DMA_Cmd(tch->dma->ref, DISABLE);
    TIM_DMACmd(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], DISABLE);
    TIM_Cmd(tch->timHw->tim, ENABLE);
}
