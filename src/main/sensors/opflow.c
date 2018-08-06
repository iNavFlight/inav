/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <platform.h>

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/time.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/io.h"
#include "drivers/logging.h"
#include "drivers/time.h"

#include "drivers/opflow/opflow.h"
#include "drivers/opflow/opflow_fake.h"
#include "drivers/opflow/opflow_virtual.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"
#include "sensors/opflow.h"

#include "scheduler/scheduler.h"

#include "io/opflow.h"

#include "build/debug.h"

opflow_t opflow;

#define OPFLOW_SQUAL_THRESHOLD_HIGH     35      // TBD
#define OPFLOW_SQUAL_THRESHOLD_LOW      10      // TBD
#define OPFLOW_UPDATE_TIMEOUT_US        200000  // At least 5Hz updates required

#ifdef USE_OPTICAL_FLOW
PG_REGISTER_WITH_RESET_TEMPLATE(opticalFlowConfig_t, opticalFlowConfig, PG_OPFLOW_CONFIG, 1);

PG_RESET_TEMPLATE(opticalFlowConfig_t, opticalFlowConfig,
    .opflow_hardware = OPFLOW_NONE,
    .opflow_align = CW0_DEG_FLIP,
    .opflow_scale = 1.0f,
);

static bool opflowDetect(opflowDev_t * dev, uint8_t opflowHardwareToUse)
{
    opticalFlowSensor_e opflowHardware = OPFLOW_NONE;
    requestedSensors[SENSOR_INDEX_OPFLOW] = opflowHardwareToUse;

    switch (opflowHardwareToUse) {
        case OPFLOW_FAKE:
#if defined(USE_OPFLOW_FAKE)
            if (fakeOpflowDetect(dev)) {
                opflowHardware = OPFLOW_FAKE;
            }
#endif
            break;

        case OPFLOW_CXOF:
#if defined(USE_OPFLOW_CXOF)
            if (virtualOpflowDetect(dev, &opflowCxofVtable)) {
                opflowHardware = OPFLOW_CXOF;
            }
#endif
            break;

        case OPFLOW_MSP:
#if defined(USE_OPFLOW_MSP)
            if (virtualOpflowDetect(dev, &opflowMSPVtable)) {
                opflowHardware = OPFLOW_MSP;
            }
#endif
            break;

        case OPFLOW_NONE:
            opflowHardware = OPFLOW_NONE;
            break;
    }

    addBootlogEvent6(BOOT_EVENT_OPFLOW_DETECTION, BOOT_EVENT_FLAGS_NONE, opflowHardware, 0, 0, 0);

    if (opflowHardware == OPFLOW_NONE) {
        sensorsClear(SENSOR_OPFLOW);
        return false;
    }

    detectedSensors[SENSOR_INDEX_OPFLOW] = opflowHardware;
    sensorsSet(SENSOR_OPFLOW);
    return true;
}

static void opflowZeroBodyGyroAcc(void)
{
    opflow.gyroBodyRateTimeUs = 0;
    opflow.gyroBodyRateAcc[X] = 0;
    opflow.gyroBodyRateAcc[Y] = 0;
}

bool opflowInit(void)
{
    if (!opflowDetect(&opflow.dev, opticalFlowConfig()->opflow_hardware)) {
        return false;
    }

    if (!opflow.dev.initFn(&opflow.dev)) {
        sensorsClear(SENSOR_OPFLOW);
        return false;
    }

    opflowZeroBodyGyroAcc();

    return true;
}

/*
 * This is called periodically by the scheduler
 */
