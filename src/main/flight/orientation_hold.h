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
    uint8_t hoverGainLearned;      // %, LEARNED angle-gain scale per regime:
                                   // the normal-flight gains are the reference
                                   // (100), the limit-cycle detector backs the
                                   // scale off while that regime oscillates and
                                   // writes it back at regime exit; saved on
                                   // disarm. The next entry (and the next
                                   // flight) starts at the learned value
                                   // instead of oscillating its way down again.
    uint8_t invertedGainLearned;   // %, learned scale for the inverted hold
    uint8_t knifeGainLearned;      // %, learned scale for the knife edge holds
    uint8_t figureGainLearned;     // %, learned scale while a figure flies
    uint16_t entryRateDps;         // deg/s target slew for PRESET entries.
                                   // Separate from fig_roll_rate on purpose:
                                   // a deliberate slow roll figure and a snappy
                                   // entry into a hold are different intents.
    uint8_t stickAngleMaxDeg;      // deg of body-frame target offset at full
                                   // roll/pitch stick: ANGLE semantics around
                                   // the rotated reference (deflection = held
                                   // angle offset, release = slow return).
                                   // 0 = sticks stay raw rate commands.
    uint8_t stickReturnRateDps;    // deg/s the target returns to the preset
                                   // after the sticks center
} orientationHoldConfig_t;

PG_DECLARE(orientationHoldConfig_t, orientationHoldConfig);

// Compute the body frame attitude error (deg, per body axis) between qEst
// and the tilt part of qTarget. The rotation of qEst about the earth
// vertical axis (heading / twist) is removed before the error is formed, so
// the returned error never asks for a heading change. Pure shortest-tilt
// rotation: large-error entry paths are shaped by the target slew inside
// orientationHoldComputeError(), not here.
void orientationHoldComputeAttitudeError(fpVector3_t *errDeg, const fpQuaternion_t *qEst, const fpQuaternion_t *qTarget);

// Build a target quaternion from roll/pitch (deg, yaw = 0) using the same
// Euler convention as the attitude estimator.
void orientationHoldTargetFromRP(fpQuaternion_t *qTarget, float rollDeg, float pitchDeg);

// True when any orientation hold box (INVERTED / KNIFE EDGE / PROP HANG) is
// selected on the transmitter.
bool orientationHoldIsRequested(void);

// True while a released hold still owns the exit handover toward ANGLE:
// the hold slews its target to level first so the Euler level controller
// never sees the full attitude error as one step (a prop hang exit whips
// through nose down otherwise). Latches on the release edge when the
// attitude is far from level; clears when level is captured, the pilot
// deflects a stick, or a timeout expires.
bool orientationHoldExitSlewPending(void);

// Body frame attitude error (deg) for the currently selected target.
// The target is a persistent attitude quaternion seeded on the actual
// attitude at engage and slewed toward the requested attitude (fig_roll_rate
// for preset entries), so the error stays small and the entry path is an
// explicit trajectory. Returns false when no orientation hold box is active.
bool orientationHoldComputeError(fpVector3_t *errDeg, float dT);

// Re-seed the persistent target on the actual attitude. Call every cycle
// the regulator is bypassed while a source is active (figure IMPULSE
// segments), so the catch afterwards starts from the actual attitude.
void orientationHoldSyncTargetToAttitude(void);

// Call while ORIENTATION_HOLD_MODE is inactive: resets the target-source
// tracking (and the rate-loop I accumulators once on the exit edge)
void orientationHoldResetSourceTracking(void);

// True while the PROP HANG preset is the active hold target (used by the
// hover throttle to own the altitude axis)
bool orientationHoldIsPropHang(void);

// True while a knife edge or inverted preset is the active hold target
// (used by the knife/inverted throttle assist, criterion vz -> 0)
bool orientationHoldIsKnifeOrInverted(void);

// True while the FLAT SPIN family owns the target (flat / inverted / knife /
// torque roll): the pilot's rudder commands rotation about the earth
// vertical instead of the body yaw axis
bool orientationHoldIsSpinAboutVertical(void);

// Earth-up direction expressed in the body frame (the spin distribution
// axis; also the axis the reduced attitude error leaves free)
void orientationHoldUpInBody(fpVector3_t *upBody);

// True while roll/pitch sticks act as TARGET OFFSETS around the rotated
// reference (preset hold active and ohold_stick_angle > 0): the rate path
// must then not also feed them as rate commands. Yaw stays a rate command,
// it is the free axis.
bool orientationHoldSticksAreTargetOffsets(void);

// Learned damping reserve for the hover regime: scale factor (0.3..1.0) on
// the angle-loop gain. A detected limit cycle around the vertical backs it
// off fast, quiet time recovers it slowly; 1.0 outside the hang. Apply to
// the angle error before the LEVEL P gain.
float orientationHoldLevelGainScale(void);
