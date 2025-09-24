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

#pragma once

#include "drivers/time.h"
#include "drivers/serial.h"
#include "config/mztc_camera.h"

#ifdef USE_MZTC

// External variables for CLI access
extern serialPort_t *mztcSerialPort;
extern mztcStatus_t mztcStatus;

// Function declarations
void mztcInit(void);
void mztcUpdate(timeUs_t currentTimeUs);
// Check if MassZero Thermal Camera is enabled
bool mztcIsEnabled(void);

// Check if MassZero Thermal Camera is connected
bool mztcIsConnected(void);

// Get MassZero Thermal Camera status
mztcStatus_t* mztcGetStatus(void);
bool mztcTriggerCalibration(void);
bool mztcSetMode(mztcMode_e mode);
bool mztcSetPalette(mztcPaletteMode_e palette);
bool mztcSetZoom(mztcZoomLevel_e zoom);
bool mztcSetImageParams(uint8_t brightness, uint8_t contrast, uint8_t enhancement);
bool mztcSetDenoising(uint8_t spatial, uint8_t temporal);
bool mztcSetTemperatureAlerts(bool enabled, float high_temp, float low_temp);
void mztcRequestReconnect(void);

// Get thermal frame data from the camera
bool mztcGetFrameData(mztcFrameData_t *frameData);

#endif // USE_MZTC
