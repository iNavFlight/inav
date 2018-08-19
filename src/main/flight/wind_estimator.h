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

#if defined(USE_WIND_ESTIMATOR)
#if !defined(USE_GPS)
#error Wind Estimator requires GPS support
#endif

#include "common/axis.h"
#include "common/time.h"

bool isEstimatedWindSpeedValid(void);
// wind velocity vectors in cm / sec relative to the earth frame
float getEstimatedWindSpeed(int axis);
// Returns the horizontal wind velocity as a magnitude in cm/s and,
// optionally, its heading in EF in 0.01deg ([0, 360*100)).
float getEstimatedHorizontalWindSpeed(uint16_t *angle);

void updateWindEstimator(timeUs_t currentTimeUs);

#endif
