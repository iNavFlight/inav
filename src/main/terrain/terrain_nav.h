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

#include <stdbool.h>
#include <stdint.h>

#include "io/gps.h"

/*
 * Navigation-facing terrain API.
 *
 * Contract for every function here: success/failure is an explicit boolean,
 * values are returned through out-parameters and are written ONLY on success.
 * No caller ever sees a sentinel, a stale value or a substituted value.
 */

typedef struct {
    float climbNeededM;     // extra climb needed now to clear terrain along the path (>= 0)
    float escapeDeficitM;   // worst height deficit vs the escapeRatio slope (>= 0) - the escape test's raw input
    uint8_t samplesTotal;   // points sampled along the path
    uint8_t samplesMissed;  // points that had no terrain data (loads were scheduled)
    bool complete;          // true when every sampled point had terrain data
} terrainNavLookaheadResult_t;

// True only while terrain data is usable: module enabled, no IO failure
// latched, home anchor found and data fresher than TERRAIN_NO_DATA_DELAY_MS
bool terrainNavIsHealthy(void);

// Height above ground at the current position, in cm
bool terrainNavGetAGLCm(int32_t *aglCm);

// Terrain height (map datum, meters) at an arbitrary location. On a cache
// miss the block is scheduled for loading and the call fails - retry later.
bool terrainNavGetHeightAtLocation(const gpsLocation_t *loc, float *heightM);

// Terrain height (map datum, meters) at the GPS origin (home)
bool terrainNavGetHomeTerrainHeight(float *heightM);

// Walk the map from the current position along bearingDeg for distanceM,
// accumulating achievable climb (climbRatio = m of climb per m travelled),
// and report the worst height deficit found. A descending path (negative
// climbRatio) correctly increases the reported deficit. Sampled blocks not
// yet cached are scheduled for loading and counted in samplesMissed.
// One walk, two slopes: climbNeededM is the deficit vs climbRatio (the
// early-climb demand model), escapeDeficitM the deficit vs escapeRatio (the
// full-capability escape test) - same samples, no extra SD load
bool terrainNavLookahead(float bearingDeg, float distanceM, float climbRatio, float escapeRatio, terrainNavLookaheadResult_t *result);
