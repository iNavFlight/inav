/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "config/persistent_flags.h"

PG_REGISTER(persistentFlags_t, persistentFlags, PG_PERSISTENT_FLAGS_CONFIG, 0);

void persistentFlagClearAll()
{
    persistentFlags.persistentFlags = 0;
}

bool persistentFlag(uint8_t mask)
{
    return persistentFlags.persistentFlags & mask;
}

void persistentFlagSet(uint8_t mask)
{
    persistentFlags.persistentFlags |= mask;
}

void persistentFlagClear(uint8_t mask)
{
    persistentFlags.persistentFlags &= ~(mask);
}
