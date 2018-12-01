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

#ifdef USE_MAG_IST8308

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"

#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/bus.h"
#include "drivers/light_led.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_ist8308.h"

#define IST8308_ADDRESS                                 0x0C

#define IST8308_REG_WHOAMI                              0x00
#define     IST8308_CHIP_ID                             0x08

#define IST8308_REG_DATA                                0x11    // XYZ High&Low


// Noise suppression filtering
#define IST8308_REG_CNTRL1                              0x30    // Control setting register 1
//bit6~bit5
#define     IST8308_NSF_DISABLE                         0x00
#define     IST8308_NSF_LOW                             0x20
#define     IST8308_NSF_MIDDLE                          0x40
#define     IST8308_NSF_HIGH                            0x60

#define IST8308_REG_CNTRL2                              0x31    // Control setting register 2
//bit4~bit0  Operation mode
#define     IST8308_OPMODE_STANDBY_MODE                 0x00    // Default
#define     IST8308_OPMODE_SINGLE_MODE                  0x01
#define     IST8308_OPMODE_CONTINUOS_MODE_10HZ          0x02
#define     IST8308_OPMODE_CONTINUOS_MODE_20HZ          0x04
#define     IST8308_OPMODE_CONTINUOS_MODE_50HZ          0x06
#define     IST8308_OPMODE_CONTINUOS_MODE_100HZ         0x08
#define     IST8308_OPMODE_CONTINUOS_MODE_200HZ         0x0A
#define     IST8308_OPMODE_CONTINUOS_MODE_8HZ           0x0B
#define     IST8308_OPMODE_CONTINUOS_MODE_1HZ           0x0C
#define     IST8308_OPMODE_CONTINUOS_MODE_0_5HZ         0x0D
#define     IST8308_OPMODE_SELF_TEST_MODE               0x10

// Sensitivity / measure dynamic range - default 660 for LSB2FSV = 1.5
#define IST8308_REG_CNTRL4                              0x34
#define     IST8308_SENSITIVITY_6_6                     0x00
#define     IST8308_SENSITIVITY_13_2                    0x01

// Over sampling ratio
#define IST8308_REG_OSR_CNTRL                           0x41    //Average control register
//bit2~bit0
#define     IST8308_X_Z_SENSOR_OSR_1                    0x00
#define     IST8308_X_Z_SENSOR_OSR_2                    0x01
#define     IST8308_X_Z_SENSOR_OSR_4                    0x02
#define     IST8308_X_Z_SENSOR_OSR_8                    0x03
#define     IST8308_X_Z_SENSOR_OSR_16                   0x04    //Default (ODRmax=100Hz)
#define     IST8308_X_Z_SENSOR_OSR_32                   0x05
//bit5~bit3
#define     IST8308_Y_SENSOR_OSR_1                      (0x00<<3)
#define     IST8308_Y_SENSOR_OSR_2                      (0x01<<3)
#define     IST8308_Y_SENSOR_OSR_4                      (0x02<<3)
#define     IST8308_Y_SENSOR_OSR_8                      (0x03<<3)
#define     IST8308_Y_SENSOR_OSR_16                     (0x04<<3)   //Default (ODRmax=100Hz)
#define     IST8308_Y_SENSOR_OSR_32                     (0x05<<3)



static bool ist8308Init(magDev_t * mag)
{
    bool ack =   busWrite(mag->busDev, IST8308_REG_OSR_CNTRL, IST8308_X_Z_SENSOR_OSR_16 | IST8308_Y_SENSOR_OSR_16);
    ack = ack && busWrite(mag->busDev, IST8308_REG_CNTRL1,    IST8308_NSF_LOW);
    ack = ack && busWrite(mag->busDev, IST8308_REG_CNTRL2,    IST8308_OPMODE_CONTINUOS_MODE_50HZ);
    ack = ack && busWrite(mag->busDev, IST8308_REG_CNTRL4,    IST8308_SENSITIVITY_6_6);
    delay(5);

    return true;
}

static bool ist8308Read(magDev_t * mag)
{
    const float LSB2FSV = 1.5; // 1.5mG

    uint8_t buf[6];

    bool ack = busReadBuf(mag->busDev, IST8308_REG_DATA, buf, 6);
    if (!ack) {
        // set magData to zero for case of failed read
        mag->magADCRaw[X] = 0;
        mag->magADCRaw[Y] = 0;
        mag->magADCRaw[Z] = 0;

        return false;
    }

    mag->magADCRaw[X] = (int16_t)(buf[1] << 8 | buf[0]) * LSB2FSV;
    mag->magADCRaw[Y] = (int16_t)(buf[3] << 8 | buf[2]) * LSB2FSV;
    mag->magADCRaw[Z] = (int16_t)(buf[5] << 8 | buf[4]) * LSB2FSV;

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, IST8308_REG_WHOAMI, &sig);

        if (ack && sig == IST8308_CHIP_ID) {
            return true;
        }
    }

    return false;
}

bool ist8308Detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_IST8308, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = ist8308Init;
    mag->read = ist8308Read;

    return true;
}

#endif
