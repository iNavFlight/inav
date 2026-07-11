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

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/axis.h"
#include "common/maths.h"
#include "common/quaternion.h"
#include "common/utils.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/config.h"
#include "fc/control_profile.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/altitude_floor.h"
#include "flight/figure_sequencer.h"

#include "navigation/navigation.h"
#include "flight/imu.h"
#include "flight/orientation_hold.h"
#include "flight/pid.h"

PG_REGISTER_WITH_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig, PG_ORIENTATION_HOLD_CONFIG, 0);

PG_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig,
    .invertedPitchTrim = SETTING_OHOLD_INVERTED_PITCH_TRIM_DEFAULT,
    .knifeLeftPitchTrim = SETTING_OHOLD_KNIFE_LEFT_PITCH_TRIM_DEFAULT,
    .knifeRightPitchTrim = SETTING_OHOLD_KNIFE_RIGHT_PITCH_TRIM_DEFAULT,
    .hoverGainLearned = SETTING_OHOLD_HOVER_GAIN_DEFAULT,
    .entryRateDps = SETTING_OHOLD_ENTRY_RATE_DEFAULT,
    .stickAngleMaxDeg = SETTING_OHOLD_STICK_ANGLE_DEFAULT,
    .stickReturnRateDps = SETTING_OHOLD_STICK_RETURN_RATE_DEFAULT,
);

typedef struct {
    boxId_e box;
    float rollDeg;
    float pitchDeg;
} orientationHoldPreset_t;

// First matching box wins. Targets are tilt only (yaw = 0); heading is
// always left free by the twist removal in the error computation.
static const orientationHoldPreset_t orientationHoldPresets[] = {
    { BOXINVERTED,      180.0f,  0.0f },
    { BOXKNIFELEFT,     -90.0f,  0.0f },
    { BOXKNIFERIGHT,     90.0f,  0.0f },
    { BOXPROPHANG,        0.0f, 90.0f },
};

static const orientationHoldPreset_t * orientationHoldActivePreset(void)
{
    for (unsigned i = 0; i < ARRAYLEN(orientationHoldPresets); i++) {
        if (IS_RC_MODE_ACTIVE(orientationHoldPresets[i].box)) {
            return &orientationHoldPresets[i];
        }
    }
    return NULL;
}

bool orientationHoldIsRequested(void)
{
    return figureSequencerRequested() || orientationHoldActivePreset() != NULL
        || IS_RC_MODE_ACTIVE(BOXATTLOCK);
}

static bool orientationHoldSticksDeflected(void)
{
    return ABS(rcCommand[ROLL]) > rcControlsConfig()->deadband
        || ABS(rcCommand[PITCH]) > rcControlsConfig()->deadband
        || ABS(rcCommand[YAW]) > rcControlsConfig()->yaw_deadband;
}

// Deadbanded stick position normalized to -1..1
static float orientationHoldStickNorm(int16_t rc, uint8_t deadband)
{
    if (ABS(rc) <= deadband) {
        return 0.0f;
    }
    const float span = 500.0f - deadband;
    return (rc > 0 ? (rc - deadband) : (rc + deadband)) / span;
}

// Same Euler to quaternion convention as imuComputeQuaternionFromRPY (yaw = 0)
void orientationHoldTargetFromRP(fpQuaternion_t *qTarget, float rollDeg, float pitchDeg)
{
    const float cosRoll = cos_approx(DEGREES_TO_RADIANS(rollDeg) * 0.5f);
    const float sinRoll = sin_approx(DEGREES_TO_RADIANS(rollDeg) * 0.5f);
    const float cosPitch = cos_approx(DEGREES_TO_RADIANS(pitchDeg) * 0.5f);
    const float sinPitch = sin_approx(DEGREES_TO_RADIANS(pitchDeg) * 0.5f);

    qTarget->q0 = cosRoll * cosPitch;
    qTarget->q1 = sinRoll * cosPitch;
    qTarget->q2 = cosRoll * sinPitch;
    qTarget->q3 = -sinRoll * sinPitch;
}

