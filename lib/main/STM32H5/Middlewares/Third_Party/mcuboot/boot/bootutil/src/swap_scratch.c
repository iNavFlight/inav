/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2019 JUUL Labs
 * Copyright (c) 2023 STMicroelectronics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "bootutil/bootutil.h"
#include "bootutil_priv.h"
#include "swap_priv.h"
#include "bootutil/bootutil_log.h"

#include "mcuboot_config/mcuboot_config.h"

MCUBOOT_LOG_MODULE_DECLARE(mcuboot);

#if !defined(MCUBOOT_SWAP_USING_MOVE)

#if defined(MCUBOOT_VALIDATE_PRIMARY_SLOT)
/*
 * FIXME: this might have to be updated for threaded sim
 */
int boot_status_fails = 0;
#define BOOT_STATUS_ASSERT(x)                \
    do {                                     \
        if (!(x)) {                          \
            boot_status_fails++;             \
        }                                    \
    } while (0)
#else
#define BOOT_STATUS_ASSERT(x) ASSERT(x)
#endif

int
boot_read_image_header(struct boot_loader_state *state, int slot,
                       struct image_header *out_hdr, struct boot_status *bs)
{
    const struct flash_area *fap;
    int area_id;
    int rc;

    (void)bs;

#if (BOOT_IMAGE_NUMBER == 1)
    (void)state;
#endif

    area_id = flash_area_id_from_multi_image_slot(BOOT_CURR_IMG(state), slot);
    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = flash_area_read(fap, 0, out_hdr, sizeof *out_hdr);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}
#if !defined(MCUBOOT_DIRECT_XIP) && !defined(MCUBOOT_RAM_LOAD) && !defined(MCUBOOT_PRIMARY_ONLY)
int
swap_read_status_bytes(const struct flash_area *fap,
        struct boot_loader_state *state, struct boot_status *bs);
/**
 * Reads the status of a partially-completed swap, if any.  This is necessary
 * to recover in case the boot lodaer was reset in the middle of a swap
 * operation.
 */
int
swap_read_status_bytes(const struct flash_area *fap,
        struct boot_loader_state *state, struct boot_status *bs)
{
    uint32_t off;
    uint8_t status;
    int max_entries;
    int found;
    int found_idx;
    int invalid;
    int rc;
    int i;

    off = boot_status_off(fap);
    max_entries = boot_status_entries(BOOT_CURR_IMG(state), fap);
    if (max_entries < 0) {
        return BOOT_EBADARGS;
    }

