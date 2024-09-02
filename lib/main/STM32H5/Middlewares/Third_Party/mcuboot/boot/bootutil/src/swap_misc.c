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

#if defined(MCUBOOT_SWAP_USING_SCRATCH) || defined(MCUBOOT_SWAP_USING_MOVE)
int
swap_erase_trailer_sectors(const struct boot_loader_state *state,
                           const struct flash_area *fap)
{
    uint8_t slot;
    uint32_t sector;
    uint32_t trailer_sz;
    uint32_t total_sz;
    uint32_t off;
    uint32_t sz;
    int fa_id_primary;
    int fa_id_secondary;
    uint8_t image_index;
    int rc;

    BOOT_LOG_DBG("erasing trailer; fa_id=%d", fap->fa_id);

    image_index = BOOT_CURR_IMG(state);
    fa_id_primary = flash_area_id_from_multi_image_slot(image_index,
            BOOT_PRIMARY_SLOT);
    fa_id_secondary = flash_area_id_from_multi_image_slot(image_index,
            BOOT_SECONDARY_SLOT);

    if (fap->fa_id == fa_id_primary) {
        slot = BOOT_PRIMARY_SLOT;
    } else if (fap->fa_id == fa_id_secondary) {
        slot = BOOT_SECONDARY_SLOT;
    } else {
        return BOOT_EFLASH;
    }

    /* delete starting from last sector and moving to beginning */
    sector = boot_img_num_sectors(state, slot) - 1;
    trailer_sz = boot_trailer_sz(BOOT_WRITE_SZ(state));
    total_sz = 0;
    do {
        sz = boot_img_sector_size(state, slot, sector);
        off = boot_img_sector_off(state, slot, sector);
        rc = boot_erase_region(fap, off, sz);
        assert(rc == 0);

        sector--;
        total_sz += sz;
    } while (total_sz < trailer_sz);

    return rc;
}

int
swap_status_init(const struct boot_loader_state *state,
                 const struct flash_area *fap,
                 const struct boot_status *bs)
{
    struct boot_swap_state swap_state;
    uint8_t image_index;
    int rc;

#if (BOOT_IMAGE_NUMBER == 1)
    (void)state;
#endif

    image_index = BOOT_CURR_IMG(state);

    BOOT_LOG_DBG("initializing status; fa_id=%d", fap->fa_id);

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(image_index),
            &swap_state);
    (void)rc;
    assert(rc == 0);

    if (bs->swap_type != BOOT_SWAP_TYPE_NONE) {
        rc = boot_write_swap_info(fap, bs->swap_type, image_index);
        assert(rc == 0);
    }

    if (swap_state.image_ok == BOOT_FLAG_SET) {
        rc = boot_write_image_ok(fap);
        assert(rc == 0);
    }

    rc = boot_write_swap_size(fap, bs->swap_size);
    assert(rc == 0);

#ifdef MCUBOOT_ENC_IMAGES
    rc = boot_write_enc_key(fap, 0, bs);
    assert(rc == 0);

    rc = boot_write_enc_key(fap, 1, bs);
    assert(rc == 0);
#endif

    rc = boot_write_magic(fap);
    assert(rc == 0);

    return 0;
}

int
swap_read_status(struct boot_loader_state *state, struct boot_status *bs)
{
    const struct flash_area *fap;
    uint32_t off;
    uint8_t swap_info;
    int area_id;
    int rc;

    bs->source = swap_status_source(state);
    switch (bs->source) {
    case BOOT_STATUS_SOURCE_NONE:
        return 0;

#if MCUBOOT_SWAP_USING_SCRATCH
    case BOOT_STATUS_SOURCE_SCRATCH:
        area_id = FLASH_AREA_IMAGE_SCRATCH;
        break;
#endif

    case BOOT_STATUS_SOURCE_PRIMARY_SLOT:
        area_id = FLASH_AREA_IMAGE_PRIMARY(BOOT_CURR_IMG(state));
        break;

    default:
        assert(0);
        return BOOT_EBADARGS;
    }

    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    rc = swap_read_status_bytes(fap, state, bs);
    if (rc == 0) {
        off = boot_swap_info_off(fap);
        rc = flash_area_read(fap, off, &swap_info, sizeof swap_info);
        if (rc != 0) {
            return BOOT_EFLASH;
        }

#ifdef MCUBOOT_USE_MCE
        if (bootutil_buffer_is_erased(fap->fa_off + off,
                                      fap, &swap_info, sizeof swap_info)) {
#else /* not MCUBOOT_USE_MCE */
        if (bootutil_buffer_is_erased(fap, &swap_info, sizeof swap_info)) {
#endif /* MCUBOOT_USE_MCE */
            BOOT_SET_SWAP_INFO(swap_info, 0, BOOT_SWAP_TYPE_NONE);
            rc = 0;
        }

        /* Extract the swap type info */
        bs->swap_type = BOOT_GET_SWAP_TYPE(swap_info);
    }

    flash_area_close(fap);

    return rc;
}

int
swap_set_copy_done(uint8_t image_index)
{
    const struct flash_area *fap;
    int rc;

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index),
            &fap);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    rc = boot_write_copy_done(fap);
    flash_area_close(fap);
    return rc;
}

int
swap_set_image_ok(uint8_t image_index)
{
    const struct flash_area *fap;
    struct boot_swap_state state;
    int rc;

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index),
            &fap);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    rc = boot_read_swap_state(fap, &state);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto out;
    }

    if (state.image_ok == BOOT_FLAG_UNSET) {
        rc = boot_write_image_ok(fap);
    }

out:
    flash_area_close(fap);
    return rc;
}


#endif /* defined(MCUBOOT_SWAP_USING_SCRATCH) || defined(MCUBOOT_SWAP_USING_MOVE) */
