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
#include <math.h>

#include <platform.h>

#ifdef OPTICAL_FLOW

#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/filter.h"

#include "scheduler/scheduler.h"

#include "config/feature.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "drivers/io.h"
#include "drivers/sensor.h"
#include "drivers/opflow.h"
#include "drivers/opflow_adns3080.h"

#include "sensors/sensors.h"
#include "sensors/opflow.h"
#include "sensors/battery.h"

// Driver access functions
opflow_t opflow;

// Optical flow data LPF
static pt1Filter_t opflowVelXLpfState;
static pt1Filter_t opflowVelYLpfState;

#define OPTICAL_FLOW_LOWPASS_HZ         20.0f

void opticalFlowUpdate(timeDelta_t deltaTime)
{
    const float opflowDt = deltaTime * 1e-6f;
    opflow_data_t driverFlowData;

    if (!sensors(SENSOR_OPTICAL_FLOW)) {
        return;
    }

    /* We receive raw motion data from the sensor (X - forward, Y - right coordinated), we need to compensate them for pitch and roll rate as aircraft rotation introduces change
     * in what the sensor sees and we receive that as "fake" motion. We also need to scale the motion according to FOV. We can't scale the motion to real units as we don't know the
     * altitude of the drone - we leave that high level processing to INAV Position Estimation core.
     */
    if (opflow.dev.read((int16_t *)&driverFlowData)) {
        /*
        debug[0] = driverFlowData.delta.V.X;
        debug[1] = driverFlowData.delta.V.Y;
        debug[2] = driverFlowData.quality;
        */
    }
    else {
        driverFlowData.delta.V.X = 0;
        driverFlowData.delta.V.Y = 0;
    }

    opflow.flowVelX = pt1FilterApply4(&opflowVelXLpfState, driverFlowData.delta.V.X, OPTICAL_FLOW_LOWPASS_HZ, opflowDt);
    opflow.flowVelY = pt1FilterApply4(&opflowVelYLpfState, driverFlowData.delta.V.Y, OPTICAL_FLOW_LOWPASS_HZ, opflowDt);

    debug[0] = opflow.flowVelX;
    debug[1] = opflow.flowVelY;
    debug[2] = driverFlowData.quality;
}

bool isOpticalFlowHealthy(void)
{
    return true;
}

bool opticalFlowDetect(opflowDev_t * dev, uint8_t opflowHardwareToUse)
{
    opticalFlowSensor_e opflowHardware = OPTICAL_FLOW_NONE;
    requestedSensors[SENSOR_INDEX_OPTICAL_FLOW] = opflowHardwareToUse;

    memset(dev, 0, sizeof(opflowDev_t));
    dev->hasSoftSPI = feature(FEATURE_SOFTSPI);

    switch(opflowHardwareToUse) {
        case OPTICAL_FLOW_AUTODETECT:
        case OPTICAL_FLOW_ADNS3080:
#if defined(USE_OPTICAL_FLOW_ADNS3080_SOFTSPI)
            if (opflowADNS3080Detect(dev)) {
                opflowHardware = OPTICAL_FLOW_ADNS3080;
                break;
            }
#endif
            /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
            if (opflowHardwareToUse != OPTICAL_FLOW_AUTODETECT) {
                break;
           }

        case OPTICAL_FLOW_NONE:
            opflowHardware = OPTICAL_FLOW_NONE;
            break;
    }

    // Detection finished - store sensor type
    if (opflowHardware == OPTICAL_FLOW_NONE) {
        sensorsClear(SENSOR_OPTICAL_FLOW);
        return false;
    }

    detectedSensors[SENSOR_INDEX_OPTICAL_FLOW] = opflowHardware;
    sensorsSet(SENSOR_OPTICAL_FLOW);

    return true;
}

bool opticalFlowInit(void)
{
    delay(10);
    opflow.dev.init();
    return true;
}

#endif

