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

const uint16_t lookupDMASourceTable[4] = { TMR_C1_DMA_REQUEST, TMR_C2_DMA_REQUEST, TMR_C3_DMA_REQUEST, TMR_C4_DMA_REQUEST };

const uint8_t lookupTIMChannelTable[4] = { TMR_C1_FLAG, TMR_C2_FLAG, TMR_C3_FLAG, TMR_C4_FLAG };

void impl_timerInitContext(timHardwareContext_t * timCtx)
{
    (void)timCtx;   // NoOp
}
// Configure the interrupt priority 
void impl_timerNVICConfigure(TCH_t * tch, int irqPriority)
{
    if (tch->timCtx->timDef->irq) {
        nvic_irq_enable(tch->timCtx->timDef->irq, irqPriority, 0);
    }

    if (tch->timCtx->timDef->secondIrq) {
        nvic_irq_enable(tch->timCtx->timDef->secondIrq, irqPriority, 0);
    }
}

void impl_timerConfigBase(TCH_t * tch, uint16_t period, uint32_t hz)
{
    tmr_type * tim = tch->timCtx->timDef->tim;
    tmr_base_init(tim, (period - 1) & 0xffff,  lrintf((float)timerGetBaseClock(tch) / hz + 0.01f) - 1);
    tmr_clock_source_div_set(tim, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(tim, TMR_COUNT_UP);  //Count up (default)
}

void impl_enableTimer(TCH_t * tch)
{
    tmr_counter_enable(tch->timHw->tim, TRUE);
}

void impl_timerPWMStart(TCH_t * tch)
{
    tmr_output_enable(tch->timHw->tim,TRUE);
}

void impl_timerEnableIT(TCH_t * tch, uint32_t interrupt)
{
    tmr_interrupt_enable(tch->timHw->tim, interrupt, TRUE);
}

void impl_timerDisableIT(TCH_t * tch, uint32_t interrupt)
{
     tmr_interrupt_enable(tch->timHw->tim, interrupt, FALSE);
}

void impl_timerClearFlag(TCH_t * tch, uint32_t flag)
{
    tmr_flag_clear(tch->timHw->tim, flag);
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
    tmr_input_config_type tmr_input_config_struct;

    tmr_input_default_para_init(&tmr_input_config_struct);
    tmr_input_config_struct.input_channel_select = lookupTIMChannelTable[tch->timHw->channelIndex];
    tmr_input_config_struct.input_mapped_select = TMR_CC_CHANNEL_MAPPED_DIRECT; 
    tmr_input_config_struct.input_polarity_select = polarityRising ? TMR_INPUT_RISING_EDGE:TMR_INPUT_FALLING_EDGE;
    tmr_input_config_struct.input_filter_value = getFilter(inputFilterTicks);
    tmr_input_channel_init(tch->timHw->tim,&tmr_input_config_struct,TMR_CHANNEL_INPUT_DIV_1);
}

void impl_timerCaptureCompareHandler(tmr_type *tim, timHardwareContext_t *timerCtx)
{
    unsigned tim_status = tim->ists & tim->iden;

    while (tim_status) {
        // flags will be cleared by reading CCR in dual capture, make sure we call handler correctly
        // currrent order is highest bit first. Code should not rely on specific order (it will introduce race conditions anyway)
        unsigned bit = __builtin_clz(tim_status);
        unsigned mask = ~(0x80000000 >> bit);
        tim->ists = mask;
        tim_status &= mask;
 
        if (timerCtx) {
            switch (bit) {
                case __builtin_clz(TMR_OVF_INT): {
                    const uint16_t capture = tim->pr;
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
                case __builtin_clz(TMR_C1_INT):
                    timerCtx->ch[0].cb->callbackEdge(&timerCtx->ch[0], tim->c1dt);
                    break;
                case __builtin_clz(TMR_C2_INT):
                    timerCtx->ch[1].cb->callbackEdge(&timerCtx->ch[1], tim->c2dt);
                    break;
                case __builtin_clz(TMR_C3_INT):
                    timerCtx->ch[2].cb->callbackEdge(&timerCtx->ch[2], tim->c3dt);
                    break;
                case __builtin_clz(TMR_C4_INT):
                    timerCtx->ch[3].cb->callbackEdge(&timerCtx->ch[3], tim->c4dt);
                    break;
            }
        }
        else {
            // timerConfig == NULL
            volatile uint32_t tmp;

            switch (bit) {
                case __builtin_clz(TMR_OVF_INT):
                    tmp = tim->pr;
                    break;
                case __builtin_clz(TMR_C1_INT):
                    tmp = tim->c1dt;
                    break;
                case __builtin_clz(TMR_C2_INT):
                    tmp = tim->c2dt;
                    break;
                case __builtin_clz(TMR_C3_INT):
                    tmp = tim->c3dt;
                    break;
                case __builtin_clz(TMR_C4_INT):
                    tmp = tim->c4dt;
                    break;
            }

            (void)tmp;
        }
    }
}

void impl_timerPWMConfigChannel(TCH_t * tch, uint16_t value)
{
    const bool inverted = tch->timHw->output & TIMER_OUTPUT_INVERTED;
    tmr_output_config_type tmr_output_struct;
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A; 
     
    if (tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
            tmr_output_struct.oc_output_state = FALSE;
            tmr_output_struct.occ_output_state = TRUE;
            tmr_output_struct.occ_polarity =  inverted ? TMR_OUTPUT_ACTIVE_LOW : TMR_OUTPUT_ACTIVE_HIGH;
            tmr_output_struct.occ_idle_state = FALSE;

        } else {
            tmr_output_struct.oc_output_state = TRUE;
            tmr_output_struct.occ_output_state = FALSE;
            tmr_output_struct.oc_polarity =  inverted ? TMR_OUTPUT_ACTIVE_LOW : TMR_OUTPUT_ACTIVE_HIGH;
            tmr_output_struct.oc_idle_state = TRUE;
        }
    switch (tch->timHw->channelIndex) {
        case 0:
            tmr_output_channel_config(tch->timHw->tim,TMR_SELECT_CHANNEL_1, &tmr_output_struct);
            tmr_channel_value_set(tch->timHw->tim, TMR_SELECT_CHANNEL_1, value);
            tmr_output_channel_buffer_enable(tch->timHw->tim,TMR_SELECT_CHANNEL_1,TRUE); 
             break;
        case 1:
            tmr_output_channel_config(tch->timHw->tim,TMR_SELECT_CHANNEL_2, &tmr_output_struct);
            tmr_channel_value_set(tch->timHw->tim, TMR_SELECT_CHANNEL_2, value);
            tmr_output_channel_buffer_enable(tch->timHw->tim,TMR_SELECT_CHANNEL_2,TRUE); 

            break;
        case 2:
            tmr_output_channel_config(tch->timHw->tim,TMR_SELECT_CHANNEL_3, &tmr_output_struct);
            tmr_channel_value_set(tch->timHw->tim, TMR_SELECT_CHANNEL_3, value);
            tmr_output_channel_buffer_enable(tch->timHw->tim,TMR_SELECT_CHANNEL_3,TRUE); 
 
            break;
        case 3:
            tmr_output_channel_config(tch->timHw->tim,TMR_SELECT_CHANNEL_4, &tmr_output_struct);
            tmr_channel_value_set(tch->timHw->tim, TMR_SELECT_CHANNEL_4, value);
            tmr_output_channel_buffer_enable(tch->timHw->tim,TMR_SELECT_CHANNEL_4,TRUE); 

            break;
    }
  
}

volatile timCCR_t * impl_timerCCR(TCH_t * tch)
{
    switch (tch->timHw->channelIndex) {
        case 0:
            return &tch->timHw->tim->c1dt;
            break;
        case 1:
            return &tch->timHw->tim->c2dt;
            break;
        case 2:
            return &tch->timHw->tim->c3dt;
            break;
        case 3:
            return &tch->timHw->tim->c4dt;
            break;
    }
    return NULL;
}

// Set the channel control register
void impl_timerChCaptureCompareEnable(TCH_t * tch, bool enable)
{
    tmr_channel_enable(tch->timHw->tim, lookupTIMChannelTable[tch->timHw->channelIndex],(enable ? TRUE : FALSE));
}

// lookupDMASourceTable
static void impl_timerDMA_IRQHandler(DMA_t descriptor)
{
    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        TCH_t * tch = (TCH_t *)descriptor->userParam;
        tch->dmaState = TCH_DMA_IDLE;
        dma_channel_enable(tch->dma->ref,FALSE);
        tmr_dma_request_enable(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], FALSE);
        DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
    }
}

