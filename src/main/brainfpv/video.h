/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_VIDEO Code for OSD video generator
 * @brief Output video (black & white pixels) over SPI
 * @{
 *
 * @file       pios_video.h
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013-2014
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2014.
 * @brief      OSD gen module, handles OSD draw. Parts from CL-OSD and SUPEROSD projects
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef VIDEO_H
#define VIDEO_H

#include "target.h"
#include <stm32f4xx_qspi.h>

// PAL/NTSC specific boundary values
struct video_type_boundary {
	uint16_t graphics_right;
	uint16_t graphics_bottom;
};

// 3D Mode
enum video_3d_mode {
    VIDEO_3D_DISABLED,
    VIDEO_3D_SBS3D,
};

// PAL/NTSC specific config values
struct video_type_cfg {
	uint16_t graphics_hight_real;
	uint16_t graphics_column_start;
	uint8_t  graphics_line_start;
	uint8_t  dma_buffer_length;
};


void Video_Init(void);
//void Video_SetLevels(uint8_t, uint8_t, uint8_t, uint8_t);
//void Video_SetXOffset(int8_t);
//void Video_SetYOffset(int8_t);
//void Video_SetXScale(uint8_t pal_x_scale, uint8_t ntsc_x_scale);
//void Video_Set3DConfig(enum video_3d_mode mode, uint8_t right_eye_x_shift);

uint16_t Video_GetLines(void);
uint16_t Video_GetType(void);

// video boundary values
extern const struct video_type_boundary *video_type_boundary_act;
#define GRAPHICS_LEFT        0
#define GRAPHICS_TOP         0
#define GRAPHICS_RIGHT       video_type_boundary_act->graphics_right
#define GRAPHICS_BOTTOM      video_type_boundary_act->graphics_bottom

#define GRAPHICS_X_MIDDLE	((GRAPHICS_RIGHT + 1) / 2)
#define GRAPHICS_Y_MIDDLE	((GRAPHICS_BOTTOM + 1) / 2)

// video type defs for autodetect
#define VIDEO_TYPE_NONE      0
#define VIDEO_TYPE_NTSC      1
#define VIDEO_TYPE_PAL       2
#define VIDEO_TYPE_PAL_ROWS  300

// draw area buffer values, for memory allocation, access and calculations we suppose the larger values for PAL, this also works for NTSC
#define GRAPHICS_WIDTH_REAL  376                            // max columns
#define GRAPHICS_HEIGHT_REAL 266                            // max lines
#define BUFFER_WIDTH_TMP     (GRAPHICS_WIDTH_REAL / (8 / VIDEO_BITS_PER_PIXEL))
#define BUFFER_WIDTH (BUFFER_WIDTH_TMP + BUFFER_WIDTH_TMP % 4)
#define BUFFER_HEIGHT        (GRAPHICS_HEIGHT_REAL)

#endif /* VIDEO_H */
