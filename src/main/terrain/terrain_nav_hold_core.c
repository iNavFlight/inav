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

#include "terrain_nav_hold_core.h"

void terrainNavHoldCoreReset(terrainNavHoldState_t *state)
{
    state->status = TERRAIN_NAV_HOLD_INACTIVE;
    state->targetAglCm = 0;
    state->usableSinceMs = 0;
    state->lastUsableMs = 0;
    state->reCapturePending = false;
}

static void computeTarget(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out)
{
    // Hold: climb or descend by exactly the difference between the height
    // above ground we have and the one we are holding
    float targetZCm = in->currentZCm + (state->targetAglCm - in->aglCm);

    // The lookahead cannot scan ahead (heading estimate invalid) - the
    // reactive hold keeps tracking, but the pilot loses the early-climb
    // layer and must be told. The ceiling warnings below override this one
    if (in->lookaheadDegraded) {
        out->warning = TERRAIN_NAV_HOLD_WARN_NO_HEADING;
    }

    // Lookahead: if terrain ahead demands more, climb early so the worst
    // point along the path still clears the minimum AGL
    if (in->lookaheadValid && in->lookaheadClimbCm > 0.0f) {
        const float lookaheadZCm = in->currentZCm + (in->lookaheadClimbCm + in->minAglCm - in->aglCm);
        if (lookaheadZCm > targetZCm) {
            targetZCm = lookaheadZCm;
        }
    }

    // Ceiling arbitration: nav_max_altitude always wins - the funnel clamps
    // the target for real, here we only detect the conflict and warn. Warns
    // ahead of the pinch too, because the lookahead demand is in targetZCm
    if (in->maxAltCm > 0 && targetZCm > in->maxAltCm) {
        out->warning = (in->aglCm < in->minAglCm) ? TERRAIN_NAV_HOLD_WARN_PULL_UP : TERRAIN_NAV_HOLD_WARN_MAX_ALT;
    }

    out->writeTarget = true;
    out->targetZCm = targetZCm;
}

static void engage(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out)
{
    // Capture the current AGL: zero commanded movement at engagement.
    // Sole exception: below the minimum - the target becomes the minimum and
    // the slew-limited funnel turns that into a gentle climb
    state->targetAglCm = (in->aglCm > in->minAglCm) ? in->aglCm : in->minAglCm;
    state->reCapturePending = false;
    state->status = TERRAIN_NAV_HOLD_ACTIVE;

    if (in->stickAdjusting) {
        state->reCapturePending = true;     // stock stick behavior runs first
        return;
    }

    computeTarget(state, in, out);
}

void terrainNavHoldCoreUpdate(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out)
{
    out->writeTarget = false;
    out->targetZCm = 0.0f;
    out->warning = TERRAIN_NAV_HOLD_WARN_NONE;

    if (!in->eligible) {
        // Disengagement always clears the captured target - a fresh
        // engagement captures a fresh AGL, a stale setpoint never survives
        terrainNavHoldCoreReset(state);
        out->status = state->status;
        return;
    }

    // Track data usability for the freeze grace period and resume hysteresis
    const bool usable = in->healthy && in->aglValid;
    if (usable) {
        if (state->usableSinceMs == 0) {
            state->usableSinceMs = (in->nowMs != 0) ? in->nowMs : 1;
        }
        state->lastUsableMs = in->nowMs;
    } else {
        state->usableSinceMs = 0;
    }
    const bool usableStable = usable &&
        (in->nowMs - state->usableSinceMs >= TERRAIN_NAV_HOLD_RESUME_HYSTERESIS_MS);

    switch (state->status) {
        case TERRAIN_NAV_HOLD_INACTIVE:
        case TERRAIN_NAV_HOLD_WAITING:
            if (!usable) {
                // Refuse engagement instead of engaging and degrading:
                // cruise keeps flying exactly as stock, the pilot is told why
                state->status = TERRAIN_NAV_HOLD_WAITING;
                out->warning = TERRAIN_NAV_HOLD_WARN_NOT_READY;
                break;
            }
            engage(state, in, out);
            break;

        case TERRAIN_NAV_HOLD_ACTIVE:
            if (!usable) {
                if (in->nowMs - state->lastUsableMs > TERRAIN_NAV_HOLD_FREEZE_GRACE_MS) {
                    // Freeze: stop commanding, the last target holds.
                    // Never descend on dead data
                    state->status = TERRAIN_NAV_HOLD_FROZEN;
                    out->warning = TERRAIN_NAV_HOLD_WARN_DATA_LOST;
                }
                // Inside the grace period: a single missed read just pauses
                // target updates for a cycle - no drama, no warning
                break;
            }
            if (in->stickAdjusting) {
                // Pilot input wins: stock climb-rate control runs, and the
                // AGL present at release becomes the new held target
                state->reCapturePending = true;
                break;
            }
            if (state->reCapturePending) {
                state->targetAglCm = (in->aglCm > in->minAglCm) ? in->aglCm : in->minAglCm;
                state->reCapturePending = false;
            }
            computeTarget(state, in, out);
            break;

        case TERRAIN_NAV_HOLD_FROZEN:
            if (usableStable) {
                // Resume the same held AGL, slew-limited by the funnel rate
                state->status = TERRAIN_NAV_HOLD_ACTIVE;
                computeTarget(state, in, out);
            } else {
                out->warning = TERRAIN_NAV_HOLD_WARN_DATA_LOST;
            }
            break;
    }

    out->status = state->status;
}
