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
 * 
 * This code is a derivative of work done in EmuFlight Project https://github.com/emuflight/EmuFlight
 * 
 */

#pragma once

#include <stdint.h>
#include "common/filter.h"

#define MAX_SMITH_SAMPLES 64

typedef struct smithPredictor_s {
    bool enabled;
    uint8_t samples;
    uint8_t idx;
    float data[MAX_SMITH_SAMPLES + 1];
    pt1Filter_t smithPredictorFilter;
    float measurementSmithPredictor;
} smithPredictor_t;

float smithPredictorApply(smithPredictor_t *predictor, float sample);
void smithPredictorInit(smithPredictor_t *predictor, float delay, float strength, uint16_t filterLpfHz, uint32_t looptime);