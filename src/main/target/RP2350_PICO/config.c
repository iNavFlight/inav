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
#ifdef USE_DYNAMIC_FILTERS
    // Disable dynamic notch filter by default (performance optimization for wing)
    // This board is performance-constrained and wing aircraft typically don't need
    // dynamic notch filtering (designed for multirotor motor noise)
    gyroConfigMutable()->dynamicGyroNotchEnabled = 0;
#endif
}
