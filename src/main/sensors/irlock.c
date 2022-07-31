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

#include "stdbool.h"
#include "stdint.h"
#include <string.h>
#include <math.h>

#include "platform.h"

#include "build/debug.h"

#include "common/maths.h"
#include "common/log.h"
#include "common/printf.h"

#include "drivers/irlock.h"
#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "flight/ahrs.h"

#include "navigation/navigation.h"

#include "sensors/sensors.h"
#include "sensors/barometer.h"
#include "sensors/irlock.h"


#define IRLOCK_TIMEOUT 100

#if defined(USE_IRLOCK)

static irlockDev_t irlockDev;
static bool irlockDetected = false;
static bool measurementValid = false;
static irlockData_t irlockData;
static timeMs_t lastUpdateMs = 0;

void irlockInit(void)
{
    irlockDetected = irlockDetect(&irlockDev);
}

bool irlockHasBeenDetected(void)
{
    return irlockDetected;
}

bool irlockMeasurementIsValid(void) {
    return measurementValid;
}

void irlockUpdate(void)
{
    if (irlockDetected && irlockDev.read(&irlockDev, &irlockData)) lastUpdateMs = millis();
    measurementValid = millis() - lastUpdateMs < IRLOCK_TIMEOUT;
}

#define X_TO_DISTANCE_FACTOR -0.0029387573f
#define X_TO_DISTANCE_OFFSET 0.4702011635f
#define Y_TO_DISTANCE_FACTOR -0.0030568431f
#define Y_TO_DISTANCE_OFFSET 0.3056843086f

#define LENS_X_CORRECTION 4.4301355e-6f
#define LENS_Y_CORRECTION 4.7933139e-6f

// calculate distance relative to center at 1 meter distance from absolute object coordinates and taking into account lens distortion
static void irlockCoordinatesToDistance(uint16_t pixX, uint16_t pixY, float *distX, float *distY)
{
    int16_t xCenterOffset = pixX - IRLOCK_RES_X / 2;
    int16_t yCenterOffset = pixY - IRLOCK_RES_Y / 2;
    float lensDistortionCorrectionFactor = LENS_X_CORRECTION * xCenterOffset * xCenterOffset + LENS_Y_CORRECTION * yCenterOffset * yCenterOffset - 1.0f;
    *distX = (X_TO_DISTANCE_FACTOR * pixX + X_TO_DISTANCE_OFFSET) / lensDistortionCorrectionFactor;
    *distY = (Y_TO_DISTANCE_FACTOR * pixY + Y_TO_DISTANCE_OFFSET) / lensDistortionCorrectionFactor;
}

bool irlockGetPosition(float *distX, float *distY)
{
    if (!measurementValid) return false;

    // calculate edges of the object
    int16_t corner1X = irlockData.posX - irlockData.sizeX / 2;
    int16_t corner1Y = irlockData.posY - irlockData.sizeY / 2;
    int16_t corner2X = irlockData.posX + irlockData.sizeX / 2;
    int16_t corner2Y = irlockData.posY + irlockData.sizeY / 2;

    // convert pixel position to distance
    float corner1DistX, corner1DistY, corner2DistX, corner2DistY;
    irlockCoordinatesToDistance(corner1X, corner1Y, &corner1DistX, &corner1DistY);
    irlockCoordinatesToDistance(corner2X, corner2Y, &corner2DistX, &corner2DistY);

    // return center of object
    float uDistX = (corner1DistX + corner2DistX) / 2.0f; // lateral movement, positive means the aircraft is to the left of the beacon
    float uDistY = (corner1DistY + corner2DistY) / 2.0f; // longitudinal movement, positive means the aircraft is in front of the beacon

    // compensate for altitude and attitude
    float altitude = CENTIMETERS_TO_METERS(getEstimatedActualPosition(Z));
    *distX = altitude * tan_approx(atan2_approx(uDistX, 1.0f) - DECIDEGREES_TO_RADIANS(attitude.values.roll));
    *distY = altitude * tan_approx(atan2_approx(uDistY, 1.0f) + DECIDEGREES_TO_RADIANS(attitude.values.pitch));

    return true;
}

#endif /* USE_IRLOCK */
