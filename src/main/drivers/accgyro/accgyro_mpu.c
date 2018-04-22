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
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"

// Check busDevice scratchpad memory size
STATIC_ASSERT(sizeof(mpuContextData_t) < BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

static const gyroFilterAndRateConfig_t mpuGyroConfigs[] = {
    { GYRO_LPF_256HZ,   8000,   { MPU_DLPF_256HZ,   0  } },
    { GYRO_LPF_256HZ,   4000,   { MPU_DLPF_256HZ,   1  } },
    { GYRO_LPF_256HZ,   2000,   { MPU_DLPF_256HZ,   3  } },
    { GYRO_LPF_256HZ,   1000,   { MPU_DLPF_256HZ,   7  } },
    { GYRO_LPF_256HZ,    666,   { MPU_DLPF_256HZ,   11  } },
    { GYRO_LPF_256HZ,    500,   { MPU_DLPF_256HZ,   15 } },

    { GYRO_LPF_188HZ,   1000,   { MPU_DLPF_188HZ,   0  } },
    { GYRO_LPF_188HZ,    500,   { MPU_DLPF_188HZ,   1  } },

    { GYRO_LPF_98HZ,    1000,   { MPU_DLPF_98HZ,    0  } },
    { GYRO_LPF_98HZ,     500,   { MPU_DLPF_98HZ,    1  } },

    { GYRO_LPF_42HZ,    1000,   { MPU_DLPF_42HZ,    0  } },
    { GYRO_LPF_42HZ,     500,   { MPU_DLPF_42HZ,    1  } },

    { GYRO_LPF_20HZ,    1000,   { MPU_DLPF_20HZ,    0  } },
    { GYRO_LPF_20HZ,     500,   { MPU_DLPF_20HZ,    1  } },

    { GYRO_LPF_10HZ,    1000,   { MPU_DLPF_10HZ,    0  } },
    { GYRO_LPF_10HZ,     500,   { MPU_DLPF_10HZ,    1  } }
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

    gyro->gyroADCRaw[X] = (int16_t)((data[0] << 8) | data[1]);
    gyro->gyroADCRaw[Y] = (int16_t)((data[2] << 8) | data[3]);
    gyro->gyroADCRaw[Z] = (int16_t)((data[4] << 8) | data[5]);

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
        gyro->gyroADCRaw[X] = (int16_t)((ctx->gyroRaw[0] << 8) | ctx->gyroRaw[1]);
        gyro->gyroADCRaw[Y] = (int16_t)((ctx->gyroRaw[2] << 8) | ctx->gyroRaw[3]);
        gyro->gyroADCRaw[Z] = (int16_t)((ctx->gyroRaw[4] << 8) | ctx->gyroRaw[5]);
        return true;
    }

    return false;
}

bool mpuAccReadScratchpad(accDev_t *acc)
{
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);

    if (ctx->lastReadStatus) {
        acc->ADCRaw[X] = (int16_t)((ctx->accRaw[0] << 8) | ctx->accRaw[1]);
        acc->ADCRaw[Y] = (int16_t)((ctx->accRaw[2] << 8) | ctx->accRaw[3]);
        acc->ADCRaw[Z] = (int16_t)((ctx->accRaw[4] << 8) | ctx->accRaw[5]);
        return true;
    }

    return false;
}

bool mpuTemperatureReadScratchpad(gyroDev_t *gyro, int16_t * data)
{
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);

    if (ctx->lastReadStatus) {
        // Convert to degC*10: degC = raw / 340 + 36.53
        *data = (int16_t)((ctx->tempRaw[0] << 8) | ctx->tempRaw[1]) / 34 + 365;
        return true;
    }

    return false;
}
