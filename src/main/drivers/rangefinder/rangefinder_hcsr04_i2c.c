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

#include "platform.h"

#if defined(USE_RANGEFINDER) && defined(USE_RANGEFINDER_HCSR04_I2C)

#include "build/build_config.h"

#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "drivers/rangefinder/rangefinder.h"
#include "drivers/rangefinder/rangefinder_hcsr04_i2c.h"

#include "build/debug.h"

#define HCSR04_I2C_MAX_RANGE_CM 400
#define HCSR04_I2C_DETECTION_CONE_DECIDEGREES 300
#define HCSR04_I2C_DETECTION_CONE_EXTENDED_DECIDEGREES 450

#define HCSR04_I2C_Address 0x14

#define HCSR04_I2C_REGISTRY_STATUS 0x00
#define HCSR04_I2C_REGISTRY_DISTANCE_HIGH 0x01
#define HCSR04_I2C_REGISTRY_DISTANCE_LOW 0x02

volatile int32_t hcsr04i2cMeasurementCm = RANGEFINDER_OUT_OF_RANGE;
static bool isHcsr04i2cResponding = false;

static void hcsr04i2cInit(rangefinderDev_t *rangefinder)
{
    UNUSED(rangefinder);
}

void hcsr04i2cUpdate(rangefinderDev_t *rangefinder)
{
    uint8_t response[3];

    isHcsr04i2cResponding = busReadBuf(rangefinder->busDev, HCSR04_I2C_REGISTRY_STATUS, response, 3);

    if (!isHcsr04i2cResponding) {
        hcsr04i2cMeasurementCm = RANGEFINDER_HARDWARE_FAILURE;
        return;
    }

    if (response[HCSR04_I2C_REGISTRY_STATUS] == 0) {

        hcsr04i2cMeasurementCm =
            (int32_t)((int32_t)response[HCSR04_I2C_REGISTRY_DISTANCE_HIGH] << 8) +
            response[HCSR04_I2C_REGISTRY_DISTANCE_LOW];

    } else {
        /*
         * Rangefinder is reporting out-of-range situation
         */
        hcsr04i2cMeasurementCm = RANGEFINDER_OUT_OF_RANGE;
    }
}

/**
 * Get the distance that was measured by the last pulse, in centimeters.
 */
int32_t hcsr04i2cGetDistance(rangefinderDev_t *rangefinder)
{
    UNUSED(rangefinder);
    return hcsr04i2cMeasurementCm;
}

static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < 5; retry++) {
        uint8_t inquiryResult;
        delay(150);

        bool ack = busRead(busDev, HCSR04_I2C_REGISTRY_STATUS, &inquiryResult);
        if (ack) {
            return true;
        }
    };

    return false;
}

bool hcsr04i2c0Detect(rangefinderDev_t *rangefinder)
{
    rangefinder->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_HCSR04_I2C, 0, OWNER_RANGEFINDER);
    if (rangefinder->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(rangefinder->busDev)) {
        busDeviceDeInit(rangefinder->busDev);
        return false;
    }

    rangefinder->delayMs = RANGEFINDER_HCSR04_i2C_TASK_PERIOD_MS;
    rangefinder->maxRangeCm = HCSR04_I2C_MAX_RANGE_CM;
    rangefinder->detectionConeDeciDegrees = HCSR04_I2C_DETECTION_CONE_DECIDEGREES;
    rangefinder->detectionConeExtendedDeciDegrees = HCSR04_I2C_DETECTION_CONE_EXTENDED_DECIDEGREES;

    rangefinder->init = &hcsr04i2cInit;
    rangefinder->update = &hcsr04i2cUpdate;
    rangefinder->read = &hcsr04i2cGetDistance;

    return true;
}
#endif