    found = 0;
    found_idx = 0;
    invalid = 0;
    for (i = 0; i < max_entries; i++) {
        rc = flash_area_read(fap, off + i * BOOT_WRITE_SZ(state),
                &status, 1);
        if (rc < 0) {
            return BOOT_EFLASH;
        }

#ifdef MCUBOOT_USE_MCE
        if (bootutil_buffer_is_erased(fap->fa_off + off + i * BOOT_WRITE_SZ(state),
                                      fap, &status, 1)) {
#else /* not MCUBOOT_USE_MCE */
        if (bootutil_buffer_is_erased(fap, &status, 1)) {
#endif /* MCUBOOT_USE_MCE */
            if (found && !found_idx) {
                found_idx = i;
            }
        } else if (!found) {
            found = 1;
        } else if (found_idx) {
            invalid = 1;
            break;
        }
    }

    if (invalid) {
        /* This means there was an error writing status on the last
         * swap. Tell user and move on to validation!
         */
#if !defined(__BOOTSIM__)
        BOOT_LOG_ERR("Detected inconsistent status!");
#endif

#if !defined(MCUBOOT_VALIDATE_PRIMARY_SLOT)
        /* With validation of the primary slot disabled, there is no way
         * to be sure the swapped primary slot is OK, so abort!
         */
        assert(0);
#endif
    }

    if (found) {
        if (!found_idx) {
            found_idx = i;
        }
        bs->idx = (found_idx / BOOT_STATUS_STATE_COUNT) + 1;
        bs->state = (found_idx % BOOT_STATUS_STATE_COUNT) + 1;
    }

    return 0;
}

uint32_t
boot_status_internal_off(const struct boot_status *bs, int elem_sz)
{
    int idx_sz;

    idx_sz = elem_sz * BOOT_STATUS_STATE_COUNT;

    return (bs->idx - BOOT_STATUS_IDX_0) * idx_sz +
           (bs->state - BOOT_STATUS_STATE_0) * elem_sz;
}

/*
 * Slots are compatible when all sectors that store up to to size of the image
 * round up to sector size, in both slot's are able to fit in the scratch
 * area, and have sizes that are a multiple of each other (powers of two
 * presumably!).
 */
int
boot_slots_compatible(struct boot_loader_state *state)
{
    size_t num_sectors_primary;
    size_t num_sectors_secondary;
    size_t sz0, sz1;
    size_t primary_slot_sz, secondary_slot_sz;
#ifndef MCUBOOT_OVERWRITE_ONLY
    size_t scratch_sz;
#endif
    size_t i, j;
    int8_t smaller;

    num_sectors_primary = boot_img_num_sectors(state, BOOT_PRIMARY_SLOT);
    num_sectors_secondary = boot_img_num_sectors(state, BOOT_SECONDARY_SLOT);
    if ((num_sectors_primary > BOOT_MAX_IMG_SECTORS) ||
        (num_sectors_secondary > BOOT_MAX_IMG_SECTORS)) {
        BOOT_LOG_WRN("Cannot upgrade: more sectors than allowed");
        return 0;
    }

#ifndef MCUBOOT_OVERWRITE_ONLY
    scratch_sz = boot_scratch_area_size(state);
#endif

    /*
     * The following loop scans all sectors in a linear fashion, assuring that
     * for each possible sector in each slot, it is able to fit in the other
     * slot's sector or sectors. Slot's should be compatible as long as any
     * number of a slot's sectors are able to fit into another, which only
     * excludes cases where sector sizes are not a multiple of each other.
     */
    i = sz0 = primary_slot_sz = 0;
    j = sz1 = secondary_slot_sz = 0;
    smaller = 0;
    while (i < num_sectors_primary || j < num_sectors_secondary) {
        if (sz0 == sz1) {
            sz0 += boot_img_sector_size(state, BOOT_PRIMARY_SLOT, i);
            sz1 += boot_img_sector_size(state, BOOT_SECONDARY_SLOT, j);
            i++;
            j++;
        } else if (sz0 < sz1) {
            sz0 += boot_img_sector_size(state, BOOT_PRIMARY_SLOT, i);
            /* Guarantee that multiple sectors of the secondary slot
             * fit into the primary slot.
             */
            if (smaller == 2) {
                BOOT_LOG_WRN("Cannot upgrade: slots have non-compatible sectors");
                return 0;
            }
            smaller = 1;
            i++;
        } else {
            sz1 += boot_img_sector_size(state, BOOT_SECONDARY_SLOT, j);
            /* Guarantee that multiple sectors of the primary slot
             * fit into the secondary slot.
             */
            if (smaller == 1) {
                BOOT_LOG_WRN("Cannot upgrade: slots have non-compatible sectors");
                return 0;
            }
            smaller = 2;
            j++;
        }
#ifndef MCUBOOT_OVERWRITE_ONLY
        if (sz0 == sz1) {
            primary_slot_sz += sz0;
            secondary_slot_sz += sz1;
            /* Scratch has to fit each swap operation to the size of the larger
             * sector among the primary slot and the secondary slot.
             */
            if (sz0 > scratch_sz || sz1 > scratch_sz) {
                BOOT_LOG_WRN("Cannot upgrade: not all sectors fit inside scratch");
                return 0;
            }
            smaller = sz0 = sz1 = 0;
        }
#endif
    }

    if ((i != num_sectors_primary) ||
        (j != num_sectors_secondary) ||
        (primary_slot_sz != secondary_slot_sz)) {
        BOOT_LOG_WRN("Cannot upgrade: slots are not compatible");
        return 0;
    }

    return 1;
}

#define BOOT_LOG_SWAP_STATE(area, state)                            \
    BOOT_LOG_INF("%s: magic=%s, swap_type=0x%x, copy_done=0x%x, "   \
                 "image_ok=0x%x",                                   \
                 (area),                                            \
                 ((state)->magic == BOOT_MAGIC_GOOD ? "good" :      \
                  (state)->magic == BOOT_MAGIC_UNSET ? "unset" :    \
                  "bad"),                                           \
                 (state)->swap_type,                                \
                 (state)->copy_done,                                \
                 (state)->image_ok)

struct boot_status_table {
    uint8_t bst_magic_primary_slot;
    uint8_t bst_magic_scratch;
    uint8_t bst_copy_done_primary_slot;
    uint8_t bst_status_source;
};

/**
 * This set of tables maps swap state contents to boot status location.
 * When searching for a match, these tables must be iterated in order.
 */
static const struct boot_status_table boot_status_tables[] = {
    {
        /*           | primary slot | scratch      |
         * ----------+--------------+--------------|
         *     magic | Good         | Any          |
         * copy-done | Set          | N/A          |
         * ----------+--------------+--------------'
         * source: none                            |
         * ----------------------------------------'
         */
        .bst_magic_primary_slot =     BOOT_MAGIC_GOOD,
        .bst_magic_scratch =          BOOT_MAGIC_NOTGOOD,
        .bst_copy_done_primary_slot = BOOT_FLAG_SET,
        .bst_status_source =          BOOT_STATUS_SOURCE_NONE,
    },

    {
        /*           | primary slot | scratch      |
         * ----------+--------------+--------------|
         *     magic | Good         | Any          |
         * copy-done | Unset        | N/A          |
         * ----------+--------------+--------------'
         * source: primary slot                    |
         * ----------------------------------------'
         */
        .bst_magic_primary_slot =     BOOT_MAGIC_GOOD,
        .bst_magic_scratch =          BOOT_MAGIC_NOTGOOD,
        .bst_copy_done_primary_slot = BOOT_FLAG_UNSET,
        .bst_status_source =          BOOT_STATUS_SOURCE_PRIMARY_SLOT,
    },

    {
        /*           | primary slot | scratch      |
         * ----------+--------------+--------------|
         *     magic | Any          | Good         |
         * copy-done | Any          | N/A          |
         * ----------+--------------+--------------'
         * source: scratch                         |
         * ----------------------------------------'
         */
        .bst_magic_primary_slot =     BOOT_MAGIC_ANY,
        .bst_magic_scratch =          BOOT_MAGIC_GOOD,
        .bst_copy_done_primary_slot = BOOT_FLAG_ANY,
        .bst_status_source =          BOOT_STATUS_SOURCE_SCRATCH,
    },
    {
        /*           | primary slot | scratch      |
         * ----------+--------------+--------------|
         *     magic | Unset        | Any          |
         * copy-done | Unset        | N/A          |
         * ----------+--------------+--------------|
         * source: varies                          |
         * ----------------------------------------+--------------------------+
         * This represents one of two cases:                                  |
         * o No swaps ever (no status to read, so no harm in checking).       |
         * o Mid-revert; status in primary slot.                              |
         * -------------------------------------------------------------------'
         */
        .bst_magic_primary_slot =     BOOT_MAGIC_UNSET,
        .bst_magic_scratch =          BOOT_MAGIC_ANY,
        .bst_copy_done_primary_slot = BOOT_FLAG_UNSET,
        .bst_status_source =          BOOT_STATUS_SOURCE_PRIMARY_SLOT,
    },
};

#define BOOT_STATUS_TABLES_COUNT \
    (sizeof boot_status_tables / sizeof boot_status_tables[0])
int
swap_status_source(struct boot_loader_state *state);
/**
 * Determines where in flash the most recent boot status is stored. The boot
 * status is necessary for completing a swap that was interrupted by a boot
 * loader reset.
 *
 * @return      A BOOT_STATUS_SOURCE_[...] code indicating where status should
 *              be read from.
 */
int
swap_status_source(struct boot_loader_state *state)
{
    const struct boot_status_table *table;
    struct boot_swap_state state_scratch;
    struct boot_swap_state state_primary_slot;
    int rc;
    size_t i;
    uint8_t source;
    uint8_t image_index;

#if (BOOT_IMAGE_NUMBER == 1)
    (void)state;
#endif

    image_index = BOOT_CURR_IMG(state);
    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(image_index),
            &state_primary_slot);
    (void)rc;
    assert(rc == 0);

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SCRATCH, &state_scratch);
    assert(rc == 0);

    BOOT_LOG_SWAP_STATE("Primary image", &state_primary_slot);
    BOOT_LOG_SWAP_STATE("Scratch", &state_scratch);

    for (i = 0; i < BOOT_STATUS_TABLES_COUNT; i++) {
        table = &boot_status_tables[i];

        if (boot_magic_compatible_check(table->bst_magic_primary_slot,
                          state_primary_slot.magic) &&
            boot_magic_compatible_check(table->bst_magic_scratch,
                          state_scratch.magic) &&
            (table->bst_copy_done_primary_slot == BOOT_FLAG_ANY ||
             table->bst_copy_done_primary_slot == state_primary_slot.copy_done))
        {
            source = table->bst_status_source;

#if (BOOT_IMAGE_NUMBER > 1)
            /* In case of multi-image boot it can happen that if boot status
             * info is found on scratch area then it does not belong to the
             * currently examined image.
             */
            if (source == BOOT_STATUS_SOURCE_SCRATCH &&
                state_scratch.image_num != BOOT_CURR_IMG(state)) {
                source = BOOT_STATUS_SOURCE_NONE;
            }
#endif

            BOOT_LOG_INF("Boot source: %s",
                         source == BOOT_STATUS_SOURCE_NONE ? "none" :
                         source == BOOT_STATUS_SOURCE_SCRATCH ? "scratch" :
                         source == BOOT_STATUS_SOURCE_PRIMARY_SLOT ?
                                   "primary slot" : "BUG; can't happen");
            return source;
        }
    }

    BOOT_LOG_INF("Boot source: none");
    return BOOT_STATUS_SOURCE_NONE;
}

