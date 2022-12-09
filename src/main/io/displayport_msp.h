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

#pragma once

#include "config/parameter_group.h"
#include "drivers/display.h"

// MSP Display Port commands
typedef enum {
    MSP_DP_HEARTBEAT = 0,       // Ensure display is not released, and prevent 'disconnected' status
    MSP_DP_RELEASE = 1,         // Release the display after clearing and updating
    MSP_DP_CLEAR_SCREEN = 2,    // Clear the display
    MSP_DP_WRITE_STRING = 3,    // Write a string at given coordinates
    MSP_DP_DRAW_SCREEN = 4,     // Trigger a screen draw
    MSP_DP_OPTIONS = 5,         // Not used by Betaflight. Reserved by Ardupilot and INAV
    MSP_DP_SYS = 6,             // Display system element displayportSystemElement_e at given coordinates
    MSP_DP_COUNT,
} displayportMspCommand_e;

struct displayPort_s;
struct displayPort_s *displayPortMspInit(void);
