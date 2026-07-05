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

#include "fc/rc_modes.h"

#include "flight/imu.h"
#include "flight/orientation_hold.h"

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

void orientationHoldComputeAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget)
{
    // Swing-twist decomposition: split qEst into a rotation about the earth
    // vertical axis (twist = heading) and the remaining tilt (swing), then
    // form the error against the tilt only. Exact for large angles, unlike
    // projecting the final rotation vector.
    fpQuaternion_t qTwist = { .q0 = qEst->q0, .q1 = 0.0f, .q2 = 0.0f, .q3 = qEst->q3 };
    const float twistNormSq = sq(qTwist.q0) + sq(qTwist.q3);

    fpQuaternion_t qSwing;
    if (twistNormSq > 1e-6f) {
        const float twistNormInv = 1.0f / sqrtf(twistNormSq);
        qTwist.q0 *= twistNormInv;
        qTwist.q3 *= twistNormInv;

        // qEst = qTwist (about earth Z) * qSwing  =>  qSwing = qTwist^-1 * qEst
        fpQuaternion_t qTwistInv;
        quaternionConjugate(&qTwistInv, &qTwist);
        quaternionMultiply(&qSwing, &qTwistInv, qEst);
    } else {
        // Degenerate at 180 deg of twist (e.g. inverted flying the opposite
        // heading): treat the full rotation as swing, shortest path handling
        // below keeps the error bounded.
        qSwing = *qEst;
    }

    // Attitude error in the rotation group: qErr = qTarget^-1 * qSwing
    fpQuaternion_t qTargetInv, qErr;
    quaternionConjugate(&qTargetInv, qTarget);
    quaternionMultiply(&qErr, &qTargetInv, &qSwing);
    quaternionNormalize(&qErr, &qErr);

    // Shortest path: q and -q encode the same rotation, pick |angle| <= 180 deg
    if (qErr.q0 < 0.0f) {
        quaternionScale(&qErr, &qErr, -1.0f);
    }

    // Rotation vector err = 2 * log(qErr), valid for large error angles
    fpAxisAngle_t axisAngle;
    quaternionToAxisAngle(&axisAngle, &qErr);

    // Error is "attitude ahead of target", the controller must command the
    // opposite rate, matching pidLevel() where error = target - attitude
    errDeg->x = -RADIANS_TO_DEGREES(axisAngle.axis.x * axisAngle.angle);
    errDeg->y = -RADIANS_TO_DEGREES(axisAngle.axis.y * axisAngle.angle);
    errDeg->z = -RADIANS_TO_DEGREES(axisAngle.axis.z * axisAngle.angle);
}

bool orientationHoldComputeError(fpVector3_t *errDeg)
{
    const orientationHoldPreset_t *preset = orientationHoldActivePreset();
    if (!preset) {
        return false;
    }

    fpQuaternion_t qTarget;
    orientationHoldTargetFromRP(&qTarget, preset->rollDeg, preset->pitchDeg);
    orientationHoldComputeAttitudeError(errDeg, &orientation, &qTarget);
    return true;
}

#endif // USE_ORIENTATION_HOLD
