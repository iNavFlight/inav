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
#include <math.h>

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/axis.h"
#include "common/maths.h"
#include "common/quaternion.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/hover_throttle.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/orientation_hold.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

PG_REGISTER_WITH_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig, PG_HOVER_THROTTLE_CONFIG, 2);

PG_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig,
    .pGain = SETTING_OHOLD_HOVER_THR_P_DEFAULT,
    .iGain = SETTING_OHOLD_HOVER_THR_I_DEFAULT,
    .dGain = SETTING_OHOLD_HOVER_THR_D_DEFAULT,
    .minThrottle = SETTING_OHOLD_HOVER_THR_MIN_DEFAULT,
    .assistVzP = SETTING_OHOLD_ASSIST_THR_P_DEFAULT,
    .assistVzI = SETTING_OHOLD_ASSIST_THR_I_DEFAULT,
    .hoverBaroWeight = SETTING_OHOLD_HOVER_BARO_WEIGHT_DEFAULT,
);

// Engage only when the nose is this close to the zenith; once engaged,
// stay active down to the release threshold. Without the hysteresis the
// attitude wobble around the hang flaps the controller and every
// re-engage captures a NEW altitude target at the current height -- a
// ratcheting drift.
#define HOVER_ENGAGE_NOSE_ELEVATION_DEG  60.0f
#define HOVER_RELEASE_NOSE_ELEVATION_DEG 45.0f

// The target latches only once the vertical motion has settled: engaging
// mid pull-up (the normal way to enter a hang) must not freeze the target
// at some fly-through altitude
#define HOVER_LATCH_CLIMB_CMS 200.0f

// Pilot correction: the throttle stick outside the mid deadband commands a
// CLIMB RATE (full deflection = this many m/s) while the controller keeps
// owning the throttle. Direct pilot throttle on top of the altitude PID
// would be two controllers fighting over one actuator - a classic
// oscillation; commanding the target instead leaves a single loop.
// Centered stick = hold; releasing latches the new altitude.
#define HOVER_STICK_CLIMB_MS 2.0f

static bool hoverActive = false;
static bool hoverLatched = false;
static float targetAltCm;
static float iTermUs;
static int16_t stickRefUs;
static timeUs_t lastUpdateUs;

// ---- Knife/inverted throttle assist ----------------------------------------
//
// In a knife edge or inverted hold the attitude controller owns the surfaces
// and the pitch-based altitude assist owns the flight path, but the throttle
// is a frozen pilot stick: if the speed is too low for the attitude's lift
// (fuselage lift at knife edge, inverted wing lift), the hold can only sink
// and the pitch assist saturates against the missing energy. The throttle
// criterion of these holds is vz -> 0: a slow, integrating TRIM around the
// pilot's stick adds throttle while the hold sinks and takes it back while
// it climbs. The pilot stays the base - moving the stick moves the whole
// operating point, the learned trim rides on top.

#define ASSIST_TRIM_MAX_US     150.0f
#define ASSIST_VZ_CLAMP_MS       4.0f   // |vz| beyond this is an entry/zoom
                                        // transient: freeze the trim, cap the
                                        // damping term
#define ASSIST_OSC_RAISE_US_S   30.0f   // trim raise rate while the hold
                                        // oscillates (starved surfaces)
#define ASSIST_COS_FLOOR         0.5f   // forward-component compensation cap
                                        // (above ~60 deg the hover PID owns
                                        // the throttle anyway)

static bool assistActive = false;
static float assistTrimUs;
static float assistCosRef;
static timeUs_t assistLastUs;

