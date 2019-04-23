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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "common/maths.h"

#include "fc/controlrate_profile.h"
#include "fc/rc_command.h"
#include "fc/rc_controls.h"
#include "fc/rc_curves.h"

#include "flight/mixer.h"

#include "rx/rx.h"


static float throttleRcMid;
static float throttleRcExpo;

void rcCurveGenerateThrottle(const controlRateConfig_t *controlRateConfig)
{
    throttleRcMid = controlRateConfig->throttle.rcMid8 / 100.0f + -0.5f;
    throttleRcExpo = constrainf(controlRateConfig->throttle.rcExpo8 / 100.0f, 0, 1);
#if 0
    // TODO: Proper curve
    for (int i = 0; i < THROTTLE_LOOKUP_LENGTH; i++) {
        const int16_t tmp = 10 * i - controlRateConfig->throttle.rcMid8;
        uint8_t y = 1;
        if (tmp > 0)
            y = 100 - controlRateConfig->throttle.rcMid8;
        if (tmp < 0)
            y = controlRateConfig->throttle.rcMid8;
        lookupThrottleRC[i] = 10 * controlRateConfig->throttle.rcMid8 + tmp * (100 - controlRateConfig->throttle.rcExpo8 + (int32_t) controlRateConfig->throttle.rcExpo8 * (tmp * tmp) / (y * y)) / 10;
        lookupThrottleRC[i] = motorConfig()->minthrottle + (int32_t) (motorConfig()->maxthrottle - motorConfig()->minthrottle) * lookupThrottleRC[i] / 1000; // [MINTHROTTLE;MAXTHROTTLE]
    }
#endif
}

float rcCurveApplyExpo(float deflection, float expo)
{
    return (1 + expo * (deflection * deflection - 1)) * deflection;
}

float rcCurveApplyThrottleExpo(float absoluteDeflection)
{
    if (absoluteDeflection < 0) {
        return -rcCurveApplyThrottleExpo(-absoluteDeflection);
    }

    // absoluteDeflection is always >= 0 past this point

    if (absoluteDeflection > RC_COMMAND_MAX * 0.99f) {
        return RC_COMMAND_MAX;
    }

    return absoluteDeflection;
}

float rcCurveGetThrottleMid(void)
{
    return rcCurveApplyThrottleExpo(RC_COMMAND_MAX / 2.0f);
}
