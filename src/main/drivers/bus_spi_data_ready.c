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
#include <stdint.h>

#include <platform.h>

#include "drivers/bus_spi.h"

#if defined(USE_SPI_DATA_READY)

#include "build/atomic.h"

#include "drivers/bus.h"
#include "drivers/bus_spi_data_ready_impl.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "drivers/time.h"

// Reads started by a data-ready interrupt take turns with the main loop's transfers:
// spiBusAcquire() waits for one on its way, and one asked for meanwhile starts at the last
// spiBusRelease(). The two nest: a speed change can come while a device is selected

typedef struct {
    const busDevice_t *dev;             // NULL: no reads started by a data-ready on this bus
    spiDataReadyCallback_t callback;
    void *arg;
    volatile bool running;              // a read is on the bus
    volatile bool pending;              // a data-ready came while the bus was taken
    volatile uint8_t mainLoopHolds;     // the main loop's acquires not yet released
    uint8_t failures;                   // reads that never ended
} spiDataReadyState_t;

static spiDataReadyState_t spiDataReady[SPIDEV_COUNT];

// A read that started must end within this, even at a slow bus speed
#define SPI_DATA_READY_WAIT_US  300
// After this many reads that never ended the device goes back to being polled
#define SPI_DATA_READY_MAX_FAILURES 3

static void spiDataReadyStart(SPIDevice device)
{
    spiDataReadyState_t *r = &spiDataReady[device];

    r->running = true;
    IOLo(r->dev->busdev.spi.csnPin);
    spiDataReadyHwStart(device);
}

// Takes a read off the bus, finished or not
static void spiDataReadyStop(SPIDevice device)
{
    spiDataReadyState_t *r = &spiDataReady[device];

    spiDataReadyHwStop(device);
    IOHi(r->dev->busdev.spi.csnPin);
    r->running = false;
}

void spiDataReadyDone(SPIDevice device, const uint8_t *data)
{
    spiDataReadyState_t *r = &spiDataReady[device];

    if (!r->running) {
        return;
    }
    spiDataReadyStop(device);

    // One more data-ready came while this read was on the bus
    if (r->pending && !r->mainLoopHolds) {
        r->pending = false;
        spiDataReadyStart(device);
    }

    r->callback(r->arg, data);
}

bool spiDataReadyInit(const busDevice_t *dev, uint8_t reg, uint8_t len, spiDataReadyCallback_t callback, void *arg)
{
    if (dev->busType != BUSTYPE_SPI || len == 0) {
        return false;
    }

    const SPIDevice device = dev->busdev.spi.spiBus;
    if ((unsigned)device >= SPIDEV_COUNT || spiDataReady[device].dev || !spiDataReadyHwInit(device, reg, len)) {
        return false;
    }

    spiDataReadyState_t *r = &spiDataReady[device];
    r->callback = callback;
    r->arg = arg;
    r->running = false;
    r->pending = false;
    r->mainLoopHolds = 0;

    // Last: from here on the main loop takes turns on this bus
    r->dev = dev;
    return true;
}

void spiDataReadyRequest(const busDevice_t *dev)
{
    const SPIDevice device = dev->busdev.spi.spiBus;
    spiDataReadyState_t *r = &spiDataReady[device];

    if (!r->dev) {
        // Given up on this bus
        return;
    }
    if (r->mainLoopHolds || r->running) {
        r->pending = true;
        return;
    }
    spiDataReadyStart(device);
}

void spiBusAcquire(SPIDevice device)
{
    if ((unsigned)device >= SPIDEV_COUNT || !spiDataReady[device].dev) {
        return;
    }

    spiDataReadyState_t *r = &spiDataReady[device];
    ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
        r->mainLoopHolds++;
    }

    // A read the data-ready started may still be on the bus
    const timeUs_t start = micros();
    while (r->running && cmpTimeUs(micros(), start) < SPI_DATA_READY_WAIT_US);

    ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
        if (r->running) {
            // It never ended: taken off the bus rather than waited for
            spiDataReadyStop(device);
            spiTimeoutUserCallback(spiInstanceByDevice(device));
            if (++r->failures >= SPI_DATA_READY_MAX_FAILURES) {
                spiDataReadyHwDisable(device);
                r->pending = false;
                r->dev = NULL;
            }
        }
    }
}

void spiBusRelease(SPIDevice device)
{
    if ((unsigned)device >= SPIDEV_COUNT || !spiDataReady[device].dev) {
        return;
    }

    spiDataReadyState_t *r = &spiDataReady[device];
    ATOMIC_BLOCK(NVIC_PRIO_GYRO_DATA_READY) {
        // An acquire from before the reads were set up has nothing to release
        if (r->mainLoopHolds && --r->mainLoopHolds == 0 && r->pending) {
            r->pending = false;
            spiDataReadyStart(device);
        }
    }
}
#endif
