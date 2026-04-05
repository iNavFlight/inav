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

#include "platform.h"

#include "sensors/rpm_source.h"

#include "drivers/pwm_output.h"

#include "fc/runtime_config.h"

#include "flight/mixer.h"

#ifdef USE_ESC_SENSOR
#include "sensors/esc_sensor.h"
#endif

static uint32_t dshotBidirRpm[MAX_SUPPORTED_MOTORS];
static uint8_t dshotBidirDataAge[MAX_SUPPORTED_MOTORS];

bool rpmSourceIsDshotBidirConfigured(void)
{
#ifdef USE_DSHOT_BIDIR
    return isDshotBidirModeActive();
#else
    return false;
#endif
}

bool rpmSourceIsConfigured(void)
{
    if (rpmSourceIsDshotBidirConfigured()) {
        return true;
    }

#ifdef USE_ESC_SENSOR
    return STATE(ESC_SENSOR_ENABLED);
#else
    return false;
#endif
}

uint8_t rpmSourceGetDshotBidirValidCount(void)
{
    uint8_t validCount = 0;

    for (uint8_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        if (dshotBidirDataAge[i] < RPM_SOURCE_DATA_INVALID) {
            validCount++;
        }
    }

    return validCount;
}

bool rpmSourceIsDshotBidirActive(void)
{
    return rpmSourceGetDshotBidirValidCount() > 0;
}

bool rpmSourceGetAverageRpm(uint32_t *rpm)
{
    if (!rpm) {
        return false;
    }

    uint32_t rpmAccumulator = 0;
    uint8_t validMotorCount = 0;

    for (uint8_t i = 0; i < getMotorCount(); i++) {
        uint32_t motorRpm;

        if (!rpmSourceGetMotorRpm(i, &motorRpm)) {
            continue;
        }

        rpmAccumulator += motorRpm;
        validMotorCount++;
    }

    if (!validMotorCount) {
        return false;
    }

    *rpm = rpmAccumulator / validMotorCount;
    return true;
}

bool rpmSourceGetMotorRpm(uint8_t motor, uint32_t *rpm)
{
    if (motor >= MAX_SUPPORTED_MOTORS || !rpm) {
        return false;
    }

    if (rpmSourceIsDshotBidirConfigured() && dshotBidirDataAge[motor] < RPM_SOURCE_DATA_INVALID) {
        *rpm = dshotBidirRpm[motor];
        return true;
    }

#ifdef USE_ESC_SENSOR
    if (STATE(ESC_SENSOR_ENABLED)) {
        const escSensorData_t *escState = getEscTelemetry(motor);

        if (escState != NULL && escState->dataAge < ESC_DATA_INVALID) {
            *rpm = escState->rpm;
            return true;
        }
    }
#endif

    return false;
}

void rpmSourceResetDshotBidir(void)
{
    for (uint8_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        dshotBidirRpm[i] = 0;
        dshotBidirDataAge[i] = RPM_SOURCE_DATA_INVALID;
    }
}

void rpmSourceSetDshotBidirRpm(uint8_t motor, uint32_t rpm)
{
    if (motor >= MAX_SUPPORTED_MOTORS) {
        return;
    }

    dshotBidirRpm[motor] = rpm;
    dshotBidirDataAge[motor] = 0;
}

void rpmSourceInvalidateDshotBidir(uint8_t motor)
{
    if (motor >= MAX_SUPPORTED_MOTORS) {
        return;
    }

    if (dshotBidirDataAge[motor] < RPM_SOURCE_DATA_INVALID) {
        dshotBidirDataAge[motor]++;
    }
}
