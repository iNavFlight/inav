/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, contents of this file may be used under terms
 * of GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "sensors/aoa.h"

#include <math.h>
#include <stdlib.h>
#include <platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "build/build_config.h"
#include "build/debug.h"
#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "drivers/aoa/aoa_virtual.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"
#include "io/aoa.h"
#include "rx/rx.h"
#include "scheduler/scheduler.h"
#include "sensors/sensors.h"

aoa_t aoaSensor;

FASTRAM int16_t aoaPidOutput;
bool isAoaControlEnabled;
static int16_t prev_aoa = 0;

#define AOA_HARDWARE_TIMEOUT_MS 500

#ifdef USE_AOA

PG_REGISTER_WITH_RESET_TEMPLATE(aoaConfig_t, aoaConfig, PG_AOA_CONFIG, 0);

PG_RESET_TEMPLATE(aoaConfig_t, aoaConfig,
    .aoa_hardware = SETTING_AOA_HARDWARE_DEFAULT,
    .aoa_offset = 0,
    .aoa_max_angle = 60,
    .aoa_min_angle = -60,
);

PG_REGISTER_WITH_RESET_TEMPLATE(aoaControlConfig_t, aoaControlConfig, PG_AOA_CONTROL_CONFIG, 1);

PG_RESET_TEMPLATE(aoaControlConfig_t, aoaControlConfig,
    .fw_aoa_control_channel = -1,
    .fw_aoa_gvar_index = -1,
    .fw_aoa_deg2pwm = 110,
    .fw_aoa_trim_angle = 6,
    .fw_aoa_upper_limit_angle = 36,
    .fw_aoa_lower_limit_angle = -36,
    .fw_aoa_aircraft_type = 0,
    .fw_aoa_kp = 100,
    .fw_aoa_intervention_threshold = 80,
);

static bool aoaDetect(aoaDev_t* dev, uint8_t aoaHardwareToUse)
{
    aoaType_e aoaHardware = AOA_NONE;
    requestedSensors[SENSOR_INDEX_AOA] = aoaHardwareToUse;

    switch (aoaHardwareToUse) {
        case AOA_MSP:
#if defined(USE_AOA_MSP)
            if (virtualAoaDetect(dev, &aoaMSPVtable)) {
                aoaHardware = AOA_MSP;
                rescheduleTask(TASK_AOA, TASK_PERIOD_MS(AOA_VIRTUAL_TASK_PERIOD_MS));
            }
#endif
            break;

        case AOA_FAKE:
#if defined(USE_AOA_FAKE)
            if (virtualAoaDetect(dev, &aoaFakeVtable)) {
                aoaHardware = AOA_FAKE;
                rescheduleTask(TASK_AOA, TASK_PERIOD_MS(AOA_VIRTUAL_TASK_PERIOD_MS));
            }
#endif
            break;
        case AOA_NONE:
            aoaHardware = AOA_NONE;
            break;
    }

    if (aoaHardware == AOA_NONE) {
        sensorsClear(SENSOR_AOA);
        return false;
    }

    detectedSensors[SENSOR_INDEX_AOA] = aoaHardware;
    sensorsSet(SENSOR_AOA);
    return true;
}

bool aoaInit(void)
{
    if (!aoaDetect(&aoaSensor.dev, aoaConfig()->aoa_hardware)) {
        return false;
    }

    aoaSensor.dev.init(&aoaSensor.dev);
    aoaSensor.aoa = AOA_NO_NEW_DATA;
    aoaSensor.lastValidResponseTimeMs = millis();

    return true;
}

timeDelta_t aoaUpdate(void)
{
    if (aoaSensor.dev.update) {
        aoaSensor.dev.update(&aoaSensor.dev);
    }

    return MS2US(aoaSensor.dev.delayMs);
}

bool aoaProcess(void)
{
    if (aoaSensor.dev.read) {
        int16_t rawAoa, rawSideslip;
        aoaSensor.dev.read(&aoaSensor.dev, &rawAoa, &rawSideslip);
        int16_t aoaWithOffset = rawAoa + DEGREES_TO_DECIDEGREES(aoaConfig()->aoa_offset);
        aoaSensor.aoa = constrain(aoaWithOffset, DEGREES_TO_DECIDEGREES(aoaConfig()->aoa_min_angle), DEGREES_TO_DECIDEGREES(aoaConfig()->aoa_max_angle));
        aoaSensor.sideslip = rawSideslip;
        aoaSensor.lastValidResponseTimeMs = millis();
        DEBUG_SET(DEBUG_AOA, 0, aoaSensor.aoa);
    } else {
        aoaSensor.aoa = AOA_OUT_OF_RANGE;
        aoaSensor.sideslip = AOA_OUT_OF_RANGE;
    }

    return true;
}

void aoaGetLatestData(int16_t *aoa, int16_t *sideslip)
{
    *aoa = aoaSensor.aoa;
    *sideslip = aoaSensor.sideslip;
}

bool aoaIsHealthy(void)
{
    return (millis() - aoaSensor.lastValidResponseTimeMs) < AOA_HARDWARE_TIMEOUT_MS;
}