void opflowUpdate(timeUs_t currentTimeUs)
{
    if (!opflow.dev.updateFn)
        return;

    if (opflow.dev.updateFn(&opflow.dev)) {
        // Indicate valid update
        opflow.lastValidUpdate = currentTimeUs;
        opflow.isHwHealty = true;
        opflow.rawQuality = opflow.dev.rawData.quality;

        // Handle state switching
        switch (opflow.flowQuality) {
            case OPFLOW_QUALITY_INVALID:
                if (opflow.dev.rawData.quality >= OPFLOW_SQUAL_THRESHOLD_HIGH) {
                    opflow.flowQuality = OPFLOW_QUALITY_VALID;
                }
                break;

            case OPFLOW_QUALITY_VALID:
                if (opflow.dev.rawData.quality <= OPFLOW_SQUAL_THRESHOLD_LOW) {
                    opflow.flowQuality = OPFLOW_QUALITY_INVALID;
                }
                break;
        }

        if ((opflow.flowQuality == OPFLOW_QUALITY_VALID) && (opflow.gyroBodyRateTimeUs > 0)) {
            const float integralToRateScaler = (1.0e6 / opflow.dev.rawData.deltaTime) / (float)opticalFlowConfig()->opflow_scale;

            // Apply sensor alignment
            applySensorAlignment(opflow.dev.rawData.flowRateRaw, opflow.dev.rawData.flowRateRaw, opticalFlowConfig()->opflow_align);
            //applyBoardAlignment(opflow.dev.rawData.flowRateRaw);

            opflow.flowRate[X] = DEGREES_TO_RADIANS(opflow.dev.rawData.flowRateRaw[X] * integralToRateScaler);
            opflow.flowRate[Y] = DEGREES_TO_RADIANS(opflow.dev.rawData.flowRateRaw[Y] * integralToRateScaler);

            opflow.bodyRate[X] = DEGREES_TO_RADIANS(opflow.gyroBodyRateAcc[X] / opflow.gyroBodyRateTimeUs);
            opflow.bodyRate[Y] = DEGREES_TO_RADIANS(opflow.gyroBodyRateAcc[Y] / opflow.gyroBodyRateTimeUs);

            DEBUG_SET(DEBUG_FLOW_RAW, 0, RADIANS_TO_DEGREES(opflow.dev.rawData.flowRateRaw[X]));
            DEBUG_SET(DEBUG_FLOW_RAW, 1, RADIANS_TO_DEGREES(opflow.dev.rawData.flowRateRaw[Y]));
            DEBUG_SET(DEBUG_FLOW_RAW, 2, RADIANS_TO_DEGREES(opflow.bodyRate[X]));
            DEBUG_SET(DEBUG_FLOW_RAW, 3, RADIANS_TO_DEGREES(opflow.bodyRate[Y]));
        }
        else {
            // Opflow updated but invalid - zero out flow rates and body
            opflow.flowRate[X] = 0;
            opflow.flowRate[Y] = 0;

            opflow.bodyRate[X] = 0;
            opflow.bodyRate[Y] = 0;
        }

        // Zero out gyro accumulators to calculate rotation per flow update
        opflowZeroBodyGyroAcc();
    }
    else {
        // No new data available
        if (opflow.isHwHealty && ((currentTimeUs - opflow.lastValidUpdate) > OPFLOW_UPDATE_TIMEOUT_US)) {
            opflow.isHwHealty = false;

            opflow.flowQuality = OPFLOW_QUALITY_INVALID;
            opflow.rawQuality = 0;

            opflow.flowRate[X] = 0;
            opflow.flowRate[Y] = 0;
            opflow.bodyRate[X] = 0;
            opflow.bodyRate[Y] = 0;

            opflowZeroBodyGyroAcc();
        }
    }
}

/* Run a simple gyro update integrator to estimate average body rate between two optical flow updates */
void opflowGyroUpdateCallback(timeUs_t gyroUpdateDeltaUs)
{
    if (!opflow.isHwHealty)
        return;

    for (int axis = 0; axis < 2; axis++) {
        opflow.gyroBodyRateAcc[axis] += gyro.gyroADCf[axis] * gyroUpdateDeltaUs;
    }

    opflow.gyroBodyRateTimeUs += gyroUpdateDeltaUs;
}

bool opflowIsHealthy(void)
{
    return opflow.isHwHealty;
}
#endif
