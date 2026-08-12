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
    TERRAIN_NAV_HOLD_WARN_TERRAIN_AHEAD, // escape test failing: terrain ahead cannot be out-climbed at full rate
    TERRAIN_NAV_HOLD_WARN_TURN_AWAY,    // floor alarm where pulling cannot help: the full-rate climb is losing, or the target is pinned at the ceiling
} terrainNavHoldWarning_e;

typedef struct {
    uint32_t nowMs;
    bool eligible;              // armed + cruise whitelist + switch on + no hard-disengage condition
    bool stickAdjusting;        // pilot is moving the pitch stick - stock behavior runs, re-capture on release
    bool healthy;               // terrain module healthy this cycle
    bool aglValid;              // AGL answer available (may lag one query cycle behind healthy)
    int32_t aglCm;              // current height above ground, valid only when aglValid
    float currentZCm;           // current altitude in the local frame (same frame as the target funnel)
    float actualClimbRateCmS;   // the aircraft's real vertical speed (seeds the handover blend)
    bool stickWishValid;        // the pilot's stick climb-rate wish is known this cycle
    float stickWishCmS;         // what the stick currently asks for, manual-clamped by the caller
    bool stickFullPull;         // the pitch stick is at (or near) full pull - the pilot's vertical channel is maxed
    bool pilotHasMoreClimb;     // nav_fw_manual_climb_rate > nav_fw_auto_climb_rate: a pull adds climb the hold cannot
    bool autoBeatsManual;       // nav_fw_auto_climb_rate > nav_fw_manual_climb_rate: releasing the stick adds climb a pull cannot
    bool lookaheadValid;        // lookahead answer available
    float lookaheadClimbCm;     // worst height deficit along the path ahead (>= 0)
    bool escapeDeficitValid;    // escape-test answer available (same walk as the lookahead)
    float escapeDeficitCm;      // worst deficit vs the FULL climb rate along the path (>= 0)
    bool lookaheadDegraded;     // lookahead wanted but unavailable for an abnormal reason (heading estimate invalid) - warn the pilot
    int32_t minAglCm;           // configured minimum AGL
    int32_t maxAltCm;           // nav_max_altitude in the same frame, 0 = no ceiling
} terrainNavHoldInput_t;

typedef struct {
    bool writeTarget;           // when true the caller commands targetZCm through the funnel
    float targetZCm;
    bool writeRate;             // when true the caller commands rateCmS instead (handover blend window)
    float rateCmS;              // signed climb rate; bounded by the caller's clamp on the blend inputs
    bool autoClimbRunning;      // the hold is climbing to the minimum right now (for the OSD to alternate the info under a covering warning)
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
    int32_t climbBestAglCm;     // best AGL of the current below-minimum fight (the auto-climb phase, and the breach episode after the minimum was reached)
    bool pullUpActive;          // floor alarm latched: on below (min - margin), off at/above the minimum
    bool pullUpRatchet;         // this red latched via the losing-auto-climb ratchet - pulling cannot help, the text says TURN AWAY
    bool pullUpCapFight;        // this red episode ran against the ceiling clamp - same TURN AWAY text, latched for the episode
    bool uncoverCapFight;       // one-shot: a cap-fight red just cleared at the minimum - the escape alarm may fast-clear this cycle
    bool stickWasAdjusting;     // previous-cycle stick state (grab-edge detection for the blend)
    bool blendActive;           // handover blend running (stick grab through the retake dwell)
    float blendRateCmS;         // the blend's current climb-rate command
    uint32_t blendStartMs;      // when this blend began (hard lifetime bound)
    uint32_t blendLastMs;       // last blend update (dt for the first-order chase)
    uint32_t stickQuietSinceMs; // stick back inside the deadband since (retake dwell); 0 = not quiet
    bool terrainAheadActive;    // predictive escape alarm latched
    uint32_t escapeBadSinceMs;  // escape test failing since (persistence); 0 = not failing
    uint32_t escapeClearSinceMs;// escape test passing with margin since (clear hysteresis); 0 = not counting
    bool escapeDeepFail;        // the current escape shortfall is at least the deep threshold - only such a failure births TURN AWAY prospectively
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

// Handover blend: when the pilot grabs the stick during an active hold the
// climb-rate command must not step (a large hold demand collapsing into a
// small stick demand pitches the nose the wrong way). The blend seeds from
// the aircraft's real climb rate and chases the stick's wish with this time
// constant - most of the pilot's wish arrives within the first tau
#define TERRAIN_NAV_HOLD_BLEND_TAU_MS 300

// The blend never outlives this: a hard bound on the shared-authority window,
// after which the stock stick control flies alone
#define TERRAIN_NAV_HOLD_BLEND_MAX_MS 1000

// The blend ends early once this close to the stick's wish - close enough
// that the switch to the stock writer is seamless
#define TERRAIN_NAV_HOLD_BLEND_DONE_CM_S 25.0f

// Retake dwell: after the stick returns inside the deadband it must stay
// there this long before the hold captures and resumes writing - a stick
// hovering at the deadband edge otherwise alternates two writers within
// milliseconds (the LOG00010 whiplash)
#define TERRAIN_NAV_HOLD_RETAKE_DWELL_MS 250

// TERRAIN AHEAD (predictive escape test): the failing condition must hold
// this long before the alarm fires - no flicker on single noisy samples
#define TERRAIN_NAV_HOLD_ESCAPE_PERSIST_MS 1000

// ...and clears only after the test passes with the aircraft at/above the
// minimum, both sustained this long (typically because the pilot turned,
// slowed or climbed); a returning threat simply re-fires the series
#define TERRAIN_NAV_HOLD_ESCAPE_CLEAR_MS 2000

// A red born under a latched TERRAIN AHEAD says TURN AWAY outright only when
// the current escape shortfall is at least this deep: a marginal map-step
// deficit self-resolves in seconds and stays PULL UP, while the deficit of a
// genuine wall grows so fast at cruise closure that it crosses this line
// within a second of the alarm
#define TERRAIN_NAV_HOLD_ESCAPE_DEEP_CM 1000.0f

void terrainNavHoldCoreReset(terrainNavHoldState_t *state);
void terrainNavHoldCoreUpdate(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out);
