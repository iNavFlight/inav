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
 * Register mapping (2 of the 8 watchdog scratch registers are used):
 *   scratch[PERSISTENT_OBJECT_MAGIC]        — magic value ("iNav")
 *   scratch[PERSISTENT_OBJECT_RESET_REASON] — bootloader/MSC request code
 *
 * NOTE: scratch[4..7] are used by the SDK itself (watchdog_reboot() stores
 * the reboot address; watchdog_enable() stores WATCHDOG_NON_REBOOT_MAGIC in
 * scratch[4] to distinguish a timeout crash from a deliberate reboot), so
 * new persistent objects must stay within scratch[0..3].  PERSISTENT_OBJECT_COUNT
 * must therefore never exceed 4 for objects stored here (the enum below
 * currently defines only MAGIC=0 and RESET_REASON=1).
 */

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#include "drivers/persistent.h"

#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#define PERSISTENT_OBJECT_MAGIC_VALUE (('i' << 24) | ('N' << 16) | ('a' << 8) | ('v' << 0))

// Keep the object-id enum within the SDK-unused scratch[0..3] range.
_Static_assert(PERSISTENT_OBJECT_COUNT <= 4, "RP2350 persistent objects must fit in watchdog scratch[0..3]");

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
    // Mirror STM32 persistent.c semantics: only a *deliberate* software reset
    // (systemReset() → watchdog_reboot()) preserves the stored reset reason;
    // a genuine watchdog timeout crash must not leave a stale bootloader/MSC
    // request that could hijack the next boot.  watchdog_caused_reboot() alone
    // is true for both cases; watchdog_enable_caused_reboot() is true only for
    // an armed-timeout crash (watchdog_enable() sets scratch[4] to
    // WATCHDOG_NON_REBOOT_MAGIC, while watchdog_reboot() leaves it cleared).
    bool wasSoftReset = watchdog_caused_reboot() && !watchdog_enable_caused_reboot();

    if (!wasSoftReset || (persistentObjectRead(PERSISTENT_OBJECT_MAGIC) != PERSISTENT_OBJECT_MAGIC_VALUE)) {
        for (int i = 1; i < PERSISTENT_OBJECT_COUNT; i++) {
            persistentObjectWrite(i, 0);
        }
        persistentObjectWrite(PERSISTENT_OBJECT_MAGIC, PERSISTENT_OBJECT_MAGIC_VALUE);
    }
}
