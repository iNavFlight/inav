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
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
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

/*
 * Persistent data storage for RP2350 using watchdog scratch registers.
 *
 * The RP2350 watchdog peripheral provides eight 32-bit scratch registers
 * (watchdog_hw->scratch[0..7]) that survive a watchdog-triggered reboot
 * but are zeroed on power-on reset.  This is the direct equivalent of the
 * STM32 RTC backup registers used by persistent.c.
 *
 * Register mapping:
 *   scratch[PERSISTENT_OBJECT_MAGIC]        — magic value ("iNav")
 *   scratch[PERSISTENT_OBJECT_RESET_REASON] — bootloader/MSC request code
 *
 * Capacity: 8 registers available, PERSISTENT_OBJECT_COUNT currently = 2.
 */

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#include "drivers/persistent.h"

#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#define PERSISTENT_OBJECT_MAGIC_VALUE (('i' << 24) | ('N' << 16) | ('a' << 8) | ('v' << 0))

uint32_t persistentObjectRead(persistentObjectId_e id)
{
    return watchdog_hw->scratch[id];
}

void persistentObjectWrite(persistentObjectId_e id, uint32_t value)
{
    watchdog_hw->scratch[id] = value;
}

void persistentObjectInit(void)
{
    bool wasSoftReset = watchdog_caused_reboot();

    if (!wasSoftReset || (persistentObjectRead(PERSISTENT_OBJECT_MAGIC) != PERSISTENT_OBJECT_MAGIC_VALUE)) {
        for (int i = 1; i < PERSISTENT_OBJECT_COUNT; i++) {
            persistentObjectWrite(i, 0);
        }
        persistentObjectWrite(PERSISTENT_OBJECT_MAGIC, PERSISTENT_OBJECT_MAGIC_VALUE);
    }
}
