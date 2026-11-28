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

#include "common/vector.h"

#include "drivers/sensor.h"

// Non-blocking read step, see busReadStepResult_e in drivers/bus.h
typedef busReadStepResult_e (*sensorMagReadStartFuncPtr)(struct magDev_s *mag, bool firstStep);

typedef struct magDev_s {
    busDevice_t * busDev;
    sensorMagInitFuncPtr init;  // initialize function
    sensorMagReadStartFuncPtr readStart;    // optional: start the next non-blocking transfer of a sample, read() then only parses
    sensorMagReadFuncPtr read;  // read 3 axis data function (blocking when readStart is NULL)
    struct {
        bool useExternal;
        union {
            fpMat3_t externalRotation;
            sensor_align_e onBoard;
        };
    } magAlign;
    uint8_t magSensorToUse;
    int16_t magADCRaw[XYZ_AXIS_COUNT];
} magDev_t;
