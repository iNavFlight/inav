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

#include "common/time.h"
#include "config/parameter_group.h"

#ifndef VBEC_SCALE_DEFAULT
#define VBEC_SCALE_DEFAULT 1100
#endif

typedef struct becConfig_s {
    uint16_t scale;             // Divider ratio x 100, as vbat_scale
    uint16_t warningVoltage;    // 0.01 V, 0 for no warning
} becConfig_t;

PG_DECLARE(becConfig_t, becConfig);

typedef enum {
    BEC_SOURCE_NONE,
    BEC_SOURCE_ADC,
    BEC_SOURCE_ESC,
} becSource_e;

bool becIsConfigured(void);
void becUpdate(timeDelta_t timeDelta);
becSource_e becGetSource(void);
uint16_t becGetVoltage(void);   // 0.01 V, while becGetSource() is not BEC_SOURCE_NONE
bool becIsVoltageLow(void);
