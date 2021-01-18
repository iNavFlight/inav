/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_VIDEO Code for OSD video generator
 * @brief Output video (black & white pixels) over SPI
 * @{
 *
 * @file       pios_video.c
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013-2015
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2014.
 * @brief      OSD gen module, handles OSD draw. Parts from CL-OSD and SUPEROSD projects
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
//#include "system.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/light_led.h"
#include "drivers/time.h"

static IO_t debugPin = IO_NONE;
static IO_t hsync_io;
static IO_t vsync_io;

#if defined(INCLUDE_VIDEO_QUADSPI) | 1

#if !defined(VIDEO_QUADSPI_Y_OFFSET)
#define VIDEO_QUADSPI_Y_OFFSET 0
#endif /* !defined(VIDEO_QUADSPI_Y_OFFSET) */

#include "ch.h"
#include "video.h"

// How many frames until we redraw
#define VSYNC_REDRAW_CNT 2

// Minimum micro seconds between VSYNCS
#define MIN_DELTA_VSYNC 10000
#define MAX_SPURIOUS_VSYNCS 10

extiCallbackRec_t vsyncIntCallbackRec;
extiCallbackRec_t hsyncIntCallbackRec;

binary_semaphore_t onScreenDisplaySemaphore;

#define GRPAHICS_RIGHT_NTSC 351
#define GRPAHICS_RIGHT_PAL  359

static const struct video_type_boundary video_type_boundary_ntsc = {
    .graphics_right  = GRPAHICS_RIGHT_NTSC, // must be: graphics_width_real - 1
    .graphics_bottom = 239,                 // must be: graphics_hight_real - 1
};

static const struct video_type_boundary video_type_boundary_pal = {
    .graphics_right  = GRPAHICS_RIGHT_PAL, // must be: graphics_width_real - 1
    .graphics_bottom = 265,                // must be: graphics_hight_real - 1
};

#define NTSC_BYTES (GRPAHICS_RIGHT_NTSC / (8 / VIDEO_BITS_PER_PIXEL) + 1)
#define PAL_BYTES (GRPAHICS_RIGHT_PAL / (8 / VIDEO_BITS_PER_PIXEL) + 1)

static const struct video_type_cfg video_type_cfg_ntsc = {
    .graphics_hight_real   = 240,   // Real visible lines
    .graphics_column_start = 260,   // First visible OSD column (after Hsync)
    .graphics_line_start   = 22,    // First visible OSD line
    .dma_buffer_length     = NTSC_BYTES + NTSC_BYTES % 4, // DMA buffer length in bytes (has to be multiple of 4)
};

static const struct video_type_cfg video_type_cfg_pal = {
    .graphics_hight_real   = 266,   // Real visible lines
    .graphics_column_start = 420,   // First visible OSD column (after Hsync)
    .graphics_line_start   = 28,    // First visible OSD line
    .dma_buffer_length     = PAL_BYTES + PAL_BYTES % 4, // DMA buffer length in bytes (has to be multiple of 4)
};

// Allocate buffers.
// Must be allocated in one block, so it is in a struct.
//struct _buffers {
//    uint8_t buffer0[BUFFER_HEIGHT * BUFFER_WIDTH];
//    uint8_t buffer1[BUFFER_HEIGHT * BUFFER_WIDTH];
//} buffers;
//
//// Remove the struct definition (makes it easier to write for).
//#define buffer0 (buffers.buffer0)
//#define buffer1 (buffers.buffer1)

uint8_t buffer0[BUFFER_HEIGHT * BUFFER_WIDTH];

// Pointers to each of these buffers.
uint8_t *draw_buffer;
uint8_t *disp_buffer;


const struct video_type_boundary *video_type_boundary_act = &video_type_boundary_pal;

// Private variables
static uint8_t spurious_vsync_cnt = 0;
static int16_t active_line = 0;
static uint32_t buffer_offset;
static int8_t y_offset = 0;
static uint16_t num_video_lines = 0;
static bool trigger_redraw;
static int8_t video_type_tmp = VIDEO_TYPE_PAL;
static int8_t video_type_act = VIDEO_TYPE_NONE;
static const struct video_type_cfg *video_type_cfg_act = &video_type_cfg_pal;

uint8_t black_pal = 30;
uint8_t white_pal = 110;
uint8_t black_ntsc = 10;
uint8_t white_ntsc = 110;


// Re-enable the video if it has been disabled
void video_qspi_enable(void)
{
    // re-enable interrupts
    spurious_vsync_cnt = 0;
    EXTIEnable(vsync_io, true);
    EXTIEnable(hsync_io, true);
}

/**
 * @brief Vsync interrupt service routine
 */
void Vsync_ISR(extiCallbackRec_t *cb)
{
    (void)cb;
    static uint32_t t_last = 0;
    static uint16_t Vsync_update = 0;

    uint32_t t_now;

    CH_IRQ_PROLOGUE();

    t_now = microsISR();

    if (t_now - t_last < MIN_DELTA_VSYNC) {
        spurious_vsync_cnt += 1;
        if (spurious_vsync_cnt >= MAX_SPURIOUS_VSYNCS) {
            // spurious detections: disable interrupts
            EXTIEnable(vsync_io, false);
            EXTIEnable(hsync_io, false);
        }
        CH_IRQ_EPILOGUE();
        return;
    }
    else {
        spurious_vsync_cnt = 0;
    }
    t_last = t_now;


    // discard spurious vsync pulses (due to improper grounding), so we don't overload the CPU
    if (active_line > 0 && active_line < video_type_cfg_ntsc.graphics_hight_real - 10) {
        CH_IRQ_EPILOGUE();
    }

    // Update the number of video lines
    num_video_lines = active_line + video_type_cfg_act->graphics_line_start + y_offset;

    // check video type
    if (num_video_lines > VIDEO_TYPE_PAL_ROWS) {
        video_type_tmp = VIDEO_TYPE_PAL;
    }

    // if video type has changed set new active values
    if (video_type_act != video_type_tmp) {
        video_type_act = video_type_tmp;
        if (video_type_act == VIDEO_TYPE_NTSC) {
            video_type_boundary_act = &video_type_boundary_ntsc;
            video_type_cfg_act = &video_type_cfg_ntsc;
            //dev_cfg->set_bw_levels(black_ntsc, white_ntsc);
        } else {
            video_type_boundary_act = &video_type_boundary_pal;
            video_type_cfg_act = &video_type_cfg_pal;
            //dev_cfg->set_bw_levels(black_pal, white_pal);
        }
    }

    video_type_tmp = VIDEO_TYPE_NTSC;

    // Every VSYNC_REDRAW_CNT field: swap buffers and trigger redraw
    if (++Vsync_update >= VSYNC_REDRAW_CNT) {
        Vsync_update = 0;
        trigger_redraw = true;
    }
    else {
        trigger_redraw = false;
    }

    // Get ready for the first line. We will start outputting data at line zero.
    active_line = 0 - (video_type_cfg_act->graphics_line_start + y_offset);
    buffer_offset = 0;
    CH_IRQ_EPILOGUE();
}

void Hsync_ISR(extiCallbackRec_t *cb)
{
    (void)cb;
    active_line++;
    //LED1_TOGGLE;
    EXTI->PR = 0x04;

    if ((active_line >= 0) && (active_line < video_type_cfg_act->graphics_hight_real)) {
        // Check if QUADSPI is busy
        if (QUADSPI->SR & 0x20)
            return;

        // Disable DMA
        DMA2_Stream7->CR &= ~(uint32_t)DMA_SxCR_EN;

        // Clear the DMA interrupt flags
        DMA2->HIFCR  |= DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_DMEIF7;

        // Load new line
        DMA2_Stream7->M0AR = (uint32_t)&disp_buffer[buffer_offset];

        // Set length
        DMA2_Stream7->NDTR = (uint16_t)video_type_cfg_act->dma_buffer_length;
        QUADSPI->DLR = (uint32_t)video_type_cfg_act->dma_buffer_length - 1;

        if (trigger_redraw && (active_line == video_type_cfg_act->graphics_hight_real - 1)) {
            // Last line: Enable DMA TC interrupt
            DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
        }

        // Enable DMA
        uint32_t cr = DMA2_Stream7->CR;
        IOHi(debugPin);
        DMA2_Stream7->CR = cr | (uint32_t)DMA_SxCR_EN;
        IOLo(debugPin);

        buffer_offset += BUFFER_WIDTH;
    }
}

// ISR runs after last OSD line has been output
void DMA2_Stream7_IRQHandler(void)
{
    CH_IRQ_PROLOGUE();

    if (DMA2->HISR & DMA_FLAG_TCIF7) {
        // Clear flag and disable interrupt
        DMA2->HIFCR  |= DMA_FLAG_TCIF7;
		DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, DISABLE);

		// Trigger OSD redraw
        chSysLockFromISR();
        chBSemSignalI(&onScreenDisplaySemaphore);
        chSysUnlockFromISR();
	}

    CH_IRQ_EPILOGUE();
}

