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
#include <dirent.h>
#include "platform.h"
#include "drivers/system.h"
#include "config/config_streamer.h"
#include "common/utils.h"

#if defined(CONFIG_IN_FILE)

#include <stdio.h>
#include <errno.h>


#define FLASH_PAGE_SIZE  (0x400)

static FILE *eepromFd = NULL;
static bool streamerLocked = true;
static char eepromPath[260] = EEPROM_FILENAME;

bool configFileSetPath(char* path)
{
    if(!path || strlen(path) > 260) {
        return false;
    }
    
    strcpy(eepromPath, path);
    return true;
}

void config_streamer_impl_unlock(void)
{
    if (eepromFd != NULL) {
        fprintf(stderr, "[EEPROM] Unable to load %s\n", eepromPath);
        return;
    }

    // open or create
    eepromFd = fopen(eepromPath,"r+");
    if (eepromFd != NULL) {
        // obtain file size:
        fseek(eepromFd , 0 , SEEK_END);
        size_t size = ftell(eepromFd);
        rewind(eepromFd);

        size_t n = fread(eepromData, 1, sizeof(eepromData), eepromFd);
        if (n == size) {
            fprintf(stderr,"[EEPROM] Loaded '%s' (%ld of %ld bytes)\n", eepromPath, size, sizeof(eepromData));
            streamerLocked = false;
        } else {
            fprintf(stderr, "[EEPROM] Failed to load '%s'\n", eepromPath);
        }
    } else {
        printf("[EEPROM] Created '%s', size = %ld\n", eepromPath, sizeof(eepromData));
        streamerLocked = false;
        if ((eepromFd = fopen(eepromPath, "w+")) == NULL) {
            fprintf(stderr, "[EEPROM] Failed to create '%s'\n", eepromPath);
            streamerLocked = true;
        }
        if (fwrite(eepromData, sizeof(eepromData), 1, eepromFd) != 1) {
            fprintf(stderr, "[EEPROM] Write failed: %s\n", strerror(errno));
            streamerLocked = true;
        }
    }
}

void config_streamer_impl_lock(void)
{
    // flush & close
    if (eepromFd != NULL) {
        fseek(eepromFd, 0, SEEK_SET);
        fwrite(eepromData, 1, sizeof(eepromData), eepromFd);
        fclose(eepromFd);
        eepromFd = NULL;
        fprintf(stderr, "[EEPROM] Saved '%s'\n", eepromPath);
        streamerLocked = false;
    } else {
        fprintf(stderr, "[EEPROM] Unlock error\n");
    }
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (streamerLocked) {
        return -1;
    }

    if ((c->address >= (uintptr_t)eepromData) && (c->address < (uintptr_t)ARRAYEND(eepromData))) {
        *((uint32_t*)c->address) = *buffer;
        fprintf(stderr, "[EEPROM] Program word  %p = %08x\n", (void*)c->address, *((uint32_t*)c->address));
    } else {
        fprintf(stderr, "[EEPROM] Program word %p out of range!\n", (void*)c->address);
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

#endif