// Earth vertical (up) expressed in the body frame, normalized. Works for a
// slightly denormalized quaternion as well.
static void earthUpInBodyFrame(fpVector3_t *up, const fpQuaternion_t *q)
{
    fpVector3_t v = { .v = { 0.0f, 0.0f, 1.0f } };
    quaternionRotateVector(&v, &v, q);
    const float norm = fast_fsqrtf(sq(v.x) + sq(v.y) + sq(v.z));
    up->x = v.x / norm;
    up->y = v.y / norm;
    up->z = v.z / norm;
}

void orientationHoldComputeAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget)
{
    // Reduced attitude control: regulate the direction of the earth vertical
    // in the body frame instead of the full rotation. The rotation about the
    // vertical axis (heading in normal/inverted flight, body roll at prop
    // hang) is free by construction. Unlike a swing-twist decomposition
    // about earth Z this has no degenerate region near inverted, where
    // w^2 + z^2 vanishes for every heading and the extracted twist direction
    // is noise driven. Large errors do not occur in operation: the target
    // is a persistent state slewed toward the requested attitude, never
    // stepped (the entry path is chosen by the slew, see
    // orientationHoldSlewTarget), so no axis preference is needed here.
    fpVector3_t upEst, upTarget;
    earthUpInBodyFrame(&upEst, qEst);
    earthUpInBodyFrame(&upTarget, qTarget);

    // Shortest rotation taking upEst to upTarget; operand order gives the
    // pidLevel() sign convention (error = target - attitude), pinned by the
    // host convention tests
    fpVector3_t cross = { .v = {
        upTarget.y * upEst.z - upTarget.z * upEst.y,
        upTarget.z * upEst.x - upTarget.x * upEst.z,
        upTarget.x * upEst.y - upTarget.y * upEst.x,
    }};
    const float crossNorm = fast_fsqrtf(sq(cross.x) + sq(cross.y) + sq(cross.z));
    const float dot = upEst.x * upTarget.x + upEst.y * upTarget.y + upEst.z * upTarget.z;
    const float angle = atan2_approx(crossNorm, dot);

    fpVector3_t axis;
    if (crossNorm > 1e-6f) {
        axis.x = cross.x / crossNorm;
        axis.y = cross.y / crossNorm;
        axis.z = cross.z / crossNorm;
    } else if (dot < 0.0f) {
        // Exactly 180 deg of tilt error: rotation axis is ambiguous, pick a
        // deterministic body axis orthogonal to the target up direction
        // (the one least aligned with it)
        fpVector3_t seed = { .v = { 0.0f, 0.0f, 0.0f } };
        if (fabsf(upTarget.x) <= fabsf(upTarget.y) && fabsf(upTarget.x) <= fabsf(upTarget.z)) {
            seed.x = 1.0f;
        } else if (fabsf(upTarget.y) <= fabsf(upTarget.z)) {
            seed.y = 1.0f;
        } else {
            seed.z = 1.0f;
        }
        axis.x = upTarget.y * seed.z - upTarget.z * seed.y;
        axis.y = upTarget.z * seed.x - upTarget.x * seed.z;
        axis.z = upTarget.x * seed.y - upTarget.y * seed.x;
        const float norm = fast_fsqrtf(sq(axis.x) + sq(axis.y) + sq(axis.z));
        axis.x /= norm;
        axis.y /= norm;
        axis.z /= norm;
    } else {
        errDeg->x = errDeg->y = errDeg->z = 0.0f;
        return;
    }

    // Sign matches pidLevel(): error = target - attitude
    errDeg->x = RADIANS_TO_DEGREES(axis.x * angle);
    errDeg->y = RADIANS_TO_DEGREES(axis.y * angle);
    errDeg->z = RADIANS_TO_DEGREES(axis.z * angle);
}

// Rotation vector (body frame, deg) to quaternion. Same sign convention as
// quaternionToAxisAngle / orientationHoldTargetFromRP: positive x = positive
// roll. Do NOT use axisAngleToQuaternion here, it negates the axis.
static void quatFromRotVecDeg(fpQuaternion_t *q, const fpVector3_t *rotVecDeg)
{
    const float angleDeg = fast_fsqrtf(sq(rotVecDeg->x) + sq(rotVecDeg->y) + sq(rotVecDeg->z));
    if (angleDeg < 1e-4f) {
        quaternionInitUnit(q);
        return;
    }
    const float halfRad = DEGREES_TO_RADIANS(angleDeg) * 0.5f;
    const float s = sin_approx(halfRad) / angleDeg;
    q->q0 = cos_approx(halfRad);
    q->q1 = rotVecDeg->x * s;
    q->q2 = rotVecDeg->y * s;
    q->q3 = rotVecDeg->z * s;
}

