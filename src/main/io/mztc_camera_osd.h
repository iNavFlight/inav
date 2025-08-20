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
 * MERCHANTABILITY or FITNESS FOR ANY PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "config/parameter_group.h"

// OSD element IDs for MassZero Thermal Camera
#define MZTC_OSD_ELEMENT_TEMPERATURE    0x01
#define MZTC_OSD_ELEMENT_STATUS         0x02
#define MZTC_OSD_ELEMENT_ALERTS         0x03
#define MZTC_OSD_ELEMENT_CALIBRATION    0x04
#define MZTC_OSD_ELEMENT_CONNECTION     0x05

// OSD element visibility flags
#define MZTC_OSD_VISIBLE_ALWAYS          0x01
#define MZTC_OSD_VISIBLE_ALERTS          0x02
#define MZTC_OSD_VISIBLE_CALIBRATION     0x04
#define MZTC_OSD_VISIBLE_ERROR           0x08

// OSD configuration structure
typedef struct mztcOsdConfig_s {
    uint8_t enabled;                    // Enable/disable OSD elements
    uint8_t temperature_display;         // Show temperature readings
    uint8_t status_display;             // Show camera status
    uint8_t alerts_display;             // Show temperature alerts
    uint8_t calibration_display;         // Show calibration status
    uint8_t connection_display;          // Show connection quality
    uint8_t position_x;                 // X position offset
    uint8_t position_y;                 // Y position offset
    uint8_t visibility_flags;           // Visibility control flags
} mztcOsdConfig_t;

// Parameter group declaration
PG_DECLARE(mztcOsdConfig_t, mztcOsdConfig);

// Function declarations
void mztcOsdInit(void);
void mztcOsdUpdate(void);
void mztcOsdSetVisibility(uint8_t flags);
void mztcOsdSetPosition(uint8_t x, uint8_t y);
void mztcOsdSetElementEnabled(uint8_t element, bool enabled);
const mztcOsdConfig_t* mztcOsdGetConfig(void);
