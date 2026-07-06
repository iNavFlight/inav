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

#include "config/parameter_group.h"

// Figure sequencer: flies aerobatic figures as time parameterized
// orientation-hold targets (roll/pitch trajectories). Because the
// orientation hold controller is heading free, a figure needs no attitude
// capture: a roll always rotates about the current heading, a loop flies in
// the current heading plane. Figures start when their box goes active
// (from roughly level flight), hold level when complete, and re-arm when
// the box is released.
//
// Altitude assist: a PID on altitude/climb rate adds an earth referenced
// "nose above horizon" offset to the figure target. The controller
// distributes it to elevator and rudder as the roll phase demands (the
// classic slow-roll coordination), blended out as the nose approaches
// vertical where altitude belongs to the thrust axis.

typedef struct figureSequencerConfig_s {
    uint16_t rollRate;        // deg/s target roll rate for roll figures
    uint16_t loopRate;        // deg/s target pitch rate for the loop
    uint16_t pointDwellMs;    // ms dwell on each point of the 4 point roll
    uint8_t  assistZGain;     // deg of nose-up per 10 m of altitude error
    uint8_t  assistVzGain;    // deg of nose-up per m/s of sink
    uint8_t  assistMax;       // deg cap on the altitude assist offset
} figureSequencerConfig_t;

PG_DECLARE(figureSequencerConfig_t, figureSequencerConfig);

// Run once per RC processing cycle (before flight mode selection)
void figureSequencerUpdate(void);

// True while a figure box is active (sequencer wants ORIENTATION_HOLD_MODE)
bool figureSequencerRequested(void);

// Current figure target, valid while requested
void figureSequencerGetTarget(float *rollDeg, float *pitchDeg);
