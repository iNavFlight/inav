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

#pragma once

#include <stdint.h>
#include "config/parameter_group.h"
#include "drivers/opflow/opflow.h"

typedef enum {
    OPFLOW_NONE         = 0,
    OPFLOW_PMW3901      = 1,
    OPFLOW_CXOF         = 2,
    OPFLOW_MSP          = 3,
    OPFLOW_FAKE         = 4,
} opticalFlowSensor_e;

typedef enum {
    OPFLOW_QUALITY_INVALID,
    OPFLOW_QUALITY_VALID
} opflowQuality_e;

typedef struct opticalFlowConfig_s  {
    uint8_t opflow_hardware;
    uint8_t opflow_align;
    float   opflow_scale;       // Scaler value to convert between raw sensor units to [deg/s]
} opticalFlowConfig_t;

PG_DECLARE(opticalFlowConfig_t, opticalFlowConfig);

typedef struct opflow_s {
    opflowDev_t dev;

    opflowQuality_e flowQuality;
    timeUs_t        lastValidUpdate;
    bool            isHwHealty;
    float           flowRate[2];    // optical flow angular rate in rad/sec measured about the X and Y body axis
    float           bodyRate[2];    // body inertial angular rate in rad/sec measured about the X and Y body axis

    float           gyroBodyRateAcc[2];
    timeUs_t        gyroBodyRateTimeUs;

    uint8_t         rawQuality;
} opflow_t;

extern opflow_t opflow;

void opflowGyroUpdateCallback(timeUs_t gyroUpdateDeltaUs);
bool opflowInit(void);
void opflowUpdate(timeUs_t currentTimeUs);
bool opflowIsHealthy(void);
void opflowStartCalibration(void);
