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
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_data_ready.h"

// Check busDevice scratchpad memory size
STATIC_ASSERT(sizeof(mpuContextData_t) < BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

static const gyroFilterAndRateConfig_t mpuGyroConfigs[] = {
    { GYRO_LPF_256HZ,   8000,   { MPU_DLPF_256HZ,   0  } },
    { GYRO_LPF_256HZ,   4000,   { MPU_DLPF_256HZ,   1  } },
    { GYRO_LPF_256HZ,   2000,   { MPU_DLPF_256HZ,   3  } },
    { GYRO_LPF_256HZ,   1000,   { MPU_DLPF_256HZ,   7  } },
    { GYRO_LPF_256HZ,    666,   { MPU_DLPF_256HZ,   11 } },
    { GYRO_LPF_256HZ,    500,   { MPU_DLPF_256HZ,   15 } },
};

const gyroFilterAndRateConfig_t * mpuChooseGyroConfig(uint8_t desiredLpf, uint16_t desiredRateHz)
{
    return chooseGyroConfig(desiredLpf, desiredRateHz, &mpuGyroConfigs[0], ARRAYLEN(mpuGyroConfigs));
}

bool mpuGyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, MPU_RA_GYRO_XOUT_H, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_big_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_big_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_big_endian(data, 2);

    return true;
}

static bool mpuUpdateSensorContext(busDevice_t * busDev, mpuContextData_t * ctx)
{
    ctx->lastReadStatus = busReadBuf(busDev, MPU_RA_ACCEL_XOUT_H, ctx->accRaw, 6 + 2 + 6);
    return ctx->lastReadStatus;
}

bool mpuGyroReadScratchpad(gyroDev_t *gyro)
{
    busDevice_t * busDev = gyro->busDev;
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(busDev);

    if (mpuUpdateSensorContext(busDev, ctx)) {
        gyro->gyroADCRaw[X] = (float) int16_val_big_endian(ctx->gyroRaw, 0);
        gyro->gyroADCRaw[Y] = (float) int16_val_big_endian(ctx->gyroRaw, 1);
        gyro->gyroADCRaw[Z] = (float) int16_val_big_endian(ctx->gyroRaw, 2);
        return true;
    }

    return false;
}

bool mpuAccReadScratchpad(accDev_t *acc)
{
#if defined(USE_SPI_DATA_READY)
    // The read on data-ready brought the accelerometer too
    int16_t v[XYZ_AXIS_COUNT];
    if (gyroDataReadyAcc(acc->busDev, v)) {
        acc->ADCRaw[X] = v[X];
        acc->ADCRaw[Y] = v[Y];
        acc->ADCRaw[Z] = v[Z];
        return true;
    }
#endif

    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);

    if (ctx->lastReadStatus) {
        acc->ADCRaw[X] = (float) int16_val_big_endian(ctx->accRaw, 0);
        acc->ADCRaw[Y] = (float) int16_val_big_endian(ctx->accRaw, 1);
        acc->ADCRaw[Z] = (float) int16_val_big_endian(ctx->accRaw, 2);
        return true;
    }

    return false;
}

bool mpuTemperatureReadScratchpad(gyroDev_t *gyro, int16_t * data)
{
#if defined(USE_SPI_DATA_READY)
    int16_t raw;
    if (gyroDataReadyTemperature(gyro->busDev, &raw)) {
        *data = raw / 34 + 365;
        return true;
    }
#endif

    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);

    if (ctx->lastReadStatus) {
        // Convert to degC*10: degC = raw / 340 + 36.53
        *data = int16_val_big_endian(ctx->tempRaw, 0) / 34 + 365;
        return true;
    }

    return false;
}

#if defined(USE_SPI_DATA_READY)
// From ACCEL_XOUT_H: accelerometer X, Y, Z, temperature, gyro X, Y, Z, big-endian
static void mpuDataReadyParse(const uint8_t *data, bool withAccAndTemp, int16_t *gyro, int16_t *acc, int16_t *temp)
{
    if (withAccAndTemp) {
        acc[X] = int16_val_big_endian(data, 0);
        acc[Y] = int16_val_big_endian(data, 1);
        acc[Z] = int16_val_big_endian(data, 2);
        *temp = int16_val_big_endian(data, 3);
        data += 8;
    }
    gyro[X] = int16_val_big_endian(data, 0);
    gyro[Y] = int16_val_big_endian(data, 1);
    gyro[Z] = int16_val_big_endian(data, 2);
}

static const gyroDataReadyDriver_t mpuDataReady = {
    .withAccAndTemp = { MPU_RA_ACCEL_XOUT_H | 0x80, 14 },
    .gyroOnly = { MPU_RA_GYRO_XOUT_H | 0x80, 6 },
    .hasTemp = true,
    .parse = mpuDataReadyParse,
    .registerRead = mpuGyroReadScratchpad,
    .tested = true,
};

// A 50 us pulse on INT at each new sample. Written at the initialisation speed: the MPU6000 takes
// register writes at 1 MHz at most
void mpuDataReadySetup(gyroDev_t *gyro)
{
    busWrite(gyro->busDev, MPU_RA_INT_PIN_CFG, MPU_INT_ANYRD_2CLEAR);
    delayMicroseconds(15);
    busWrite(gyro->busDev, MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
    gyro->dataReadyDriver = &mpuDataReady;
}
#endif
