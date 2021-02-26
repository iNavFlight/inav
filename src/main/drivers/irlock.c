/*
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav.  If not, see <http://www.gnu.org/licenses/>.
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


#if defined(USE_NAV) && defined(USE_IRLOCK)

static bool irlockHealthy = false;

static bool irlockFrameSync(irlockDev_t *irlockDev)
{
    uint32_t sync_word = 0;
    uint8_t count = 10;
    while (count-- && sync_word != IRLOCK_FRAME_SYNC) {
        uint8_t sync_byte;
        irlockHealthy = busRead(irlockDev->busDev, 0xFF, &sync_byte);
        if (!(irlockHealthy && sync_byte)) return false;
        sync_word = (sync_word >> 8) | (((uint32_t)sync_byte) << 24);
    }
    return sync_word == IRLOCK_FRAME_SYNC;
}

static bool irlockRead(irlockDev_t *irlockDev, irlockData_t *irlockData)
{
    if (irlockFrameSync(irlockDev) && busReadBuf(irlockDev->busDev, 0xFF, (void*)irlockData, sizeof(*irlockData))) {
        uint16_t cksum = irlockData->signature + irlockData->posX + irlockData->posY + irlockData->sizeX + irlockData->sizeY;
        if (irlockData->cksum == cksum) return true;
    }
    return false;
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
