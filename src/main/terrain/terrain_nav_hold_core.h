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

/*
 * TERRAIN AGL HOLD decision core.
 *
 * Pure logic, no firmware dependencies: everything it needs comes in through
 * terrainNavHoldInput_t and everything it decides goes out through
 * terrainNavHoldOutput_t. It never touches the altitude controllers - the
 * caller applies out->targetZCm through the one altitude-target funnel.
 */

typedef enum {
    TERRAIN_NAV_HOLD_INACTIVE = 0,  // not eligible (switch off, not in cruise, disengage condition)
    TERRAIN_NAV_HOLD_WAITING,       // eligible but terrain data unusable - engagement refused
    TERRAIN_NAV_HOLD_ACTIVE,        // holding height above ground
    TERRAIN_NAV_HOLD_FROZEN,        // data lost mid-hold - altitude target frozen, never descend on dead data
} terrainNavHoldStatus_e;

typedef enum {
    TERRAIN_NAV_HOLD_WARN_NONE = 0,
    TERRAIN_NAV_HOLD_WARN_NOT_READY,    // switch is on but terrain data is not usable
    TERRAIN_NAV_HOLD_WARN_DATA_LOST,    // frozen: holding last altitude on stale map data
    TERRAIN_NAV_HOLD_WARN_MAX_ALT,      // terrain needs more altitude than nav_max_altitude allows
    TERRAIN_NAV_HOLD_WARN_PULL_UP,      // below the minimum AGL and not recovering - fires with or without a ceiling
    TERRAIN_NAV_HOLD_WARN_NO_HEADING,   // heading estimate invalid: lookahead off, reactive hold continues
    TERRAIN_NAV_HOLD_WARN_AUTO_CLIMB,   // engaged below the minimum AGL - automatic climb to the minimum in progress
} terrainNavHoldWarning_e;

typedef struct {
    uint32_t nowMs;
    bool eligible;              // armed + cruise whitelist + switch on + no hard-disengage condition
    bool stickAdjusting;        // pilot is moving the pitch stick - stock behavior runs, re-capture on release
    bool healthy;               // terrain module healthy this cycle
    bool aglValid;              // AGL answer available (may lag one query cycle behind healthy)
    int32_t aglCm;              // current height above ground, valid only when aglValid
    float currentZCm;           // current altitude in the local frame (same frame as the target funnel)
    bool lookaheadValid;        // lookahead answer available
    float lookaheadClimbCm;     // worst height deficit along the path ahead (>= 0)
    bool lookaheadDegraded;     // lookahead wanted but unavailable for an abnormal reason (heading estimate invalid) - warn the pilot
    int32_t minAglCm;           // configured minimum AGL
    int32_t maxAltCm;           // nav_max_altitude in the same frame, 0 = no ceiling
} terrainNavHoldInput_t;

typedef struct {
    bool writeTarget;           // when true the caller commands targetZCm through the funnel
    float targetZCm;
    terrainNavHoldStatus_e status;
    terrainNavHoldWarning_e warning;
} terrainNavHoldOutput_t;

typedef struct {
    terrainNavHoldStatus_e status;
    int32_t targetAglCm;        // the held height above ground; captured at engagement, never persisted across disengagement
    uint32_t usableSinceMs;     // 0 = data not usable right now (for the re-engage hysteresis)
    uint32_t lastUsableMs;      // last time data was usable (for the freeze grace period)
    bool reCapturePending;      // stick released - capture a new target AGL on the next usable cycle
    bool minAglReached;         // AGL has reached the minimum since this capture - arms the floor alarm
    int32_t climbBestAglCm;     // best AGL seen while still below the minimum (the auto-climb phase)
    bool pullUpActive;          // floor alarm latched: on below (min - margin), off at/above the minimum
} terrainNavHoldState_t;

// A momentary AGL gap (single cache miss) only pauses target updates; the hold
// freezes - with a pilot warning - when data stays unusable longer than this
#define TERRAIN_NAV_HOLD_FREEZE_GRACE_MS 500

// After a freeze, data must be continuously usable this long before the hold
// resumes (slew-limited); prevents flapping on marginal data
#define TERRAIN_NAV_HOLD_RESUME_HYSTERESIS_MS 3000

// Floor alarm margin: terrain naturally breathes around the minimum when
// riding the floor, so once the minimum has been reached PULL UP fires only
// below (min - margin) and clears at/above the minimum. The same margin
// decides "losing height" during the initial automatic climb
#define TERRAIN_NAV_HOLD_PULLUP_HYST_CM 500

void terrainNavHoldCoreReset(terrainNavHoldState_t *state);
void terrainNavHoldCoreUpdate(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out);
