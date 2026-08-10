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
    state->minAglReached = false;
    state->climbBestAglCm = 0;
    state->pullUpActive = false;
    state->pullUpRatchet = false;
    state->pullUpCapFight = false;
    state->uncoverCapFight = false;
    state->stickWasAdjusting = false;
    state->blendActive = false;
    state->blendRateCmS = 0.0f;
    state->blendStartMs = 0;
    state->blendLastMs = 0;
    state->stickQuietSinceMs = 0;
    state->terrainAheadActive = false;
    state->escapeBadSinceMs = 0;
    state->escapeClearSinceMs = 0;
}

// TERRAIN AHEAD - the predictive escape test. Fails when even the FULL
// configured climb rate, applied from this moment, arrives below the
// minimum AGL at some point along the course ahead ("at this speed, this
// slope will beat you"). Below the minimum, closeness alone is not a
// forward threat - AUTO CLIMB and PULL UP own that situation - so only the
// terrain-relative deficit counts there; above the minimum the altitude
// cushion legitimately buys escape headroom and stays in the sum.
// Persistence keeps single noisy samples silent; clearing needs the test
// passing AND the aircraft at/above the minimum, both sustained - normally
// the pilot turning away (the scan follows the course), slowing or
// climbing. A threat that returns simply re-fires the series.
// Without a trusted view ahead the alarm cannot exist - the reactive floor
// alarm stands alone and TERRAIN LOOKAHEAD OFF explains why
static void updateEscapeAlarm(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in)
{
    if (!in->escapeDeficitValid) {
        state->terrainAheadActive = false;
        state->escapeBadSinceMs = 0;
        state->escapeClearSinceMs = 0;
        state->uncoverCapFight = false;
        return;
    }

    // Altitude cushion above the minimum offsets the deficit; below the
    // minimum the same term would fire the alarm over flat ground, so
    // there it is clamped out and only a real forward deficit remains
    float cushionCm = in->minAglCm - in->aglCm;
    if (cushionCm > 0.0f) {
        cushionCm = 0.0f;
    }
    const float shortfallCm = in->escapeDeficitCm + cushionCm;

    // A red that ran against the ceiling clamp just cleared: a latched
    // caution pointing at sky the aircraft has already escaped must not
    // linger - if the escape test passes RIGHT NOW it drops immediately.
    // Every other uncover keeps the sustained clear below (anti-flicker)
    if (state->uncoverCapFight) {
        state->uncoverCapFight = false;
        if (state->terrainAheadActive && shortfallCm <= 0.0f) {
            state->terrainAheadActive = false;
            state->escapeBadSinceMs = 0;
            state->escapeClearSinceMs = 0;
            return;
        }
    }

    if (shortfallCm > 0.0f) {
        state->escapeClearSinceMs = 0;
        if (state->escapeBadSinceMs == 0) {
            state->escapeBadSinceMs = (in->nowMs != 0) ? in->nowMs : 1;
        }
        if (in->nowMs - state->escapeBadSinceMs >= TERRAIN_NAV_HOLD_ESCAPE_PERSIST_MS) {
            state->terrainAheadActive = true;
        }
    } else {
        state->escapeBadSinceMs = 0;
        if (in->aglCm >= in->minAglCm) {
            if (state->escapeClearSinceMs == 0) {
                state->escapeClearSinceMs = (in->nowMs != 0) ? in->nowMs : 1;
            }
            if (in->nowMs - state->escapeClearSinceMs >= TERRAIN_NAV_HOLD_ESCAPE_CLEAR_MS) {
                state->terrainAheadActive = false;
            }
        } else {
            // Passing but still below the minimum: not clear yet - the
            // aircraft is not where it should be until the minimum is back
            state->escapeClearSinceMs = 0;
        }
    }
}

// Handover blend: one continuous writer across the whole stick episode.
// Seeded from the aircraft's real climb rate at the grab (zero step by
// construction), it chases the wish - the stick's clamped demand while
// deflected, zero after release (the stock release semantics) - and goes
// silent once close enough or past the lifetime bound. While it runs the
// caller commands out->rateCmS; the stock writer takes over seamlessly after
static void runBlend(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out, float wishCmS)
{
    if (!state->blendActive) {
        return;
    }

    if (in->nowMs - state->blendStartMs >= TERRAIN_NAV_HOLD_BLEND_MAX_MS) {
        state->blendActive = false;
        return;
    }

    // First-order chase, dt-based so the loop rate does not matter
    float alpha = (float)(in->nowMs - state->blendLastMs) / TERRAIN_NAV_HOLD_BLEND_TAU_MS;
    if (alpha > 1.0f) {
        alpha = 1.0f;
    }
    state->blendLastMs = in->nowMs;
    state->blendRateCmS += (wishCmS - state->blendRateCmS) * alpha;

    const float remaining = wishCmS - state->blendRateCmS;
    if (remaining < TERRAIN_NAV_HOLD_BLEND_DONE_CM_S && remaining > -TERRAIN_NAV_HOLD_BLEND_DONE_CM_S) {
        state->blendActive = false;
        return;
    }

    out->writeRate = true;
    out->rateCmS = state->blendRateCmS;
}

