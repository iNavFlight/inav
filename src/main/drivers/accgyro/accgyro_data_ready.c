/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <platform.h>

#include "drivers/bus_spi.h"

#if defined(USE_SPI_DATA_READY)

#include "build/atomic.h"

#include "common/utils.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_data_ready.h"
#include "drivers/bus.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "drivers/time.h"

// The IMU's data-ready interrupt starts an SPI read the CPU does not wait on, and its end queues
// the sample for the gyro task. A gyro nobody reads fills its queue, and its interrupt goes off

// Without a sample for this long the registers are read as they would be without data-ready
#define GYRO_DATA_READY_TIMEOUT_US  2000

struct gyroDataReady_s {
    extiCallbackRec_t exti;             // first: the EXTI callback is handed a pointer to it
    gyroDev_t *gyro;                    // NULL: free
    const gyroDataReadyDriver_t *driver;
    bool withAccAndTemp;
    volatile bool paused;               // the queue filled up and the interrupt is off
    volatile int16_t acc[XYZ_AXIS_COUNT];
    volatile int16_t temp;
    volatile timeUs_t lastSampleUs;
    timeUs_t resumedUs;                 // when a read after a pause turned the interrupt back on
};

// The primary gyro and the secondary
static gyroDataReady_t gyroDataReady[2];

// NULL unless the last read brought the accelerometer and is recent, so values never freeze
static const gyroDataReady_t *gyroDataReadyRecent(const busDevice_t *dev)
{
    for (unsigned i = 0; i < ARRAYLEN(gyroDataReady); i++) {
        const gyroDataReady_t *d = &gyroDataReady[i];
        if (d->gyro && d->gyro->busDev == dev) {
            if (!d->withAccAndTemp || cmpTimeUs(micros(), d->lastSampleUs) > GYRO_DATA_READY_TIMEOUT_US) {
                return NULL;
            }
            return d;
        }
    }
    return NULL;
}

bool gyroDataReadyAcc(const busDevice_t *dev, int16_t *acc)
{
    const gyroDataReady_t *d = gyroDataReadyRecent(dev);
    if (!d) {
        return false;
    }
    ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
        acc[X] = d->acc[X];
        acc[Y] = d->acc[Y];
        acc[Z] = d->acc[Z];
    }
    return true;
}

bool gyroDataReadyTemperature(const busDevice_t *dev, int16_t *temp)
{
    const gyroDataReady_t *d = gyroDataReadyRecent(dev);
    if (!d || !d->driver->hasTemp) {
        return false;
    }
    *temp = d->temp;
    return true;
}

// In the interrupt that ended the read, with the bytes that followed the address
static void gyroDataReadySample(void *arg, const uint8_t *data)
{
    gyroDataReady_t *d = arg;
    int16_t gyro[XYZ_AXIS_COUNT];
    int16_t acc[XYZ_AXIS_COUNT];
    int16_t temp = 0;

    d->driver->parse(data, d->withAccAndTemp, gyro, acc, &temp);
    if (d->withAccAndTemp) {
        d->acc[X] = acc[X];
        d->acc[Y] = acc[Y];
        d->acc[Z] = acc[Z];
        d->temp = temp;
    }

    if (!gyroSampleQueuePush(d->gyro, gyro[X], gyro[Y], gyro[Z])) {
        // No sample taken for as long as the queue lasts
        EXTIEnable(d->gyro->busDev->irqPin, false);
        d->paused = true;
    }
    d->lastSampleUs = microsISR();
}

static void gyroDataReadyExti(extiCallbackRec_t *cb)
{
    gyroDataReady_t *d = (gyroDataReady_t *)cb;
    spiDataReadyRequest(d->gyro->busDev);
}

