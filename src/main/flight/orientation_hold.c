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

#include "drivers/time.h"

#include "flight/altitude_floor.h"
#include "flight/figure_sequencer.h"

#include "navigation/navigation.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/orientation_hold.h"
#include "flight/pid.h"

#include "flight/servos.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"

PG_REGISTER_WITH_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig, PG_ORIENTATION_HOLD_CONFIG, 2);

PG_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig,
    .invertedPitchTrim = SETTING_OHOLD_INVERTED_PITCH_TRIM_DEFAULT,
    .knifeLeftPitchTrim = SETTING_OHOLD_KNIFE_LEFT_PITCH_TRIM_DEFAULT,
    .knifeRightPitchTrim = SETTING_OHOLD_KNIFE_RIGHT_PITCH_TRIM_DEFAULT,
    .hoverGainLearned = SETTING_OHOLD_HOVER_GAIN_DEFAULT,
    .invertedGainLearned = SETTING_OHOLD_INVERTED_GAIN_DEFAULT,
    .knifeGainLearned = SETTING_OHOLD_KNIFE_GAIN_DEFAULT,
    .figureGainLearned = SETTING_OHOLD_FIGURE_GAIN_DEFAULT,
    .entryRateDps = SETTING_OHOLD_ENTRY_RATE_DEFAULT,
    .stickAngleMaxDeg = SETTING_OHOLD_STICK_ANGLE_DEFAULT,
    .stickReturnRateDps = SETTING_OHOLD_STICK_RETURN_RATE_DEFAULT,
    .knifeSpeedFF = SETTING_OHOLD_KNIFE_SPEED_FF_DEFAULT,
    .loadLimitG = SETTING_OHOLD_LOAD_LIMIT_DEFAULT,
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

// FLAT SPIN is a spin BEHAVIOR, not one fixed attitude: the tilt is
// regulated onto the held target while the pilot's rudder commands a
// rotation about the EARTH VERTICAL - the axis the reduced attitude error
// leaves free by construction, in every attitude. The attitude selector
// picks the held target (none = flat): INVERTED = inverted flat spin,
// KNIFE = knife edge spin, PROP HANG = torque roll. Releasing the rudder
// stops the rotation with the attitude still held; releasing the box
// recovers normally. No altitude assist: a spin descends by design (the
// altitude floor still preempts globally).
static const orientationHoldPreset_t orientationHoldSpinPresets[] = {
    { BOXFSPIN,         180.0f,  0.0f },   // + INVERTED
    { BOXFSPIN,         -90.0f,  0.0f },   // + KNIFE LEFT
    { BOXFSPIN,          90.0f,  0.0f },   // + KNIFE RIGHT
    { BOXFSPIN,           0.0f, 90.0f },   // + PROP HANG (torque roll)
    { BOXFSPIN,           0.0f,  0.0f },   // alone: flat spin
};

