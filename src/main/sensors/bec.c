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

#include "fc/settings.h"

#include "sensors/bec.h"

PG_REGISTER_WITH_RESET_TEMPLATE(becConfig_t, becConfig, PG_BEC_CONFIG, 0);

PG_RESET_TEMPLATE(becConfig_t, becConfig,
    .scale = VBEC_SCALE_DEFAULT,
    .warningVoltage = SETTING_VBEC_WARNING_VOLTAGE_DEFAULT,
);

// Faster than the pack's 1 Hz, so a sag under a servo load still shows
#define BEC_LPF_HZ 5

static uint16_t becVoltage;
static pt1Filter_t becFilter;
static bool becFilterStarted;

bool becIsConfigured(void)
{
    return adcIsFunctionAssigned(ADC_BEC);
}

void becUpdate(timeDelta_t timeDelta)
{
    if (!becIsConfigured()) {
        return;
    }

    const uint16_t sample = (uint64_t)adcGetChannel(ADC_BEC) * becConfig()->scale * ADCVREF / (0xFFF * 1000);
    if (!becFilterStarted) {
        pt1FilterSetCutoff(&becFilter, BEC_LPF_HZ);
        pt1FilterReset(&becFilter, sample);
        becFilterStarted = true;
    }
    becVoltage = lrintf(pt1FilterApply3(&becFilter, sample, US2S(timeDelta)));
}

uint16_t becGetVoltage(void)
{
    return becVoltage;
}

bool becIsVoltageLow(void)
{
    return becIsConfigured() && becConfig()->warningVoltage && becVoltage < becConfig()->warningVoltage;
}

#endif
