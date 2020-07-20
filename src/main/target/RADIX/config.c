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

#include <platform.h>

#include "fc/config.h"
#include "sensors/gyro.h"
#include "sensors/battery.h"
#include "blackbox/blackbox.h"
#include "io/osd.h"

void targetConfiguration(void)
{
    gyroConfigMutable()->looptime = 1000;
    //blackboxConfigMutable()->p_denom = 128;
    osdConfigMutable()->rssi_alarm = 70; // for CRSF

    systemConfigMutable()->i2c_speed = I2C_SPEED_800KHZ;

    if (brainfpv_is_radixli()) {
        // value for RADIX LI wPB
        batteryMetersConfigMutable()->current.scale = 500;
    }
}
