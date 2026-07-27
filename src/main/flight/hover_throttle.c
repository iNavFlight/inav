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

#ifdef USE_FW_AEROBATICS

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

#include "flight/altitude_floor.h"
#include "flight/rotor_guard.h"
#include "flight/hover_throttle.h"
#include "flight/imu.h"
#include "flight/mixer.h"

#include "sensors/acceleration.h"
#include "flight/orientation_hold.h"
#include "flight/pid.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/battery.h"

PG_REGISTER_WITH_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig, PG_HOVER_THROTTLE_CONFIG, 4);

PG_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig,
    // Baro trust while the hover throttle owns the altitude. Kept as a field
    // (runtime-tunable) but no longer a CLI setting; 100 = the experimentally
    // found floor over inav_w_z_baro_p for the hover regime.
    .hoverBaroWeight = 100,
);

// Derived throttle gains. The one airframe fact they all share is the
// throttle-to-thrust slope: at the hover point thrust equals weight, so
// (hover throttle - idle) is the throttle span per 1 g of specific force.
// The hover point is LEARNED at engage (I-term seeded from the pilot's
// stick), so every "throttle us per unit of motion" gain derives from it at
// runtime; the loop-shaping constants below are dimensionless rates, fixed
// at the values the rig sweep found, and scale to any airframe through the
// learned span. Replaces the former ohold_hover_thr_p/i/d and
// ohold_assist_thr_p/i settings - nobody can set "throttle us per m/s"
// better than this identity derives it.
#define HOVER_THR_STIFFNESS_S2   1.5f    // [1/s^2] altitude error -> accel
#define HOVER_THR_DAMPING_S      1.8f    // [1/s]   climb rate -> decel
#define HOVER_THR_TRIM_S2        0.18f   // [1/s^2] slow altitude trim (I)
#define ASSIST_DAMPING_S         0.8f    // [1/s]   knife/inverted vz damping
#define ASSIST_TRIM_S2           0.4f    // [1/s^2] knife/inverted vz trim
#define GRAVITY_MSS              (GRAVITY_CMSS / 100.0f)

// throttle us that produce 1 g of specific-force change, from the learned
// base throttle (floored: a wrong low base must not collapse the gains)
static float usPerG(float baseUs)
{
    return MAX(baseUs - getThrottleIdleValue(), 100.0f);
}

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
// slow filter of the APPLIED hover throttle: the thrust that actually
// carries the weight is the true 1 g point, independent of where the
// pilot's stick happened to sit at engage - the derived gains anchor here
// (an engage seed 200 us low made the altitude loop 36% too soft in SITL)
#define HOVER_THR_ANCHOR_TAU_S 2.0f
static float hoverThrAnchorUs;

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

// Stall reserve: a good regulator MASKS the approach to the envelope edge -
// the attitude stays clean while the surfaces silently work their way
// toward saturation, then everything lets go at once (a ramp becomes a
// cliff; field observation). The mean control effort is therefore the EARLY
// escalation criterion, ahead of sinking and far ahead of oscillation:
// above the effort threshold the assist raises the speed while reserve is
// still left.
#define ASSIST_EFFORT_TAU_S      1.5f   // effort trend low-pass
#define ASSIST_EFFORT_THRESHOLD  0.7f   // of the pidSum authority
#define ASSIST_EFFORT_RAISE_US_S 40.0f  // full raise rate at 100% effort

static bool assistActive = false;
static float assistTrimUs;
static float assistCosRef;
static float assistEffortFilt;
static timeUs_t assistLastUs;

static float assistControlEffort(void)
{
    float effort = 0.0f;
    for (int axis = FD_ROLL; axis <= FD_YAW; axis++) {
        const uint16_t limit = getPidSumLimit(axis);
        if (limit > 0) {
            effort = MAX(effort, fabsf((float)axisPID[axis]) / limit);
        }
    }
    return MIN(effort, 1.0f);
}

