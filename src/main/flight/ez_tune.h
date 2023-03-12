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

#include "config/parameter_group.h"

typedef struct ezTuneSettings_s {
    uint8_t enabled;
    uint16_t filterHz;
    uint8_t axisRatio;
    uint8_t response;
    uint8_t damping;
    uint8_t stability;
    uint8_t aggressiveness;
} ezTuneSettings_t;

#define EZ_TUNE_PID_RP_DEFAULT {40, 75, 23, 100}
#define EZ_TUNE_PID_YAW_DEFAULT {45, 80, 0, 100}

PG_DECLARE_PROFILE(ezTuneSettings_t, ezTune);

void ezTuneUpdate(void);