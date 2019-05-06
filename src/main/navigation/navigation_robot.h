/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"
#include "common/vector.h"

typedef enum {
	NAV_ROBOT_MOVE_NOOP = 0,		// Do nothing
    NAV_ROBOT_MOVE_ABSOLUTE = 1,	// Set to absolute value ([cm] for position, [deg] for heading)
    NAV_ROBOT_MOVE_RELATIVE = 2,	// Set relative to current drone position
    NAV_ROBOT_MOVE_INCREMENTAL = 3,	// Set relative to previous command
} navRobotMotionMode_e;

typedef struct {
    navRobotMotionMode_e    posMoveMode;
    navRobotMotionMode_e    headMoveMode;
    fpVector3_t             posMotion;
    float					headMotion;
} navRobotMovement_t;

extern void navRobotModeMoveHandler(const navRobotMovement_t * move);

