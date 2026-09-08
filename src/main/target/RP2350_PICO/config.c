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
#include <platform.h>

#include "config/config_master.h"
#include "sensors/gyro.h"

void targetConfiguration(void)
{
    // NOTE: dynamic gyro notch (USE_DYNAMIC_FILTERS) is compiled out on RP2350 —
    // the CMSIS-DSP FFT it needs cannot be built for ARMv8-M with the pico-sdk
    // CMSIS headers (see target.h), so no runtime disable is required here.

    // Run PID at 1 kHz. The RP2350 PID loop averages ~360 µs at 192 MHz; the
    // 500 µs default looptime leaves insufficient headroom when GPS/nav tasks
    // are active. 1000 µs gives comfortable margin without affecting control
    // quality (INAV's fixed-wing tuning is designed for 1 kHz).
    gyroConfigMutable()->looptime = 1000;
}
