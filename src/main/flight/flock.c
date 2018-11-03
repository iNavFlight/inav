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
 * along with INAV. If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Alberto Garcia Hierro <alberto@garciahierro.com>
 */

#include <string.h>

#include "platform.h"

#include "common/utils.h"

#include "drivers/time.h"

#include "flight/flock.h"

#ifdef USE_FLOCK

#define FLOCK_TIMEOUT_MS 15000
#define FLOCK_RAVEN_MAGIC_V1 "RVN1"

#define FLOCK_BIRD_IS_VALID(b) ((b)->lastUpdate > 0)

typedef struct flockRavenMessageInfo_s {
    flockBirdInfo_t info;
}__attribute__((packed)) flockRavenMessageInfo_t;

typedef struct flockRavenMessagePosvel_s {
    flockAddress_t rxAddr;
    flockBirdPosvel_t posvel;
}__attribute__((packed)) flockRavenMessagePosvel_t;

typedef enum {
    FLOCK_RAVEN_MESSAGE_INFO = 1, // flockRavenMessageInfo_t
    FLOCK_RAVEN_MESSAGE_POSVEL = 2, // flockRavenMessagePosvel_t
} flockRavenMessageType_e;

typedef struct flock_s {
    flockBird_t birds[FLOCK_MAX_BIRDS];
    int count;
} flock_t;

static flock_t flock;

void flockInit(void)
{
    memset(flock.birds, 0, sizeof(flock.birds));
    flock.count = 0;
}

void flockUpdate(timeUs_t currentTimeUs)
{
    timeMs_t now = currentTimeUs / 1000;

    if (now <= FLOCK_TIMEOUT_MS) {
        return;
    }

    timeMs_t threshold = now - FLOCK_TIMEOUT_MS;
    for (unsigned ii = 0; ii < ARRAYLEN(flock.birds); ii++) {
        flockBird_t *bird = &flock.birds[ii];
        if (FLOCK_BIRD_IS_VALID(bird) &&
            bird->lastUpdate < threshold) {

            bird->lastUpdate = 0;
            flock.count--;
        }
    }
}

static flockBird_t *flockGetBird(const flockAddress_t *rxAddr)
{
    for (unsigned ii = 0; ii < ARRAYLEN(flock.birds); ii++) {
        flockBird_t *bird = &flock.birds[ii];
        if (FLOCK_BIRD_IS_VALID(bird) &&
            memcmp(&bird->info.rxAddr, rxAddr, sizeof(*rxAddr)) == 0) {

            return bird;
        }
    }
    return NULL;
}

static flockBird_t *flockGetFreeSlot(void)
{
    for (unsigned ii = 0; ii < ARRAYLEN(flock.birds); ii++) {
        if (!FLOCK_BIRD_IS_VALID(&flock.birds[ii])) {
            return &flock.birds[ii];
        }
    }
    return NULL;
}

static int flockWriteRavenInfo(const void *data, size_t dataSize, void *buf, size_t bufSize)
{
    UNUSED(buf);
    UNUSED(bufSize);

    if (dataSize == sizeof(flockRavenMessageInfo_t)) {
        const flockRavenMessageInfo_t *msg = data;
        flockBird_t *bird = flockGetBird(&msg->info.rxAddr);
        if (!bird) {
            bird = flockGetFreeSlot();
            if (!bird) {
                return FLOCK_ENOMEM;
            }
            bird->flags = 0;
            flock.count++;
        }
        memcpy(&bird->info, &msg->info, sizeof(bird->info));
        bird->flags |= FLOCK_BIRD_FLAG_HAS_INFO;
        bird->lastUpdate = millis();
        return 0;
    }
    return FLOCK_EINVALIDINPUT;
}

static int flockWriteRavenPosvel(const void *data, size_t dataSize, void *buf, size_t bufSize)
{
    UNUSED(buf);
    UNUSED(bufSize);

    if (dataSize == sizeof(flockRavenMessagePosvel_t)) {
        const flockRavenMessagePosvel_t *msg = data;
        flockBird_t *bird = flockGetBird(&msg->rxAddr);
        if (!bird) {
            bird = flockGetFreeSlot();
            if (!bird) {
                return FLOCK_ENOMEM;
            }
            bird->flags = 0;
            flock.count++;
        }
        memcpy(&bird->posvel, &msg->posvel, sizeof(bird->posvel));
        bird->flags |= FLOCK_BIRD_FLAG_HAS_POSVEL;
        bird->lastUpdate = millis();
        return 0;
    }
    return FLOCK_EINVALIDINPUT;
}

static int flockWriteRaven(const uint8_t *data, size_t dataSize, void *buf, size_t bufSize)
{
    if (dataSize > 0) {
        switch (*data) {
            case FLOCK_RAVEN_MESSAGE_INFO:
                return flockWriteRavenInfo(&data[1], dataSize - 1, buf, bufSize);
            case FLOCK_RAVEN_MESSAGE_POSVEL:
                return flockWriteRavenPosvel(&data[1], dataSize - 1, buf, bufSize);
        }
    }
    return FLOCK_EINVALIDINPUT;
}

int flockWrite(const void *data, size_t dataSize, void *buf, size_t bufSize)
{
    if (dataSize > 4) {
        const uint8_t *p = data;
        if (p[0] == FLOCK_RAVEN_MAGIC_V1[0] &&
            p[1] == FLOCK_RAVEN_MAGIC_V1[1] &&
            p[2] == FLOCK_RAVEN_MAGIC_V1[2] &&
            p[3] == FLOCK_RAVEN_MAGIC_V1[3]) {

            return flockWriteRaven(&p[4], dataSize - 4, buf, bufSize);
        }
    }
    return FLOCK_EINVALIDINPUT;
}

int flockGetCount(void)
{
    return flock.count;
}

void flockIteratorInit(flockIterator_t *it)
{
    it->pos = -1;
}

bool flockIteratorNext(flockIterator_t *it)
{
    while (it->pos < (int)ARRAYLEN(flock.birds)) {
        it->pos++;
        if (FLOCK_BIRD_IS_VALID(&flock.birds[it->pos])) {
            return true;
        }
    }
    return false;
}

const flockBird_t *flockIteratorGet(flockIterator_t *it)
{
    if (it->pos >= 0 && it->pos < (int)ARRAYLEN(flock.birds) &&
        FLOCK_BIRD_IS_VALID(&flock.birds[it->pos])) {

        return &flock.birds[it->pos];
    }
    return NULL;
}

#endif
