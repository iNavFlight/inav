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
#include <stdint.h>
#include <string.h>

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
    [0]  = DEFINE_DMA_CHANNEL(1, 1, 0),     // DMA1_ST1
    [1]  = DEFINE_DMA_CHANNEL(1, 2, 4),     // DMA1_ST2
    [2]  = DEFINE_DMA_CHANNEL(1, 3, 8),    // DMA1_ST3
    [3]  = DEFINE_DMA_CHANNEL(1, 4, 12),    // DMA1_ST4
    [4]  = DEFINE_DMA_CHANNEL(1, 5, 16),    // DMA1_ST5
    [5]  = DEFINE_DMA_CHANNEL(1, 6, 20),    // DMA1_ST6
    [6]  = DEFINE_DMA_CHANNEL(1, 7, 24),    // DMA1_ST7

    [7]  = DEFINE_DMA_CHANNEL(2, 1, 0),     // DMA2_ST1
    [8] =  DEFINE_DMA_CHANNEL(2, 2, 4),    // DMA2_ST2
    [9] =  DEFINE_DMA_CHANNEL(2, 3, 8),    // DMA2_ST3
    [10] = DEFINE_DMA_CHANNEL(2, 4, 12),    // DMA2_ST4
    [11] = DEFINE_DMA_CHANNEL(2, 5, 16),    // DMA2_ST5
    [12] = DEFINE_DMA_CHANNEL(2, 6, 20),    // DMA2_ST6
    [13] = DEFINE_DMA_CHANNEL(2, 7, 24)     // DMA2_ST7
    /*
    //EDMA
    [14] = DEFINE_EDMA_CHANNEL(0, 1, 6),     // EDMA_1
    [15] = DEFINE_EDMA_CHANNEL(0, 2, 12),    // EDMA_2
    [16] = DEFINE_EDMA_CHANNEL(0, 3, 18),    // EDMA_3
    [17] = DEFINE_EDMA_CHANNEL(0, 4, 24),    // EDMA_4
    [18] = DEFINE_EDMA_CHANNEL(0, 5, 30),    // EDMA_5
    [19] = DEFINE_EDMA_CHANNEL(0, 6, 36),    // EDMA_6
    [20] = DEFINE_EDMA_CHANNEL(0, 7, 42)     // EDMA_7
    [21] = DEFINE_EDMA_CHANNEL(0, 8, 48)     // EDMA_8
    */
};

/*
 * DMA IRQ Handlers
 */

DEFINE_DMA_IRQ_HANDLER(1, 1, 0)
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
DEFINE_DMA_IRQ_HANDLER(2, 6, 12)
DEFINE_DMA_IRQ_HANDLER(2, 7, 13)
/*
// edma
DEFINE_EDMA_IRQ_HANDLER(0, 0, 16)
DEFINE_EDMA_IRQ_HANDLER(0, 1, 17)
DEFINE_EDMA_IRQ_HANDLER(0, 2, 18)
DEFINE_EDMA_IRQ_HANDLER(0, 3, 19)
DEFINE_EDMA_IRQ_HANDLER(0, 4, 20)
DEFINE_EDMA_IRQ_HANDLER(0, 5, 21)
DEFINE_EDMA_IRQ_HANDLER(0, 6, 22)
DEFINE_EDMA_IRQ_HANDLER(0, 7, 23)
DEFINE_EDMA_IRQ_HANDLER(0, 8, 24) 
*/
// Obtain DMA_t through DMA ID & DMA channel
DMA_t dmaGetByTag(dmaTag_t tag)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        // On F4/F7 we match only DMA and Stream. Channel is needed when connecting DMA to peripheral
        if (DMATAG_GET_DMA(dmaDescriptors[i].tag) == DMATAG_GET_DMA(tag) && DMATAG_GET_STREAM(dmaDescriptors[i].tag) == DMATAG_GET_STREAM(tag)) {
            return (DMA_t)&dmaDescriptors[i];
        }
    }

    return (DMA_t) NULL;
}

void dmaEnableClock(DMA_t dma)
{
    if (dma->dma == DMA1) {
        RCC_ClockCmd(RCC_AHB1(DMA1), ENABLE);
    }
    else {
        RCC_ClockCmd(RCC_AHB1(DMA2), ENABLE);
    }
    dmamux_enable(dma->dma,TRUE);
}

void dmaMuxEnable(DMA_t dma, uint32_t dmaMuxid)
{ 
	dmamux_init(dma->dmaMuxref, dmaMuxid);
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
    dmaEnableClock(dma);

    dma->irqHandlerCallback = callback;
    dma->userParam = userParam;
	nvic_irq_enable(dma->irqNumber, priority,0);  
     
}
// This function is not used , use ChannelByTag instead
uint32_t dmaGetChannelByTag(dmaTag_t tag)
{
    static const dma_channel_type * dmaChannel[14] = { DMA1_CHANNEL1, DMA1_CHANNEL2, DMA1_CHANNEL3, DMA1_CHANNEL4, DMA1_CHANNEL5, DMA1_CHANNEL6, DMA1_CHANNEL7, 
    DMA2_CHANNEL1, DMA2_CHANNEL2, DMA2_CHANNEL3, DMA2_CHANNEL4, DMA2_CHANNEL5, DMA2_CHANNEL6, DMA2_CHANNEL7,
     };

    return (uint32_t) dmaChannel[(DMATAG_GET_DMA(tag)-1)*7 + DMATAG_GET_STREAM(tag)-1];
}

// Obtain DMA_t through DMA channel
DMA_t dmaGetByRef(const dma_channel_type* ref)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        if (ref == dmaDescriptors[i].ref) {
            return &dmaDescriptors[i];
        }
    }
    return NULL;
}

/*
DMA_t dmaGetByRef(const edma_stream_type* ref)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        if (ref == dmaDescriptors[i].sref) {
            return &dmaDescriptors[i];
        }
    }
    return NULL;
}
*/