/**
 * Init
 */
void Video_Init(void)
{
    chBSemObjectInit(&onScreenDisplaySemaphore, FALSE);

    /* Enable QUADSPI clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_QSPI, ENABLE);

    /* Map pins to QUADSPI */
    IOInit(IOGetByTag(IO_TAG(VIDEO_QSPI_CLOCK_PIN)),  OWNER_OSD, RESOURCE_SPI_SCK, 0);
    IOInit(IOGetByTag(IO_TAG(VIDEO_QSPI_IO0_PIN)), OWNER_OSD, RESOURCE_SPI_MOSI, 0);
    IOInit(IOGetByTag(IO_TAG(VIDEO_QSPI_IO1_PIN)), OWNER_OSD, RESOURCE_SPI_MOSI, 0);

    IOConfigGPIOAF(IOGetByTag(IO_TAG(VIDEO_QSPI_CLOCK_PIN)), IOCFG_AF_PP, GPIO_AF9_QUADSPI);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(VIDEO_QSPI_IO0_PIN)), IOCFG_AF_PP, GPIO_AF9_QUADSPI);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(VIDEO_QSPI_IO1_PIN)), IOCFG_AF_PP, GPIO_AF9_QUADSPI);

    /* Configure QUADSPI */
    QSPI_InitTypeDef qspi_init = {
        .QSPI_SShift     = QSPI_SShift_NoShift,
        .QSPI_Prescaler  = 12,  // 180MHz / 12 = 15MHz
        .QSPI_CKMode     = QSPI_CKMode_Mode0,
        .QSPI_CSHTime    = QSPI_CSHTime_1Cycle,
        .QSPI_FSize      = 0x1F,
        .QSPI_FSelect    = QSPI_FSelect_1,
        .QSPI_DFlash     = QSPI_DFlash_Disable};

    QSPI_Init(&qspi_init);

    QSPI_ComConfig_InitTypeDef qspi_com_config;
    QSPI_ComConfig_StructInit(&qspi_com_config);

    qspi_com_config.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    qspi_com_config.QSPI_ComConfig_DDRMode     = QSPI_ComConfig_DDRMode_Disable;
    qspi_com_config.QSPI_ComConfig_DHHC        = QSPI_ComConfig_DHHC_Disable;
    qspi_com_config.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    qspi_com_config.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_2Line;
    qspi_com_config.QSPI_ComConfig_DummyCycles = 0;
    qspi_com_config.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    qspi_com_config.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    qspi_com_config.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_NoInstruction;
    QSPI_ComConfig_Init(&qspi_com_config);

    QSPI_SetFIFOThreshold(3);

    /* Configure DMA */
    DMA_InitTypeDef dma_cfg = {
        .DMA_Channel            = DMA_Channel_3,
        .DMA_PeripheralBaseAddr = (uint32_t)&(QUADSPI->DR),
        .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
        .DMA_BufferSize         = 400,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Enable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_Word,
        .DMA_Mode               = DMA_Mode_Normal,
        .DMA_Priority           = DMA_Priority_VeryHigh,
        .DMA_FIFOMode           = DMA_FIFOMode_Enable,
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
        .DMA_MemoryBurst        = DMA_MemoryBurst_INC4,
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single};
    DMA_Init(DMA2_Stream7, &dma_cfg);

    /* Configure and clear buffers */
    draw_buffer = buffer0;
    disp_buffer = buffer0;

    /* Enable TC interrupt */
    QSPI_ITConfig(QSPI_IT_TC, ENABLE);
    QSPI_ITConfig(QSPI_IT_FT, ENABLE);

    /* Enable DMA */
    QSPI_DMACmd(ENABLE);

    // Enable the QUADSPI
    QSPI_Cmd(ENABLE);

    // VSYNC interrupt
    vsync_io = IOGetByTag(IO_TAG(VIDEO_VSYNC));
    IOInit(vsync_io, OWNER_OSD, RESOURCE_EXTI, 0);
    IOConfigGPIO(vsync_io, IOCFG_IN_FLOATING);
    EXTIHandlerInit(&vsyncIntCallbackRec, Vsync_ISR);
    EXTIConfig(vsync_io, &vsyncIntCallbackRec, NVIC_PRIO_GYRO_INT_EXTI, EXTI_Trigger_Falling);

    // HSYNC interrupt
    hsync_io = IOGetByTag(IO_TAG(VIDEO_HSYNC));
    IOInit(hsync_io, OWNER_OSD, RESOURCE_EXTI, 0);
    IOConfigGPIO(hsync_io, IOCFG_IN_FLOATING);
    EXTIHandlerInit(&hsyncIntCallbackRec, Hsync_ISR);
    EXTIConfig(hsync_io, &hsyncIntCallbackRec, NVIC_PRIO_GYRO_INT_EXTI, EXTI_Trigger_Falling);

    // DMA TC interrupt for last line
    NVIC_SetPriority(DMA2_Stream7_IRQn, NVIC_PRIO_GYRO_INT_EXTI);
    NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, DISABLE); // will be enabled later

    // Enable interrupts
    EXTIEnable(vsync_io, true);
    EXTIEnable(hsync_io, true);
}

/**
 *
 */
uint16_t Video_GetLines(void)
{
    return num_video_lines;
}

/**
 *
 */
uint16_t Video_GetType(void)
{
    return video_type_act;
}

#endif /* INCLUDE_VIDEO_QUADSPI */
