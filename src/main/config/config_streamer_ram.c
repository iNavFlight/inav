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

#include <string.h>
#include "platform.h"
#include "drivers/system.h"
#include "config/config_streamer.h"

#if defined(CONFIG_IN_RAM)

static bool streamerLocked = true;

void config_streamer_impl_unlock(void)
{
    streamerLocked = false;
}

void config_streamer_impl_lock(void)
{
    streamerLocked = true;
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (streamerLocked) {
        return -1;
    }

    if (c->address == (uintptr_t)&eepromData[0]) {
        memset(eepromData, 0, sizeof(eepromData));
    }

    config_streamer_buffer_align_type_t *destAddr = (config_streamer_buffer_align_type_t *)c->address;
    config_streamer_buffer_align_type_t *srcAddr = buffer;

    uint8_t nRows = CONFIG_STREAMER_BUFFER_SIZE / sizeof(config_streamer_buffer_align_type_t);

    do {
        *destAddr++ = *srcAddr++;
    } while(--nRows != 0);

    c->address += CONFIG_STREAMER_BUFFER_SIZE;

    return 0;
}

#endif
