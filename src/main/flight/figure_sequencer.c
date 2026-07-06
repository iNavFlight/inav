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

typedef enum {
    FIGURE_NONE = 0,
    FIGURE_ROLL,
    FIGURE_LOOP,
    FIGURE_POINT_ROLL,
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

static figureType_e requestedFigure(void)
{
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
static float altitudeAssistDeg(float nosePitchDeg)
{
    if (!navIsAltitudeEstimateTrusted()) {
        return 0.0f;
    }

    const float zErrM = (startAltitudeCm - getEstimatedActualPosition(Z)) / 100.0f;
    const float sinkMs = -getEstimatedActualVelocity(Z) / 100.0f;

    float offset = (figureSequencerConfig()->assistZGain / 10.0f) * zErrM
                 + figureSequencerConfig()->assistVzGain * sinkMs;
    offset = constrainf(offset, -figureSequencerConfig()->assistMax, figureSequencerConfig()->assistMax);

    // cos blend toward nose-vertical
    return offset * cos_approx(DEGREES_TO_RADIANS(constrainf(nosePitchDeg, -90.0f, 90.0f)));
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
    }

    const float tS = (millis() - startTimeMs) * 0.001f;
    float roll = 0.0f;
    float pitch = 0.0f;
    bool assist = false;

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
    targetPitchDeg = pitch + (assist ? altitudeAssistDeg(pitch) : 0.0f);
}

bool figureSequencerRequested(void)
{
    return activeFigure != FIGURE_NONE && state != FIG_STATE_IDLE;
}

void figureSequencerGetTarget(float *rollDeg, float *pitchDeg)
{
    *rollDeg = targetRollDeg;
    *pitchDeg = targetPitchDeg;
}

#endif // USE_ORIENTATION_HOLD
