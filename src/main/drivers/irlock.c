/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include <math.h>

#include "platform.h"

#include "drivers/sensor.h"
#include "drivers/irlock.h"
#include "drivers/time.h"

#define IRLOCK_OBJECT_SYNC ((uint16_t)0xaa55)
#define IRLOCK_FRAME_SYNC ((uint32_t)(IRLOCK_OBJECT_SYNC | (IRLOCK_OBJECT_SYNC << 16)))


#if defined(USE_IRLOCK)

static bool irlockHealthy = false;

// The sensor streams frames; the blocking driver scanned up to this many bytes for the frame sync per call
#define IRLOCK_SYNC_READ_LIMIT  10

static irlockReadResult_e irlockRead(irlockDev_t *irlockDev, irlockData_t *irlockData)
{
    static uint32_t syncWord = 0;
    static uint8_t syncReads = 0;
    static uint8_t syncByte;                // target of the one byte sync reads, filled in the background
    static bool transferPending = false;
    static bool frameRequested = false;     // the pending transfer is the frame itself
    bool busError = false;

    if (transferPending) {
        if (busIsBusy(irlockDev->busDev, &busError) && !busError) {
            return IRLOCK_READ_PENDING;
        }
        transferPending = false;
        irlockHealthy = !busError;

        if (busError) {
            syncWord = 0;
            syncReads = 0;
            frameRequested = false;
            return IRLOCK_READ_NO_DATA;
        }

        if (frameRequested) {
            frameRequested = false;
            syncWord = 0;
            syncReads = 0;
            const uint16_t cksum = irlockData->signature + irlockData->posX + irlockData->posY + irlockData->sizeX + irlockData->sizeY;
            return (irlockData->cksum == cksum) ? IRLOCK_READ_FRAME : IRLOCK_READ_NO_DATA;
        }

        // one more byte of the sync scan arrived
        if (syncByte == 0) {
            syncWord = 0;                   // nothing streaming, give up until the next regular call
            syncReads = 0;
            return IRLOCK_READ_NO_DATA;
        }
        syncWord = (syncWord >> 8) | (((uint32_t)syncByte) << 24);
        if (syncWord == IRLOCK_FRAME_SYNC) {
            transferPending = busReadBufStart(irlockDev->busDev, 0xFF, (uint8_t *)irlockData, sizeof(*irlockData));
            frameRequested = transferPending;
            return IRLOCK_READ_PENDING;
        }
        if (++syncReads >= IRLOCK_SYNC_READ_LIMIT) {
            syncWord = 0;
            syncReads = 0;
            return IRLOCK_READ_NO_DATA;
        }
    }

    // scan for the frame sync one byte at a time
    transferPending = busReadBufStart(irlockDev->busDev, 0xFF, &syncByte, 1);
    return IRLOCK_READ_PENDING;
}

static bool deviceDetect(irlockDev_t *irlockDev)
{
    uint8_t buf;
    bool detected = busRead(irlockDev->busDev, 0xFF, &buf);
    return !!detected;
}

bool irlockDetect(irlockDev_t *irlockDev)
{
    irlockDev->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_IRLOCK, 0, OWNER_IRLOCK);
    if (irlockDev->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(irlockDev)) {
        busDeviceDeInit(irlockDev->busDev);
        return false;
    }

    irlockDev->read = irlockRead;

    return true;
}

bool irlockIsHealthy(void)
{
    return irlockHealthy;
}

#endif /* USE_IRLOCK */