// The persistent target attitude q_soll: seeded from the estimated attitude
// when a target source engages, then SLEWED toward the source's requested
// attitude instead of stepping there. The regulator error therefore stays
// small at all times; the entry path is an explicit target trajectory.
static fpQuaternion_t qSollState;

// Rotate the tilt of qSoll toward qDesired by at most maxStepDeg.
// Returns the tilt angle (deg) still remaining AFTER the step.
//
// Toward the antipode (engaging inverted from level) the shortest-rotation
// cross product barely rises above noise, so the entry path would be an
// arbitrary mix of roll and yaw (seen as a heading swing while rolling in).
// Do what a pilot does and roll about body X: blend the slew axis
// CONTINUOUSLY from the cross product (tilt <= 120 deg) to body X projected
// orthogonal to the desired up (>= 150 deg). The ramp avoids chattering at
// a hard threshold. Because this shapes the TARGET trajectory, the
// regulator error itself needs no axis preference.
static float orientationHoldSlewTarget(fpQuaternion_t *qSoll, const fpQuaternion_t *qDesired, float maxStepDeg)
{
    fpVector3_t upSoll, upDes;
    earthUpInBodyFrame(&upSoll, qSoll);
    earthUpInBodyFrame(&upDes, qDesired);

    fpVector3_t cross = { .v = {
        upDes.y * upSoll.z - upDes.z * upSoll.y,
        upDes.z * upSoll.x - upDes.x * upSoll.z,
        upDes.x * upSoll.y - upDes.y * upSoll.x,
    }};
    const float crossNorm = fast_fsqrtf(sq(cross.x) + sq(cross.y) + sq(cross.z));
    const float dot = upSoll.x * upDes.x + upSoll.y * upDes.y + upSoll.z * upDes.z;
    const float angleDeg = RADIANS_TO_DEGREES(atan2_approx(crossNorm, dot));

    fpVector3_t axis;
    bool axisValid = false;
    const float wPref = constrainf((-dot - 0.5f) / 0.37f, 0.0f, 1.0f);
    if (wPref > 0.0f) {
        fpVector3_t pref = { .v = {
            1.0f - upDes.x * upDes.x,
            -upDes.x * upDes.y,
            -upDes.x * upDes.z,
        }};
        const float prefNorm = fast_fsqrtf(sq(pref.x) + sq(pref.y) + sq(pref.z));
        if (prefNorm > 1e-3f) {
            float s = 1.0f / prefNorm;
            if (crossNorm > 1e-3f
                && (pref.x * cross.x + pref.y * cross.y + pref.z * cross.z) < 0.0f) {
                s = -s;      // keep the roll direction the cross product started
            }
            const float wCross = (crossNorm > 1e-6f) ? (1.0f - wPref) / crossNorm : 0.0f;
            axis.x = wPref * s * pref.x + wCross * cross.x;
            axis.y = wPref * s * pref.y + wCross * cross.y;
            axis.z = wPref * s * pref.z + wCross * cross.z;
            const float n = fast_fsqrtf(sq(axis.x) + sq(axis.y) + sq(axis.z));
            if (n > 1e-6f) {
                axis.x /= n; axis.y /= n; axis.z /= n;
                axisValid = true;
            }
        }
        // body X parallel to the desired up (prop hang entry from a dive):
        // fall through to the shortest rotation below
    }
    if (!axisValid) {
        if (crossNorm > 1e-6f) {
            axis.x = cross.x / crossNorm;
            axis.y = cross.y / crossNorm;
            axis.z = cross.z / crossNorm;
        } else {
            // aligned (nothing to do) or exact antipode with body X vertical:
            // leave the target where it is, the next cycle disambiguates
            return (dot < 0.0f) ? angleDeg : 0.0f;
        }
    }

    const float stepDeg = MIN(angleDeg, maxStepDeg);
    if (stepDeg > 1e-3f) {
        const fpVector3_t stepVec = { .v = { axis.x * stepDeg, axis.y * stepDeg, axis.z * stepDeg } };
        fpQuaternion_t qStep;
        quatFromRotVecDeg(&qStep, &stepVec);
        quaternionMultiply(qSoll, qSoll, &qStep);
        quaternionNormalize(qSoll, qSoll);
    }
    return angleDeg - stepDeg;
}

