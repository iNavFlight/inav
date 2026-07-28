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

#include <stdint.h>

#include "config/parameter_group.h"

// Crash detection for any flying platform (fixed wing and multirotor):
// after an impact the motor otherwise keeps running on the pilot's
// throttle. An impact (acceleration spike) followed by stillness (no
// rotation, resting 1 g) within a short window CUTS the motor while staying
// armed; the pilot re-allows it by moving the throttle to zero and up again
// (short bursts help locating the aircraft in high grass or corn). A flying
// aircraft is never still, so aggressive maneuvers (snaps, spins, hard
// gusts, freestyle) cannot trigger it - the stillness confirmation is the
// filter. Only armed AFTER the aircraft is clearly in the air: a fixed-wing
// hand launch, or throttle held above cruise for a moment (both platforms).

// Master enable is FEATURE_CRASH_DETECTION (a GUI feature toggle,
// default off) - no config group of its own.

// Call once per main PID loop iteration (after the IMU update)
void crashDetectionUpdate(float dT);

// True while the post-crash motor cut is active (mixer forces idle)
bool crashDetectionMotorCut(void);
