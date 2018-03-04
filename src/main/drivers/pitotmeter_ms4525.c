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

#include "drivers/bus_i2c.h"
#include "pitotmeter.h"
#include "drivers/time.h"

#include "common/utils.h"

// MS4525, Standard address 0x28
#define MS4525_ADDR                 0x28

static uint16_t ms4525_ut;  // static result of temperature measurement
static uint16_t ms4525_up;  // static result of pressure measurement
static uint8_t rxbuf[4];

static void ms4525_start(pitotDev_t * pitot)
{
    busReadBuf( pitot->busDev, 0xFF, rxbuf, 4 );
}

static void ms4525_read(pitotDev_t * pitot)
{
    if (busReadBuf( pitot->busDev, 0xFF, rxbuf, 4 )) {
        ms4525_up = (rxbuf[0] << 8) | (rxbuf[1] << 0);
        ms4525_ut = ((rxbuf[2] << 8) | (rxbuf[3] << 0))>>5;
    }
}

static void ms4525_calculate(pitotDev_t * pitot, float *pressure, float *temperature)
{
    UNUSED(pitot);

    uint8_t status = (ms4525_up & 0xC000) >> 14;
    switch (status) {
        case 0:
            break;
        case 1:
            /* fallthrough */
        case 2:
            /* fallthrough */
        case 3:
            return;
    }
    int16_t dp_raw = 0, dT_raw = 0;

    /* mask the used bits */
    dp_raw = 0x3FFF & ms4525_up;
    dT_raw = ms4525_ut;

    float dP = (10 * (int32_t)(dp_raw)) * 0.1052120688f;
    float T  = (float)(200 * (int32_t)dT_raw - 102350) / 2047 + 273.15f;

    if (pressure)
        *pressure = dP;    // Pa
    if (temperature)
        *temperature = T; // K
}

bool ms4525Detect(pitotDev_t * pitot)
{
    pitot->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_MS4525, 0, OWNER_AIRSPEED);
    if (pitot->busDev == NULL) {
        return false;
    }

    bool ack = false;

    // Read twice to fix:
    // Sending a start-stop condition without any transitions on the SCL line (no clock pulses in between) creates a
    // communication error for the next communication, even if the next start condition is correct and the clock pulse is applied.
    // An additional start condition must be sent, which results in restoration of proper communication.
    ack = busReadBuf( pitot->busDev, 0xFF, rxbuf, 4 );
    ack = busReadBuf( pitot->busDev, 0xFF, rxbuf, 4 );
    if (!ack) {
        return false;
    }

    pitot->delay = 10000;
    pitot->start = ms4525_start;
    pitot->get = ms4525_read;
    pitot->calculate = ms4525_calculate;
    ms4525_read(pitot);
    return true;
}
