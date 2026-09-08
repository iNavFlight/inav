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

#include "navigation/navigation.h"

#define LOCATION_SCALING_FACTOR 0.011131884502145034f   // m per 1e-7 degree
#define LOCATION_SCALING_FACTOR_INV 89.83204953368922f // 1 / LOCATION_SCALING_FACTOR;

#define DEG2RAD 0.01745329252f

#define TERRAIN_LATLON_EQUAL(v1, v2) ((unsigned long)labs((v1) - (v2)) <= 500UL)

typedef struct {
    float north;
    float east;
} neVector_t;

void offsetLatlng(gpsLocation_t *loc, float north_m, float east_m);
neVector_t gpsGetDistanceNE(const gpsLocation_t *a, const gpsLocation_t *b);