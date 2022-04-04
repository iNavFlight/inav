/*
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "drivers/sensor.h"
#include "drivers/io_types.h"

#if defined(USE_IRLOCK)

#define IRLOCK_RES_X 320
#define IRLOCK_RES_Y 200

typedef struct {
    uint16_t cksum;
    uint16_t signature;
    uint16_t posX;
    uint16_t posY;
    uint16_t sizeX;
    uint16_t sizeY;
} irlockData_t;

typedef struct irlockDev_s {
    busDevice_t *busDev;
    bool (*read)(struct irlockDev_s *irlockDev, irlockData_t *irlockData);
} irlockDev_t;

bool irlockDetect(irlockDev_t *irlockDev);
bool irlockIsHealthy(void);

#endif /* USE_IRLOCK */
