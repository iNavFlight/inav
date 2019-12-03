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

#include "flight/rpm_filter.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/utils.h"
#include "common/maths.h"
#include "common/filter.h"
#include "flight/mixer.h"
#include "sensors/esc_sensor.h"
#include "fc/config.h"

#ifdef USE_RPM_FILTER

#define RPM_TO_HZ 60.0f
#define RPM_FILTER_RPM_LPF_HZ 150
#define RPM_FILTER_HARMONICS 3

PG_REGISTER_WITH_RESET_TEMPLATE(rpmFilterConfig_t, rpmFilterConfig, PG_RPM_FILTER_CONFIG, 0);

PG_RESET_TEMPLATE(rpmFilterConfig_t, rpmFilterConfig,
                  .gyro_filter_enabled = 0,
                  .dterm_filter_enabled = 0,
                  .gyro_harmonics = 1,
                  .gyro_min_hz = 100,
                  .gyro_q = 500,
                  .dterm_harmonics = 1,
                  .dterm_min_hz = 100,
                  .dterm_q = 500, );

typedef struct
{
    float q;
    float minHz;
    float maxHz;
    uint8_t harmonics;
    biquadFilter_t filters[XYZ_AXIS_COUNT][MAX_SUPPORTED_MOTORS][RPM_FILTER_HARMONICS];
} rpmFilterBank_t;

typedef float (*rpmFilterApplyFnPtr)(rpmFilterBank_t *filter, uint8_t axis, float input);
typedef void (*rpmFilterUpdateFnPtr)(rpmFilterBank_t *filterBank, uint8_t motor, float baseFrequency);

static EXTENDED_FASTRAM pt1Filter_t motorFrequencyFilter[MAX_SUPPORTED_MOTORS];
static EXTENDED_FASTRAM float erpmToHz;
static EXTENDED_FASTRAM rpmFilterBank_t gyroRpmFilters;
static EXTENDED_FASTRAM rpmFilterBank_t dtermRpmFilters;
static EXTENDED_FASTRAM rpmFilterApplyFnPtr rpmGyroApplyFn;
static EXTENDED_FASTRAM rpmFilterApplyFnPtr rpmDtermApplyFn;
static EXTENDED_FASTRAM rpmFilterUpdateFnPtr rpmGyroUpdateFn;
static EXTENDED_FASTRAM rpmFilterUpdateFnPtr rpmDtermUpdateFn;

float nullRpmFilterApply(rpmFilterBank_t *filter, uint8_t axis, float input)
{
    UNUSED(filter);
    UNUSED(axis);
    return input;
}

void nullRpmFilterUpdate(rpmFilterBank_t *filterBank, uint8_t motor, float baseFrequency) {
    UNUSED(filterBank);
    UNUSED(motor);
    UNUSED(baseFrequency);
}

float FAST_CODE rpmFilterApply(rpmFilterBank_t *filterBank, uint8_t axis, float input)
{
    float output = input;

    for (uint8_t motor = 0; motor < getMotorCount(); motor++)
    {
        for (int harmonicIndex = 0; harmonicIndex < filterBank->harmonics; harmonicIndex++)
        {
            output = biquadFilterApplyDF1(
                &filterBank->filters[axis][motor][harmonicIndex],
                output
            );
        }
    }

    return output;
}

static void rpmFilterInit(rpmFilterBank_t *filter, uint16_t q, uint8_t minHz, uint8_t harmonics)
{
    filter->q = q / 100.0f;
    filter->minHz = minHz;
    filter->harmonics = harmonics;
    /*
     * Max frequency has to be lower than Nyquist frequency for looptime
     */
    filter->maxHz = 0.48f * 1000000.0f / getLooptime();

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++)
    {
        for (int motor = 0; motor < getMotorCount(); motor++)
        {

            /*
             * Harmonics are indexed from 1 where 1 means base frequency
             * C indexes arrays from 0, so we need to shift
             */
            for (int harmonicIndex = 0; harmonicIndex < harmonics; harmonicIndex++)
            {
                biquadFilterInit(
                    &filter->filters[axis][motor][harmonicIndex],
                    filter->minHz * (harmonicIndex + 1),
                    getLooptime(),
                    filter->q,
                    FILTER_NOTCH);
            }
        }
    }
}