// Error leash (ArduPlane qacro pattern): the target never runs further
// ahead of the attitude than the rate loop can catch up within this time.
// Clamping the error BEFORE the re-anchor below pulls the target back by
// the excess -- anti-windup at the target level. It only binds when the
// aircraft cannot follow (saturation, stall, blocked surfaces): in normal
// operation the slewed target keeps the error far smaller.
#define OHOLD_LEASH_TIME_S 0.2f

// Regulator core: tilt error between the estimated attitude and q_soll, then
// re-anchor q_soll on the attitude composed with that error. The twist (the
// free axis: heading in level/inverted flight, body roll at prop hang) of
// the target thereby follows the actual attitude every cycle -- axis
// compliance w_yaw = 0. Held-twist sources (course hold bridging) will skip
// this re-anchoring and feed the full error instead.
static void orientationHoldRegulate(fpVector3_t *errDeg)
{
    orientationHoldComputeAttitudeError(errDeg, &orientation, &qSollState);

    // leash: slowest axis rate bounds what the rate loop can catch up
    // (the tilt error can sit on any body axis, yaw included at the hang)
    uint16_t slowestRate = currentControlProfile->stabilized.rates[FD_ROLL];
    slowestRate = MIN(slowestRate, currentControlProfile->stabilized.rates[FD_PITCH]);
    slowestRate = MIN(slowestRate, currentControlProfile->stabilized.rates[FD_YAW]);
    const float leashDeg = slowestRate * 10.0f * OHOLD_LEASH_TIME_S;
    const float errMag = fast_fsqrtf(sq(errDeg->x) + sq(errDeg->y) + sq(errDeg->z));
    if (errMag > leashDeg) {
        const float s = leashDeg / errMag;
        errDeg->x *= s;
        errDeg->y *= s;
        errDeg->z *= s;
    }

    fpQuaternion_t qErr;
    quatFromRotVecDeg(&qErr, errDeg);
    quaternionMultiply(&qSollState, &orientation, &qErr);
    quaternionNormalize(&qSollState, &qSollState);
}

// Rate-loop I-term reset on target-source switches (e.g. prop hang ->
// knife edge): the accumulated I trims the OLD attitude's holding load
// (propwash vs knife rudder load) and would discharge as a disturbance
// into the new attitude. Within a figure (continuous trajectory) the
// source stays the same and the I-term is kept.
#define OHOLD_SOURCE_NONE   (-1)
#define OHOLD_SOURCE_FLOOR  (-2)
#define OHOLD_SOURCE_FIGURE (-3)
#define OHOLD_SOURCE_LOCK   (-4)

static int activeTargetSource = OHOLD_SOURCE_NONE;

static float holdRefAltCm = 0.0f;   // altitude assist reference, captured at hold entry
static bool presetSlewCaptured = false;   // entry slew has reached the preset once
static void orientationHoldCheckSourceSwitch(int source)
{
    if (source != activeTargetSource) {
        if (activeTargetSource != OHOLD_SOURCE_NONE || source != OHOLD_SOURCE_NONE) {
            pidResetErrorAccumulators();
        }
        activeTargetSource = source;
        // capture the altitude reference for the hold altitude assist at the
        // moment the target engages (same pattern as the figure sequencer)
        holdRefAltCm = getEstimatedActualPosition(Z);
        // seed the persistent target on the actual attitude: the regulator
        // error starts at zero and the entry happens as a target slew
        qSollState = orientation;
        presetSlewCaptured = false;
    }
}

bool orientationHoldIsPropHang(void)
{
    return activeTargetSource == BOXPROPHANG;
}