static int16_t knifeInvertedAssistApply(int16_t pilotThrottle, float elevDeg)
{
    // a deliberate throttle cut stays a throttle cut
    if (!navIsAltitudeEstimateTrusted()
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
        assistEffortFilt = 0.0f;
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

    // gains derived from the pilot's own operating point: the throttle span
    // above idle is the thrust the pilot flies with, and the us-per-motion
    // gains scale with it (same identity as the hover PID above)
    const float assistUsPerG = usPerG(baseUs);
    const float climbMs = getEstimatedActualVelocity(Z) / 100.0f;
    if (fabsf(climbMs) < ASSIST_VZ_CLAMP_MS) {
        assistTrimUs = constrainf(assistTrimUs - ASSIST_TRIM_S2 * assistUsPerG / GRAVITY_MSS * climbMs * dT,
                                  -ASSIST_TRIM_MAX_US, ASSIST_TRIM_MAX_US);
    }
    // an oscillating hold means the surfaces are starving: raise the
    // operating point (more airflow), the gain learner only treats the
    // symptom
    if (orientationHoldRegimeOscillating()) {
        assistTrimUs = constrainf(assistTrimUs + ASSIST_OSC_RAISE_US_S * dT,
                                  -ASSIST_TRIM_MAX_US, ASSIST_TRIM_MAX_US);
    }
    // stall reserve (the EARLY criterion): sustained control effort toward
    // saturation raises the speed while the attitude still looks clean
    assistEffortFilt += (assistControlEffort() - assistEffortFilt)
                        * MIN(dT / ASSIST_EFFORT_TAU_S, 1.0f);
    if (assistEffortFilt > ASSIST_EFFORT_THRESHOLD) {
        const float urgency = (assistEffortFilt - ASSIST_EFFORT_THRESHOLD)
                            / (1.0f - ASSIST_EFFORT_THRESHOLD);
        assistTrimUs = constrainf(assistTrimUs + ASSIST_EFFORT_RAISE_US_S * urgency * dT,
                                  -ASSIST_TRIM_MAX_US, ASSIST_TRIM_MAX_US);
    }
    const float damping = -ASSIST_DAMPING_S * assistUsPerG / GRAVITY_MSS
                          * constrainf(climbMs, -ASSIST_VZ_CLAMP_MS, ASSIST_VZ_CLAMP_MS);

    // throttle_rule (flight contract, cap-only): the pilot's stick is the
    // MAXIMUM - trim, damping, cos scale and stall reserve shape the power
    // BELOW it, never above. Too little stick = controlled descent with the
    // attitude held; an estimator faking "sinking" can never command
    // unexpected power. The thumb is the motor.
    return constrain(lrintf(baseUs + assistTrimUs + damping),
                     getThrottleIdleValue(), pilotThrottle);
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
        // Recovery throttle claims are OWNED by their modules (the floor's
        // climb math, the rotor guard's boost/burst) - and per the
        // throttle_rule (cap-only, NO exceptions - Daniel) they too are
        // CAPPED at the pilot's stick: the recovery flies its computed
        // need bounded by the thumb. PILOT WARNING (manual, fat print):
        // during a floor or rotor-guard catch KEEP THE THROTTLE UP - the
        // stick is the catch's power budget; a chopped stick leaves the
        // catch attitude authority but NO climb power.
        const int16_t recoveryFloor = MAX(altitudeFloorClimbThrottleUs(),
                                          rotorGuardThrottleFloorUs());
        if (ARMING_FLAG(ARMED) && recoveryFloor > 0) {
            // MIN, not constrain-to-idle: a chopped stick stays a chopped
            // stick (throttle-0 rule) - the mixer's normal armed handling
            // applies, exactly as in the passthrough below
            return MIN(recoveryFloor, pilotThrottle);
        }
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
        hoverThrAnchorUs = pilotThrottle;
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

    // throttle floor: never cut the throttle below the motor idle, which keeps
    // the prop wash (and with it the control authority) alive -- excess lift,
    // e.g. in an updraft, is accepted as a climb instead
    const int16_t floorThrottle = getThrottleIdleValue();

    // gains derived from the learned hover point (see the constants above):
    // us-per-motion = loop constant x (hover span per 1 g). The anchor is
    // the slow-filtered APPLIED throttle - the thrust that actually holds
    // the aircraft - not the engage seed
    const float spanUsPerG = usPerG(hoverThrAnchorUs);
    // throttle_rule (cap-only): the I-term may never wind above the pilot's
    // stick - the stick is the power ceiling AND the anti-windup bound
    iTermUs = constrainf(iTermUs + HOVER_THR_TRIM_S2 * spanUsPerG / GRAVITY_MSS * zErrM * dT,
                         floorThrottle, pilotThrottle);

    // thrust supports the weight with its vertical component only:
    // compensate the tilt away from the zenith (capped, the elevation
    // gate keeps this bounded anyway)
    const float vertical = constrainf(sin_approx(DEGREES_TO_RADIANS(elevDeg)), 0.5f, 1.0f);
    const float correction = (HOVER_THR_STIFFNESS_S2 * spanUsPerG / GRAVITY_MSS * zErrM
                              - HOVER_THR_DAMPING_S * spanUsPerG / GRAVITY_MSS * climbMs) / vertical;

    // throttle_rule (cap-only): the hover PID owns the altitude BELOW the
    // pilot's stick - the stick must sit above the hover point, the loop
    // trims down from it (stick low = commanded sink, stick up = climb
    // command AND the headroom for it). Recovery floors keep their own
    // raise path above (the two contract exceptions).
    const int16_t outUs = constrain(lrintf(iTermUs + correction), floorThrottle, pilotThrottle);
    hoverThrAnchorUs += (outUs - hoverThrAnchorUs) * MIN(dT / HOVER_THR_ANCHOR_TAU_S, 1.0f);
    return outUs;
}

#endif // USE_FW_AEROBATICS
