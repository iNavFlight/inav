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

#include "common/bitarray.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "flight/flock.h"

#include "navigation/navigation.h"

#define FLOCK_RAVEN_MAGIC_V1 "RVN1"

#define FLOCK_TIMEOUT_MS 15000

#define FLOCK_MAX_UPDATES_PER_ITERATION 5

// Flock keeps degrees in deg / 10`000 to save some memory, while
// GPS/NAV/etc... use deg / 10`000`000
#define FLOCK_DEGREES_MULTIPLIER 1000
// Reverse flock +1000 meters offset, convert to cm
#define FLOCK_ALT_TO_ALT(alt) ((((int32_t)alt) - 1000) * 100)

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
    uint16_t max_distance; // meters
    int count;
    BITARRAY_DECLARE(dirty, FLOCK_MAX_BIRDS);
} flock_t;


#ifdef USE_FLOCK

#ifndef USE_NAV
#error "FLOCK requires NAV"
#endif

static flock_t flock;

void flockInit(void)
{
    memset(flock.birds, 0, sizeof(flock.birds));
    flock.count = 0;
}

static void flockUpdateRemoveStaleEntries(timeMs_t currentTimeMs)
{
    if (currentTimeMs <= FLOCK_TIMEOUT_MS) {
        return;
    }

    timeMs_t threshold = currentTimeMs - FLOCK_TIMEOUT_MS;
    for (unsigned ii = 0; ii < ARRAYLEN(flock.birds); ii++) {
        flockBird_t *bird = &flock.birds[ii];
        if (FLOCK_BIRD_IS_VALID(bird) &&
            bird->lastUpdate < threshold) {

            bird->lastUpdate = 0;
            flock.count--;
        }
    }
}

static bool flockUpdateBirdRelativePosition(flockBird_t *bird)
{
    if (bird->flags & FLOCK_BIRD_FLAG_HAS_POSVEL) {
        fpVector3_t pos;
        gpsLocation_t llh = {
            .lat = bird->posvel.lat * FLOCK_DEGREES_MULTIPLIER,
            .lon = bird->posvel.lon * FLOCK_DEGREES_MULTIPLIER,
            .alt = FLOCK_ALT_TO_ALT(bird->posvel.alt),
        };
        if (geoConvertGeodeticToLocalOrigin(&pos, &llh, GEO_ALT_ABSOLUTE)) {
            navDestinationPath_t path;
            if (navCalculatePathToDestination(&path, &pos)) {
                bird->rel.ground_distance = constrain(path.distance / 100, 0, UINT16_MAX);
                bird->rel.vertical_distance = constrain((pos.z - getEstimatedActualPosition(Z)) / 100, -1000, 7192) + 1000;
                bird->rel.bearing = path.bearing / (100 / 5);
                return true;
            }
        }
    }
    return false;
}

static void flockUpdateDirty(void)
{
    int p = BITARRAY_FIND_FIRST_SET(flock.dirty, 0);
    if (p < 0) {
        // We've passed over all entries. Update max values and start again.
        flock.max_distance = 0;
        for (unsigned ii = 0; ii < ARRAYLEN(flock.birds); ii++) {
            flockBird_t *b = &flock.birds[p];
            if (FLOCK_BIRD_IS_VALID(b) && (b->flags & FLOCK_BIRD_FLAG_HAS_REL_POS)) {
                flock.max_distance = MAX(flock.max_distance, b->rel.ground_distance);
            }
        }
        BITARRAY_SET_ALL(flock.dirty);
        p = 0;
    }
    flockBird_t *bird = &flock.birds[p];
    if (FLOCK_BIRD_IS_VALID(bird)) {
        if (flockUpdateBirdRelativePosition(bird)) {
            bird->flags |= FLOCK_BIRD_FLAG_HAS_REL_POS;
        } else {
            bird->flags &= ~FLOCK_BIRD_FLAG_HAS_REL_POS;
        }
    }
    bitArrayClr(flock.dirty, p);
}

void flockUpdate(timeUs_t currentTimeUs)
{
    // Shortcircuit to avoid doing useless work
    if (flock.count > 0) {
        flockUpdateRemoveStaleEntries(currentTimeUs / 1000);
        for (int ii = 0; ii < FLOCK_MAX_UPDATES_PER_ITERATION; ii++) {
            flockUpdateDirty();
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