static const orientationHoldPreset_t * orientationHoldActivePreset(void)
{
    // A knife edge is physically held on the yaw effector (rudder or TVC
    // yaw vane). On a mixer without one (flying wing) the knife boxes are
    // not offered (fc_msp_box), and a stale configuration that still maps
    // them is ignored here - the laws of aerodynamics outrank the switch.
    const bool knifePossible = servoMixerHasYawControl();
    if (IS_RC_MODE_ACTIVE(BOXFSPIN)) {
        if (IS_RC_MODE_ACTIVE(BOXINVERTED))   return &orientationHoldSpinPresets[0];
        if (IS_RC_MODE_ACTIVE(BOXKNIFELEFT) && knifePossible)  return &orientationHoldSpinPresets[1];
        if (IS_RC_MODE_ACTIVE(BOXKNIFERIGHT) && knifePossible) return &orientationHoldSpinPresets[2];
        if (IS_RC_MODE_ACTIVE(BOXPROPHANG))   return &orientationHoldSpinPresets[3];
        return &orientationHoldSpinPresets[4];
    }
    for (unsigned i = 0; i < ARRAYLEN(orientationHoldPresets); i++) {
        if ((orientationHoldPresets[i].box == BOXKNIFELEFT
             || orientationHoldPresets[i].box == BOXKNIFERIGHT) && !knifePossible) {
            continue;
        }
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

static void orientationHoldComputeFullAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget);

// Full-attitude slew (rate-limited slerp) for the figure line-hold: the
// reduced slew above works on the up vectors only and is heading-free by
// design - correct for pilot holds, but it can never close the heading gap
// to a yaw-anchored figure target (the slewed target simply inherits the
// drifted heading). Anchored figures slew the FULL rotation instead. No
// antipode axis preference needed: figures start on the current attitude
// and the trajectory is fortgeschrieben, the relative angle stays small.
static float orientationHoldSlewTargetFull(fpQuaternion_t *qSoll, const fpQuaternion_t *qDesired, float maxStepDeg)
{
    fpVector3_t errDeg;
    orientationHoldComputeFullAttitudeError(&errDeg, qSoll, qDesired);
    const float angleDeg = fast_fsqrtf(sq(errDeg.x) + sq(errDeg.y) + sq(errDeg.z));
    const float stepDeg = MIN(angleDeg, maxStepDeg);
    if (stepDeg > 1e-3f) {
        const float s = stepDeg / angleDeg;
        const fpVector3_t stepVec = { .v = { errDeg.x * s, errDeg.y * s, errDeg.z * s } };
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
static bool figureLineAnchored = false;
static fpQuaternion_t qFigureYawAnchor;
static void orientationHoldComputeFullAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget);

static void orientationHoldRegulate(fpVector3_t *errDeg)
{
    if (figureLineAnchored) {
        // figure on a line: the full error adds the heading (twist)
        // component the reduced error deliberately drops
        orientationHoldComputeFullAttitudeError(errDeg, &orientation, &qSollState);
    } else {
        orientationHoldComputeAttitudeError(errDeg, &orientation, &qSollState);
    }

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
#define OHOLD_SOURCE_EXIT   (-5)

// Exit handover thresholds: engage only when the released attitude is far
// enough from level that the instant Euler error would command full rates;
// hand to ANGLE once the attitude has followed the target to the horizon
#define OHOLD_EXIT_SLEW_ENGAGE_DEG   30.0f
#define OHOLD_EXIT_SLEW_DONE_DEG     10.0f
#define OHOLD_EXIT_SLEW_TIMEOUT_S     3.0f

static int activeTargetSource = OHOLD_SOURCE_NONE;
static bool exitSlewActive = false;
static float exitSlewTimeS = 0.0f;

static float holdRefAltCm = 0.0f;   // altitude assist reference, captured at hold entry
static bool presetSlewCaptured = false;   // entry slew has reached the preset once
static void orientationHoldCheckSourceSwitch(int source)
{
    if (source != activeTargetSource) {
        if (activeTargetSource != OHOLD_SOURCE_NONE || source != OHOLD_SOURCE_NONE) {
            pidResetErrorAccumulators();
        }
        activeTargetSource = source;
        // any real hold taking over cancels a pending exit handover
        if (source != OHOLD_SOURCE_EXIT) {
            exitSlewActive = false;
        }
        figureLineAnchored = false;
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

bool orientationHoldIsKnifeOrInverted(void)
{
    return activeTargetSource == BOXINVERTED
        || activeTargetSource == BOXKNIFELEFT
        || activeTargetSource == BOXKNIFERIGHT;
}

bool orientationHoldIsSpinAboutVertical(void)
{
    return activeTargetSource == BOXFSPIN;
}

void orientationHoldUpInBody(fpVector3_t *upBody)
{
    const fpVector3_t upEarth = { .v = { 0.0f, 0.0f, 1.0f } };
    quaternionRotateVector(upBody, &upEarth, &orientation);
}

// ---- Figure line-hold ------------------------------------------------------
//
// Pilot holds are heading-free by design (the reduced attitude error drops
// the rotation about the earth vertical), but a FIGURE flown on a line must
// not be: with the target rotating about the body axis and heading free, a
// slow roll wandered ~15 deg of course per roll in SITL - the roll axis
// follows wherever the nose drifts and nothing pulls it back. Figures
// therefore anchor their trajectory to the heading captured at figure start
// and regulate the FULL attitude error. Verified identity (bench
// math_verify G3): the full error differs from the reduced one exactly by
// the twist about body-up, so tilt regulation is unchanged and the line
// hold is purely additive. State lives next to the source tracking above.
static void figureCaptureYawAnchor(void)
{
    const float halfPsiRad = DECIDEGREES_TO_RADIANS(attitude.values.yaw) * 0.5f;
    qFigureYawAnchor.q0 = cos_approx(halfPsiRad);
    qFigureYawAnchor.q1 = 0.0f;
    qFigureYawAnchor.q2 = 0.0f;
    qFigureYawAnchor.q3 = sin_approx(halfPsiRad);
}

// Full attitude error: q_err = conj(q_est) (x) q_target, as a rotation
// vector in the body frame (deg). Same sign convention as the reduced
// error / pidLevel; quaternionToAxisAngle wraps to the shortest path.
// Bench mirror: math_verify.py section G (checked against scipy).
static void orientationHoldComputeFullAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget)
{
    fpQuaternion_t qConj, qErr;
    quaternionConjugate(&qConj, qEst);
    quaternionMultiply(&qErr, &qConj, qTarget);
    fpAxisAngle_t aa;
    quaternionToAxisAngle(&aa, &qErr);
    const float angleDeg = RADIANS_TO_DEGREES(aa.angle);
    errDeg->x = aa.axis.x * angleDeg;
    errDeg->y = aa.axis.y * angleDeg;
    errDeg->z = aa.axis.z * angleDeg;
}

bool orientationHoldSticksAreTargetOffsets(void)
{
    // preset sources carry the box id (positive); the special sources
    // (floor/figure/lock/none) are negative
    return activeTargetSource >= 0
        && orientationHoldConfig()->stickAngleMaxDeg > 0;
}

// ---- Learned damping reserve per regime ------------------------------------
//
// The NORMAL-FLIGHT gains are the reference; every hold regime runs on a
// single learned SCALE of them instead of its own gain set. Aerobatic
// regimes change the plant gain (prop wash instead of airflow at the hang,
// fuselage lift at the knife edge, transients at figure boundaries), so
// gains that are well damped in forward flight can limit cycle there - a
// growing oscillation with the surfaces far from saturation. Instead of
// hand tuning each regime the controller LEARNS its damping reserve:
//  - detect the limit cycle per tilt axis: decisive zero crossings of the
//    attitude error at 0.4..8 Hz with amplitude above a floor
//  - each detected half wave backs the angle-gain scale off fast (attack)
//  - quiet time recovers it slowly toward 1.0 (release)
// One scale per regime (hover / inverted / knife / figure), each persisted:
// flying the same figure repeatedly converges its scale, so the figures
// get better with every flight. The scale settles just below the stability
// boundary for the actual airframe, CG and battery state. Spins are
// excluded: their rotation is not a limit cycle to tune away.

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

// The scales PERSIST: each freezes at regime exit (the next entry starts at
// the learned value instead of oscillating its way down again), is written
// back to the config at exit and saved to EEPROM on disarm. A value learned
// under worse conditions self-corrects upward through the release while
// flying that regime quietly.
typedef enum {
    OHOLD_REGIME_NONE = -1,
    OHOLD_REGIME_HOVER = 0,
    OHOLD_REGIME_INVERTED,
    OHOLD_REGIME_KNIFE,
    OHOLD_REGIME_FIGURE,
    OHOLD_REGIME_COUNT
} oholdRegime_e;

typedef struct {
    float scale;
    bool wasActive;
    hoverOscDetector_t osc[2];
} regimeGainState_t;

static regimeGainState_t regimeGain[OHOLD_REGIME_COUNT];
static bool regimeGainInitialized = false;
static bool regimeGainDirty = false;     // learned value awaiting the disarm save

static uint8_t * regimeGainConfigField(oholdRegime_e regime)
{
    switch (regime) {
        case OHOLD_REGIME_HOVER:    return &orientationHoldConfigMutable()->hoverGainLearned;
        case OHOLD_REGIME_INVERTED: return &orientationHoldConfigMutable()->invertedGainLearned;
        case OHOLD_REGIME_KNIFE:    return &orientationHoldConfigMutable()->knifeGainLearned;
        default:                    return &orientationHoldConfigMutable()->figureGainLearned;
    }
}

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

// freeze the learned value when a regime ends; write it back once so the
// disarm save picks it up
static void regimeGainFreeze(oholdRegime_e regime)
{
    regimeGainState_t *g = &regimeGain[regime];
    if (g->wasActive) {
        const uint8_t learned = lrintf(g->scale * 100.0f);
        if (learned != *regimeGainConfigField(regime)) {
            *regimeGainConfigField(regime) = learned;
            regimeGainDirty = true;
        }
        g->wasActive = false;
    }
}

static void regimeGainFreezeAll(void)
{
    for (int r = 0; r < OHOLD_REGIME_COUNT; r++) {
        regimeGainFreeze((oholdRegime_e)r);
    }
}

// which learning regime the current target source belongs to; spins and
// the special sources (lock / floor / exit handover) learn nothing
static oholdRegime_e regimeGainActiveRegime(void)
{
    switch (activeTargetSource) {
        case BOXPROPHANG: {
            // nose elevation gate, same release threshold as the hover throttle
            fpVector3_t nose = { .v = { 1.0f, 0.0f, 0.0f } };
            quaternionRotateVectorInv(&nose, &nose, &orientation);
            return RADIANS_TO_DEGREES(asin_approx(constrainf(-nose.z, -1.0f, 1.0f))) > 45.0f
                 ? OHOLD_REGIME_HOVER : OHOLD_REGIME_NONE;
        }
        case BOXINVERTED:
            return OHOLD_REGIME_INVERTED;
        case BOXKNIFELEFT:
        case BOXKNIFERIGHT:
            return OHOLD_REGIME_KNIFE;
        case OHOLD_SOURCE_FIGURE:
            return OHOLD_REGIME_FIGURE;
        default:
            return OHOLD_REGIME_NONE;
    }
}

static void regimeGainUpdate(const fpVector3_t *errDeg, float dT)
{
    if (!regimeGainInitialized) {
        for (int r = 0; r < OHOLD_REGIME_COUNT; r++) {
            regimeGain[r].scale = constrainf(*regimeGainConfigField((oholdRegime_e)r) / 100.0f,
                                             HOVER_GAIN_FLOOR, 1.0f);
        }
        regimeGainInitialized = true;
    }

    const oholdRegime_e active = regimeGainActiveRegime();
    for (int r = 0; r < OHOLD_REGIME_COUNT; r++) {
        if (r != active && regimeGain[r].wasActive) {
            regimeGainFreeze((oholdRegime_e)r);
            regimeGain[r].osc[0] = regimeGain[r].osc[1] = (hoverOscDetector_t){ 0 };
        }
    }
    if (active == OHOLD_REGIME_NONE) {
        return;
    }
    regimeGainState_t *g = &regimeGain[active];
    g->wasActive = true;

    // the hang limit cycles on body pitch/yaw (prop wash axes); the other
    // regimes on the tilt axes roll/pitch
    const float sigA = (active == OHOLD_REGIME_HOVER) ? errDeg->y : errDeg->x;
    const float sigB = (active == OHOLD_REGIME_HOVER) ? errDeg->z : errDeg->y;
    bool osc = hoverOscDetectAxis(&g->osc[0], sigA, dT);
    osc = hoverOscDetectAxis(&g->osc[1], sigB, dT) || osc;

    if (osc) {
        g->scale = MAX(HOVER_GAIN_FLOOR, g->scale * HOVER_GAIN_ATTACK);
    } else {
        g->scale += (1.0f - g->scale) * MIN(dT / HOVER_GAIN_RELEASE_TAU_S, 1.0f);
    }
}

bool orientationHoldRegimeOscillating(void)
{
    // true while the ACTIVE regime's limit-cycle detector recently fired:
    // the learned scale sits measurably below the reference. At the knife
    // edge instability usually means "too slow, the surfaces are starving" -
    // the throttle assist raises the speed on this signal (more airflow is
    // the physical cure, backing the gain off only treats the symptom).
    if (!regimeGainInitialized) {
        return false;
    }
    const oholdRegime_e active = regimeGainActiveRegime();
    return active != OHOLD_REGIME_NONE
        && regimeGain[active].wasActive
        && regimeGain[active].scale < 0.9f;
}

float orientationHoldLevelGainScale(void)
{
    // the learned damping reserve of the ACTIVE regime; everything else
    // (normal flight, lock, spins, handover) runs the reference gains.
    // The scale of the target regime applies from the moment the source
    // switches, so the entry slew already flies with it.
    if (!regimeGainInitialized) {
        return 1.0f;
    }
    const oholdRegime_e active = regimeGainActiveRegime();
    return (active != OHOLD_REGIME_NONE && regimeGain[active].wasActive)
         ? regimeGain[active].scale : 1.0f;
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
    exitSlewActive = false;

    // leaving the mode ends every learning regime: freeze the learned
    // gains (landing straight out of a hold and disarming must not lose them)
    regimeGainFreezeAll();

    // persist the learned regime gains once the aircraft is on the ground
    // (never write EEPROM while armed, the flight loop would stall)
    if (regimeGainDirty && !ARMING_FLAG(ARMED)) {
        regimeGainDirty = false;
        saveConfigAndNotify();
    }
}

// Exit handover toward ANGLE: releasing a hold far from level must not drop
// the full attitude error onto the Euler level controller at once. A prop
// hang exit otherwise whips: ~90 deg of error at near zero airspeed commands
// the full rates, the airframe has no authority to arrest the resulting
// pitch rate at the horizon and slices through to nose down before ANGLE
// catches it. Instead the hold keeps the aircraft one more transition and
// slews its target to level; ANGLE takes over once the attitude is there.
bool orientationHoldExitSlewPending(void)
{
    if (exitSlewActive) {
        return true;
    }
    // only a released real hold (preset / figure / lock) hands over; floor
    // recovery ends level by construction and an idle mode has nothing to do
    if (activeTargetSource == OHOLD_SOURCE_NONE || activeTargetSource == OHOLD_SOURCE_FLOOR
        || activeTargetSource == OHOLD_SOURCE_EXIT) {
        return false;
    }
    if (orientationHoldIsRequested()) {
        return false;
    }
    fpQuaternion_t qLevel;
    fpVector3_t tiltErr;
    orientationHoldTargetFromRP(&qLevel, 0.0f, 0.0f);
    orientationHoldComputeAttitudeError(&tiltErr, &orientation, &qLevel);
    if (fabsf(tiltErr.x) < OHOLD_EXIT_SLEW_ENGAGE_DEG && fabsf(tiltErr.y) < OHOLD_EXIT_SLEW_ENGAGE_DEG) {
        return false;
    }
    exitSlewActive = true;
    exitSlewTimeS = 0.0f;
    return true;
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
        // line-hold: fly the figure about the heading captured at figure
        // start instead of wherever the nose currently points. Segments
        // that change the heading on purpose (WAIT_POS banks toward home)
        // or fly open loop (impulse, spin) release the anchor; it
        // re-captures on the CURRENT heading when they complete.
        if (figureSequencerHeadingAnchored()) {
            if (!figureLineAnchored) {
                figureCaptureYawAnchor();
                figureLineAnchored = true;
            }
            quaternionMultiply(&qDesired, &qFigureYawAnchor, &qDesired);
        } else {
            figureLineAnchored = false;
        }
        // the trajectory is already rate shaped; the slew only smooths the
        // engage and absolute HOLD segment steps. The slew must OUTRUN the
        // trajectory: at exactly the figure rate it chases the rotating
        // target saturated, the entire budget goes into the figure and the
        // line-hold's heading correction never closes - the slewed target
        // absorbs the heading drift instead of holding the line (seen as
        // 12 deg of FC-frame course walk during one slow roll)
        slewRateDegS = MAX(figureSequencerConfig()->rollRate, figureSequencerConfig()->loopRate) + 90.0f;
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
    } else if (exitSlewActive && orientationHoldActivePreset() == NULL) {
        // exit handover: slew the target to level at the entry rate, then
        // hand to ANGLE. The pilot deflecting a stick takes over instantly.
        orientationHoldCheckSourceSwitch(OHOLD_SOURCE_EXIT);
        orientationHoldTargetFromRP(&qDesired, 0.0f, 0.0f);
        fpVector3_t tiltErr;
        orientationHoldComputeAttitudeError(&tiltErr, &orientation, &qDesired);
        exitSlewTimeS += dT;
        if (orientationHoldSticksDeflected()
            || exitSlewTimeS > OHOLD_EXIT_SLEW_TIMEOUT_S
            || (presetSlewCaptured
                && fabsf(tiltErr.x) < OHOLD_EXIT_SLEW_DONE_DEG
                && fabsf(tiltErr.y) < OHOLD_EXIT_SLEW_DONE_DEG)) {
            exitSlewActive = false;
            return false;
        }
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
        } else if (preset->box == BOXKNIFELEFT || preset->box == BOXKNIFERIGHT) {
            pitchTrim = (preset->box == BOXKNIFELEFT)
                      ? orientationHoldConfig()->knifeLeftPitchTrim
                      : orientationHoldConfig()->knifeRightPitchTrim;
            // Knife edge speed feedforward: the fuselage side force carries
            // the weight and scales with v^2, so LESS speed needs MORE nose
            // angle IMMEDIATELY - not only after an altitude error has built
            // up for the (reactive) assist. Without an airspeed sensor the
            // own throttle is the v^2 proxy (T ~ v^2 in steady flight); the
            // prop wash over the tail linearizes the theoretical hyperbola,
            // so a linear term around the mid-throttle trim point is the
            // honest model. 0 disables (default).
            if (orientationHoldConfig()->knifeSpeedFF > 0) {
                const float uGas = constrainf((rcCommand[THROTTLE] - 1000) / 1000.0f, 0.0f, 1.0f);
                pitchTrim += orientationHoldConfig()->knifeSpeedFF * (0.5f - uGas);
            }
        }
        // Active altitude hold on top of the static trim: same assist as the
        // figure sequencer, referenced to the entry altitude. The cos-blend
        // inside fades it out toward nose-vertical (prop hang), where
        // altitude is owned by the hover throttle controller instead.
        // The assist only engages once the attitude has captured the preset:
        // during the entry the transient altitude error would deflect the
        // target (seen as a knife-edge entry stalling at half the bank) and
        // the entry itself must stay a pure attitude move.
        // Pilot stick offsets, ANGLE semantics around the rotated reference:
        // deflection = body-frame angle offset from the preset, held while
        // deflected; centered sticks return the target slowly. Yaw stays a
        // rate command (the free axis). While this is active the rate path
        // must not also feed roll/pitch sticks as rates, see
        // orientationHoldSticksAreTargetOffsets().
        float rollOffDeg = 0.0f;
        float pitchOffDeg = 0.0f;
        if (orientationHoldConfig()->stickAngleMaxDeg > 0) {
            rollOffDeg = orientationHoldStickNorm(rcCommand[ROLL], rcControlsConfig()->deadband)
                         * orientationHoldConfig()->stickAngleMaxDeg;
            pitchOffDeg = orientationHoldStickNorm(rcCommand[PITCH], rcControlsConfig()->deadband)
                          * orientationHoldConfig()->stickAngleMaxDeg;
        }

        orientationHoldTargetFromRP(&qDesired, preset->rollDeg, preset->pitchDeg + pitchTrim);
        fpVector3_t entryErr;
        orientationHoldComputeAttitudeError(&entryErr, &orientation, &qDesired);
        // The altitude assist yields to a deliberate pitch input (same
        // pattern as the hover throttle stick override): the pilot owns the
        // altitude while the pitch stick is deflected, and the reference
        // tracks so the release locks the NEW altitude. The FLAT SPIN mode
        // never gets the assist: a spin descends by design.
        if (fabsf(entryErr.x) < 25.0f && fabsf(entryErr.y) < 25.0f && pitchOffDeg == 0.0f
            && preset->box != BOXFSPIN) {
            const float assistDeg = figureAltitudeAssistDeg(preset->pitchDeg + pitchTrim, holdRefAltCm);
            orientationHoldTargetFromRP(&qDesired, preset->rollDeg, preset->pitchDeg + pitchTrim + assistDeg);
        } else {
            // still capturing or pilot pitching: keep the altitude reference
            // tracking so the assist later holds the altitude where the
            // attitude settled / the pilot leveled off
            holdRefAltCm = getEstimatedActualPosition(Z);
        }

        if (rollOffDeg != 0.0f || pitchOffDeg != 0.0f) {
            const fpVector3_t offVec = { .v = { rollOffDeg, pitchOffDeg, 0.0f } };
            fpQuaternion_t qOff;
            quatFromRotVecDeg(&qOff, &offVec);
            quaternionMultiply(&qDesired, &qDesired, &qOff);
            // carving: keep following at the entry rate
        } else if (orientationHoldConfig()->stickAngleMaxDeg > 0 && presetSlewCaptured) {
            // sticks centered after capture: gentle return to the preset
            slewRateDegS = orientationHoldConfig()->stickReturnRateDps;
        }
    }

    if (slewRateDegS > 0.0f) {
        // the load governor also paces the target slew: the hardest load of
        // a maneuver is the catch-up pull toward a distant target (the loop
        // exit's level recapture read 13 g ungoverned) - and that pull is
        // set by the slew rate, not by the rate clamp
        const float governedStepDeg = slewRateDegS * orientationHoldLoadGovernorScale() * dT;
        const float remainingDeg = figureLineAnchored
            ? orientationHoldSlewTargetFull(&qSollState, &qDesired, governedStepDeg)
            : orientationHoldSlewTarget(&qSollState, &qDesired, governedStepDeg);
        if (remainingDeg < 1.0f) {
            presetSlewCaptured = true;
        }
    } else {
        qSollState = qDesired;
    }

    orientationHoldRegulate(errDeg);
    regimeGainUpdate(errDeg, dT);
    return true;
}

// ---- Load governor ----------------------------------------------------------
//
// A display figure flies "fast AND tight" - and both are the same boundary:
// the load budget (ohold_load_limit, a fact about the airframe). Centripetal
// load a = v * omega, radius r = v^2 / a, so at a given speed the budget is
// simultaneously the fastest rotation and the tightest radius the airframe
// pulls (ungoverned SITL loops read 13 g at the exit pull). Two channels:
//
//  - rotation/slew scale, INTEGRAL: trims down while the filtered load sits
//    above budget, recovers while below - no proportional droop, the load
//    converges ON the budget instead of somewhere above it.
//  - throttle scale, PROPORTIONAL, only while a figure or spin flies: a
//    governed rotation at full power just converts into speed (a = v*omega
//    stays, the loop only widens - measured); bleeding throttle with the
//    overload breaks that energy feedback. Plain holds at 1 g never touch
//    the pilot's throttle.
#define LOAD_GOVERNOR_TAU_S           0.15f
#define LOAD_GOVERNOR_MIN_SCALE       0.2f
#define LOAD_GOVERNOR_TRIM_PER_S      1.5f    // integral trim speed at 100% overload
#define LOAD_GOVERNOR_RECOVER_PER_S   0.5f
#define LOAD_GOVERNOR_THR_MIN_SCALE   0.6f

static float loadGovScale = 1.0f;
static float loadGovThrScale = 1.0f;
static float loadGovAccG = 1.0f;
static timeMs_t loadGovLastMs;

void orientationHoldLoadGovernorUpdate(void)
{
    const timeMs_t nowMs = millis();
    const float dT = constrainf((nowMs - loadGovLastMs) * 0.001f, 0.0f, 0.1f);
    loadGovLastMs = nowMs;

    fpVector3_t accG;
    accGetMeasuredAcceleration(&accG);          // cm/s^2
    const float magG = fast_fsqrtf(sq(accG.x) + sq(accG.y) + sq(accG.z)) / GRAVITY_CMSS;
    loadGovAccG += (magG - loadGovAccG) * MIN(dT / LOAD_GOVERNOR_TAU_S, 1.0f);

    if (orientationHoldConfig()->loadLimitG == 0) {
        loadGovScale = 1.0f;
        loadGovThrScale = 1.0f;
        return;
    }
    const float budgetG = orientationHoldConfig()->loadLimitG / 10.0f;
    const float over = loadGovAccG / budgetG - 1.0f;
    if (over > 0.0f) {
        loadGovScale -= LOAD_GOVERNOR_TRIM_PER_S * over * dT;
    } else {
        loadGovScale += LOAD_GOVERNOR_RECOVER_PER_S * dT;
    }
    loadGovScale = constrainf(loadGovScale, LOAD_GOVERNOR_MIN_SCALE, 1.0f);
    loadGovThrScale = constrainf(1.0f / (1.0f + MAX(over, 0.0f)),
                                 LOAD_GOVERNOR_THR_MIN_SCALE, 1.0f);
}

float orientationHoldLoadGovernorScale(void)
{
    // The governor owns MANEUVERS: figures and spins, including their exit
    // pulls while the box is still up. A plain hold fighting a gust keeps
    // its full slew and rate authority - the load spikes there ARE the
    // gust, not a commanded trajectory, and throttling the recovery lets
    // the disturbance win (a governed TVC hang oscillated itself into
    // never recapturing the hold in the SITL gust matrix).
    if (!(figureSequencerRequested() || orientationHoldIsSpinAboutVertical())) {
        return 1.0f;
    }
    return loadGovScale;
}

int16_t orientationHoldLoadGovernorThrottle(int16_t throttle)
{
    // only a governed maneuver (figure or spin) bleeds throttle; plain
    // holds and normal flight pass through untouched
    if (loadGovThrScale >= 1.0f
        || !(figureSequencerRequested() || orientationHoldIsSpinAboutVertical())) {
        return throttle;
    }
    const int16_t idle = getThrottleIdleValue();
    return idle + lrintf((throttle - idle) * loadGovThrScale);
}

// ---- Thrust-first-guess authority scaling ------------------------------------
//
// The moment a control surface produces scales with the airflow over it
// squared - forward speed at cruise, prop wash in the slow regimes. Without
// an airspeed sensor (not carried; GPS is gone in aerobatic attitudes) the
// THRUST is the first guess for that airflow: above the airframe's cruise
// throttle (an existing fact the floor recovery already uses) the commanded
// hold authority is scaled back, so a hot 3D throw does not command
// over-deflection at speed - the "45 deg throws vs a smooth cruise tune"
// dilemma. At or below cruise the full authority applies, and the HOVER
// regime always gets it (high thrust but zero forward speed: the surfaces
// need their full throw against the wash alone). The guess only bounds the
// COMMAND; the closed rate loop refines it - it deflects no further than
// the achieved rate demands.
#define AUTHORITY_MIN_SCALE 0.5f
#define AUTHORITY_HOVER_ELEVATION_DEG 45.0f

float orientationHoldAuthorityScale(void)
{
    // hover/harrier band: nose high, airflow = wash, full throw
    fpVector3_t nose = { .v = { 1.0f, 0.0f, 0.0f } };
    quaternionRotateVectorInv(&nose, &nose, &orientation);
    const float elevDeg = RADIANS_TO_DEGREES(asin_approx(constrainf(-nose.z, -1.0f, 1.0f)));
    if (elevDeg > AUTHORITY_HOVER_ELEVATION_DEG) {
        return 1.0f;
    }

    const int16_t idle = getThrottleIdleValue();
    const float cruiseSpan = MAX(currentBatteryProfile->nav.fw.cruise_throttle - idle, 100);
    const float thrustNorm = (mixerThrottleCommand - idle) / cruiseSpan;
    if (thrustNorm <= 1.0f) {
        return 1.0f;
    }
    // surface moment ~ airflow^2 ~ thrust: scale the command with 1/thrust
    return constrainf(1.0f / thrustNorm, AUTHORITY_MIN_SCALE, 1.0f);
}

#endif // USE_ORIENTATION_HOLD
