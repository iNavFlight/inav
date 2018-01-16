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

#include "drivers/bus_i2c.h"
#include "drivers/bus_spi.h"
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

/*
 * Gyro interrupt service routine
 */
#if defined(USE_MPU_DATA_READY_SIGNAL) && defined(USE_EXTI)
static void mpuIntExtiHandler(extiCallbackRec_t *cb)
{
    gyroDev_t *gyro = container_of(cb, gyroDev_t, exti);
    gyro->dataReady = true;
    if (gyro->updateFn) {
        gyro->updateFn(gyro);
    }
}
#endif

void mpuIntExtiInit(gyroDev_t *gyro)
{
    if (!gyro->busDev->irqPin) {
        return;
    }

#if defined(USE_MPU_DATA_READY_SIGNAL) && defined(USE_EXTI)
#ifdef ENSURE_MPU_DATA_READY_IS_LOW
    uint8_t status = IORead(gyro->busDev->irqPin);
    if (status) {
        return;
    }
#endif

#if defined (STM32F7)
    IOInit(gyro->busDev->irqPin, OWNER_MPU, RESOURCE_EXTI, 0);

    EXTIHandlerInit(&gyro->exti, mpuIntExtiHandler);
    EXTIConfig(gyro->busDev->irqPin, &gyro->exti, NVIC_PRIO_MPU_INT_EXTI, IO_CONFIG(GPIO_MODE_INPUT,0,GPIO_NOPULL));   // TODO - maybe pullup / pulldown ?
#else
    IOInit(gyro->busDev->irqPin, OWNER_MPU, RESOURCE_EXTI, 0);
    IOConfigGPIO(gyro->busDev->irqPin, IOCFG_IN_FLOATING);

    EXTIHandlerInit(&gyro->exti, mpuIntExtiHandler);
    EXTIConfig(gyro->busDev->irqPin, &gyro->exti, NVIC_PRIO_MPU_INT_EXTI, EXTI_Trigger_Rising);
    EXTIEnable(gyro->busDev->irqPin, true);
#endif
#endif
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

bool mpuAccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, MPU_RA_ACCEL_XOUT_H, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (int16_t)((data[0] << 8) | data[1]);
    acc->ADCRaw[Y] = (int16_t)((data[2] << 8) | data[3]);
    acc->ADCRaw[Z] = (int16_t)((data[4] << 8) | data[5]);

    return true;
}

bool mpuCheckDataReady(gyroDev_t* gyro)
{
    bool ret;
    if (gyro->dataReady) {
        ret = true;
        gyro->dataReady= false;
    } else {
        ret = false;
    }
    return ret;
}

/*
void mpuGyroSetIsrUpdate(gyroDev_t *gyro, sensorGyroUpdateFuncPtr updateFn)
{
    ATOMIC_BLOCK(NVIC_PRIO_MPU_INT_EXTI) {
        gyro->updateFn = updateFn;
    }
}
*/