bool impl_timerPWMConfigChannelDMA(TCH_t * tch, void * dmaBuffer, uint8_t dmaBufferElementSize, uint32_t dmaBufferElementCount)
{
    dma_init_type dma_init_struct = {0};
    tmr_type * timer = tch->timHw->tim;
    
    tch->dma = dmaGetByTag(tch->timHw->dmaTag);
    if (tch->dma == NULL) {
        return false;  
    }

    // If DMA is already in use - abort
    if (tch->dma->owner != OWNER_FREE) {
        return false;
    }

    // We assume that timer channels are already initialized by calls to
    tmr_output_enable(timer, TRUE);

    // enable The TMR periodic buffer register
    tmr_period_buffer_enable(timer,TRUE);

    tmr_channel_enable(timer, lookupTIMChannelTable[tch->timHw->channelIndex],TRUE);

    tmr_counter_enable(timer, TRUE);
    dmaInit(tch->dma, OWNER_TIMER, 0);
    
    dmaSetHandler(tch->dma, impl_timerDMA_IRQHandler, NVIC_PRIO_TIMER_DMA, (uint32_t)tch);
    dma_reset(tch->dma->ref);
    dma_channel_enable(tch->dma->ref,FALSE);
    
    dma_reset(tch->dma->ref);
    dma_default_para_init(&dma_init_struct);

    dma_init_struct.peripheral_base_addr = (uint32_t)impl_timerCCR(tch);
    dma_init_struct.buffer_size = dmaBufferElementCount;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.loop_mode_enable = FALSE;

    switch (dmaBufferElementSize) {
        case 1:
            dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
            dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;  
            break;
        case 2:
            dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
            dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
            break;
        case 4:
            dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_WORD;
            dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_WORD;
            break;
        default:
            // Programmer error
            while(1) {
            }
    }

    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = (uint32_t)dmaBuffer;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init(tch->dma->ref, &dma_init_struct);

    //Set DMA request Mux mapping
    dmaMuxEnable(tch->dma, tch->timHw->dmaMuxid);
    dma_interrupt_enable(tch->dma->ref, DMA_IT_TCIF, TRUE);

    return true;
}

