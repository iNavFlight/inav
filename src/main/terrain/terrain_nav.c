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

#include "common/maths.h"

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

// Home terrain height, latched once the GPS origin is known and re-latched if
// the origin moves - the same relative datum the data layer uses for its own
// AGL, so map-datum errors cancel (docs: C-2)
static bool homeHeightValid;
static float homeHeightM;
static int32_t homeHeightLat;
static int32_t homeHeightLon;

static bool terrainNavUpdateHomeHeight(void)
{
    if (!posControl.gpsOrigin.valid) {
        homeHeightValid = false;
        return false;
    }
    if (homeHeightValid && homeHeightLat == posControl.gpsOrigin.lat && homeHeightLon == posControl.gpsOrigin.lon) {
        return true;
    }
    float heightM;
    homeHeightValid = terrainNavGetHomeTerrainHeight(&heightM);
    if (homeHeightValid) {
        homeHeightM = heightM;
        homeHeightLat = posControl.gpsOrigin.lat;
        homeHeightLon = posControl.gpsOrigin.lon;
    }
    return homeHeightValid;
}

// The aircraft's position as the estimator sees it (not the raw GPS fix), so
// the layer follows whatever INAV trusts for position
static bool terrainNavCurrentLocation(gpsLocation_t *llh)
{
    if (posControl.flags.estPosStatus < EST_USABLE) {
        return false;
    }
    return geoConvertLocalToGeodetic(llh, &posControl.gpsOrigin, &navGetCurrentActualPositionAndVelocity()->pos);
}

// Healthy = terrain enabled, the SD reader alive, a usable position estimate
// and the home terrain height known. Per-sample data availability is answered
// by the height queries themselves (no data = block not loaded yet)
bool terrainNavIsHealthy(void)
{
    if (!terrainConfig()->terrainEnabled || isTerrainIoFailure()) {
        return false;
    }
    if (posControl.flags.estPosStatus < EST_USABLE) {
        return false;
    }
    return terrainNavUpdateHomeHeight();
}

bool terrainNavGetAGLCm(int32_t *aglCm)
{
    if (!terrainNavIsHealthy()) {
        return false;
    }

    gpsLocation_t here;
    float heightHereM;
    if (!terrainNavCurrentLocation(&here) || !terrainNavGetHeightAtLocation(&here, &heightHereM)) {
        return false;
    }

    // AGL = estimated altitude above home minus the terrain rise above home
    const int32_t agl = (int32_t)navGetCurrentActualPositionAndVelocity()->pos.z
                        - (int32_t)((heightHereM - homeHeightM) * 100.0f);
    *aglCm = MAX(0, agl);
    return true;
}

bool terrainNavGetHeightAtLocation(const gpsLocation_t *loc, float *heightM)
{
    if (!terrainConfig()->terrainEnabled) {
        return false;
    }

    // The data layer's own lookup: block cache only, schedules the SD read
    // when the block is not loaded yet and reports no data meanwhile
    const float heightAmslM = getHeightAmslMeters(loc);
    if (heightAmslM == (float)TERRAIN_STATUS_NO_AMSL_DATA) {
        return false;
    }

    *heightM = heightAmslM;
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

bool terrainNavLookahead(float bearingDeg, float distanceM, float climbRatio, float escapeRatio, terrainNavLookaheadResult_t *result)
{
    if (result == NULL || !terrainNavIsHealthy()) {
        return false;
    }

    result->climbNeededM = 0.0f;
    result->escapeDeficitM = 0.0f;
    result->samplesTotal = 0;
    result->samplesMissed = 0;

    gpsLocation_t pos;
    if (!terrainNavCurrentLocation(&pos)) {
        return false;
    }
    float baseHeightM;
    if (!terrainNavGetHeightAtLocation(&pos, &baseHeightM)) {
        return false;
    }

    const float bearingRad = bearingDeg * DEG2RAD;
    const float stepNorthM = cosf(bearingRad) * TERRAIN_NAV_STEP_M;
    const float stepEastM = sinf(bearingRad) * TERRAIN_NAV_STEP_M;
    const float climbPerStepM = climbRatio * TERRAIN_NAV_STEP_M;
    const float escapePerStepM = escapeRatio * TERRAIN_NAV_STEP_M;

    float achievableClimbM = 0.0f;
    float escapeClimbM = 0.0f;

    for (float travelledM = TERRAIN_NAV_STEP_M; travelledM <= distanceM; travelledM += TERRAIN_NAV_STEP_M) {
        if (result->samplesTotal >= TERRAIN_NAV_LOOKAHEAD_MAX_SAMPLES) {
            break;
        }

        offsetLatlng(&pos, stepNorthM, stepEastM);
        achievableClimbM += climbPerStepM;
        escapeClimbM += escapePerStepM;
        result->samplesTotal++;

        float heightM;
        if (terrainNavGetHeightAtLocation(&pos, &heightM)) {
            const float riseM = heightM - baseHeightM;
            const float deficitM = riseM - achievableClimbM;
            if (deficitM > result->climbNeededM) {
                result->climbNeededM = deficitM;
            }
            const float escapeM = riseM - escapeClimbM;
            if (escapeM > result->escapeDeficitM) {
                result->escapeDeficitM = escapeM;
            }
        } else {
            result->samplesMissed++;
        }
    }

    return true;
}

#endif
