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

#include "platform.h"

#ifdef USE_TERRAIN

#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"
#include "flight/pid.h"

#include "io/gps.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "terrain/terrain_nav.h"
#include "terrain/terrain_nav_hold.h"

PG_REGISTER_WITH_RESET_TEMPLATE(terrainNavConfig_t, terrainNavConfig, PG_TERRAIN_NAV_CONFIG, 0);

PG_RESET_TEMPLATE(terrainNavConfig_t, terrainNavConfig,
    .minAglCm = 6000,       // 60 m: worst-case map error in steep terrain + grid blind spots + canopy
    .lookaheadDistM = 1000,
);

// Terrain queries run at this interval; the block budget below is defined
// per query cycle (the cache starves when a cycle exceeds it - C-9)
#define TERRAIN_NAV_HOLD_QUERY_INTERVAL_MS 100

// Worst-case meters of straight flight per newly touched cache block: block
// stride is 720 x 840 m ground (24 x 28 points at 30 m), the worst bearing
// crosses a block boundary every 1/sqrt(1/720^2 + 1/840^2) = 546 m
#define TERRAIN_NAV_HOLD_M_PER_BLOCK_WORST 540

// Below this ground speed the course over ground is too noisy for a lookahead
#define TERRAIN_NAV_HOLD_MIN_LOOKAHEAD_SPEED_CM_S 300

static terrainNavHoldState_t holdState;
static terrainNavHoldOutput_t holdOutput;

// Latest terrain answers, refreshed once per query interval
static timeMs_t lastQueryTimeMs;
static bool queryAglValid;
static int32_t queryAglCm;
static bool queryLookaheadValid;
static float queryLookaheadClimbCm;

// The hard-disengage conditions: launch, any landing, emergency landing,
// VTOL transition, non-altitude platforms, degraded GPS, rangefinder SURFACE
// mode. The cruise whitelist is the primary defense - this is the explicit
// override on top of it
static bool terrainNavHoldMustDisengage(void)
{
    if (navGetCurrentStateFlags() & (NAV_CTL_LAUNCH | NAV_CTL_LAND | NAV_CTL_EMERG | NAV_MIXERAT)) {
        return true;
    }

    if (FLIGHT_MODE(NAV_LAUNCH_MODE) || FLIGHT_MODE(NAV_FW_AUTOLAND)) {
        return true;
    }

    if (!STATE(ALTITUDE_CONTROL)) {
        return true;
    }

    if (!STATE(GPS_FIX)) {
        return true;
    }

#ifdef USE_GPS_FIX_ESTIMATION
    if (STATE(GPS_ESTIMATED_FIX)) {
        return true;
    }
#endif

    // Never stack with the rangefinder surface mode
    if (posControl.flags.isTerrainFollowEnabled) {
        return true;
    }

    return false;
}

static bool terrainNavHoldEligible(void)
{
    if (!ARMING_FLAG(ARMED) || !STATE(AIRPLANE)) {
        return false;
    }

    if (!IS_RC_MODE_ACTIVE(BOXTERRAINAGLHOLD)) {
        return false;
    }

    // The whitelist: 3D cruise only
    if (posControl.navState != NAV_STATE_CRUISE_IN_PROGRESS && posControl.navState != NAV_STATE_CRUISE_ADJUSTING) {
        return false;
    }

    return !terrainNavHoldMustDisengage();
}

static void terrainNavHoldRunQueries(timeMs_t currentTimeMs)
{
    lastQueryTimeMs = currentTimeMs;

    queryAglValid = terrainNavGetAGLCm(&queryAglCm);
    queryLookaheadValid = false;

    // No course to scan along: lookahead disabled by config, too slow for a
    // usable course over ground, or the heading estimate itself is invalid
    // (the cog would be stale - never scan a direction we cannot trust)
    if (terrainNavConfig()->lookaheadDistM == 0 || gpsSol.groundSpeed < TERRAIN_NAV_HOLD_MIN_LOOKAHEAD_SPEED_CM_S || !isImuHeadingValid()) {
        return;
    }

    // Global block budget per query cycle: cache size minus 2 (C-9). The AGL
    // query above uses the current block, so the lookahead may touch at most
    // cacheSize - 3 new blocks - its distance is capped accordingly
    const int32_t lookaheadBudgetM = (TERRAIN_GRID_BLOCK_CACHE_SIZE - 3) * TERRAIN_NAV_HOLD_M_PER_BLOCK_WORST;
    const float lookaheadDistM = MIN((int32_t)terrainNavConfig()->lookaheadDistM, lookaheadBudgetM);

    // Conservative achievable climb slope: half the configured climb rate at
    // the current ground speed - underestimating it makes climbs start early
    const float groundSpeedCmS = MAX(gpsSol.groundSpeed, 800);
    const float climbRatio = 0.5f * navConfig()->fw.max_auto_climb_rate / groundSpeedCmS;

    const float bearingDeg = CENTIDEGREES_TO_DEGREES((float)posControl.actualState.cog);

    terrainNavLookaheadResult_t lookahead;
    if (terrainNavLookahead(bearingDeg, lookaheadDistM, climbRatio, &lookahead)) {
        // A partial answer (samples missed at the far end) is still a valid
        // lower bound; the missed blocks are already scheduled for loading
        queryLookaheadValid = true;
        queryLookaheadClimbCm = lookahead.climbNeededM * 100.0f;
    }
}