bool orientationHoldSticksAreTargetOffsets(void)
{
    // preset sources carry the box id (positive); the special sources
    // (floor/figure/lock/none) are negative
    return activeTargetSource >= 0
        && orientationHoldConfig()->stickAngleMaxDeg > 0;
}

// ---- Learned damping reserve for the hover regime -------------------------
//
// Hovering has almost no natural aerodynamic damping (no airflow from
// forward motion over the tail) and the prop-wash moment responds with a
// lag, so angle-loop gains that are well damped in forward flight can limit
// cycle around the vertical: a growing 1-2 Hz pitch/yaw oscillation with
// the surfaces far from saturation. Instead of a hand-tuned hover gain the
// controller LEARNS its own damping reserve, the same philosophy as the
// hover throttle learning its hover point:
//  - detect the limit cycle per tilt axis: decisive zero crossings of the
//    attitude error at 0.4..3 Hz with amplitude above a floor
//  - each detected half wave backs the angle gain off fast (attack)
//  - quiet time recovers it slowly toward 1.0 (release)
// The scale settles just below the stability boundary for the actual
// airframe, CG and battery state. Active only while the PROP HANG preset
// holds near vertical; it re-learns on every hang on purpose (no setting,
// no persistence).

// Wide band on purpose: a 1.5 m aerobat limit cycles at 1-2 Hz, a 0.7 m
// model with its small inertia rather at 4-8 Hz. Noise rejection is the
// job of the amplitude gates below, not of the band.
#define HOVER_OSC_MIN_HALFWAVE_S 0.06f   // 0.4..8 Hz band
#define HOVER_OSC_MAX_HALFWAVE_S 1.2f
#define HOVER_OSC_AMPLITUDE_DEG  2.0f    // ignore noise-level wobble
#define HOVER_OSC_CROSS_DEG      0.5f    // decisive zero crossing
#define HOVER_GAIN_ATTACK        0.85f   // per detected half wave
#define HOVER_GAIN_FLOOR         0.3f
#define HOVER_GAIN_RELEASE_TAU_S 4.0f

typedef struct {
    float sign;         // sign of the current half wave
    float peakDeg;      // amplitude seen since the last crossing
    float sinceFlipS;
} hoverOscDetector_t;

// The scale PERSISTS: it freezes at hang exit (the next hang starts at the
// learned value instead of oscillating its way down again), is written back
// to the config at exit and saved to EEPROM on disarm. A value learned
// under worse conditions self-corrects upward through the release while
// hovering quietly.
static float hoverGainScale;
static bool hoverGainInitialized = false;
static bool hoverGainWasActive = false;
static bool hoverGainDirty = false;      // learned value awaiting the disarm save
static hoverOscDetector_t hoverOsc[2];   // body pitch, body yaw

static bool hoverOscDetectAxis(hoverOscDetector_t *d, float sigDeg, float dT)
{
    d->sinceFlipS += dT;
    d->peakDeg = MAX(d->peakDeg, fabsf(sigDeg));
    if (d->sign == 0.0f) {
        d->sign = (sigDeg >= 0.0f) ? 1.0f : -1.0f;
        return false;
    }
    if (sigDeg * d->sign < 0.0f && fabsf(sigDeg) > HOVER_OSC_CROSS_DEG) {
        const bool osc = d->sinceFlipS > HOVER_OSC_MIN_HALFWAVE_S
                      && d->sinceFlipS < HOVER_OSC_MAX_HALFWAVE_S
                      && d->peakDeg > HOVER_OSC_AMPLITUDE_DEG;
        d->sign = (sigDeg > 0.0f) ? 1.0f : -1.0f;
        d->peakDeg = fabsf(sigDeg);
        d->sinceFlipS = 0.0f;
        return osc;
    }
    return false;
}