#ifndef MCUBOOT_OVERWRITE_ONLY
/**
 * Calculates the number of sectors the scratch area can contain.  A "last"
 * source sector is specified because images are copied backwards in flash
 * (final index to index number 0).
 *
 * @param last_sector_idx       The index of the last source sector
 *                                  (inclusive).
 * @param out_first_sector_idx  The index of the first source sector
 *                                  (inclusive) gets written here.
 *
 * @return                      The number of bytes comprised by the
 *                                  [first-sector, last-sector] range.
 */
static uint32_t
boot_copy_sz(const struct boot_loader_state *state, int last_sector_idx,
             int *out_first_sector_idx)
{
    size_t scratch_sz;
    uint32_t new_sz;
    uint32_t sz;
    int i;

    sz = 0;

    scratch_sz = boot_scratch_area_size(state);
    for (i = last_sector_idx; i >= 0; i--) {
        new_sz = sz + boot_img_sector_size(state, BOOT_PRIMARY_SLOT, i);
        /*
         * The secondary slot is not being checked here, because
         * `boot_slots_compatible` already provides assurance that the copy size
         * will be compatible with the primary slot and scratch.
         */
        if (new_sz > scratch_sz) {
            break;
        }
        sz = new_sz;
    }

    /* i currently refers to a sector that doesn't fit or it is -1 because all
     * sectors have been processed.  In both cases, exclude sector i.
     */
    *out_first_sector_idx = i + 1;
    return sz;
}

