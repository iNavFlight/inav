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

#include "flight/dynamic_lpf.h"
#include "sensors/gyro.h"
#include "flight/mixer.h"
#include "fc/rc_controls.h"
#include "build/debug.h"

static float dynLpfCutoffFreq(float throttle, uint16_t dynLpfMin, uint16_t dynLpfMax, uint8_t expo) {
    const float expof = expo / 10.0f;
    static float curve;
    curve = throttle * (1 - throttle) * expof + throttle;
    return (dynLpfMax - dynLpfMin) * curve + dynLpfMin;
}

void dynamicLpfGyroTask(void) {

    if (!gyroConfig()->useDynamicLpf) {
        return;
    }

    const float throttle = scaleRangef((float) rcCommand[THROTTLE], getThrottleIdleValue(), motorConfig()->maxthrottle, 0.0f, 1.0f);
    const float cutoffFreq = dynLpfCutoffFreq(throttle, gyroConfig()->gyroDynamicLpfMinHz, gyroConfig()->gyroDynamicLpfMaxHz, gyroConfig()->gyroDynamicLpfCurveExpo);

    DEBUG_SET(DEBUG_DYNAMIC_GYRO_LPF, 0, cutoffFreq);

    gyroUpdateDynamicLpf(cutoffFreq);
}