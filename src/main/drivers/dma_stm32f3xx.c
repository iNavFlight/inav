/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <platform.h>

#include "build/debug.h"
#include "common/utils.h"
#include "drivers/nvic.h"
#include "drivers/dma.h"
#include "drivers/rcc.h"

/*
 * DMA descriptors.
 */
static dmaChannelDescriptor_t dmaDescriptors[] = {
    [0]  = DEFINE_DMA_CHANNEL(1, 1, 0),         // DMA1_CH1
    [1]  = DEFINE_DMA_CHANNEL(1, 2, 4),         // DMA1_CH2
    [2]  = DEFINE_DMA_CHANNEL(1, 3, 8),         // DMA1_CH3
    [3]  = DEFINE_DMA_CHANNEL(1, 4, 12),        // DMA1_CH4
    [4]  = DEFINE_DMA_CHANNEL(1, 5, 16),        // DMA1_CH5
    [5]  = DEFINE_DMA_CHANNEL(1, 6, 20),        // DMA1_CH6
    [6]  = DEFINE_DMA_CHANNEL(1, 7, 24),        // DMA1_CH7

    [7]  = DEFINE_DMA_CHANNEL(2, 1, 0),         // DMA2_CH1
    [8]  = DEFINE_DMA_CHANNEL(2, 2, 4),         // DMA2_CH2
    [9]  = DEFINE_DMA_CHANNEL(2, 3, 8),         // DMA2_CH3
    [10] = DEFINE_DMA_CHANNEL(2, 4, 12),        // DMA2_CH4
    [11] = DEFINE_DMA_CHANNEL(2, 5, 16),        // DMA2_CH5
};

/*
 * DMA IRQ Handlers
 */
DEFINE_DMA_IRQ_HANDLER(1, 1, 0)     // // DMA1_CH1 = dmaDescriptors[0]
DEFINE_DMA_IRQ_HANDLER(1, 2, 1)
DEFINE_DMA_IRQ_HANDLER(1, 3, 2)
DEFINE_DMA_IRQ_HANDLER(1, 4, 3)
DEFINE_DMA_IRQ_HANDLER(1, 5, 4)
DEFINE_DMA_IRQ_HANDLER(1, 6, 5)
DEFINE_DMA_IRQ_HANDLER(1, 7, 6)
DEFINE_DMA_IRQ_HANDLER(2, 1, 7)
DEFINE_DMA_IRQ_HANDLER(2, 2, 8)
DEFINE_DMA_IRQ_HANDLER(2, 3, 9)
DEFINE_DMA_IRQ_HANDLER(2, 4, 10)
DEFINE_DMA_IRQ_HANDLER(2, 5, 11)

DMA_t dmaGetByTag(dmaTag_t tag)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        // On F3 we match DMA and Channel, stream not used
        if (DMATAG_GET_DMA(dmaDescriptors[i].tag) == DMATAG_GET_DMA(tag) && DMATAG_GET_CHANNEL(dmaDescriptors[i].tag) == DMATAG_GET_CHANNEL(tag)) {
            return (DMA_t)&dmaDescriptors[i];
        }
    }

    return (DMA_t) NULL;
}

void dmaEnableClock(DMA_t dma)
{
    if (dma->dma == DMA1) {
        RCC_ClockCmd(RCC_AHB(DMA1), ENABLE);
    }
    else {
        RCC_ClockCmd(RCC_AHB(DMA2), ENABLE);
    }
}

resourceOwner_e dmaGetOwner(DMA_t dma)
{
    return dma->owner;
}

void dmaInit(DMA_t dma, resourceOwner_e owner, uint8_t resourceIndex)
{
    dmaEnableClock(dma);
    dma->owner = owner;
    dma->resourceIndex = resourceIndex;
}

void dmaSetHandler(DMA_t dma, dmaCallbackHandlerFuncPtr callback, uint32_t priority, uint32_t userParam)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    dmaEnableClock(dma);

    dma->irqHandlerCallback = callback;
    dma->userParam = userParam;

    NVIC_InitStructure.NVIC_IRQChannel = dma->irqNumber;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_BASE(priority);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_PRIORITY_SUB(priority);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

DMA_t dmaGetByRef(const DMA_Channel_TypeDef * ref)
{
    for (unsigned i = 0; i < (sizeof(dmaDescriptors) / sizeof(dmaDescriptors[0])); i++) {
        if (ref == dmaDescriptors[i].ref) {
            return &dmaDescriptors[i];
        }
    }

    return NULL;
}
