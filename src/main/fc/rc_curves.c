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
#include "fc/rc_controls.h"
#include "fc/rc_curves.h"

#include "flight/mixer.h"

#include "rx/rx.h"


#define PITCH_LOOKUP_LENGTH 7
#define YAW_LOOKUP_LENGTH 7
#define THROTTLE_LOOKUP_LENGTH 11

static EXTENDED_FASTRAM int16_t lookupThrottleRC[THROTTLE_LOOKUP_LENGTH];    // lookup table for expo & mid THROTTLE
int16_t lookupThrottleRCMid;                         // THROTTLE curve mid point

void generateThrottleCurve(const controlRateConfig_t *controlRateConfig)
{
    const int minThrottle = getThrottleIdleValue();
    lookupThrottleRCMid = minThrottle + (int32_t)(motorConfig()->maxthrottle - minThrottle) * controlRateConfig->throttle.rcMid8 / 100; // [MINTHROTTLE;MAXTHROTTLE]

    for (int i = 0; i < THROTTLE_LOOKUP_LENGTH; i++) {
        const int16_t tmp = 10 * i - controlRateConfig->throttle.rcMid8;
        uint8_t y = 1;
        if (tmp > 0)
            y = 100 - controlRateConfig->throttle.rcMid8;
        if (tmp < 0)
            y = controlRateConfig->throttle.rcMid8;
        lookupThrottleRC[i] = 10 * controlRateConfig->throttle.rcMid8 + tmp * (100 - controlRateConfig->throttle.rcExpo8 + (int32_t) controlRateConfig->throttle.rcExpo8 * (tmp * tmp) / (y * y)) / 10;
        lookupThrottleRC[i] = minThrottle + (int32_t) (motorConfig()->maxthrottle - minThrottle) * lookupThrottleRC[i] / 1000; // [MINTHROTTLE;MAXTHROTTLE]
    }
}

int16_t rcLookup(int32_t stickDeflection, uint8_t expo)
{
    float tmpf = stickDeflection / 100.0f;
    return lrintf((2500.0f + (float)expo * (tmpf * tmpf - 25.0f)) * tmpf / 25.0f);
}

uint16_t rcLookupThrottle(uint16_t absoluteDeflection)
{
    if (absoluteDeflection > 999)
        return motorConfig()->maxthrottle;

    const uint8_t lookupStep = absoluteDeflection / 100;
    return lookupThrottleRC[lookupStep] + (absoluteDeflection - lookupStep * 100) * (lookupThrottleRC[lookupStep + 1] - lookupThrottleRC[lookupStep]) / 100;
}

int16_t rcLookupThrottleMid(void)
{
    return lookupThrottleRCMid;
}
