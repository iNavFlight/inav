/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * Bosch BMP580 / BMP581 / BMP585 barometer driver.
 *
 * References:
 * BMP581 datasheet - https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp581/
 * BMP5-Sensor-API  - https://github.com/boschsensortec/BMP5_SensorAPI
 */

#pragma once

#include "drivers/barometer/barometer.h"

bool bmp58xDetect(baroDev_t *baro);