/**
 * Swaps the contents of two flash regions within the two image slots.
 *
 * @param idx                   The index of the first sector in the range of
 *                                  sectors being swapped.
 * @param sz                    The number of bytes to swap.
 * @param bs                    The current boot status.  This struct gets
 *                                  updated according to the outcome.
 *
 * @return                      0 on success; nonzero on failure.
 */
static void
boot_swap_sectors(int idx, uint32_t sz, struct boot_loader_state *state,
        struct boot_status *bs)
{
    const struct flash_area *fap_primary_slot;
    const struct flash_area *fap_secondary_slot;
    const struct flash_area *fap_scratch;
    uint32_t copy_sz;
    uint32_t trailer_sz;
    uint32_t img_off;
    uint32_t scratch_trailer_off;
    struct boot_swap_state swap_state;
    size_t last_sector;
    bool erase_scratch;
    uint8_t image_index;
    int rc;

    /* Calculate offset from start of image area. */
    img_off = boot_img_sector_off(state, BOOT_PRIMARY_SLOT, idx);

    copy_sz = sz;
    trailer_sz = boot_trailer_sz(BOOT_WRITE_SZ(state));

    /* sz in this function is always sized on a multiple of the sector size.
     * The check against the start offset of the last sector
     * is to determine if we're swapping the last sector. The last sector
     * needs special handling because it's where the trailer lives. If we're
     * copying it, we need to use scratch to write the trailer temporarily.
     *
     * NOTE: `use_scratch` is a temporary flag (never written to flash) which
     * controls if special handling is needed (swapping last sector).
     */
    last_sector = boot_img_num_sectors(state, BOOT_PRIMARY_SLOT) - 1;
    if ((img_off + sz) >
        boot_img_sector_off(state, BOOT_PRIMARY_SLOT, last_sector)) {
        copy_sz -= trailer_sz;
    }

