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
#include "drivers/time.h"
#include "build/debug.h"

static busDevice_t * busDev;

static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < 5; retry++) {
        uint8_t sig;

        delay(150);

        bool ack = busRead(busDev, 0x00, &sig);
        if (ack) {
            return true;
        }
    };

    return false;
}

bool bno055Init(void)
{
    busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_BNO055, 0, 0);
    if (busDev == NULL) {
        DEBUG_SET(DEBUG_IMU2, 2, 1);
        return false;
    }

    if (!deviceDetect(busDev)) {
        DEBUG_SET(DEBUG_IMU2, 2, 2);
        busDeviceDeInit(busDev);
        return false;
    }

    // /* Reset device */
    // busWrite(busDev, PCA9685_MODE1, 0x00);

    // /* Set refresh rate */
    // pca9685setPWMFreq(PCA9685_SERVO_FREQUENCY);

    // delay(1);

    // for (uint8_t i = 0; i < PCA9685_SERVO_COUNT; i++) {
    //     pca9685setPWMOn(i, 0);
    //     pca9685setPWMOff(i, 1500);
    // }

    return true;
}