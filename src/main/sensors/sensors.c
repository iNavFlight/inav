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

#include "platform.h"

#include "build/debug.h"

#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "io/beeper.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/sensors.h"

sensor_compensation_t sensor_comp_data[SENSOR_INDEX_COUNT];

float applySensorTempCompensation(int16_t sensorTemp, float sensorMeasurement, sensorIndex_e sensorType)
{
    float setting = 0.0f;
    if (sensorType == SENSOR_INDEX_ACC) {
        setting = accelerometerConfig()->acc_temp_correction;
    }
#ifdef USE_BARO
    else if (sensorType == SENSOR_INDEX_BARO) {
        setting = barometerConfig()->baro_temp_correction;
    }
#endif
    if (!setting) {
        return 0.0f;
    }

    if (sensor_comp_data[sensorType].calibrationState == SENSOR_TEMP_CAL_COMPLETE) {
        return sensor_comp_data[sensorType].correctionFactor * CENTIDEGREES_TO_DEGREES(sensor_comp_data[sensorType].referenceTemp - sensorTemp);
    }

    static timeMs_t startTimeMs = 0;

    if (!ARMING_FLAG(WAS_EVER_ARMED)) {
        if (sensor_comp_data[sensorType].calibrationState == SENSOR_TEMP_CAL_INITIALISE) {
            sensor_comp_data[sensorType].referenceTemp = sensorTemp;
            sensor_comp_data[sensorType].calibrationState = SENSOR_TEMP_CAL_IN_PROGRESS;
        }

        if (setting == 51.0f) {   // initiate auto calibration
            if (sensor_comp_data[sensorType].referenceTemp == sensorTemp) {
                sensor_comp_data[sensorType].referenceMeasurement = sensorMeasurement;
                sensor_comp_data[sensorType].lastTemp = sensorTemp;
                startTimeMs = millis();
            }

            float referenceDeltaTemp = ABS(sensorTemp - sensor_comp_data[sensorType].referenceTemp);    // centidegrees
            if (referenceDeltaTemp > 300 && referenceDeltaTemp > ABS(sensor_comp_data[sensorType].lastTemp - sensor_comp_data[sensorType].referenceTemp)) {
                /* Min 3 deg reference temperature difference required for valid calibration.
                 * Correction adjusted only if temperature difference to reference temperature increasing
                 * Calibration assumes a simple linear relationship */
                sensor_comp_data[sensorType].lastTemp = sensorTemp;
                sensor_comp_data[sensorType].correctionFactor = 0.9f * sensor_comp_data[sensorType].correctionFactor + 0.1f * (sensorMeasurement - sensor_comp_data[sensorType].referenceMeasurement) / CENTIDEGREES_TO_DEGREES(sensor_comp_data[sensorType].lastTemp - sensor_comp_data[sensorType].referenceTemp);
                sensor_comp_data[sensorType].correctionFactor = constrainf(sensor_comp_data[sensorType].correctionFactor, -50.0f, 50.0f);
            }
        } else {
            sensor_comp_data[sensorType].correctionFactor = setting;
            sensor_comp_data[sensorType].calibrationState = SENSOR_TEMP_CAL_COMPLETE;
        }
    }

    // Calibration ends on first Arm or after 5 min timeout
    if (sensor_comp_data[sensorType].calibrationState == SENSOR_TEMP_CAL_IN_PROGRESS && (ARMING_FLAG(WAS_EVER_ARMED) || millis() > startTimeMs + 300000)) {
        if (!ARMING_FLAG(WAS_EVER_ARMED)) {
            beeper(sensor_comp_data[sensorType].correctionFactor ? BEEPER_ACTION_SUCCESS : BEEPER_ACTION_FAIL);
        }

        if (sensorType == SENSOR_INDEX_ACC) {
            accelerometerConfigMutable()->acc_temp_correction = sensor_comp_data[sensorType].correctionFactor;
        }
#ifdef USE_BARO
        else if (sensorType == SENSOR_INDEX_BARO) {
            barometerConfigMutable()->baro_temp_correction = sensor_comp_data[sensorType].correctionFactor;
        }
#endif
        sensor_comp_data[sensorType].calibrationState = SENSOR_TEMP_CAL_COMPLETE;
        startTimeMs = 0;
    }

    return 0.0f;
}