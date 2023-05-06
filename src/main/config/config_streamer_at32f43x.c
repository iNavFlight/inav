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

#if defined(AT32F43x) && !defined(CONFIG_IN_RAM) && !defined(CONFIG_IN_EXTERNAL_FLASH)


#if defined(AT32F437ZMT7) || defined(AT32F437VMT7) || defined(AT32F435RMT7)
    #define FLASH_PAGE_SIZE ((uint32_t)0x1000)  
#elif defined(AT32F437ZGT7) || defined(AT32F437VGT7) || defined(AT32F435RGT7)
    #define FLASH_PAGE_SIZE ((uint32_t)0x800)  
#endif

void config_streamer_impl_unlock(void)
{
    flash_unlock();
    flash_flag_clear(FLASH_ODF_FLAG|FLASH_PRGMERR_FLAG|FLASH_EPPERR_FLAG);
}

void config_streamer_impl_lock(void)
{
    flash_lock();
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (c->err != 0) {
        return c->err;
    }
    // Erases sectors from the start address
    if (c->address % FLASH_PAGE_SIZE == 0) {
        const flash_status_type status =flash_sector_erase(c->address);
		   if (status != FLASH_OPERATE_DONE) {
			   return -1;
		   }
	   }
    
    const flash_status_type status = flash_word_program(c->address, *buffer);
    if (status != FLASH_OPERATE_DONE) {
        return -2;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

#endif
