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

// Test-only access shim for pid_turn_assist_pitch_unittest.cc.
// pidTurnAssistant() and pidState[] are file-static in flight/pid.c, so the
// real, unmodified pid.c is #included here (single translation unit).

#include "flight/pid.c"

void testPidTurnAssistantReset(void)
{
    memset(pidState, 0, sizeof(pidState));
}

void testPidTurnAssistantCall(float bankRad)
{
    pidTurnAssistant(pidState, bankRad);
}

float testPidRateTarget(int axis) { return pidState[axis].rateTarget; }