static void hoverGainUpdate(const fpVector3_t *errDeg, float dT)
{
    if (!hoverGainInitialized) {
        hoverGainScale = constrainf(orientationHoldConfig()->hoverGainLearned / 100.0f,
                                    HOVER_GAIN_FLOOR, 1.0f);
        hoverGainInitialized = true;
    }

    bool active = activeTargetSource == BOXPROPHANG;
    if (active) {
        // nose elevation gate, same release threshold as the hover throttle
        fpVector3_t nose = { .v = { 1.0f, 0.0f, 0.0f } };
        quaternionRotateVectorInv(&nose, &nose, &orientation);
        active = RADIANS_TO_DEGREES(asin_approx(constrainf(-nose.z, -1.0f, 1.0f))) > 45.0f;
    }

    if (!active) {
        // freeze the learned value across hang exits; write it back once so
        // the disarm save picks it up
        if (hoverGainWasActive) {
            const uint8_t learned = lrintf(hoverGainScale * 100.0f);
            if (learned != orientationHoldConfig()->hoverGainLearned) {
                orientationHoldConfigMutable()->hoverGainLearned = learned;
                hoverGainDirty = true;
            }
            hoverGainWasActive = false;
        }
        hoverOsc[0] = hoverOsc[1] = (hoverOscDetector_t){ 0 };
        return;
    }
    hoverGainWasActive = true;

    bool osc = hoverOscDetectAxis(&hoverOsc[0], errDeg->y, dT);
    osc = hoverOscDetectAxis(&hoverOsc[1], errDeg->z, dT) || osc;

    if (osc) {
        hoverGainScale = MAX(HOVER_GAIN_FLOOR, hoverGainScale * HOVER_GAIN_ATTACK);
    } else {
        hoverGainScale += (1.0f - hoverGainScale) * MIN(dT / HOVER_GAIN_RELEASE_TAU_S, 1.0f);
    }
}

float orientationHoldLevelGainScale(void)
{
    return (hoverGainInitialized) ? hoverGainScale : 1.0f;
}

void orientationHoldSyncTargetToAttitude(void)
{
    // open-loop flying (figure IMPULSE): the persistent target must not go
    // stale while the regulator is bypassed -- re-seed it on the attitude so
    // the catch afterwards slews from where the aircraft actually is
    qSollState = orientation;
}

void orientationHoldResetSourceTracking(void)
{
    // mode left: also reset, the attitude's holding-load trim in the I-term
    // would discharge into the pilot's manual/acro flying otherwise
    if (activeTargetSource != OHOLD_SOURCE_NONE) {
        pidResetErrorAccumulators();
        activeTargetSource = OHOLD_SOURCE_NONE;
    }

    // persist the learned hover gain once the aircraft is on the ground
    // (never write EEPROM while armed, the flight loop would stall)
    if (hoverGainDirty && !ARMING_FLAG(ARMED)) {
        hoverGainDirty = false;
        saveConfigAndNotify();
    }
}

