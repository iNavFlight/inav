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

#include <stdint.h>
#include "drivers/opflow.h"

typedef enum {
    OPTICAL_FLOW_NONE = 0,
    OPTICAL_FLOW_AUTODETECT = 1,
    OPTICAL_FLOW_ADNS3080
} opticalFlowSensor_e;

typedef struct opticalFlowConfig_s {
    uint8_t opflow_hardware;                 // Pitotmeter hardware to use
} opticalFlowConfig_t;

typedef struct opflow_s {
    opflowDev_t dev;
    int         flowQuality;          // 0 to 100
    float       flowVelX;
    float       flowVelY;
} opflow_t;

extern opflow_t opflow;

extern bool opticalFlowDetect(opflowDev_t * dev, uint8_t opflowHardwareToUse);
extern void opticalFlowUpdate(timeDelta_t deltaTime);
extern bool opticalFlowInit(void);
extern bool isOpticalFlowHealthy(void);