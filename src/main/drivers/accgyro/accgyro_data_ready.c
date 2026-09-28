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

#include "common/maths.h"
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
    uint8_t fifoSamples;                // samples per interrupt through the IMU's FIFO, 0 for register reads
    uint8_t fifoPackets;                // packets each read brings: one more than that where it fits
    uint8_t fifoPacketLen;
    volatile bool fifoFlush;            // the FIFO fell behind or out of step: empty it from the task
    uint8_t fifoFruitlessReads;         // reads in a row that found packets counted but no sample
};

// Reads in a row with packets counted but none held: out of step. One alone is normal (a packet
// counted just before it can be read); a read that found the queue full doesn't count
#define GYRO_DATA_READY_FIFO_FRUITLESS 4

// Further behind than this many reads' worth, the FIFO is emptied rather than caught up
#define GYRO_DATA_READY_FIFO_BEHIND 2

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

// The count, then fifoPackets packets, oldest first. Every packet read has left the FIFO, even
// one that came after the count, so all are decoded up to the first empty one
static void gyroDataReadyFifoSamples(void *arg, const uint8_t *data)
{
    gyroDataReady_t *d = arg;
    const gyroDataReadyFifo_t *fifo = d->driver->fifo;
    const uint16_t count = (data[0] << 8) | data[1];
    const uint8_t *packet = &data[2];
    int16_t gyro[XYZ_AXIS_COUNT];
    int16_t acc[XYZ_AXIS_COUNT];
    int16_t temp = 0;

    if (count > GYRO_DATA_READY_FIFO_BEHIND * d->fifoSamples) {
        d->fifoFlush = true;
    }

    unsigned pushed = 0;
    for (unsigned i = 0; i < d->fifoPackets; i++, packet += d->fifoPacketLen) {
        const gyroFifoPacket_e what = fifo->parse(packet, d->withAccAndTemp, gyro, acc, &temp);
        if (what == GYRO_FIFO_PACKET_EMPTY) {
            break;
        }
        if (what == GYRO_FIFO_PACKET_UNEXPECTED) {
            d->fifoFlush = true;
            break;
        }
        if (what == GYRO_FIFO_PACKET_INVALID) {
            continue;
        }
        if (d->withAccAndTemp) {
            d->acc[X] = acc[X];
            d->acc[Y] = acc[Y];
            d->acc[Z] = acc[Z];
            d->temp = temp;
        }
        if (!gyroSampleQueuePush(d->gyro, gyro[X], gyro[Y], gyro[Z])) {
            EXTIEnable(d->gyro->busDev->irqPin, false);
            d->paused = true;
            break;
        }
        pushed++;
    }

    if (pushed) {
        d->fifoFruitlessReads = 0;
        d->lastSampleUs = microsISR();
    } else if (count && !d->paused && ++d->fifoFruitlessReads >= GYRO_DATA_READY_FIFO_FRUITLESS) {
        // Until the flush, and whenever no sample comes for a while, the task reads the registers
        d->fifoFlush = true;
        d->fifoFruitlessReads = 0;
    }
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

    if (d->fifoSamples && (d->paused || d->fifoFlush)) {
        // What the FIFO holds is old, or out of step with the reads
        d->driver->fifo->flush(gyro);
        d->fifoFlush = false;
        if (d->paused) {
            // No register read: a sample taken during the flush would come again from the FIFO
            ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
                gyro->sampleQueueTail = gyro->sampleQueueHead;
                d->paused = false;
                EXTIEnable(gyro->busDev->irqPin, true);
            }
            d->resumedUs = micros();
            return false;
        }
    }

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

// Before a flash write, which stalls every interrupt running from flash (over a second on H7):
// no reads meanwhile. With a DMA read around a flash write and the host closing the USB port,
// an H7 has stopped entirely
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

void gyroDataReadyStart(gyroDev_t *gyro)
{
    const gyroDataReadyDriver_t *driver = gyro->dataReadyDriver;
    busDevice_t *dev = gyro->busDev;
    if (!driver || !dev || !dev->irqPin || gyro->readOnDataReady == GYRO_DEV_DATA_READY_OFF ||
        (gyro->readOnDataReady == GYRO_DEV_DATA_READY_WHERE_TESTED && !driver->tested)) {
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

    const gyroDataReadyFifo_t *fifo = driver->fifo;
    d->fifoSamples = 0;
    if (gyro->dataReadyFifoSamples && fifo) {
        d->withAccAndTemp = gyro->readOnDataReadyWithAcc && fifo->packetLen;
        d->fifoPacketLen = d->withAccAndTemp ? fifo->packetLen : fifo->gyroOnlyPacketLen;
        // One packet more than the threshold where it fits, or a read behind by one stays behind
        for (d->fifoPackets = gyro->dataReadyFifoSamples + 1; d->fifoPacketLen && d->fifoPackets >= gyro->dataReadyFifoSamples; d->fifoPackets--) {
            if (spiDataReadyInit(dev, fifo->countReg, 2 + d->fifoPackets * d->fifoPacketLen, gyroDataReadyFifoSamples, d)) {
                d->fifoSamples = gyro->dataReadyFifoSamples;
                fifo->start(gyro, d->withAccAndTemp, d->fifoSamples);
                break;
            }
        }
    }

    if (!d->fifoSamples) {
        d->withAccAndTemp = gyro->readOnDataReadyWithAcc && driver->withAccAndTemp.len;
        if (!d->withAccAndTemp || !spiDataReadyInit(dev, driver->withAccAndTemp.reg, driver->withAccAndTemp.len, gyroDataReadySample, d)) {
            d->withAccAndTemp = false;
            if (!driver->gyroOnly.len || !spiDataReadyInit(dev, driver->gyroOnly.reg, driver->gyroOnly.len, gyroDataReadySample, d)) {
                return;
            }
        }
    }
    d->gyro = gyro;

    IOInit(dev->irqPin, OWNER_MPU, RESOURCE_EXTI, RESOURCE_INDEX(gyro->imuSensorToUse));
    EXTIHandlerInit(&d->exti, gyroDataReadyExti);
    EXTIConfig(dev->irqPin, &d->exti, NVIC_PRIO_GYRO_DATA_READY, IOCFG_IN_FLOATING);
    EXTIEnable(dev->irqPin, true);

    gyro->dataReadyState = d;
    gyro->readFn = gyroDataReadyRead;
}
#endif
