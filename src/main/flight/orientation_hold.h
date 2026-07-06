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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/quaternion.h"
#include "common/vector.h"

#include "config/parameter_group.h"

// Orientation hold: quaternion based attitude controller that can stabilise
// arbitrary target attitudes (inverted, knife edge, prop hang) on fixed wing.
// Unlike the Euler based ANGLE controller it has no singularity at
// pitch = +/-90 deg. Heading (rotation about the earth vertical axis) is
// always left free, matching ANGLE mode behaviour in normal flight.

// Per attitude pitch trim on the hold target, applied as the Euler pitch of
// the target (before the attitude's roll): positive = nose above the
// horizon in every attitude. Inverted flight needs it to hold altitude
// (down-elevator bias), knife edge to carry the fuselage lift.
typedef struct orientationHoldConfig_s {
    int8_t invertedPitchTrim;      // deg, nose above horizon in inverted flight
    int8_t knifeLeftPitchTrim;     // deg, nose above horizon in left knife edge
    int8_t knifeRightPitchTrim;    // deg, nose above horizon in right knife edge
                                   // (separate per side: prop effects break the symmetry)
} orientationHoldConfig_t;

PG_DECLARE(orientationHoldConfig_t, orientationHoldConfig);

// Compute the body frame attitude error (deg, per body axis) between qEst
// and the tilt part of qTarget. The rotation of qEst about the earth
// vertical axis (heading / twist) is removed before the error is formed, so
// the returned error never asks for a heading change.
void orientationHoldComputeAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget);

// Build a target quaternion from roll/pitch (deg, yaw = 0) using the same
// Euler convention as the attitude estimator.
void orientationHoldTargetFromRP(fpQuaternion_t *qTarget, float rollDeg, float pitchDeg);

// True when any orientation hold box (INVERTED / KNIFE EDGE / PROP HANG) is
// selected on the transmitter.
bool orientationHoldIsRequested(void);

// Body frame attitude error (deg) for the currently selected target.
// Returns false when no orientation hold box is active.
bool orientationHoldComputeError(fpVector3_t *errDeg);

// Call while ORIENTATION_HOLD_MODE is inactive: resets the target-source
// tracking (and the rate-loop I accumulators once on the exit edge)
void orientationHoldResetSourceTracking(void);
