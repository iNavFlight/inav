/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "flight/mixer_disarmed_dshot.h"

#include "common/maths.h"
#include "flight/mixer_dshot_constants.h"

uint16_t calculateDisarmedReversibleMotorsDshotValue(
    int16_t motorValue,
    int16_t deadbandLow,
    int16_t deadbandHigh,
    int16_t mincommand,
    int16_t maxThrottle)
{
    if (motorValue >= deadbandHigh) {
        if (maxThrottle <= deadbandHigh) {
            return DSHOT_MAX_THROTTLE; // zero-width range would divide by zero
        }
        int32_t scaled = (int32_t)scaleRangef(motorValue, deadbandHigh, maxThrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
        return constrain(scaled, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
    }
    if (motorValue <= deadbandLow) {
        if (deadbandLow <= mincommand) {
            return DSHOT_MIN_THROTTLE; // zero-width range would divide by zero
        }
        int32_t scaled = (int32_t)scaleRangef(motorValue, mincommand, deadbandLow, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
        return constrain(scaled, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
    }
    return DSHOT_DISARM_COMMAND;
}
