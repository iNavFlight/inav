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

// Crash detection for fixed wing: after an impact the motor otherwise keeps
// running on the pilot's throttle. An impact (acceleration spike) followed
// by stillness (no rotation, resting 1 g) within a short window disarms.
// A flying aircraft is never still, so aggressive maneuvers (snaps, spins,
// hard gusts) cannot trigger it - the stillness confirmation is the filter.
// Only armed AFTER the aircraft is clearly in the air (hand launch rule):
// nav launch completed, or throttle held above cruise level for a moment.

typedef struct crashDetectionConfig_s {
    uint8_t crashGThreshold;       // impact threshold [g * 10]; 0 disables
} crashDetectionConfig_t;

PG_DECLARE(crashDetectionConfig_t, crashDetectionConfig);

// Call once per main PID loop iteration (after the IMU update)
void crashDetectionUpdate(float dT);