// The red text is the honest instruction. Where pulling the stick genuinely
// fixes the lowness (a breached floor, floor breathing, stick pressure) it
// says PULL UP; where pulling cannot help and may delay the right reaction
// it says TURN AWAY - the losing auto-climb (a manual pull commands the
// MANUAL rate, by default LESS than the automatic climb already failing)
// and the fight pinned at the ceiling (the funnel clamps every nav climb)
static terrainNavHoldWarning_e pullUpWarning(const terrainNavHoldState_t *state)
{
    return (state->pullUpRatchet || state->pullUpCapFight)
        ? TERRAIN_NAV_HOLD_WARN_TURN_AWAY
        : TERRAIN_NAV_HOLD_WARN_PULL_UP;
}

// Floor alarm: the red must fire on real AGL, ceiling or not. Once the
// minimum has been reached, dropping below (min - margin) latches the alarm
// and only climbing back to the minimum clears it. During the initial
// automatic climb the aircraft is legitimately below the minimum, so there
// the alarm fires only when height above terrain is actually being LOST
// (terrain outclimbing the aircraft), not merely still low
static void updateFloorAlarm(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in)
{
    if (in->aglCm >= in->minAglCm) {
        state->minAglReached = true;
        // The uncover moment: a red that ran against the ceiling clamp just
        // cleared - the escape alarm may drop its stale caution this cycle
        if (state->pullUpActive && state->pullUpCapFight) {
            state->uncoverCapFight = true;
        }
        state->pullUpActive = false;
        state->pullUpRatchet = false;
        state->pullUpCapFight = false;
        return;
    }

    if (state->minAglReached) {
        if (in->aglCm < in->minAglCm - TERRAIN_NAV_HOLD_PULLUP_HYST_CM) {
            state->pullUpActive = true;
        }
    } else {
        if (in->aglCm > state->climbBestAglCm) {
            state->climbBestAglCm = in->aglCm;
        } else if (in->aglCm < state->climbBestAglCm - TERRAIN_NAV_HOLD_PULLUP_HYST_CM) {
            state->pullUpActive = true;
            state->pullUpRatchet = true;
        }
    }
}

// Every capture (engagement or stick release) starts a fresh floor-alarm
// phase: capturing at/above the minimum arms the alarm immediately, below it
// the automatic climb to the minimum begins
static void startCapture(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in)
{
    state->targetAglCm = (in->aglCm > in->minAglCm) ? in->aglCm : in->minAglCm;
    state->reCapturePending = false;
    state->minAglReached = false;
    state->climbBestAglCm = in->aglCm;
    // A fresh capture hands a below-minimum situation to the automatic climb
    // (info message): the red alarm means a live threat only, and returns via
    // the losing-ground ratchet, a pilot push below the margin, or a breached
    // floor - never just for starting low
    state->pullUpActive = false;
    state->pullUpRatchet = false;
    state->pullUpCapFight = false;
    updateFloorAlarm(state, in);
}

static void computeTarget(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out)
{
    // Hold: climb or descend by exactly the difference between the height
    // above ground we have and the one we are holding
    float targetZCm = in->currentZCm + (state->targetAglCm - in->aglCm);

    // Lookahead: if terrain ahead demands more, climb early so the worst
    // point along the path still clears the minimum AGL
    if (in->lookaheadValid && in->lookaheadClimbCm > 0.0f) {
        const float lookaheadZCm = in->currentZCm + (in->lookaheadClimbCm + in->minAglCm - in->aglCm);
        if (lookaheadZCm > targetZCm) {
            targetZCm = lookaheadZCm;
        }
    }

    // Warning ladder, least severe first - a later assignment overrides.
    // The lookahead cannot scan ahead (heading estimate invalid) - the
    // reactive hold keeps tracking, but the pilot loses the early-climb
    // layer and must be told
    if (in->lookaheadDegraded) {
        out->warning = TERRAIN_NAV_HOLD_WARN_NO_HEADING;
    }
    // Engaged below the minimum: the hold itself is climbing to the floor -
    // information, not an alarm, the pilot has nothing to do
    if (!state->minAglReached && in->aglCm < in->minAglCm) {
        out->warning = TERRAIN_NAV_HOLD_WARN_AUTO_CLIMB;
    }
    // Ceiling arbitration: nav_max_altitude always wins - the funnel clamps
    // the target for real, here we only detect the conflict and warn. Warns
    // ahead of the pinch too, because the lookahead demand is in targetZCm
    const bool capConflict = (in->maxAltCm > 0 && targetZCm > in->maxAltCm);
    if (capConflict) {
        out->warning = TERRAIN_NAV_HOLD_WARN_MAX_ALT;
    }
    // Predictive: the terrain ahead cannot be out-climbed at full rate -
    // the pilot has time to act (turn, throttle, slow) and must use it
    if (state->terrainAheadActive) {
        out->warning = TERRAIN_NAV_HOLD_WARN_TERRAIN_AHEAD;
    }
    // The floor alarm outranks everything: too low NOW and not recovering.
    // Latched against the ceiling clamp the red sticks to TURN AWAY for the
    // whole episode - a demand dipping under the cap mid-fight must not
    // flip the instruction on screen
    if (state->pullUpActive) {
        if (capConflict) {
            state->pullUpCapFight = true;
        }
        out->warning = pullUpWarning(state);
    }

    out->writeTarget = true;
    out->targetZCm = targetZCm;
}