void disableRpmFilters(void) {
    rpmGyroApplyFn = (rpmFilterApplyFnPtr)nullRpmFilterApply;
    rpmDtermApplyFn = (rpmFilterApplyFnPtr)nullRpmFilterApply;
}

void FAST_CODE NOINLINE rpmFilterUpdate(rpmFilterBank_t *filterBank, uint8_t motor, float baseFrequency)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++)
    {
        for (int harmonicIndex = 0; harmonicIndex < filterBank->harmonics; harmonicIndex++)
        {
            float harmonicFrequency = baseFrequency * (harmonicIndex + 1);
            harmonicFrequency = constrainf(harmonicFrequency, filterBank->minHz, filterBank->maxHz);

            biquadFilterUpdate(
                &filterBank->filters[axis][motor][harmonicIndex],
                harmonicFrequency,
                getLooptime(),
                filterBank->q,
                FILTER_NOTCH);
        }
    }
}

void rpmFiltersInit(void)
{
    for (uint8_t i = 0; i < MAX_SUPPORTED_MOTORS; i++)
    {
        pt1FilterInit(&motorFrequencyFilter[i], RPM_FILTER_RPM_LPF_HZ, RPM_FILTER_UPDATE_RATE_US * 1e-6f);
    }
    erpmToHz = ERPM_PER_LSB / (motorConfig()->motorPoleCount / 2) / RPM_TO_HZ;

    rpmGyroUpdateFn = (rpmFilterUpdateFnPtr)nullRpmFilterUpdate;
    rpmDtermUpdateFn = (rpmFilterUpdateFnPtr)nullRpmFilterUpdate;

    if (rpmFilterConfig()->gyro_filter_enabled)
    {
        rpmFilterInit(
            &gyroRpmFilters,
            rpmFilterConfig()->gyro_q,
            rpmFilterConfig()->gyro_min_hz,
            rpmFilterConfig()->gyro_harmonics);
        rpmGyroApplyFn = (rpmFilterApplyFnPtr)rpmFilterApply;
        rpmGyroUpdateFn = (rpmFilterUpdateFnPtr)rpmFilterUpdate;
    }

    if (rpmFilterConfig()->dterm_filter_enabled)
    {
        rpmFilterInit(
            &dtermRpmFilters,
            rpmFilterConfig()->dterm_q,
            rpmFilterConfig()->dterm_min_hz,
            rpmFilterConfig()->dterm_harmonics);
        rpmDtermApplyFn = (rpmFilterApplyFnPtr)rpmFilterApply;
        rpmDtermUpdateFn = (rpmFilterUpdateFnPtr)rpmFilterUpdate;
    }
}

void FAST_CODE NOINLINE rpmFilterUpdateTask(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    uint8_t motorCount = getMotorCount();
    /*
     * For each motor, read ERPM, filter it and update motor frequency
     */
    for (uint8_t i = 0; i < motorCount; i++)
    {
        const escSensorData_t *escState = getEscTelemetry(i); //Get ESC telemetry
        const float baseFrequency = pt1FilterApply(&motorFrequencyFilter[i], escState->rpm * erpmToHz); //Filter motor frequency

        if (i < 4) {
            DEBUG_SET(DEBUG_RPM_FREQ, i, (int)baseFrequency);
        }

        rpmGyroUpdateFn(&gyroRpmFilters, i, baseFrequency);
        rpmDtermUpdateFn(&dtermRpmFilters, i, baseFrequency);
    }
}

float FAST_CODE rpmFilterGyroApply(uint8_t axis, float input)
{
    return rpmGyroApplyFn(&gyroRpmFilters, axis, input);
}

float FAST_CODE rpmFilterDtermApply(uint8_t axis, float input)
{
    return rpmDtermApplyFn(&dtermRpmFilters, axis, input);
}

#endif