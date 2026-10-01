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
#include <math.h>

#include "platform.h"

#ifdef USE_BEC_VOLTAGE

#include "common/filter.h"

#include "config/config_reset.h"
#include "config/parameter_group_ids.h"

#include "drivers/adc.h"
#include "drivers/time.h"

#include "fc/settings.h"

#include "io/motor_srxl2.h"

#include "sensors/bec.h"

PG_REGISTER_WITH_RESET_TEMPLATE(becConfig_t, becConfig, PG_BEC_CONFIG, 0);

PG_RESET_TEMPLATE(becConfig_t, becConfig,
    .scale = VBEC_SCALE_DEFAULT,
    .warningVoltage = SETTING_VBEC_WARNING_VOLTAGE_DEFAULT,
);

// Faster than the pack's 1 Hz, so a sag under a servo load still shows
#define BEC_LPF_HZ 5

static becSource_e becSource = BEC_SOURCE_NONE;
static uint16_t becVoltage;
static pt1Filter_t becFilter;
#ifdef USE_MOTOR_SRXL2
static uint16_t escVoltage;
static timeMs_t escVoltageMs;
#endif

static becSource_e becSample(uint16_t *voltage)
{
#ifdef USE_BEC_VOLTAGE_ADC
    if (adcIsFunctionAssigned(ADC_BEC)) {
        *voltage = (uint64_t)adcGetChannel(ADC_BEC) * becConfig()->scale * ADCVREF / (0xFFF * 1000);
        return BEC_SOURCE_ADC;
    }
#endif

#ifdef USE_MOTOR_SRXL2
    // The lowest of the Smart ESCs that report one, so a sagging BEC is never hidden by another
    bool found = false;
    for (uint8_t i = 0; i < srxl2MotorCount(); i++) {
        srxl2EscTelemetry_t t;
        if (srxl2MotorGetTelemetry(i, &t) && (t.fields & SRXL2_TELEM_FIELD_VOLTAGE_BEC)
                && (!found || t.voltageBec < *voltage)) {
            *voltage = t.voltageBec;
            found = true;
        }
    }
    if (found) {
        escVoltage = *voltage;
        escVoltageMs = millis();
        return BEC_SOURCE_ESC;
    }
    // 500 ms without a reply drops the ESC's telemetry, and its next reading can be a second away
    if (becSource == BEC_SOURCE_ESC && millis() - escVoltageMs < SRXL2_TELEM_STALE_MS) {
        *voltage = escVoltage;
        return BEC_SOURCE_ESC;
    }
#endif

    return BEC_SOURCE_NONE;
}

bool becIsConfigured(void)
{
#ifdef USE_BEC_VOLTAGE_ADC
    if (adcIsFunctionAssigned(ADC_BEC)) {
        return true;
    }
#endif
#ifdef USE_MOTOR_SRXL2
    if (srxl2MotorCount() > 0) {
        return true;
    }
#endif
    return false;
}

void becUpdate(timeDelta_t timeDelta)
{
    uint16_t sample = 0;
    const becSource_e source = becSample(&sample);

    if (source != becSource) {
        // A reading that appears or changes source starts the filter afresh
        becSource = source;
        pt1FilterSetCutoff(&becFilter, BEC_LPF_HZ);
        pt1FilterReset(&becFilter, sample);
        becVoltage = sample;
        return;
    }
    becVoltage = lrintf(pt1FilterApply3(&becFilter, sample, US2S(timeDelta)));
}

becSource_e becGetSource(void)
{
    return becSource;
}

uint16_t becGetVoltage(void)
{
    return becVoltage;
}

bool becIsVoltageLow(void)
{
    return becSource != BEC_SOURCE_NONE && becConfig()->warningVoltage && becVoltage < becConfig()->warningVoltage;
}

#endif