void impl_timerPWMPrepareDMA(TCH_t * tch, uint32_t dmaBufferElementCount)
{ 
    tch->dma = dmaGetByTag(tch->timHw->dmaTag);
    if (tch->dma == NULL) {
        return ;  
    }

    // Make sure we terminate any DMA transaction currently in progress
    // Clear the flag as well, so even if DMA transfer finishes while within ATOMIC_BLOCK
    // the resulting IRQ won't mess up the DMA state
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        dma_channel_enable(tch->dma->ref,FALSE);
        tmr_dma_request_enable(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], FALSE);
        // clear dma flag
        DMA_CLEAR_FLAG(tch->dma, DMA_IT_TCIF);
        
    }
    dma_data_number_set(tch->dma->ref, dmaBufferElementCount); 
     
    dma_channel_enable(tch->dma->ref,TRUE);
    tch->dmaState = TCH_DMA_READY;
}

void impl_timerPWMStartDMA(TCH_t * tch)
{
    uint16_t dmaSources = 0;
    timHardwareContext_t * timCtx = tch->timCtx;

    if (timCtx->ch[0].dmaState == TCH_DMA_READY) {
        timCtx->ch[0].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TMR_C1_DMA_REQUEST;
    }

    if (timCtx->ch[1].dmaState == TCH_DMA_READY) {
        timCtx->ch[1].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TMR_C2_DMA_REQUEST;
    }

    if (timCtx->ch[2].dmaState == TCH_DMA_READY) {
        timCtx->ch[2].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TMR_C3_DMA_REQUEST;
    }

    if (timCtx->ch[3].dmaState == TCH_DMA_READY) {
        timCtx->ch[3].dmaState = TCH_DMA_ACTIVE;
        dmaSources |= TMR_C4_DMA_REQUEST;
    }

    if (dmaSources) {
        tmr_counter_value_set(tch->timHw->tim, 0);
        tmr_dma_request_enable(tch->timHw->tim, dmaSources, TRUE);
    }
}

void impl_timerPWMStopDMA(TCH_t * tch)
{
    dma_channel_enable(tch->dma->ref,FALSE);
    tmr_dma_request_enable(tch->timHw->tim, lookupDMASourceTable[tch->timHw->channelIndex], FALSE);
    tmr_counter_enable(tch->timHw->tim, TRUE);

}
