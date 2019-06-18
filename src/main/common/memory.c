/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdlib.h>
#include <stdint.h>

#include "platform.h"

#include "common/log.h"
#include "common/memory.h"

#include "drivers/resource.h"

#include "fc/runtime_config.h"

#if !defined(DYNAMIC_HEAP_SIZE)
#define DYNAMIC_HEAP_SIZE   (2048)
#endif

static uint32_t dynHeap[DYNAMIC_HEAP_SIZE / sizeof(uint32_t)];
static uint32_t dynHeapFreeWord = 0;
static size_t dynHeapUsage[OWNER_TOTAL_COUNT];

void * memAllocate(size_t wantedSize, resourceOwner_e owner)
{
    void * retPointer = NULL;
    const size_t wantedWords = (wantedSize + sizeof(uint32_t) - 1) / sizeof(uint32_t);

    if ((dynHeapFreeWord + wantedWords) <= DYNAMIC_HEAP_SIZE / sizeof(uint32_t)) {
        // Success
        retPointer = &dynHeap[dynHeapFreeWord];
        dynHeapFreeWord += wantedWords;
        dynHeapUsage[owner] += wantedWords * sizeof(uint32_t);
        LOG_D(SYSTEM, "Memory allocated. Free memory = %d", memGetAvailableBytes());
    }
    else {
        // OOM
        LOG_E(SYSTEM, "Out of memory");
        ENABLE_ARMING_FLAG(ARMING_DISABLED_OOM);
    }

    return retPointer;
}

size_t memGetUsedBytesByOwner(resourceOwner_e owner)
{
    return (owner == OWNER_FREE) ? memGetAvailableBytes() : dynHeapUsage[owner];
}

size_t memGetAvailableBytes(void)
{
    return (DYNAMIC_HEAP_SIZE / sizeof(uint32_t) - dynHeapFreeWord) * sizeof(uint32_t);
}