bool aoaControlEnable(int8_t input_rc_channel)
{
    if (input_rc_channel == -1) {
        return true;
    }
    
    if (input_rc_channel <= 0 || input_rc_channel >= 34) {
        return false;
    }
    
    int16_t rcValue = rxGetChannelValue(input_rc_channel - 1);
    return rcValue > 1666;
}

void aoaControlUpdate(float *pidPitchOutput, float rateError, float newPTerm, float newDTerm, float newFFTerm, float errorGyroIf, float limit)
{
    isAoaControlEnabled = aoaControlEnable(aoaControlConfig()->fw_aoa_control_channel);
    
    int16_t currentAoa, unused;
    aoaGetLatestData(&currentAoa, &unused);
    const int16_t filteredAoa = abs(currentAoa - prev_aoa) > 10 ? currentAoa : prev_aoa;
    prev_aoa = currentAoa;

    const float deg2pwm = aoaControlConfig()->fw_aoa_deg2pwm / 10.0f;
    const float kp = aoaControlConfig()->fw_aoa_kp * 0.01f;
    const int16_t upperLimitAngle = aoaControlConfig()->fw_aoa_upper_limit_angle;
    const int16_t lowerLimitAngle = aoaControlConfig()->fw_aoa_lower_limit_angle;
    float aoaServoOffset = 0.0f;

    if (aoaControlConfig()->fw_aoa_aircraft_type == AIRCRAFT_CANARD) {

        const float thresholdRatio = aoaControlConfig()->fw_aoa_intervention_threshold * 0.01f;
        const int16_t upperThreshold = upperLimitAngle * thresholdRatio;
        const int16_t lowerThreshold = lowerLimitAngle * thresholdRatio;
        const int16_t aoaDeg = DECIDEGREES_TO_DEGREES(filteredAoa);

        const int16_t lowerLimit = lowerLimitAngle * deg2pwm;
        const int16_t upperLimit = upperLimitAngle * deg2pwm;
        float constrainedPidOutput = constrainf(*pidPitchOutput, -upperLimit, -lowerLimit);

        float aoaError = 0.0f;
        if (aoaDeg > upperThreshold) {
            aoaError = aoaDeg - upperThreshold;
        } else if (aoaDeg < lowerThreshold) {
            aoaError = aoaDeg - lowerThreshold;
        }

        float interventionOffset = aoaError * kp * deg2pwm;
        aoaServoOffset = (DECIDEGREES_TO_DEGREES(filteredAoa) - aoaControlConfig()->fw_aoa_trim_angle) * deg2pwm;
        aoaPidOutput = isAoaControlEnabled ?  constrainedPidOutput + aoaServoOffset : constrainedPidOutput;
        aoaPidOutput = constrainf(aoaPidOutput, -limit, +limit);

        if (isAoaControlEnabled && aoaError != 0.0f) {
            *pidPitchOutput += interventionOffset;
            *pidPitchOutput = constrainf(*pidPitchOutput, -limit, +limit);
        }

        DEBUG_SET(DEBUG_AOA, 1, aoaError);
        DEBUG_SET(DEBUG_AOA, 2, *pidPitchOutput);
        DEBUG_SET(DEBUG_AOA, 3, aoaControlConfig()->fw_aoa_kp);
        DEBUG_SET(DEBUG_AOA, 4, interventionOffset);
        DEBUG_SET(DEBUG_AOA, 5, aoaServoOffset);
        DEBUG_SET(DEBUG_AOA, 6, aoaPidOutput);
        DEBUG_SET(DEBUG_AOA, 7, rateError);
    } else {
        // Normal layout: gradual intervention when approaching limits
        const float thresholdRatio = aoaControlConfig()->fw_aoa_intervention_threshold * 0.01f;
        const int16_t upperThreshold = upperLimitAngle * thresholdRatio;
        const int16_t lowerThreshold = lowerLimitAngle * thresholdRatio;
        const int16_t aoaDeg = DECIDEGREES_TO_DEGREES(filteredAoa);

        float aoaError = 0.0f;
        if (aoaDeg > upperThreshold) {
            aoaError = aoaDeg - upperThreshold;
        } else if (aoaDeg < lowerThreshold) {
            aoaError = aoaDeg - lowerThreshold;
        }

        aoaServoOffset = aoaError * kp * deg2pwm;

        aoaPidOutput = isAoaControlEnabled ? *pidPitchOutput + aoaServoOffset : *pidPitchOutput;
        aoaPidOutput = constrainf(aoaPidOutput, -limit, +limit);

        DEBUG_SET(DEBUG_AOA, 1, aoaError);
        DEBUG_SET(DEBUG_AOA, 2, *pidPitchOutput);
        DEBUG_SET(DEBUG_AOA, 3, aoaControlConfig()->fw_aoa_kp);
        DEBUG_SET(DEBUG_AOA, 4, aoaServoOffset);
        DEBUG_SET(DEBUG_AOA, 5, aoaServoOffset);
        DEBUG_SET(DEBUG_AOA, 6, aoaPidOutput);
        DEBUG_SET(DEBUG_AOA, 7, rateError);
    }
}

#endif
