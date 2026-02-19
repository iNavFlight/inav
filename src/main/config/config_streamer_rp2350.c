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
 * RP2350 flash-based config storage — Milestone 4 (missing piece)
 *
 * INAV config is stored in the FLASH_CONFIG region defined by the linker
 * script (rp2350_flash.ld):
 *   FLASH_CONFIG: 0x103F0000, 64 KB
 *
 * Flash is XIP-mapped starting at XIP_BASE (0x10000000), so the region is
 * directly readable as normal memory.  Writes use the Pico SDK flash APIs:
 *   flash_range_erase()  — erases in 4 KB (FLASH_SECTOR_SIZE) chunks
 *   flash_range_program() — programs in 256-byte (FLASH_PAGE_SIZE) chunks
 *
 * Both APIs take an offset from the start of flash (not the XIP address) and
 * MUST be called with interrupts disabled.  Multicore safety (M12) is deferred;
 * for now disabling interrupts is sufficient because core1 is not started.
 *
 * The config_streamer interface calls config_streamer_impl_write_word() once
 * per CONFIG_STREAMER_BUFFER_SIZE (4 bytes) — smaller than the 256-byte flash
 * page.  We accumulate writes in a static page buffer and flush to flash when
 * the page is full.  The final partial page is flushed in
 * config_streamer_impl_lock(), which is called by config_streamer_finish().
 */

#include <string.h>
#include "platform.h"
#include "config/config_streamer.h"

#ifdef RP2350

#include "hardware/flash.h"
#include "hardware/sync.h"

/* Flash start in the XIP address space */
#define RP2350_XIP_BASE           0x10000000u

/* RP2350 flash erase/program constraints */
#define RP2350_FLASH_PAGE_SIZE    256u   /* minimum program unit */
#define RP2350_FLASH_SECTOR_SIZE  4096u  /* minimum erase unit  */

/* One-page write buffer.  Filled word-by-word; programmed to flash when full.
 * Initialised to 0xFF (erased state) so partial last pages are padded cleanly. */
static uint8_t  pageBuffer[RP2350_FLASH_PAGE_SIZE];
static uint32_t pageBufferBase;   /* flash offset (from XIP_BASE) of pageBuffer[0] */
static uint32_t pageBufferUsed;   /* bytes filled so far in pageBuffer */

/* ── Helper: program the current page buffer to flash ──────────────────────── */

static void flushPageBuffer(void)
{
    if (pageBufferUsed == 0) {
        return;
    }
    /* Pad any unfilled tail with 0xFF (erased value) before programming */
    if (pageBufferUsed < RP2350_FLASH_PAGE_SIZE) {
        memset(pageBuffer + pageBufferUsed, 0xFF,
               RP2350_FLASH_PAGE_SIZE - pageBufferUsed);
    }

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(pageBufferBase, pageBuffer, RP2350_FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    /* Advance to next page and reset buffer */
    pageBufferBase += RP2350_FLASH_PAGE_SIZE;
    memset(pageBuffer, 0xFF, RP2350_FLASH_PAGE_SIZE);
    pageBufferUsed = 0;
}

/* ── config_streamer interface ─────────────────────────────────────────────── */

/*
 * Called once at the start of a save operation.
 * Erases the entire config region so every byte is 0xFF before programming.
 * Interrupts are disabled for the duration of the erase.
 *
 * TODO M12 (multicore): replace with flash_safe_execute() when core1 is active.
 */
void config_streamer_impl_unlock(void)
{
    uint32_t flashOffset = (uint32_t)&__config_start - RP2350_XIP_BASE;
    uint32_t eraseSize   = (uint32_t)&__config_end   - (uint32_t)&__config_start;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(flashOffset, eraseSize);
    restore_interrupts(ints);

    /* Initialise page buffer at the start of the config region */
    memset(pageBuffer, 0xFF, RP2350_FLASH_PAGE_SIZE);
    pageBufferBase = flashOffset;
    pageBufferUsed = 0;
}

/*
 * Called once at the end of a save operation (via config_streamer_finish).
 * Flushes any partial page that has not yet been programmed.
 */
void config_streamer_impl_lock(void)
{
    flushPageBuffer();
}

/*
 * Called once per CONFIG_STREAMER_BUFFER_SIZE (4 bytes) by the generic streamer.
 * Accumulates words into pageBuffer; programs a flash page when it fills up.
 *
 * c->address is the XIP-mapped flash address of the word being written.
 */
int config_streamer_impl_write_word(config_streamer_t *c,
                                    config_streamer_buffer_align_type_t *buffer)
{
    if (c->err != 0) {
        return c->err;
    }

    uint32_t flashOffset  = (uint32_t)c->address - RP2350_XIP_BASE;
    uint32_t offsetInPage = flashOffset - pageBufferBase;

    memcpy(pageBuffer + offsetInPage, buffer, CONFIG_STREAMER_BUFFER_SIZE);
    pageBufferUsed = offsetInPage + CONFIG_STREAMER_BUFFER_SIZE;

    /* Program when we have a complete page */
    if (pageBufferUsed == RP2350_FLASH_PAGE_SIZE) {
        uint32_t ints = save_and_disable_interrupts();
        flash_range_program(pageBufferBase, pageBuffer, RP2350_FLASH_PAGE_SIZE);
        restore_interrupts(ints);

        pageBufferBase += RP2350_FLASH_PAGE_SIZE;
        memset(pageBuffer, 0xFF, RP2350_FLASH_PAGE_SIZE);
        pageBufferUsed = 0;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

#endif /* RP2350 */
