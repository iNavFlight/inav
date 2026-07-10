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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_TERRAIN

#include "terrain.h"
#include "terrain_utils.h"
#include "terrain_io.h"
#include "terrain_location.h"
#include "terrain_nav.h"

#include "io/gps.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

// The module's grid spacing is fixed at 30 m (terrain_utils.c); sampling at
// the same step never skips a grid cell along the path
#define TERRAIN_NAV_STEP_M 30.0f

// Bound the work (and cache pressure) of a single lookahead call: 64 samples
// at 30 m cover ~1.9 km ahead, well beyond a practical climb-planning horizon
#define TERRAIN_NAV_LOOKAHEAD_MAX_SAMPLES 64

bool terrainNavIsHealthy(void)
{
    if (!terrainConfig()->terrainEnabled || isTerrainIoFailure()) {
        return false;
    }

    // terrainGetLastAMSL returns its no-data sentinel unless the home anchor
    // is latched AND the last update is fresher than TERRAIN_NO_DATA_DELAY_MS
    return terrainGetLastAMSL() != TERRAIN_STATUS_NO_AMSL_DATA;
}

bool terrainNavGetAGLCm(int32_t *aglCm)
{
    if (!terrainNavIsHealthy()) {
        return false;
    }

    const int32_t agl = terrainGetLastDistanceCm();
    if (agl == TERRAIN_STATUS_NO_AGL_DATA) {
        return false;
    }

    *aglCm = agl;
    return true;
}

bool terrainNavGetHeightAtLocation(const gpsLocation_t *loc, float *heightM)
{
    if (!terrainConfig()->terrainEnabled || isTerrainIoFailure()) {
        return false;
    }

    gridInfo_t info;
    calculateGridInfo(loc, &info);

    // the +1 neighbours used for interpolation must lie inside the same block
    if (info.idx_x > TERRAIN_GRID_BLOCK_SIZE_X - 2 || info.idx_y > TERRAIN_GRID_BLOCK_SIZE_Y - 2) {
        return false;
    }

    gridCache_t *cache = findGridCache(&info);
    if (cache == NULL) {
        return false;
    }
    gridBlock_t *grid = &cache->gridBlock;

    // all four surrounding grid points must have been loaded from the SD card
    if (!checkBitmap(grid, info.idx_x, info.idx_y) ||
        !checkBitmap(grid, info.idx_x, info.idx_y + 1) ||
        !checkBitmap(grid, info.idx_x + 1, info.idx_y) ||
        !checkBitmap(grid, info.idx_x + 1, info.idx_y + 1)) {
        markGridBlockNeedRead(grid);
        return false;
    }

    const int16_t h00 = grid->height[info.idx_x][info.idx_y];
    const int16_t h01 = grid->height[info.idx_x][info.idx_y + 1];
    const int16_t h10 = grid->height[info.idx_x + 1][info.idx_y];
    const int16_t h11 = grid->height[info.idx_x + 1][info.idx_y + 1];

    const float avg1 = (1.0f - info.frac_x) * h00 + info.frac_x * h10;
    const float avg2 = (1.0f - info.frac_x) * h01 + info.frac_x * h11;

    *heightM = (1.0f - info.frac_y) * avg1 + info.frac_y * avg2;
    return true;
}

bool terrainNavGetHomeTerrainHeight(float *heightM)
{
    if (!posControl.gpsOrigin.valid) {
        return false;
    }

    const gpsLocation_t homeLoc = {
        .lat = posControl.gpsOrigin.lat,
        .lon = posControl.gpsOrigin.lon,
        .alt = 0,
    };

    return terrainNavGetHeightAtLocation(&homeLoc, heightM);
}

bool terrainNavLookahead(float bearingDeg, float distanceM, float climbRatio, terrainNavLookaheadResult_t *result)
{
    if (result == NULL || !terrainNavIsHealthy()) {
        return false;
    }

    result->climbNeededM = 0.0f;
    result->samplesTotal = 0;
    result->samplesMissed = 0;
    result->complete = true;

    gpsLocation_t pos = gpsSol.llh;
    float baseHeightM;
    if (!terrainNavGetHeightAtLocation(&pos, &baseHeightM)) {
        return false;
    }

    const float bearingRad = bearingDeg * DEG2RAD;
    const float stepNorthM = cosf(bearingRad) * TERRAIN_NAV_STEP_M;
    const float stepEastM = sinf(bearingRad) * TERRAIN_NAV_STEP_M;
    const float climbPerStepM = climbRatio * TERRAIN_NAV_STEP_M;

    float achievableClimbM = 0.0f;

    for (float travelledM = TERRAIN_NAV_STEP_M; travelledM <= distanceM; travelledM += TERRAIN_NAV_STEP_M) {
        if (result->samplesTotal >= TERRAIN_NAV_LOOKAHEAD_MAX_SAMPLES) {
            break;
        }

        offsetLatlng(&pos, stepNorthM, stepEastM);
        achievableClimbM += climbPerStepM;
        result->samplesTotal++;

        float heightM;
        if (terrainNavGetHeightAtLocation(&pos, &heightM)) {
            const float deficitM = (heightM - baseHeightM) - achievableClimbM;
            if (deficitM > result->climbNeededM) {
                result->climbNeededM = deficitM;
            }
        } else {
            result->samplesMissed++;
        }
    }

    result->complete = (result->samplesMissed == 0);
    return true;
}

#endif
