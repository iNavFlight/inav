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

#include "build_config.h"

#include "gps.h"
#include "gps_i2cnav.h"

#include "gpio.h"
#include "system.h"
#include "bus_i2c.h"
#include "sonar_i2cnav.h"
#define I2C_GPS_ADDRESS               0x20 //7 bits 
#define I2C_GPS_SONAR_DISTANCE					239			//uint16_t sonar distance in cm

#ifdef SONAR_I2CNAV

int32_t i2cnavsonarRead(void){ 
uint16_t sonardistance;   
            i2cRead(I2C_GPS_ADDRESS, I2C_GPS_SONAR_DISTANCE, 2, (uint8_t*)&sonardistance);
			if (sonardistance < 350)
			return (int32_t)(sonardistance);
		else
			return -1;
}
#endif