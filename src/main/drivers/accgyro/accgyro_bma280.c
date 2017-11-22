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
#include <string.h>
#include <stdint.h>

#include <platform.h>

#include "drivers/system.h"
#include "drivers/bus.h"
#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_bma280.h"

#ifdef USE_ACC_BMA280

// BMA280, default I2C address mode 0x18
#define BMA280_WHOAMI       0x00
#define BMA280_ADDRESS      0x18
#define BMA280_ACC_X_LSB    0x02
#define BMA280_PMU_BW       0x10
#define BMA280_PMU_RANGE    0x0F

static void bma280Init(accDev_t *acc)
{
    busWrite(acc->busDev, BMA280_PMU_RANGE, 0x08); // +-8g range
    busWrite(acc->busDev, BMA280_PMU_BW, 0x0E); // 500Hz BW

    acc->acc_1G = 512 * 8;
}

static bool bma280Read(accDev_t *acc)
{
    uint8_t buf[6];

    if (!busReadBuf(acc->busDev, BMA280_ACC_X_LSB, buf, 6)) {
        return false;
    }

    // Data format is lsb<5:0><crap><new_data_bit> | msb<13:6>
    acc->ADCRaw[0] = (int16_t)((buf[0] >> 2) + (buf[1] << 8));
    acc->ADCRaw[1] = (int16_t)((buf[2] >> 2) + (buf[3] << 8));
    acc->ADCRaw[2] = (int16_t)((buf[4] >> 2) + (buf[5] << 8));

    return true;
}

static bool deviceDetect(busDevice_t * dev)
{
    bool ack = false;
    uint8_t sig = 0;

    ack = busRead(dev, BMA280_WHOAMI, &sig);
    if (!ack || sig != 0xFB)
        return false;

    return true;
}

bool bma280Detect(accDev_t *acc)
{
    acc->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_BMA280, acc->imuSensorToUse, OWNER_MPU);
    if (acc->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(acc->busDev)) {
        busDeviceDeInit(acc->busDev);
        return false;
    }

    acc->initFn = bma280Init;
    acc->readFn = bma280Read;
    return true;
}

#endif
