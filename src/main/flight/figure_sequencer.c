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

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/figure_sequencer.h"
#include "flight/imu.h"

#include "navigation/navigation.h"

PG_REGISTER_WITH_RESET_TEMPLATE(figureSequencerConfig_t, figureSequencerConfig, PG_FIGURE_SEQUENCER_CONFIG, 0);

PG_RESET_TEMPLATE(figureSequencerConfig_t, figureSequencerConfig,
    .rollRate = SETTING_FIG_ROLL_RATE_DEFAULT,
    .loopRate = SETTING_FIG_LOOP_RATE_DEFAULT,
    .pointDwellMs = SETTING_FIG_POINT_DWELL_DEFAULT,
    .assistZGain = SETTING_FIG_ASSIST_Z_GAIN_DEFAULT,
    .assistVzGain = SETTING_FIG_ASSIST_VZ_GAIN_DEFAULT,
    .assistMax = SETTING_FIG_ASSIST_MAX_DEFAULT,
);

PG_REGISTER_ARRAY(figureSegment_t, MAX_FIGURE_SEQUENCE_SEGMENTS, figureSequence, PG_FIGURE_SEQUENCE, 0);

typedef enum {
    FIGURE_NONE = 0,
    FIGURE_ROLL,
    FIGURE_LOOP,
    FIGURE_POINT_ROLL,
    FIGURE_SEQUENCE,
} figureType_e;

typedef enum {
    FIG_STATE_IDLE = 0,
    FIG_STATE_RUNNING,
    FIG_STATE_DONE,      // figure complete, holding level until box released
} figureState_e;

static figureType_e activeFigure = FIGURE_NONE;
static figureState_e state = FIG_STATE_IDLE;
static timeMs_t startTimeMs;
static float startAltitudeCm;
static float targetRollDeg;
static float targetPitchDeg;

// sequence (FIGURE SEQ) state
static int seqIndex;
static timeMs_t seqSegStartMs;
static float seqBaseRoll;
static float seqBasePitch;
static float seqSegAltCm;      // assist reference, captured at segment entry
static bool seqImpulseActive;
static float seqImpulseRates[3];
static bool seqTurnCoordination;
static float seqTurnBankDeg;
static bool seqSpinActive;
static float seqSpinYawNorm;
static bool seqSpinTracking;    // turn counter armed
static float seqSpinTurnDeg;    // accumulated (wrap-aware) yaw rotation
static float seqSpinPrevYawDeg;

static figureType_e requestedFigure(void)
{
    if (IS_RC_MODE_ACTIVE(BOXFIGSEQ)) {
        return FIGURE_SEQUENCE;
    }
    if (IS_RC_MODE_ACTIVE(BOXFIGROLL)) {
        return FIGURE_ROLL;
    }
    if (IS_RC_MODE_ACTIVE(BOXFIGLOOP)) {
        return FIGURE_LOOP;
    }
    if (IS_RC_MODE_ACTIVE(BOXFIGPOINTROLL)) {
        return FIGURE_POINT_ROLL;
    }
    return FIGURE_NONE;
}

// Altitude assist: earth referenced nose-above-horizon offset from an
// altitude/climb-rate PID, blended out as the nose approaches vertical
// (there altitude is a thrust problem, not an attitude problem)
float figureAltitudeAssistDeg(float nosePitchDeg, float refAltCm)
{
    if (!navIsAltitudeEstimateTrusted()) {
        return 0.0f;
    }

    const float zErrM = (refAltCm - getEstimatedActualPosition(Z)) / 100.0f;
    const float sinkMs = -getEstimatedActualVelocity(Z) / 100.0f;

    float offset = (figureSequencerConfig()->assistZGain / 10.0f) * zErrM
                 + figureSequencerConfig()->assistVzGain * sinkMs;
    offset = constrainf(offset, -figureSequencerConfig()->assistMax, figureSequencerConfig()->assistMax);

    // The offset must raise the NOSE ELEVATION. With an accumulated pitch
    // parameter past +/-90 (e.g. base pitch 180 after a half loop) the raw
    // pitch parameter acts inverted on the elevation: elevation = sin(pitch),
    // d(elevation)/d(pitch) flips sign with cos(pitch). Blend out toward
    // nose-vertical with |cos| (= cos of the true elevation).
    const float cosPitch = cos_approx(DEGREES_TO_RADIANS(nosePitchDeg));
    return offset * cosPitch;   // magnitude blends with |cos|, sign corrects the direction
}