    bs->use_scratch = (bs->idx == BOOT_STATUS_IDX_0 && copy_sz != sz);

    image_index = BOOT_CURR_IMG(state);

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index),
            &fap_primary_slot);
    assert (rc == 0);

    rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(image_index),
            &fap_secondary_slot);
    assert (rc == 0);

    rc = flash_area_open(FLASH_AREA_IMAGE_SCRATCH, &fap_scratch);
    assert (rc == 0);

    if (bs->state == BOOT_STATUS_STATE_0) {
        BOOT_LOG_DBG("erasing scratch area");
        rc = boot_erase_region(fap_scratch, 0, fap_scratch->fa_size);
        assert(rc == 0);

        if (bs->idx == BOOT_STATUS_IDX_0) {
            /* Write a trailer to the scratch area, even if we don't need the
             * scratch area for status.  We need a temporary place to store the
             * `swap-type` while we erase the primary trailer.
             */
            rc = swap_status_init(state, fap_scratch, bs);
            assert(rc == 0);

            if (!bs->use_scratch) {
                /* Prepare the primary status area... here it is known that the
                 * last sector is not being used by the image data so it's safe
                 * to erase.
                 */
                rc = swap_erase_trailer_sectors(state, fap_primary_slot);
                assert(rc == 0);

                rc = swap_status_init(state, fap_primary_slot, bs);
                assert(rc == 0);

                /* Erase the temporary trailer from the scratch area. */
                rc = boot_erase_region(fap_scratch, 0, fap_scratch->fa_size);
                assert(rc == 0);
            }
        }

        rc = boot_copy_region(state, fap_secondary_slot, fap_scratch,
                              img_off, 0, copy_sz);
        assert(rc == 0);

        rc = boot_write_status(state, bs);
        bs->state = BOOT_STATUS_STATE_1;
        BOOT_STATUS_ASSERT(rc == 0);
    }

    if (bs->state == BOOT_STATUS_STATE_1) {
        rc = boot_erase_region(fap_secondary_slot, img_off, sz);
        assert(rc == 0);

        rc = boot_copy_region(state, fap_primary_slot, fap_secondary_slot,
                              img_off, img_off, copy_sz);
        assert(rc == 0);

        if (bs->idx == BOOT_STATUS_IDX_0 && !bs->use_scratch) {
            /* If not all sectors of the slot are being swapped,
             * guarantee here that only the primary slot will have the state.
             */
            rc = swap_erase_trailer_sectors(state, fap_secondary_slot);
            assert(rc == 0);
        }

        rc = boot_write_status(state, bs);
        bs->state = BOOT_STATUS_STATE_2;
        BOOT_STATUS_ASSERT(rc == 0);
    }

    if (bs->state == BOOT_STATUS_STATE_2) {
        rc = boot_erase_region(fap_primary_slot, img_off, sz);
        assert(rc == 0);

        /* NOTE: If this is the final sector, we exclude the image trailer from
         * this copy (copy_sz was truncated earlier).
         */
        rc = boot_copy_region(state, fap_scratch, fap_primary_slot,
                              0, img_off, copy_sz);
        assert(rc == 0);

        if (bs->use_scratch) {
            scratch_trailer_off = boot_status_off(fap_scratch);

            /* copy current status that is being maintained in scratch */
            rc = boot_copy_region(state, fap_scratch, fap_primary_slot,
                        scratch_trailer_off, img_off + copy_sz,
                        (BOOT_STATUS_STATE_COUNT - 1) * BOOT_WRITE_SZ(state));
            BOOT_STATUS_ASSERT(rc == 0);

            rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SCRATCH,
                                            &swap_state);
            assert(rc == 0);

            if (swap_state.image_ok == BOOT_FLAG_SET) {
                rc = boot_write_image_ok(fap_primary_slot);
                assert(rc == 0);
            }

            if (swap_state.swap_type != BOOT_SWAP_TYPE_NONE) {
                rc = boot_write_swap_info(fap_primary_slot,
                        swap_state.swap_type, image_index);
                assert(rc == 0);
            }

            rc = boot_write_swap_size(fap_primary_slot, bs->swap_size);
            assert(rc == 0);

#ifdef MCUBOOT_ENC_IMAGES
            rc = boot_write_enc_key(fap_primary_slot, 0, bs);
            assert(rc == 0);

            rc = boot_write_enc_key(fap_primary_slot, 1, bs);
            assert(rc == 0);
#endif
            rc = boot_write_magic(fap_primary_slot);
            assert(rc == 0);
        }

        /* If we wrote a trailer to the scratch area, erase it after we persist
         * a trailer to the primary slot.  We do this to prevent mcuboot from
         * reading a stale status from the scratch area in case of immediate
         * reset.
         */
        erase_scratch = bs->use_scratch;
        bs->use_scratch = 0;

        rc = boot_write_status(state, bs);
        bs->idx++;
        bs->state = BOOT_STATUS_STATE_0;
        BOOT_STATUS_ASSERT(rc == 0);

        if (erase_scratch) {
            rc = boot_erase_region(fap_scratch,
                    state->scratch.sectors[state->scratch.num_sectors - 1].fs_off - state->scratch.sectors[0].fs_off,
                    state->scratch.sectors[state->scratch.num_sectors - 1].fs_size);
            assert(rc == 0);
        }
    }

    flash_area_close(fap_primary_slot);
    flash_area_close(fap_secondary_slot);
    flash_area_close(fap_scratch);
}