// The gyro's readFn: hands out the queued samples one per call
static bool gyroDataReadyRead(gyroDev_t *gyro)
{
    gyroDataReady_t *d = gyro->dataReadyState;

    if (d->paused) {
        // After a pause the queue is old: turn the interrupt back on and read the registers
        ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
            gyro->sampleQueueTail = gyro->sampleQueueHead;
            d->paused = false;
            EXTIEnable(gyro->busDev->irqPin, true);
        }
        d->resumedUs = micros();
        return d->driver->registerRead(gyro);
    }

    if (gyroSampleQueuePop(gyro)) {
        return true;
    }

    // Nothing queued: if samples stopped coming, read the registers so a lost interrupt line cannot
    // freeze the gyro (after a pause, counting from the restart)
    const timeUs_t now = micros();
    if (cmpTimeUs(now, d->lastSampleUs) > GYRO_DATA_READY_TIMEOUT_US && cmpTimeUs(now, d->resumedUs) > GYRO_DATA_READY_TIMEOUT_US) {
        return d->driver->registerRead(gyro);
    }
    return false;
}

// No reads during a flash write: with a DMA read in flight and the USB port closing, an H7 has
// stopped entirely
void gyroDataReadySuspend(void)
{
    for (unsigned i = 0; i < ARRAYLEN(gyroDataReady); i++) {
        gyroDataReady_t *d = &gyroDataReady[i];
        if (!d->gyro) {
            continue;
        }
        ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
            EXTIEnable(d->gyro->busDev->irqPin, false);
            d->paused = true;
        }
        const SPIDevice bus = d->gyro->busDev->busdev.spi.spiBus;
        spiBusAcquire(bus);
        spiDataReadyCancelPending(bus);
        spiBusRelease(bus);
    }
}

bool gyroDataReadyWanted(const gyroDev_t *gyro)
{
    const gyroDataReadyDriver_t *driver = gyro->dataReadyDriver;
    const busDevice_t *dev = gyro->busDev;
    return driver && dev && dev->irqPin && gyro->readOnDataReady != GYRO_DEV_DATA_READY_OFF &&
        (gyro->readOnDataReady != GYRO_DEV_DATA_READY_WHERE_TESTED || driver->tested);
}

void gyroDataReadyStart(gyroDev_t *gyro)
{
    const gyroDataReadyDriver_t *driver = gyro->dataReadyDriver;
    busDevice_t *dev = gyro->busDev;
    if (!gyroDataReadyWanted(gyro)) {
        return;
    }
    // A pin already owned stays with its owner; a wrong but free pin only means polling as before
    if (IOGetOwner(dev->irqPin) != OWNER_FREE) {
        return;
    }

    gyroDataReady_t *d = NULL;
    for (unsigned i = 0; i < ARRAYLEN(gyroDataReady) && !d; i++) {
        if (!gyroDataReady[i].gyro) {
            d = &gyroDataReady[i];
        }
    }
    if (!d) {
        return;
    }

    d->driver = driver;
    d->lastSampleUs = micros();

    d->withAccAndTemp = gyro->readOnDataReadyWithAcc && driver->withAccAndTemp.len;
    if (!d->withAccAndTemp || !spiDataReadyInit(dev, driver->withAccAndTemp.reg, driver->withAccAndTemp.len, gyroDataReadySample, d)) {
        d->withAccAndTemp = false;
        if (!driver->gyroOnly.len || !spiDataReadyInit(dev, driver->gyroOnly.reg, driver->gyroOnly.len, gyroDataReadySample, d)) {
            return;
        }
    }
    d->gyro = gyro;

    IOInit(dev->irqPin, OWNER_MPU, RESOURCE_EXTI, RESOURCE_INDEX(gyro->imuSensorToUse));
    EXTIHandlerInit(&d->exti, gyroDataReadyExti);
#if defined(STM32F7) || defined(STM32H7)
    EXTIConfig(dev->irqPin, &d->exti, NVIC_PRIO_GYRO_DATA_READY, IOCFG_IN_FLOATING);   // always on the rising edge
#else
    EXTIConfig(dev->irqPin, &d->exti, NVIC_PRIO_GYRO_DATA_READY, EXTI_Trigger_Rising);
#endif
    EXTIEnable(dev->irqPin, true);

    gyro->dataReadyState = d;
    gyro->readFn = gyroDataReadyRead;
}
#endif