void terrainNavCruiseHoldUpdate(void)
{
    const timeMs_t currentTimeMs = millis();

    terrainNavHoldInput_t in = { 0 };
    in.nowMs = currentTimeMs;
    in.eligible = terrainNavHoldEligible();

    if (!in.eligible) {
        // Resets the core state; cruise and every other mode fly as stock
        terrainNavHoldCoreUpdate(&holdState, &in, &holdOutput);
        return;
    }

    if (currentTimeMs - lastQueryTimeMs >= TERRAIN_NAV_HOLD_QUERY_INTERVAL_MS) {
        terrainNavHoldRunQueries(currentTimeMs);
    }

    in.stickAdjusting = posControl.flags.isAdjustingAltitude;
    in.healthy = terrainNavIsHealthy();
    in.aglValid = queryAglValid;
    in.aglCm = queryAglCm;
    in.currentZCm = navGetCurrentActualPositionAndVelocity()->pos.z;
    in.lookaheadValid = queryLookaheadValid;
    in.lookaheadClimbCm = queryLookaheadClimbCm;
    in.lookaheadDegraded = terrainNavConfig()->lookaheadDistM != 0 && !isImuHeadingValid();
    in.minAglCm = terrainNavConfig()->minAglCm;
    in.maxAltCm = navConfig()->general.max_altitude;

    // Handover blend inputs. The blend seed is the aircraft's real vertical
    // speed, bounded by the same authority limit as every hold command. The
    // stick's wish is read back from the funnel - the stock stick writer ran
    // earlier this same cycle (fc_core.c ordering) - with the same manual
    // clamp getDesiredClimbRate applies to it
    in.actualClimbRateCmS = constrainf(navGetCurrentActualPositionAndVelocity()->vel.z,
                                       -(float)navConfig()->fw.max_auto_climb_rate,
                                       (float)navConfig()->fw.max_auto_climb_rate);
    if (in.stickAdjusting && posControl.flags.rocToAltMode == ROC_TO_ALT_CONSTANT) {
        in.stickWishValid = true;
        in.stickWishCmS = constrainf(posControl.desiredState.climbRateDemand,
                                     -(float)navConfig()->fw.max_manual_climb_rate,
                                     (float)navConfig()->fw.max_manual_climb_rate);
    }

    terrainNavHoldCoreUpdate(&holdState, &in, &holdOutput);

    if (holdOutput.writeTarget) {
        // The one gate into the altitude target path. The funnel enforces the
        // slew limit (climb rate) and the nav_max_altitude clamp downstream
        updateClimbRateToAltitudeController(navConfig()->fw.max_auto_climb_rate, holdOutput.targetZCm, ROC_TO_ALT_TARGET);
    } else if (holdOutput.writeRate) {
        // Handover blend: a signed climb rate expressed through the funnel's
        // own target math (rate = response_factor * altitude_error / 100),
        // so the command is smooth in both directions and the ceiling clamp
        // still applies downstream. The rate itself is already bounded by
        // max_auto_climb_rate through the blend inputs above
        const float responseFactor = MAX(pidProfile()->fwAltControlResponseFactor, 1);
        const float blendTargetZCm = navGetCurrentActualPositionAndVelocity()->pos.z
                                     + holdOutput.rateCmS * 100.0f / responseFactor;
        updateClimbRateToAltitudeController(navConfig()->fw.max_auto_climb_rate, blendTargetZCm, ROC_TO_ALT_TARGET);
    }
}

terrainNavHoldStatus_e terrainNavHoldGetStatus(void)
{
    return holdState.status;
}

terrainNavHoldWarning_e terrainNavHoldGetWarning(void)
{
    return holdOutput.warning;
}

bool terrainNavHoldIsEngaged(void)
{
    return holdState.status == TERRAIN_NAV_HOLD_ACTIVE || holdState.status == TERRAIN_NAV_HOLD_FROZEN;
}

bool terrainNavHoldGetTargetAglCm(int32_t *targetAglCm)
{
    if (!terrainNavHoldIsEngaged()) {
        return false;
    }

    *targetAglCm = holdState.targetAglCm;
    return true;
}

#endif
