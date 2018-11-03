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
 * along with INAV. If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Alberto Garcia Hierro <alberto@garciahierro.com>
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "common/time.h"

#if !defined(FLOCK_MAX_NAME_LENGTH)
#define FLOCK_MAX_NAME_LENGTH 16
#endif

#if !defined(FLOCK_MAX_BIRDS)
#define FLOCK_MAX_BIRDS 32
#endif

#define FLOCK_ADDR_SIZE 6
#define FLOCK_EINVALIDINPUT -1
#define FLOCK_ENOMEM -2

typedef enum {
    FLOCK_VEHICLE_TYPE_AIRPLANE = 0,
    FLOCK_VEHICLE_TYPE_BOAT = 1,
    FLOCK_VEHICLE_TYPE_FLYING_WING = 2,
    FLOCK_VEHICLE_TYPE_HELICOPTER = 3,
    FLOCK_VEHICLE_TYPE_MULTIROTOR = 4,
    FLOCK_VEHICLE_TYPE_ROVER = 5,
    FLOCK_VEHICLE_TYPE_TRICOPTER = 6,
} flockVehicleType_e;

typedef struct flockAdress_s {
    uint8_t addr[FLOCK_ADDR_SIZE];
} flockAddress_t;

typedef struct flockBirdInfo_s
{
    flockAddress_t txAddr;
    flockAddress_t rxAddr;
    char txName[FLOCK_MAX_NAME_LENGTH];
    char rxName[FLOCK_MAX_NAME_LENGTH];
    unsigned vehicleType : 4; // from flockVehicleType_e
    unsigned flags : 4; // reserved
} __attribute__((packed)) flockBirdInfo_t;

#define FLOCK_ALTITUDE_BITS 13
#define FLOCK_ALTITUDE_MAX ((1 << FLOCK_ALTITUDE_BITS) - 1)
#define FLOCK_GROUND_SPEED_BITS 9
#define FLOCK_GROUND_SPEED_MAX ((1 << FLOCK_GROUND_SPEED_BITS) - 1)
#define FLOCK_VERTICAL_SPEED_BITS 7
#define FLOCK_VERTICAL_SPEED_MIN -(1 << (FLOCK_VERTICAL_SPEED_BITS - 1))
#define FLOCK_VERTICAL_SPEED_MAX ((1 << (FLOCK_VERTICAL_SPEED_BITS - 1)) - 1)
#define FLOCK_HEADING(degs) (degs * (256.0f / 360.f))

typedef struct flockBirdPosvel_s
{
    int32_t lat : 21;                                  // deg / 10`000
    int32_t lon : 22;                                  // deg / 10`000
    uint16_t alt : FLOCK_ALTITUDE_BITS;                // absolute altitude in m with +1000m offset, range [-1000, 7191] m
    uint16_t ground_speed : FLOCK_GROUND_SPEED_BITS;   // 0.1 m/s, range [0, 51.1] m/s
    int8_t vertical_speed : FLOCK_VERTICAL_SPEED_BITS; // 0.1 m/s, range [-6.4, 6.3] m/s
    uint8_t heading;                                   // deg, on 360/256 increments
} __attribute__((packed)) flockBirdPosvel_t;

typedef enum {
    FLOCK_BIRD_FLAG_HAS_INFO = 1 << 0,
    FLOCK_BIRD_FLAG_HAS_POSVEL = 1 << 1,
} flockBirdFlag_e;

typedef struct flockBird_s
{
    flockBirdInfo_t info;
    flockBirdPosvel_t posvel;
    timeMs_t lastUpdate;
    uint8_t flags; // from flockBirdFlag_e
} __attribute__((packed)) flockBird_t;

typedef struct flockIterator_s {
    int pos;
} flockIterator_t;

void flockInit(void);
void flockUpdate(timeUs_t currentTimeUs);
int flockGetCount(void);

int flockWrite(const void *data, size_t dataSize, void *buf, size_t bufSize);

void flockIteratorInit(flockIterator_t *it);
bool flockIteratorNext(flockIterator_t *it);
const flockBird_t *flockIteratorGet(flockIterator_t *it);