static void engage(terrainNavHoldState_t *state, const terrainNavHoldInput_t *in, terrainNavHoldOutput_t *out)
{
    // Capture the current AGL: zero commanded movement at engagement.
    // Sole exception: below the minimum - the target becomes the minimum and
    // the slew-limited funnel turns that into a gentle climb
    startCapture(state, in);
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
    out->writeRate = false;
    out->rateCmS = 0.0f;
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
                    // Never descend on dead data. A running handover blend
                    // and the predictive alarm die with the writes - health
                    // gates every output
                    state->status = TERRAIN_NAV_HOLD_FROZEN;
                    state->blendActive = false;
                    state->terrainAheadActive = false;
                    state->escapeBadSinceMs = 0;
                    state->escapeClearSinceMs = 0;
                    state->uncoverCapFight = false;
                    out->warning = TERRAIN_NAV_HOLD_WARN_DATA_LOST;
                }
                // Inside the grace period: a single missed read just pauses
                // target updates for a cycle - no drama, no warning
                break;
            }
            updateFloorAlarm(state, in);
            updateEscapeAlarm(state, in);
            if (in->stickAdjusting) {
                // Pilot input wins: stock climb-rate control runs, and the
                // AGL present at release becomes the new held target. The
                // alarms stay live - they report the aircraft's real state,
                // not the hold's commands
                state->reCapturePending = true;
                state->stickQuietSinceMs = 0;
                if (!state->blendActive && !state->stickWasAdjusting) {
                    // The grab: seed the handover blend from the aircraft's
                    // real climb rate - zero command step by construction.
                    // (Not re-seeded when a blend already runs: a stick
                    // re-crossing the deadband keeps one continuous command)
                    state->blendActive = true;
                    state->blendRateCmS = in->actualClimbRateCmS;
                    state->blendStartMs = in->nowMs;
                    state->blendLastMs = in->nowMs;
                }
                runBlend(state, in, out, in->stickWishValid ? in->stickWishCmS : 0.0f);
                if (state->terrainAheadActive) {
                    out->warning = TERRAIN_NAV_HOLD_WARN_TERRAIN_AHEAD;
                }
                if (state->pullUpActive) {
                    out->warning = pullUpWarning(state);
                }
                break;
            }
            if (state->reCapturePending) {
                // Retake dwell: the stick must stay inside the deadband this
                // long before the hold takes back - a stick hovering at the
                // edge otherwise alternates two writers within milliseconds.
                // The blend keeps running through the dwell, chasing zero
                // (the stock release semantics), so the whole episode has
                // one continuous writer
                if (state->stickQuietSinceMs == 0) {
                    state->stickQuietSinceMs = (in->nowMs != 0) ? in->nowMs : 1;
                }
                if (in->nowMs - state->stickQuietSinceMs < TERRAIN_NAV_HOLD_RETAKE_DWELL_MS) {
                    runBlend(state, in, out, 0.0f);
                    if (state->terrainAheadActive) {
                        out->warning = TERRAIN_NAV_HOLD_WARN_TERRAIN_AHEAD;
                    }
                    if (state->pullUpActive) {
                        out->warning = pullUpWarning(state);
                    }
                    break;
                }
                // A release is a fresh capture - fresh floor-alarm phase too
                state->blendActive = false;
                startCapture(state, in);
            }
            computeTarget(state, in, out);
            break;

        case TERRAIN_NAV_HOLD_FROZEN:
            if (usableStable) {
                // Resume the same held AGL, slew-limited by the funnel rate
                state->status = TERRAIN_NAV_HOLD_ACTIVE;
                updateFloorAlarm(state, in);
                updateEscapeAlarm(state, in);
                if (in->stickAdjusting) {
                    // Pilot is commanding at the moment of resume: yield
                    // immediately, the release becomes a fresh capture
                    state->reCapturePending = true;
                    if (state->pullUpActive) {
                        out->warning = pullUpWarning(state);
                    }
                    break;
                }
                computeTarget(state, in, out);
            } else {
                out->warning = TERRAIN_NAV_HOLD_WARN_DATA_LOST;
            }
            break;
    }

    state->stickWasAdjusting = in->stickAdjusting;
    out->status = state->status;
}
