/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

#include "common/axis.h"
#include "drivers/compass/compass.h"

bool dronecanMagDetect(magDev_t *mag);

// Magnetic field in Gauss, body frame (front, right, down)
void dronecanMagReceiveNewData(uint8_t nodeId, uint8_t sensorId, const float fieldGauss[3]);
