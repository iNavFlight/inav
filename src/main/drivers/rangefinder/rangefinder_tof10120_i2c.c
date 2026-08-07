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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#if defined(USE_RANGEFINDER) && defined(USE_RANGEFINDER_TOF10120_I2C)

#include "drivers/time.h"
#include "drivers/rangefinder/rangefinder.h"
#include "drivers/rangefinder/rangefinder_tof10120_i2c.h"

#define TOF10120_I2C_MAX_RANGE_CM 200
#define TOF10120_I2C_MAX_RANGE_MM TOF10120_I2C_MAX_RANGE_CM * 10
#define TOF10120_I2C_DETECTION_CONE_DECIDEGREES 300
#define TOF10120_I2C_DETECTION_CONE_EXTENDED_DECIDEGREES 450

#define TOF10120_I2C_ADDRESS 0x52

#define TOF10120_I2C_REGISTRY_DISTANCE 0x04    // address of first byte for reading filtered distance
#define TOF10120_I2C_REGISTRY_RT_DISTANCE 0x00 // address of first byte for reading real-time raw distance

#define TOF10120_I2C_REGISTRY_BUS_ADDR_CONFIG 0x0f
#define TOF10120_I2C_REGISTRY_SENDING_METHOD_CONFIG 0x09

volatile int32_t tof10120MeasurementCm = RANGEFINDER_OUT_OF_RANGE;
static bool isTof10120Responding = false;

static void tof10120i2cInit(rangefinderDev_t *rangefinder)
{
    busWrite(rangefinder->busDev, TOF10120_I2C_REGISTRY_SENDING_METHOD_CONFIG, 0x01);
    delay(100);
}

void tof10120i2cUpdate(rangefinderDev_t *rangefinder)
{
    uint8_t buffer[2];
    uint16_t distance_mm;

    isTof10120Responding = busReadBuf(rangefinder->busDev, TOF10120_I2C_REGISTRY_RT_DISTANCE, buffer, sizeof(buffer));

    if (!isTof10120Responding) {
        return;
    }

    distance_mm = (buffer[0] << 8) | buffer[1];

    if (distance_mm >= TOF10120_I2C_MAX_RANGE_MM) {
        tof10120MeasurementCm = RANGEFINDER_OUT_OF_RANGE;
        return;
    }

    tof10120MeasurementCm = (int32_t)roundf(distance_mm / 10);
}

/**
 * Get the distance that was measured by the last pulse, in centimeters.
 */
int32_t tof10120i2cGetDistance(rangefinderDev_t *rangefinder)
{
    UNUSED(rangefinder);

    if (isTof10120Responding) {
        return tof10120MeasurementCm;
    } else {
        return RANGEFINDER_HARDWARE_FAILURE;
    }
}

static bool deviceDetect(rangefinderDev_t *rangefinder)
{
    busWrite(rangefinder->busDev, TOF10120_I2C_REGISTRY_SENDING_METHOD_CONFIG, 0x01);
    delay(100);

    uint8_t response = 0;

    if (!busRead(rangefinder->busDev, TOF10120_I2C_REGISTRY_BUS_ADDR_CONFIG, &response) || !response) {
        return false;
    }

    return true;
}

bool tof10120Detect(rangefinderDev_t *rangefinder)
{
    rangefinder->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_TOF10120_I2C, 0, OWNER_RANGEFINDER);
    if (rangefinder->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(rangefinder)) {
        busDeviceDeInit(rangefinder->busDev);
        return false;
    }

    rangefinder->delayMs = RANGEFINDER_TOF10120_I2C_TASK_PERIOD_MS;
    rangefinder->maxRangeCm = TOF10120_I2C_MAX_RANGE_CM;
    rangefinder->detectionConeDeciDegrees = TOF10120_I2C_DETECTION_CONE_DECIDEGREES;
    rangefinder->detectionConeExtendedDeciDegrees = TOF10120_I2C_DETECTION_CONE_EXTENDED_DECIDEGREES;

    rangefinder->init = &tof10120i2cInit;
    rangefinder->update = &tof10120i2cUpdate;
    rangefinder->read = &tof10120i2cGetDistance;

    return true;
}
#endif
