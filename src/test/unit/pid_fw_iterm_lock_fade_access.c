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

// Test-only access shim for pid_fw_iterm_lock_fade_unittest.cc: pidState[] and the fixed-wing rate controller are
// file-static in flight/pid.c, so the real, unmodified pid.c is #included here and the accessors below drive one axis
// of it with the values the test chooses.

#include "flight/pid.c"

void testPidFwSetAxis(int axis, float kP, float kI, float kFF, float iTerm, float gyroRate, float rateTarget)
{
    pidState_t *s = &pidState[axis];
    s->axis = axis;
    s->kP = kP;
    s->kI = kI;
    s->kD = 0.0f;
    s->kFF = kFF;
    s->errorGyroIf = iTerm;
    s->gyroRate = gyroRate;
    s->rateTarget = rateTarget;
    s->pidSumLimit = 500;
    s->itermLimitActive = false;
    s->itermFreezeActive = false;
    s->ptermFilterApplyFn = (filterApply3FnPtr)nullFilterApply3;
    s->attenuation.targetOverThresholdTimeMs = 0;
}

void testPidFwStep(int axis, float dT)
{
    pidLoopNowMs += (timeMs_t)(dT * 1000.0f);
    pidApplyFixedWingRateController(&pidState[axis], dT, 1.0f / dT);
}

float testPidFwGetIterm(int axis) { return pidState[axis].errorGyroIf; }
float testPidFwGetAttenuationI(int axis) { return pidState[axis].attenuation.aI; }
int16_t testPidFwGetOutput(int axis) { return axisPID[axis]; }
