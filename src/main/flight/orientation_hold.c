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

#include "common/maths.h"
#include "common/quaternion.h"
#include "common/utils.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/rc_modes.h"
#include "fc/settings.h"

#include "flight/altitude_floor.h"
#include "flight/imu.h"
#include "flight/orientation_hold.h"

PG_REGISTER_WITH_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig, PG_ORIENTATION_HOLD_CONFIG, 0);

PG_RESET_TEMPLATE(orientationHoldConfig_t, orientationHoldConfig,
    .invertedPitchTrim = SETTING_OHOLD_INVERTED_PITCH_TRIM_DEFAULT,
    .knifePitchTrim = SETTING_OHOLD_KNIFE_PITCH_TRIM_DEFAULT,
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
    return orientationHoldActivePreset() != NULL;
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
    // is noise driven.
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

bool orientationHoldComputeError(fpVector3_t *errDeg)
{
    fpQuaternion_t qTarget;

    // Altitude floor recovery overrides any selected preset: upright + climb
    if (altitudeFloorRecoveryActive()) {
        orientationHoldTargetFromRP(&qTarget, 0.0f, altitudeFloorRecoveryPitchDeg());
    } else {
        const orientationHoldPreset_t *preset = orientationHoldActivePreset();
        if (!preset) {
            return false;
        }
        // Per attitude pitch trim, as Euler pitch of the target: positive is
        // always "nose above the horizon" regardless of the attitude's roll
        float pitchTrim = 0.0f;
        if (preset->box == BOXINVERTED) {
            pitchTrim = orientationHoldConfig()->invertedPitchTrim;
        } else if (preset->box == BOXKNIFELEFT || preset->box == BOXKNIFERIGHT) {
            pitchTrim = orientationHoldConfig()->knifePitchTrim;
        }
        orientationHoldTargetFromRP(&qTarget, preset->rollDeg, preset->pitchDeg + pitchTrim);
    }

    orientationHoldComputeAttitudeError(errDeg, &orientation, &qTarget);
    return true;
}

#endif // USE_ORIENTATION_HOLD
