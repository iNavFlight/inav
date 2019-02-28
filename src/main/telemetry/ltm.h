/*
 * ltm.h
 *
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

typedef enum {
    LTM_FRAME_START = 0,
    LTM_AFRAME = LTM_FRAME_START, // Attitude Frame
    LTM_SFRAME, // Status Frame
#if defined(USE_GPS)
    LTM_GFRAME, // GPS Frame
    LTM_OFRAME, // Origin Frame
    LTM_XFRAME, // Extended information data frame
#endif
#if defined(USE_NAV)
    LTM_NFRAME, // Navigation Frame (inav extension)
#endif
    LTM_FRAME_COUNT
} ltm_frame_e;

// payload size does not include the '$T' header, the frame type byte or the checksum byte
#define LTM_GFRAME_PAYLOAD_SIZE 14
#define LTM_AFRAME_PAYLOAD_SIZE  6
#define LTM_SFRAME_PAYLOAD_SIZE  7
#define LTM_OFRAME_PAYLOAD_SIZE 14
#define LTM_NFRAME_PAYLOAD_SIZE  6
#define LTM_XFRAME_PAYLOAD_SIZE  6

#define LTM_MAX_PAYLOAD_SIZE 14
#define LTM_MAX_MESSAGE_SIZE (LTM_MAX_PAYLOAD_SIZE+4)


typedef enum {
    LTM_MODE_MANUAL = 0,
    LTM_MODE_RATE,
    LTM_MODE_ANGLE,
    LTM_MODE_HORIZON,
    LTM_MODE_ACRO,
    LTM_MODE_STABALIZED1,
    LTM_MODE_STABALIZED2,
    LTM_MODE_STABILIZED3,
    LTM_MODE_ALTHOLD,
    LTM_MODE_GPSHOLD,
    LTM_MODE_WAYPOINTS,
    LTM_MODE_HEADHOLD,
    LTM_MODE_CIRCLE,
    LTM_MODE_RTH,
    LTM_MODE_FOLLOWWME,
    LTM_MODE_LAND,
    LTM_MODE_FLYBYWIRE1,
    LTM_MODE_FLYBYWIRE2,
    LTM_MODE_CRUISE,
    LTM_MODE_UNKNOWN,
        // iNav specific extensions
    LTM_MODE_LAUNCH,
    LTM_MODE_AUTOTUNE
} ltm_modes_e;

void initLtmTelemetry(void);
void handleLtmTelemetry(void);
void checkLtmTelemetryState(void);

void freeLtmTelemetryPort(void);
void configureLtmTelemetryPort(void);

int getLtmFrame(uint8_t *frame, ltm_frame_e ltmFrameType);