static int16_t knifeInvertedAssistApply(int16_t pilotThrottle, float elevDeg)
{
    // a deliberate throttle cut stays a throttle cut
    if (!navIsAltitudeEstimateTrusted()
        || hoverThrottleConfig()->assistVzI == 0
        || pilotThrottle < getThrottleIdleValue() + 50) {
        assistActive = false;
        return pilotThrottle;
    }

    const timeUs_t nowUs = micros();
    const float cosNow = MAX(cos_approx(DEGREES_TO_RADIANS(elevDeg)), ASSIST_COS_FLOOR);
    if (!assistActive) {
        assistActive = true;
        assistTrimUs = 0.0f;
        assistCosRef = cosNow;
        assistLastUs = nowUs;
    }
    const float dT = constrainf((nowUs - assistLastUs) * 1e-6f, 0.0f, 0.1f);
    assistLastUs = nowUs;

    // the CHOSEN speed is kept as the FORWARD component: when the nose
    // rises (assist, speed feedforward, harrier transition) the horizontal
    // thrust share shrinks with cos(theta) - scale the pilot's base so
    // T*cos(theta) stays at its engage value instead of bleeding speed
    const float baseUs = getThrottleIdleValue()
        + (pilotThrottle - getThrottleIdleValue()) * (assistCosRef / cosNow);

    const float climbMs = getEstimatedActualVelocity(Z) / 100.0f;
    if (fabsf(climbMs) < ASSIST_VZ_CLAMP_MS) {
        assistTrimUs = constrainf(assistTrimUs - hoverThrottleConfig()->assistVzI * climbMs * dT,
                                  -ASSIST_TRIM_MAX_US, ASSIST_TRIM_MAX_US);
    }
    // an oscillating hold means the surfaces are starving: raise the
    // operating point (more airflow), the gain learner only treats the
    // symptom
    if (orientationHoldRegimeOscillating()) {
        assistTrimUs = constrainf(assistTrimUs + ASSIST_OSC_RAISE_US_S * dT,
                                  -ASSIST_TRIM_MAX_US, ASSIST_TRIM_MAX_US);
    }
    const float damping = -hoverThrottleConfig()->assistVzP
                          * constrainf(climbMs, -ASSIST_VZ_CLAMP_MS, ASSIST_VZ_CLAMP_MS);

    return constrain(lrintf(baseUs + assistTrimUs + damping),
                     getThrottleIdleValue(), getMaxThrottle());
}

bool hoverThrottleIsEngaged(void)
{
    return hoverActive;
}

static float noseElevationDeg(void)
{
    fpVector3_t nose = { .v = { 1.0f, 0.0f, 0.0f } };
    quaternionRotateVectorInv(&nose, &nose, &orientation);   // body -> earth
    return RADIANS_TO_DEGREES(asin_approx(constrainf(-nose.z, -1.0f, 1.0f)));
}

