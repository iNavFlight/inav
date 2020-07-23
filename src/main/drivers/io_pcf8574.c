/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdbool.h>
#include <stdint.h>
#include "drivers/bus.h"
#include "drivers/io_pcf8574.h"
#include "drivers/time.h"
#include "build/debug.h"

#define PCF8574_WRITE_ADDRESS 0x40
#define PCF8574_READ_ADDRESS 0x41

static busDevice_t *busDev;

static bool deviceDetect(busDevice_t *busDev)
{
    for (int retry = 0; retry < 5; retry++)
    {
        uint8_t sig;

        delay(150);

        bool ack = busRead(busDev, 0x00, &sig);
        if (ack)
        {
            return true;
        }
    };

    return false;
}

bool pcf8574Init(void)
{
    busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_PCF8574, 0, 0);
    if (busDev == NULL)
    {
        DEBUG_SET(DEBUG_PCF8574, 0, 1);
        return false;
    }

    if (!deviceDetect(busDev))
    {
        DEBUG_SET(DEBUG_PCF8574, 0, 2);
        busDeviceDeInit(busDev);
        return false;
    }

    return true;
}

void pcf8574Write(uint8_t data)
{
    busWrite(busDev, PCF8574_WRITE_ADDRESS, data);
}

uint8_t pcf8574Read(void)
{
    uint8_t data;
    busRead(busDev, PCF8574_READ_ADDRESS, &data);
    return data;
}