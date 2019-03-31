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

#include <math.h>

#include "platform.h"

#include "build/debug.h"

#include "drivers/sensor.h"
#include "drivers/temperature/lm75.h"
#include "drivers/temperature/temperature.h"
#include "drivers/time.h"

#define LM75_TEMPERATURE_REG_ADDR 0x0


#ifdef USE_TEMPERATURE_LM75

static bool lm75Read(temperatureDev_t *tempDev, int16_t *temperature)
{
    uint8_t buf[2];

    bool ack = busReadBuf(tempDev->busDev, LM75_TEMPERATURE_REG_ADDR, buf, 2);

    if (ack) {
        if (temperature) *temperature = (int8_t)buf[0] * 10 + (buf[1] >> 7) * 5;
        return true;
    }

    return false;
}

#define DETECTION_MAX_RETRY_COUNT 5
static bool deviceDetect(temperatureDev_t *tempDev)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);
        if (lm75Read(tempDev, NULL)) return true;
    }

    return false;
}

bool lm75Detect(temperatureDev_t *tempDev, uint8_t partialAddress)
{
    if (partialAddress > 7)  return false; // invalid address

    tempDev->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_LM75_0 + partialAddress, 0, OWNER_TEMPERATURE);
    if (tempDev->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(tempDev)) {
        busDeviceDeInit(tempDev->busDev);
        return false;
    }

    tempDev->read = lm75Read;

    return true;
}

#endif /* USE_TEMPERATURE_LM75 */