int16_t hoverThrottleApply(int16_t pilotThrottle)
{
    const float elevDeg = noseElevationDeg();
    const float elevGate = hoverActive ? HOVER_RELEASE_NOSE_ELEVATION_DEG
                                       : HOVER_ENGAGE_NOSE_ELEVATION_DEG;

    // The altitude ownership follows the ATTITUDE, not the selected box:
    // above the elevation gate the thrust carries the weight (T*sin(alpha))
    // and the hover controller owns the altitude - also when the pilot
    // pulled a knife edge or inverted hold up into a harrier with the stick
    // offsets. Below the gate the vz trim is the (indirect) energy path.
    // This is the alpha continuum: knife -> harrier -> hover is one
    // mechanism whose direct-thrust share is the tilt compensation.
    const bool thrustAxisHold = orientationHoldIsPropHang()
                             || orientationHoldIsKnifeOrInverted();

    if (!ARMING_FLAG(ARMED)
        || !thrustAxisHold
        || !navIsAltitudeEstimateTrusted()
        || elevDeg < elevGate) {
        hoverActive = false;
        if (ARMING_FLAG(ARMED) && orientationHoldIsKnifeOrInverted()) {
            return knifeInvertedAssistApply(pilotThrottle, elevDeg);
        }
        assistActive = false;
        return pilotThrottle;
    }
    assistActive = false;

    const float z = getEstimatedActualPosition(Z);

    // a stick slammed to the bottom stays a hard throttle cut (bailout);
    // everything above commands a sink rate instead
    if (pilotThrottle < getThrottleIdleValue() + 50) {
        hoverActive = false;
        return pilotThrottle;
    }

    if (!hoverActive) {
        hoverActive = true;
        hoverLatched = false;
        targetAltCm = z;
        // seed the I-term with the last pilot throttle: learns the model's
        // hover throttle online instead of requiring a setting
        iTermUs = pilotThrottle;
        // the climb-rate stick references the ENGAGE position: entering the
        // hover regime out of a knife/harrier pull-up at cruise throttle
        // must not read as a climb command
        stickRefUs = pilotThrottle;
        lastUpdateUs = micros();
    }

    // pilot throttle outside the deadband around the engage reference: a
    // climb-rate command, the controller keeps the throttle (see
    // HOVER_STICK_CLIMB_MS). Linear beyond the deadband, full = full rate.
    float stickClimbMs = 0.0f;
    const int16_t stickOff = pilotThrottle - stickRefUs;
    if (ABS(stickOff) > rcControlsConfig()->mid_throttle_deadband) {
        const float span = (PWM_RANGE_MAX - PWM_RANGE_MIDDLE) - rcControlsConfig()->mid_throttle_deadband;
        const float beyond = (float)(ABS(stickOff) - rcControlsConfig()->mid_throttle_deadband);
        stickClimbMs = constrainf(beyond / MAX(span, 1.0f), 0.0f, 1.0f) * HOVER_STICK_CLIMB_MS;
        if (stickOff < 0) {
            stickClimbMs = -stickClimbMs;
        }
    }

    const timeUs_t nowUs = micros();
    const float dT = constrainf((nowUs - lastUpdateUs) * 1e-6f, 0.0f, 0.1f);
    lastUpdateUs = nowUs;

    const float climbCms = getEstimatedActualVelocity(Z);
    if (!hoverLatched) {
        targetAltCm = z;                       // follow until motion settles
        if (fabsf(climbCms) < HOVER_LATCH_CLIMB_CMS) {
            hoverLatched = true;
        }
    }
    if (stickClimbMs != 0.0f) {
        // the pilot's rate command RAMPS the altitude reference; the
        // unchanged altitude loop tracks the moving target and releasing
        // the stick latches wherever the ramp stopped. Clamping the
        // reference to the reachable neighbourhood prevents windup when
        // the aircraft cannot follow (throttle floor, saturation).
        targetAltCm += stickClimbMs * 100.0f * dT;
        targetAltCm = constrainf(targetAltCm, z - 500.0f, z + 500.0f);
    }

    const float zErrM = (targetAltCm - z) / 100.0f;
    const float climbMs = climbCms / 100.0f;

    // throttle floor: never cut the throttle below what keeps the prop wash
    // (and with it the control authority) alive -- excess lift, e.g. in an
    // updraft, is accepted as a climb instead
    const int16_t floorThrottle = MAX(getThrottleIdleValue(),
                                      (int16_t)hoverThrottleConfig()->minThrottle);

    iTermUs = constrainf(iTermUs + hoverThrottleConfig()->iGain * zErrM * dT,
                         floorThrottle, getMaxThrottle());

    // thrust supports the weight with its vertical component only:
    // compensate the tilt away from the zenith (capped, the elevation
    // gate keeps this bounded anyway)
    const float vertical = constrainf(sin_approx(DEGREES_TO_RADIANS(elevDeg)), 0.5f, 1.0f);
    const float correction = (hoverThrottleConfig()->pGain * zErrM
                              - hoverThrottleConfig()->dGain * climbMs) / vertical;

    return constrain(lrintf(iTermUs + correction), floorThrottle, getMaxThrottle());
}

#endif // USE_ORIENTATION_HOLD