void
swap_run(struct boot_loader_state *state, struct boot_status *bs,
         uint32_t copy_size)
{
    uint32_t sz;
    int first_sector_idx;
    int last_sector_idx;
    uint32_t swap_idx;
    int last_idx_secondary_slot;
    uint32_t primary_slot_size;
    uint32_t secondary_slot_size;
    primary_slot_size = 0;
    secondary_slot_size = 0;
    last_sector_idx = 0;
    last_idx_secondary_slot = 0;

    /*
     * Knowing the size of the largest image between both slots, here we
     * find what is the last sector in the primary slot that needs swapping.
     * Since we already know that both slots are compatible, the secondary
     * slot's last sector is not really required after this check is finished.
     */
    while (1) {
        if ((primary_slot_size < copy_size) ||
            (primary_slot_size < secondary_slot_size)) {
           primary_slot_size += boot_img_sector_size(state,
                                                     BOOT_PRIMARY_SLOT,
                                                     last_sector_idx);
        }
        if ((secondary_slot_size < copy_size) ||
            (secondary_slot_size < primary_slot_size)) {
           secondary_slot_size += boot_img_sector_size(state,
                                                       BOOT_SECONDARY_SLOT,
                                                       last_idx_secondary_slot);
        }
        if (primary_slot_size >= copy_size &&
                secondary_slot_size >= copy_size &&
                primary_slot_size == secondary_slot_size) {
            break;
        }
        last_sector_idx++;
        last_idx_secondary_slot++;
    }

#if defined(__ICCARM__)
    BOOT_LOG_INF("Swapping secondary and primary slots: 0x%x bytes", copy_size);
#else /* __ICCARM__ */
    BOOT_LOG_INF("Swapping secondary and primary slots: 0x%lx bytes", (long unsigned int)copy_size);
#endif /* __ICCARM__ */

    swap_idx = 0;
    while (last_sector_idx >= 0) {
        sz = boot_copy_sz(state, last_sector_idx, &first_sector_idx);
        if (swap_idx >= (bs->idx - BOOT_STATUS_IDX_0)) {
#if defined(__ICCARM__)
            BOOT_LOG_INF("Swapping: swap index 0x%x, sector index 0x%x, size 0x%x",
                         swap_idx, first_sector_idx, sz);
#else /* __ICCARM__ */
            BOOT_LOG_INF("Swapping: swap index 0x%lx, sector index 0x%lx, size 0x%lx",
                         (long unsigned int)swap_idx, (long unsigned int)first_sector_idx,
                         (long unsigned int)sz);
#endif /* __ICCARM__ */
            boot_swap_sectors(first_sector_idx, sz, state, bs);
        }

        last_sector_idx = first_sector_idx - 1;
        swap_idx++;
    }

}
#endif /* !MCUBOOT_OVERWRITE_ONLY */

#endif /* !MCUBOOT_DIRECT_XIP && !MCUBOOT_RAM_LOAD && !defined(MCUBOOT_PRIMARY_ONLY) */

#endif /* !MCUBOOT_SWAP_USING_MOVE */