bool orientationHoldComputeError(fpVector3_t *errDeg, float dT)
{
    fpQuaternion_t qDesired;
    // Preset entries slew the target with their own rate: same mechanism as
    // a figure segment, but a snappy entry and a deliberate slow roll figure
    // are different intents with different rates
    float slewRateDegS = orientationHoldConfig()->entryRateDps;

    // Altitude floor recovery overrides any selected preset: upright + climb.
    // Safety recovery tracks the requested attitude directly, no entry slew.
    if (altitudeFloorRecoveryActive()) {
        orientationHoldCheckSourceSwitch(OHOLD_SOURCE_FLOOR);
        orientationHoldTargetFromRP(&qDesired, 0.0f, altitudeFloorRecoveryPitchDeg());
        slewRateDegS = 0.0f;
    } else if (figureSequencerRequested()) {
        float figRoll, figPitch;
        orientationHoldCheckSourceSwitch(OHOLD_SOURCE_FIGURE);
        figureSequencerGetTarget(&figRoll, &figPitch);
        orientationHoldTargetFromRP(&qDesired, figRoll, figPitch);
        // the trajectory is already rate shaped; the slew only smooths the
        // engage and absolute HOLD segment steps
        slewRateDegS = MAX(figureSequencerConfig()->rollRate, figureSequencerConfig()->loopRate);
    } else if (orientationHoldActivePreset() == NULL && IS_RC_MODE_ACTIVE(BOXATTLOCK)) {
        // 3D LOCK: sticks centered = hold the attitude captured at release;
        // sticks deflected = pure rate flying, the lock target follows the
        // aircraft and freezes on the NEW attitude when the sticks center
        orientationHoldCheckSourceSwitch(OHOLD_SOURCE_LOCK);
        if (orientationHoldSticksDeflected()) {
            qSollState = orientation;
        }
        qDesired = qSollState;
        slewRateDegS = 0.0f;
    } else {
        const orientationHoldPreset_t *preset = orientationHoldActivePreset();
        if (!preset) {
            return false;
        }
        orientationHoldCheckSourceSwitch(preset->box);
        // Per attitude pitch trim, as Euler pitch of the target: positive is
        // always "nose above the horizon" regardless of the attitude's roll
        float pitchTrim = 0.0f;
        if (preset->box == BOXINVERTED) {
            pitchTrim = orientationHoldConfig()->invertedPitchTrim;
        } else if (preset->box == BOXKNIFELEFT) {
            pitchTrim = orientationHoldConfig()->knifeLeftPitchTrim;
        } else if (preset->box == BOXKNIFERIGHT) {
            pitchTrim = orientationHoldConfig()->knifeRightPitchTrim;
        }
        // Active altitude hold on top of the static trim: same assist as the
        // figure sequencer, referenced to the entry altitude. The cos-blend
        // inside fades it out toward nose-vertical (prop hang), where
        // altitude is owned by the hover throttle controller instead.
        // The assist only engages once the attitude has captured the preset:
        // during the entry the transient altitude error would deflect the
        // target (seen as a knife-edge entry stalling at half the bank) and
        // the entry itself must stay a pure attitude move.
        orientationHoldTargetFromRP(&qDesired, preset->rollDeg, preset->pitchDeg + pitchTrim);
        fpVector3_t entryErr;
        orientationHoldComputeAttitudeError(&entryErr, &orientation, &qDesired);
        if (fabsf(entryErr.x) < 25.0f && fabsf(entryErr.y) < 25.0f) {
            const float assistDeg = figureAltitudeAssistDeg(preset->pitchDeg + pitchTrim, holdRefAltCm);
            orientationHoldTargetFromRP(&qDesired, preset->rollDeg, preset->pitchDeg + pitchTrim + assistDeg);
        } else {
            // still capturing: keep the altitude reference tracking so the
            // assist later holds the altitude where the attitude settled,
            // not where the switch was flipped
            holdRefAltCm = getEstimatedActualPosition(Z);
        }

        // Pilot stick offsets, ANGLE semantics around the rotated reference:
        // deflection = body-frame angle offset from the preset, held while
        // deflected; centered sticks return the target slowly. Yaw stays a
        // rate command (the free axis). While this is active the rate path
        // must not also feed roll/pitch sticks as rates, see
        // orientationHoldSticksAreTargetOffsets().
        if (orientationHoldConfig()->stickAngleMaxDeg > 0) {
            const float rollOffDeg = orientationHoldStickNorm(rcCommand[ROLL], rcControlsConfig()->deadband)
                                     * orientationHoldConfig()->stickAngleMaxDeg;
            const float pitchOffDeg = orientationHoldStickNorm(rcCommand[PITCH], rcControlsConfig()->deadband)
                                      * orientationHoldConfig()->stickAngleMaxDeg;
            if (rollOffDeg != 0.0f || pitchOffDeg != 0.0f) {
                const fpVector3_t offVec = { .v = { rollOffDeg, pitchOffDeg, 0.0f } };
                fpQuaternion_t qOff;
                quatFromRotVecDeg(&qOff, &offVec);
                quaternionMultiply(&qDesired, &qDesired, &qOff);
                // carving: keep following at the entry rate
            } else if (presetSlewCaptured) {
                // sticks centered after capture: gentle return to the preset
                slewRateDegS = orientationHoldConfig()->stickReturnRateDps;
            }
        }
    }

    if (slewRateDegS > 0.0f) {
        const float remainingDeg = orientationHoldSlewTarget(&qSollState, &qDesired, slewRateDegS * dT);
        if (remainingDeg < 1.0f) {
            presetSlewCaptured = true;
        }
    } else {
        qSollState = qDesired;
    }

    orientationHoldRegulate(errDeg);
    hoverGainUpdate(errDeg, dT);
    return true;
}

#endif // USE_ORIENTATION_HOLD
