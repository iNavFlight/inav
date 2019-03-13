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

#include "platform.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus_spi.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_l3gd20.h"

#ifdef USE_GYRO_L3GD20

#define READ_CMD               ((uint8_t)0x80)
#define MULTIPLEBYTE_CMD       ((uint8_t)0x40)
#define DUMMY_BYTE             ((uint8_t)0x00)

#define CTRL_REG1_ADDR         0x20
#define CTRL_REG4_ADDR         0x23
#define CTRL_REG5_ADDR         0x24
#define OUT_TEMP_ADDR          0x26
#define OUT_X_L_ADDR           0x28

#define MODE_ACTIVE                   ((uint8_t)0x08)

#define OUTPUT_DATARATE_1             ((uint8_t)0x00)
#define OUTPUT_DATARATE_2             ((uint8_t)0x40)
#define OUTPUT_DATARATE_3             ((uint8_t)0x80)
#define OUTPUT_DATARATE_4             ((uint8_t)0xC0)

#define AXES_ENABLE                   ((uint8_t)0x07)

#define BANDWIDTH_1                   ((uint8_t)0x00)
#define BANDWIDTH_2                   ((uint8_t)0x10)
#define BANDWIDTH_3                   ((uint8_t)0x20)
#define BANDWIDTH_4                   ((uint8_t)0x30)

#define FULLSCALE_250                 ((uint8_t)0x00)
#define FULLSCALE_500                 ((uint8_t)0x10)
#define FULLSCALE_2000                ((uint8_t)0x20)

#define BLOCK_DATA_UPDATE_CONTINUOUS  ((uint8_t)0x00)

#define BLE_MSB                       ((uint8_t)0x40)

#define BOOT                          ((uint8_t)0x80)

void l3gd20GyroInit(gyroDev_t *gyro)
{
    busWrite(gyro->busDev, CTRL_REG5_ADDR, BOOT);
    delay(1);

    busWrite(gyro->busDev, CTRL_REG1_ADDR, MODE_ACTIVE | OUTPUT_DATARATE_3 | AXES_ENABLE | BANDWIDTH_3);
    delay(1);

    busWrite(gyro->busDev, CTRL_REG4_ADDR, BLOCK_DATA_UPDATE_CONTINUOUS | BLE_MSB | FULLSCALE_2000);
    delay(100);
}

static bool l3gd20GyroRead(gyroDev_t *gyro)
{
    uint8_t buf[6];

    busReadBuf(gyro->busDev, OUT_X_L_ADDR | READ_CMD | MULTIPLEBYTE_CMD, buf, 6);

    gyro->gyroADCRaw[0] = (int16_t)((buf[0] << 8) | buf[1]);
    gyro->gyroADCRaw[1] = (int16_t)((buf[2] << 8) | buf[3]);
    gyro->gyroADCRaw[2] = (int16_t)((buf[4] << 8) | buf[5]);

    return true;
}

static bool deviceDetect(busDevice_t * dev)
{
    UNUSED(dev);
    return true;  // blindly assume it's present, for now.
}

bool l3gd20Detect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_L3GD20, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    gyro->initFn = l3gd20GyroInit;
    gyro->readFn = l3gd20GyroRead;

    // Page 9 in datasheet, So - Sensitivity, Full Scale = 2000, 70 mdps/digit
    gyro->scale = 0.07f;

    return true;
}

#endif
