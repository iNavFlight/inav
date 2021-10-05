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

#pragma once

#include "drivers/time.h"

// Base frequencies for smoothing roll and pitch
#define NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ    2.0f
#define NAV_FW_BASE_ROLL_CUTOFF_FREQUENCY_HZ     10.0f

float getSmoothnessCutoffFreq(float baseFreq);
void updateAltitudeVelocityAndPitchController_FW(timeDelta_t deltaMicros);