void figureSequencerUpdate(void)
{
    const figureType_e req = requestedFigure();

    if (req == FIGURE_NONE || !ARMING_FLAG(ARMED) || !STATE(AIRPLANE)) {
        activeFigure = FIGURE_NONE;
        state = FIG_STATE_IDLE;
        return;
    }

    if (state == FIG_STATE_IDLE || req != activeFigure) {
        activeFigure = req;
        state = FIG_STATE_RUNNING;
        startTimeMs = millis();
        startAltitudeCm = getEstimatedActualPosition(Z);
        seqIndex = 0;
        seqSegStartMs = startTimeMs;
        seqSpinTracking = false;
        seqBaseRoll = 0.0f;
        seqBasePitch = 0.0f;
        seqSegAltCm = startAltitudeCm;
    }

    const float tS = (millis() - startTimeMs) * 0.001f;
    float roll = 0.0f;
    float pitch = 0.0f;
    bool assist = false;
    seqImpulseActive = false;   // recomputed below while an IMPULSE runs
    seqTurnCoordination = false;
    seqSpinActive = false;      // recomputed below while a SPIN runs

    switch (activeFigure) {
        case FIGURE_ROLL: {
            const float theta = figureSequencerConfig()->rollRate * tS;
            roll = MIN(theta, 360.0f);
            assist = true;
            if (theta >= 360.0f) {
                state = FIG_STATE_DONE;
                roll = 0.0f;    // 360 == 0, hold level
            }
            break;
        }

        case FIGURE_LOOP: {
            const float theta = figureSequencerConfig()->loopRate * tS;
            pitch = MIN(theta, 360.0f);
            if (theta >= 360.0f) {
                state = FIG_STATE_DONE;
                pitch = 0.0f;
                assist = true;  // level again: hold the entry altitude
            }
            break;
        }

        case FIGURE_SEQUENCE: {
            // advance through the programmed segment chain
            while (state == FIG_STATE_RUNNING) {
                if (seqIndex >= MAX_FIGURE_SEQUENCE_SEGMENTS
                    || figureSequence(seqIndex)->type == FIGSEG_END
                    || figureSequence(seqIndex)->type >= FIGSEG_TYPE_COUNT) {
                    state = FIG_STATE_DONE;
                    break;
                }

                const figureSegment_t *seg = figureSequence(seqIndex);
                const float tSeg = (millis() - seqSegStartMs) * 0.001f;
                bool segDone = false;

                switch (seg->type) {
                    case FIGSEG_ROLL: {
                        const float span = ABS((float)seg->p1);
                        const float theta = MIN(figureSequencerConfig()->rollRate * tSeg, span);
                        roll = seqBaseRoll + (seg->p1 < 0 ? -theta : theta);
                        pitch = seqBasePitch;
                        assist = seg->flags & FIGSEG_FLAG_ASSIST;
                        if (theta >= span) {
                            seqBaseRoll += seg->p1;
                            segDone = true;
                        }
                        break;
                    }

                    case FIGSEG_PITCH: {
                        const float span = ABS((float)seg->p1);
                        const float theta = MIN(figureSequencerConfig()->loopRate * tSeg, span);
                        roll = seqBaseRoll;
                        pitch = seqBasePitch + (seg->p1 < 0 ? -theta : theta);
                        assist = false;
                        if (theta >= span) {
                            seqBasePitch += seg->p1;
                            segDone = true;
                        }
                        break;
                    }

                    case FIGSEG_HOLD:
                        seqBaseRoll = seg->p1;
                        seqBasePitch = seg->p2;
                        roll = seqBaseRoll;
                        pitch = seqBasePitch;
                        assist = seg->flags & FIGSEG_FLAG_ASSIST;
                        segDone = tSeg * 1000.0f >= seg->p3;
                        break;

                    case FIGSEG_WAIT_ALT: {
                        // wings level, climb/descend to the target altitude
                        // via the assist mechanism, gate until reached
                        seqBaseRoll = 0.0f;
                        seqBasePitch = 0.0f;
                        roll = 0.0f;
                        pitch = 0.0f;
                        seqSegAltCm = seg->p1 * 100.0f;
                        assist = true;
                        const float tolCm = MAX(seg->p2, 1) * 100.0f;
                        segDone = navIsAltitudeEstimateTrusted()
                            && ABS(getEstimatedActualPosition(Z) - seqSegAltCm) < tolCm
                            && ABS(getEstimatedActualVelocity(Z)) < 150.0f;
                        break;
                    }

                    case FIGSEG_WAIT_TIME:
                        roll = seqBaseRoll;
                        pitch = seqBasePitch;
                        assist = seg->flags & FIGSEG_FLAG_ASSIST;
                        segDone = tSeg * 1000.0f >= seg->p3;
                        break;

                    case FIGSEG_IMPULSE:
                        // open-loop rate impulse (snap/spin entry): full-rate
                        // commands saturate the surfaces; the following
                        // segment (or the DONE level hold) catches whatever
                        // attitude results, shortest path
                        seqImpulseActive = true;
                        seqImpulseRates[FD_ROLL] = 0.0f;
                        seqImpulseRates[FD_PITCH] = constrainf(seg->p1, -100, 100) * 0.01f;
                        seqImpulseRates[FD_YAW] = constrainf(seg->p2, -100, 100) * 0.01f;
                        roll = seqBaseRoll;
                        pitch = seqBasePitch;
                        segDone = tSeg * 1000.0f >= seg->p3;
                        break;

                    case FIGSEG_SPIN: {
                        // controlled flat spin: roll and pitch stay CLOSED
                        // LOOP on the flat attitude (the controller actively
                        // keeps the plane flat and damps the wobble) while
                        // the rudder is held open loop for the autorotation.
                        // The segment ends after p1 full turns or the p3
                        // timeout; the altitude floor preempts globally.
                        seqSpinActive = true;
                        seqSpinYawNorm = ((seg->p2 != 0) ? constrainf(seg->p2, -100, 100) : 100.0f)
                                         * 0.01f * (seg->p1 < 0 ? -1.0f : 1.0f);
                        seqBaseRoll = 0.0f;
                        seqBasePitch = 0.0f;
                        roll = 0.0f;
                        pitch = 0.0f;
                        assist = false;      // the spin descends by design
                        const float yawDeg = DECIDEGREES_TO_DEGREES((float)attitude.values.yaw);
                        if (!seqSpinTracking) {
                            seqSpinTracking = true;
                            seqSpinTurnDeg = 0.0f;
                        } else {
                            float d = yawDeg - seqSpinPrevYawDeg;
                            while (d > 180.0f) { d -= 360.0f; }
                            while (d < -180.0f) { d += 360.0f; }
                            seqSpinTurnDeg += d;
                        }
                        seqSpinPrevYawDeg = yawDeg;
                        const float timeoutMs = (seg->p3 > 0) ? seg->p3 : 15000.0f;
                        segDone = fabsf(seqSpinTurnDeg) >= ABS(seg->p1) * 360.0f
                               || tSeg * 1000.0f >= timeoutMs;
                        if (segDone) {
                            seqSpinTracking = false;
                        }
                        break;
                    }

                    case FIGSEG_WAIT_POS: {
                        // airspace containment: bank toward HOME until the
                        // distance drops below the radius. The course loop
                        // lives only in this segment; the attitude modes
                        // stay heading-free
                        seqBaseRoll = 0.0f;
                        seqBasePitch = 0.0f;
                        pitch = 0.0f;
                        assist = true;
                        if (STATE(GPS_FIX_HOME)) {
                            const float maxBank = (seg->p2 > 0) ? seg->p2 : 30.0f;
                            float courseErr = GPS_directionToHome - DECIDEGREES_TO_DEGREES((float)attitude.values.yaw);
                            while (courseErr > 180.0f) { courseErr -= 360.0f; }
                            while (courseErr < -180.0f) { courseErr += 360.0f; }
                            roll = constrainf(0.8f * courseErr, -maxBank, maxBank);
                            // banked turn: feed the coordinated turn rates
                            // forward, otherwise the heading-free controller
                            // regulates the (physical) turn yaw rate to zero
                            seqTurnCoordination = true;
                            seqTurnBankDeg = roll;
                            segDone = GPS_distanceToHome < (uint32_t)MAX(seg->p1, 10);
                        } else {
                            roll = 0.0f;    // no home fix: hold level, gate stays
                            segDone = false;
                        }
                        break;
                    }

                    default:
                        segDone = true;
                        break;
                }

                if (!segDone) {
                    break;
                }
                seqIndex++;
                seqSegStartMs = millis();
                if (figureSequence(MIN(seqIndex, MAX_FIGURE_SEQUENCE_SEGMENTS - 1))->type != FIGSEG_WAIT_ALT) {
                    seqSegAltCm = getEstimatedActualPosition(Z);   // assist reference for the next segment
                }
            }
            if (state == FIG_STATE_DONE) {
                roll = 0.0f;
                pitch = 0.0f;
                assist = true;
            }
            break;
        }

        case FIGURE_POINT_ROLL: {
            // 4 points: rotate 90 deg at roll rate, dwell, repeat
            const float rotS = 90.0f / figureSequencerConfig()->rollRate;
            const float dwellS = figureSequencerConfig()->pointDwellMs / 1000.0f;
            const float segS = rotS + dwellS;
            const int seg = (int)(tS / segS);
            if (seg >= 4) {
                state = FIG_STATE_DONE;
                roll = 0.0f;
            } else {
                const float tInSeg = tS - seg * segS;
                roll = seg * 90.0f + MIN(tInSeg / rotS, 1.0f) * 90.0f;
            }
            assist = true;
            break;
        }

        default:
            break;
    }

    if (state == FIG_STATE_DONE) {
        roll = (activeFigure == FIGURE_LOOP) ? 0.0f : roll;
        assist = true;
    }

    targetRollDeg = roll;
    targetPitchDeg = pitch + (assist
        ? figureAltitudeAssistDeg(pitch, activeFigure == FIGURE_SEQUENCE ? seqSegAltCm : startAltitudeCm)
        : 0.0f);
}

bool figureSequencerRequested(void)
{
    return activeFigure != FIGURE_NONE && state != FIG_STATE_IDLE;
}

bool figureSequencerGetTurnBank(float *bankDeg)
{
    if (!seqTurnCoordination) {
        return false;
    }
    *bankDeg = seqTurnBankDeg;
    return true;
}

bool figureSequencerGetRateCommand(float ratesNorm[3])
{
    if (!seqImpulseActive) {
        return false;
    }
    for (int i = 0; i < 3; i++) {
        ratesNorm[i] = seqImpulseRates[i];
    }
    return true;
}

bool figureSequencerGetSpinCommand(float *yawNorm)
{
    if (!seqSpinActive) {
        return false;
    }
    *yawNorm = seqSpinYawNorm;
    return true;
}

void figureSequencerGetTarget(float *rollDeg, float *pitchDeg)
{
    *rollDeg = targetRollDeg;
    *pitchDeg = targetPitchDeg;
}

#endif // USE_ORIENTATION_HOLD
