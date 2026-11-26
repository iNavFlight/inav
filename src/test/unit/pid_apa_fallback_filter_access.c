/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify this software
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * INAV is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 */

// Test-only access shim for pid_apa_fallback_filter_unittest.cc.
// pidState[] and usedPidControllerType are file-static in flight/pid.c, so the
// real, unmodified pid.c is #included here (single translation unit) and the
// accessors below expose the state the test needs to observe.

#include "flight/pid.c"

void testPidSelectPiff(void) { usedPidControllerType = PID_TYPE_PIFF; }
float testPidGetKP(int axis) { return pidState[axis].kP; }
float testPidGetKI(int axis) { return pidState[axis].kI; }
void testPidSetArmed(void) { armingFlags |= ARMED; }
