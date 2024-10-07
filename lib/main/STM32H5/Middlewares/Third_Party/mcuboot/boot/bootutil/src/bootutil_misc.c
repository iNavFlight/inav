/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017-2019 Linaro LTD
 * Copyright (c) 2016-2019 JUUL Labs
 * Copyright (c) 2019-2020 Arm Limited
 * Copyright (c) 2023 STMicroelectronics
 *
 * Original license:
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <string.h>
#include <inttypes.h>
#include <stddef.h>

#include "sysflash/sysflash.h"
#include "flash_map_backend/flash_map_backend.h"

#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil_priv.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/fault_injection_hardening.h"
#ifdef MCUBOOT_ENC_IMAGES
#include "bootutil/enc_key.h"
#endif
#ifdef MCUBOOT_USE_MCE
#include "boot_hal_mce.h"
#endif /* MCUBOOT_USE_MCE */

MCUBOOT_LOG_MODULE_DECLARE(mcuboot);
#if !defined(MCUBOOT_PRIMARY_ONLY)
/* Currently only used by imgmgr */
int boot_current_slot;

const uint32_t boot_img_magic[] = {
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
};

#define BOOT_MAGIC_ARR_SZ \
    (sizeof boot_img_magic / sizeof boot_img_magic[0])

struct boot_swap_table {
    uint8_t magic_primary_slot;
    uint8_t magic_secondary_slot;
    uint8_t image_ok_primary_slot;
    uint8_t image_ok_secondary_slot;
    uint8_t copy_done_primary_slot;

    uint8_t swap_type;
};

/**
 * This set of tables maps image trailer contents to swap operation type.
 * When searching for a match, these tables must be iterated sequentially.
 *
 * NOTE: the table order is very important. The settings in the secondary
 * slot always are priority to the primary slot and should be located
 * earlier in the table.
 *
 * The table lists only states where there is action needs to be taken by
 * the bootloader, as in starting/finishing a swap operation.
 */
static const struct boot_swap_table boot_swap_tables[] = {
    {
        .magic_primary_slot =       BOOT_MAGIC_ANY,
        .magic_secondary_slot =     BOOT_MAGIC_GOOD,
        .image_ok_primary_slot =    BOOT_FLAG_ANY,
        .image_ok_secondary_slot =  BOOT_FLAG_UNSET,
        .copy_done_primary_slot =   BOOT_FLAG_ANY,
        .swap_type =                BOOT_SWAP_TYPE_TEST,
    },
    {
        .magic_primary_slot =       BOOT_MAGIC_ANY,
        .magic_secondary_slot =     BOOT_MAGIC_GOOD,
        .image_ok_primary_slot =    BOOT_FLAG_ANY,
        .image_ok_secondary_slot =  BOOT_FLAG_SET,
        .copy_done_primary_slot =   BOOT_FLAG_ANY,
        .swap_type =                BOOT_SWAP_TYPE_PERM,
    },
    {
        .magic_primary_slot =       BOOT_MAGIC_GOOD,
        .magic_secondary_slot =     BOOT_MAGIC_UNSET,
        .image_ok_primary_slot =    BOOT_FLAG_UNSET,
        .image_ok_secondary_slot =  BOOT_FLAG_ANY,
        .copy_done_primary_slot =   BOOT_FLAG_SET,
        .swap_type =                BOOT_SWAP_TYPE_REVERT,
    },
};

#define BOOT_SWAP_TABLES_COUNT \
    (sizeof boot_swap_tables / sizeof boot_swap_tables[0])
#endif  /* !defined(MCUBOOT_PRIMARY_ONLY)  */
/**
 * @brief Determine if the data at two memory addresses is equal
 *
 * @param s1    The first  memory region to compare.
 * @param s2    The second memory region to compare.
 * @param n     The amount of bytes to compare.
 *
 * @note        This function does not comply with the specification of memcmp,
 *              so should not be considered a drop-in replacement. It has no
 *              constant time execution. The point is to make sure that all the
 *              bytes are compared and detect if loop was abused and some cycles
 *              was skipped due to fault injection.
 *
 * @return      FIH_SUCCESS if memory regions are equal, otherwise FIH_FAILURE
 */
#ifdef MCUBOOT_FIH_PROFILE_OFF
inline
fih_int boot_fih_memequal(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}
#else
fih_int boot_fih_memequal(const void *s1, const void *s2, size_t n)
{
    size_t i;
    uint8_t *s1_p = (uint8_t*) s1;
    uint8_t *s2_p = (uint8_t*) s2;
    fih_int ret = FIH_FAILURE;

    for (i = 0; i < n; i++) {
        if (s1_p[i] != s2_p[i]) {
            goto out;
        }
    }
    if (i == n) {
        ret = FIH_SUCCESS;
    }

out:
    FIH_RET(ret);
}
#endif

#if !defined(MCUBOOT_PRIMARY_ONLY)
static int
boot_magic_decode(const uint32_t *magic)
{
    if (memcmp(magic, boot_img_magic, BOOT_MAGIC_SZ) == 0) {
        return BOOT_MAGIC_GOOD;
    }
    return BOOT_MAGIC_BAD;
}

static int
boot_flag_decode(uint8_t flag)
{
    if (flag != BOOT_FLAG_SET) {
        return BOOT_FLAG_BAD;
    }
    return BOOT_FLAG_SET;
}

/**
 * Determines if a status source table is satisfied by the specified magic
 * code.
 *
 * @param tbl_val               A magic field from a status source table.
 * @param val                   The magic value in a trailer, encoded as a
 *                                  BOOT_MAGIC_[...].
 *
 * @return                      1 if the two values are compatible;
 *                              0 otherwise.
 */
int
boot_magic_compatible_check(uint8_t tbl_val, uint8_t val)
{
    switch (tbl_val) {
    case BOOT_MAGIC_ANY:
        return 1;

    case BOOT_MAGIC_NOTGOOD:
        return val != BOOT_MAGIC_GOOD;

    default:
        return tbl_val == val;
    }
}

uint32_t
boot_status_sz(uint32_t min_write_sz)
{
    return /* state for all sectors */
           BOOT_STATUS_MAX_ENTRIES * BOOT_STATUS_STATE_COUNT * min_write_sz;
}

uint32_t
boot_trailer_sz(uint32_t min_write_sz)
{
    return /* state for all sectors */
#if !defined(MCUBOOT_OVERWRITE_ONLY)
           boot_status_sz(min_write_sz)           +
#ifdef MCUBOOT_ENC_IMAGES
           /* encryption keys */
#  if MCUBOOT_SWAP_SAVE_ENCTLV
           BOOT_ENC_TLV_ALIGN_SIZE * 2            +
#  else
           BOOT_ENC_KEY_SIZE * 2                  +
#  endif
#endif
           /* swap_type + swap_size */
           BOOT_MAX_ALIGN * 2                     +
#endif
           /* copy_done + image_ok */
           BOOT_MAX_ALIGN * 2                     +
           BOOT_MAGIC_SZ;
}

int
boot_status_entries(int image_index, const struct flash_area *fap)
{
#if MCUBOOT_SWAP_USING_SCRATCH
    if (fap->fa_id == FLASH_AREA_IMAGE_SCRATCH) {
        return BOOT_STATUS_STATE_COUNT;
    } else
#endif
    if (fap->fa_id == FLASH_AREA_IMAGE_PRIMARY(image_index) ||
               fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) {
        return BOOT_STATUS_STATE_COUNT * BOOT_STATUS_MAX_ENTRIES;
    }
    return -1;
}

uint32_t
boot_status_off(const struct flash_area *fap)
{
    uint32_t off_from_end;
    uint8_t elem_sz;

    elem_sz = flash_area_align(fap);

    off_from_end = boot_trailer_sz(elem_sz);

    assert(off_from_end <= fap->fa_size);
    return fap->fa_size - off_from_end;
}

uint32_t
boot_magic_off(const struct flash_area *fap)
{
    return fap->fa_size - BOOT_MAGIC_SZ;
}

static inline uint32_t
boot_image_ok_off(const struct flash_area *fap)
{
    return boot_magic_off(fap) - BOOT_MAX_ALIGN;
}

static inline uint32_t
boot_copy_done_off(const struct flash_area *fap)
{
    return boot_image_ok_off(fap) - BOOT_MAX_ALIGN;
}

uint32_t
boot_swap_info_off(const struct flash_area *fap)
{
    return boot_copy_done_off(fap) - BOOT_MAX_ALIGN;
}

static inline uint32_t
boot_swap_size_off(const struct flash_area *fap)
{
    return boot_swap_info_off(fap) - BOOT_MAX_ALIGN;
}

#ifdef MCUBOOT_ENC_IMAGES
static inline uint32_t
boot_enc_key_off(const struct flash_area *fap, uint8_t slot)
{
#if MCUBOOT_SWAP_SAVE_ENCTLV
    return boot_swap_size_off(fap) - ((slot + 1) *
            ((((BOOT_ENC_TLV_SIZE - 1) / BOOT_MAX_ALIGN) + 1) * BOOT_MAX_ALIGN));
#else
    return boot_swap_size_off(fap) - ((slot + 1) * BOOT_ENC_KEY_SIZE);
#endif
}
#endif

#ifdef MCUBOOT_USE_MCE
bool bootutil_buffer_is_erased(uint32_t off,
                               const struct flash_area *area,
                               const void *buffer, size_t len)
{
    size_t i;
    uint8_t *u8b;
    uint8_t erased_val;

    if (buffer == NULL || len == 0) {
        return false;
    }

    erased_val = flash_area_erased_val(area);
    if (boot_is_in_primary(area->fa_id, off, len)) {
        return boot_is_in_primary_and_erased(area->fa_id, off, len, erased_val);
    } else {
        for (i = 0, u8b = (uint8_t *)buffer; i < len; i++) {
            if (u8b[i] != erased_val) {
                return false;
            }
        }
    }

    return true;
}
#else /* not MCUBOOT_USE_MCE */
bool bootutil_buffer_is_erased(const struct flash_area *area,
                               const void *buffer, size_t len)
{
    size_t i;
    uint8_t *u8b;
    uint8_t erased_val;

    if (buffer == NULL || len == 0) {
        return false;
    }

    erased_val = flash_area_erased_val(area);
    for (i = 0, u8b = (uint8_t *)buffer; i < len; i++) {
        if (u8b[i] != erased_val) {
            return false;
        }
    }

    return true;
}
#endif /* MCUBOOT_USE_MCE */

int
boot_read_swap_state(const struct flash_area *fap,
                     struct boot_swap_state *state)
{
    uint32_t magic[BOOT_MAGIC_ARR_SZ];
    uint32_t off;
    uint8_t swap_info;
    int rc;

    off = boot_magic_off(fap);
    rc = flash_area_read(fap, off, magic, BOOT_MAGIC_SZ);
    if (rc < 0) {
        return BOOT_EFLASH;
    }
#ifdef MCUBOOT_USE_MCE
    if (bootutil_buffer_is_erased(fap->fa_off + off, fap, magic, BOOT_MAGIC_SZ)) {
#else /* not MCUBOOT_USE_MCE */
    if (bootutil_buffer_is_erased(fap, magic, BOOT_MAGIC_SZ)) {
#endif /* MCUBOOT_USE_MCE */
        state->magic = BOOT_MAGIC_UNSET;
    } else {
        state->magic = boot_magic_decode(magic);
    }

    off = boot_swap_info_off(fap);
    rc = flash_area_read(fap, off, &swap_info, sizeof swap_info);
    if (rc < 0) {
        return BOOT_EFLASH;
    }

    /* Extract the swap type and image number */
    state->swap_type = BOOT_GET_SWAP_TYPE(swap_info);
    state->image_num = BOOT_GET_IMAGE_NUM(swap_info);

#ifdef MCUBOOT_USE_MCE
    if (bootutil_buffer_is_erased(fap->fa_off + off, fap, &swap_info, sizeof swap_info) ||
#else /* not MCUBOOT_USE_MCE */
    if (bootutil_buffer_is_erased(fap, &swap_info, sizeof swap_info) ||
#endif /* MCUBOOT_USE_MCE */
            state->swap_type > BOOT_SWAP_TYPE_REVERT) {
        state->swap_type = BOOT_SWAP_TYPE_NONE;
        state->image_num = 0;
    }

    off = boot_copy_done_off(fap);
    rc = flash_area_read(fap, off, &state->copy_done, sizeof state->copy_done);
    if (rc < 0) {
        return BOOT_EFLASH;
    }
#ifdef MCUBOOT_USE_MCE
    if (bootutil_buffer_is_erased(fap->fa_off + off,
                                  fap, &state->copy_done,
                                  sizeof state->copy_done)) {
#else /* not MCUBOOT_USE_MCE */
    if (bootutil_buffer_is_erased(fap, &state->copy_done,
                sizeof state->copy_done)) {
#endif /* MCUBOOT_USE_MCE */
        state->copy_done = BOOT_FLAG_UNSET;
    } else {
        state->copy_done = boot_flag_decode(state->copy_done);
    }

    off = boot_image_ok_off(fap);
    rc = flash_area_read(fap, off, &state->image_ok, sizeof state->image_ok);
    if (rc < 0) {
        return BOOT_EFLASH;
    }
#ifdef MCUBOOT_USE_MCE
    if (bootutil_buffer_is_erased(fap->fa_off + off,
                                  fap, &state->image_ok,
                                  sizeof state->image_ok)) {
#else /* MCUBOOT_USE_MCE */
    if (bootutil_buffer_is_erased(fap, &state->image_ok,
                sizeof state->image_ok)) {
#endif /* MCUBOOT_USE_MCE */
        state->image_ok = BOOT_FLAG_UNSET;
    } else {
        state->image_ok = boot_flag_decode(state->image_ok);
    }

    return 0;
}

/**
 * Reads the image trailer from the scratch area.
 */
int
boot_read_swap_state_by_id(int flash_area_id, struct boot_swap_state *state)
{
    const struct flash_area *fap;
    int rc;

    rc = flash_area_open(flash_area_id, &fap);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    rc = boot_read_swap_state(fap, state);
    flash_area_close(fap);
    return rc;
}

/**
 * This functions tries to locate the status area after an aborted swap,
 * by looking for the magic in the possible locations.
 *
 * If the magic is successfully found, a flash_area * is returned and it
 * is the responsibility of the called to close it.
 *
 * @returns 0 on success, -1 on errors
 */
static int
boot_find_status(int image_index, const struct flash_area **fap)
{
    uint32_t magic[BOOT_MAGIC_ARR_SZ];
    uint32_t off;
    uint8_t areas[2] = {
#if MCUBOOT_SWAP_USING_SCRATCH
        FLASH_AREA_IMAGE_SCRATCH,
#endif
        FLASH_AREA_IMAGE_PRIMARY(image_index),
    };
    unsigned int i;
    int rc;

    /*
     * In the middle a swap, tries to locate the area that is currently
     * storing a valid magic, first on the primary slot, then on scratch.
     * Both "slots" can end up being temporary storage for a swap and it
     * is assumed that if magic is valid then other metadata is too,
     * because magic is always written in the last step.
     */

    for (i = 0; i < sizeof(areas) / sizeof(areas[0]); i++) {
        rc = flash_area_open(areas[i], fap);
        if (rc != 0) {
            return rc;
        }

        off = boot_magic_off(*fap);
        rc = flash_area_read(*fap, off, magic, BOOT_MAGIC_SZ);
        if (rc != 0) {
            flash_area_close(*fap);
            return rc;
        }

        if (memcmp(magic, boot_img_magic, BOOT_MAGIC_SZ) == 0) {
            return 0;
        }

        flash_area_close(*fap);
    }

    /* If we got here, no magic was found */
    return -1;
}

int
boot_read_swap_size(int image_index, uint32_t *swap_size)
{
    uint32_t off;
    const struct flash_area *fap;
    int rc;

    rc = boot_find_status(image_index, &fap);
    if (rc == 0) {
        off = boot_swap_size_off(fap);
        rc = flash_area_read(fap, off, swap_size, sizeof *swap_size);
        flash_area_close(fap);
    }

    return rc;
}

#ifdef MCUBOOT_ENC_IMAGES
int
boot_read_enc_key(int image_index, uint8_t slot, struct boot_status *bs)
{
    uint32_t off;
    const struct flash_area *fap;
#if MCUBOOT_SWAP_SAVE_ENCTLV
    int i;
#endif
    int rc;

    rc = boot_find_status(image_index, &fap);
    if (rc == 0) {
        off = boot_enc_key_off(fap, slot);
#if MCUBOOT_SWAP_SAVE_ENCTLV
        rc = flash_area_read(fap, off, bs->enctlv[slot], BOOT_ENC_TLV_ALIGN_SIZE);
        if (rc == 0) {
            for (i = 0; i < BOOT_ENC_TLV_ALIGN_SIZE; i++) {
                if (bs->enctlv[slot][i] != 0xff) {
                    break;
                }
            }
            /* Only try to decrypt non-erased TLV metadata */
            if (i != BOOT_ENC_TLV_ALIGN_SIZE) {
                rc = boot_enc_decrypt(bs->enctlv[slot], bs->enckey[slot]);
            }
        }
#else
        rc = flash_area_read(fap, off, bs->enckey[slot], BOOT_ENC_KEY_SIZE);
#endif
        flash_area_close(fap);
    }

    return rc;
}
#endif

int
boot_write_magic(const struct flash_area *fap)
{
    uint32_t off;
    int rc;

    off = boot_magic_off(fap);

    BOOT_LOG_DBG("writing magic; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off));
    rc = flash_area_write(fap, off, boot_img_magic, BOOT_MAGIC_SZ);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}

/**
 * Write trailer data; status bytes, swap_size, etc
 *
 * @returns 0 on success, != 0 on error.
 */
static int
boot_write_trailer(const struct flash_area *fap, uint32_t off,
        const uint8_t *inbuf, uint8_t inlen)
{
    uint8_t buf[BOOT_MAX_ALIGN];
    uint8_t align;
    uint8_t erased_val;
    int rc;

    align = flash_area_align(fap);
    if (inlen > BOOT_MAX_ALIGN || align > BOOT_MAX_ALIGN) {
        return -1;
    }
    erased_val = flash_area_erased_val(fap);
    if (align < inlen) {
        align = inlen;
    }
    memcpy(buf, inbuf, inlen);
    memset(&buf[inlen], erased_val, align - inlen);

    rc = flash_area_write(fap, off, buf, align);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}

static int
boot_write_trailer_flag(const struct flash_area *fap, uint32_t off,
        uint8_t flag_val)
{
    const uint8_t buf[1] = { flag_val };
    return boot_write_trailer(fap, off, buf, 1);
}

int
boot_write_copy_done(const struct flash_area *fap)
{
    uint32_t off;

    off = boot_copy_done_off(fap);
    BOOT_LOG_DBG("writing copy_done; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off));
    return boot_write_trailer_flag(fap, off, BOOT_FLAG_SET);
}

int
boot_write_image_ok(const struct flash_area *fap)
{
    uint32_t off;

    off = boot_image_ok_off(fap);
    BOOT_LOG_DBG("writing image_ok; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off));
    return boot_write_trailer_flag(fap, off, BOOT_FLAG_SET);
}

/**
 * Writes the specified value to the `swap-type` field of an image trailer.
 * This value is persisted so that the boot loader knows what swap operation to
 * resume in case of an unexpected reset.
 */
int
boot_write_swap_info(const struct flash_area *fap, uint8_t swap_type,
                     uint8_t image_num)
{
    uint32_t off;
    uint8_t swap_info;

    BOOT_SET_SWAP_INFO(swap_info, image_num, swap_type);
    off = boot_swap_info_off(fap);
    BOOT_LOG_DBG("writing swap_info; fa_id=%d off=0x%lx (0x%lx), swap_type=0x%x"
                 " image_num=0x%x",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)(fap->fa_off + off), swap_type, image_num);
    return boot_write_trailer(fap, off, (const uint8_t *) &swap_info, 1);
}

int
boot_write_swap_size(const struct flash_area *fap, uint32_t swap_size)
{
    uint32_t off;

    off = boot_swap_size_off(fap);
    BOOT_LOG_DBG("writing swap_size; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)fap->fa_off + off);
    return boot_write_trailer(fap, off, (const uint8_t *) &swap_size, 4);
}

#ifdef MCUBOOT_ENC_IMAGES
int
boot_write_enc_key(const struct flash_area *fap, uint8_t slot,
        const struct boot_status *bs)
{
    uint32_t off;
    int rc;

    off = boot_enc_key_off(fap, slot);
    BOOT_LOG_DBG("writing enc_key; fa_id=%d off=0x%lx (0x%lx)",
                 fap->fa_id, (unsigned long)off,
                 (unsigned long)fap->fa_off + off);
#if MCUBOOT_SWAP_SAVE_ENCTLV
    rc = flash_area_write(fap, off, bs->enctlv[slot], BOOT_ENC_TLV_ALIGN_SIZE);
#else
    rc = flash_area_write(fap, off, bs->enckey[slot], BOOT_ENC_KEY_SIZE);
#endif
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}
#endif

int
boot_swap_type_multi(int image_index)
{
    const struct boot_swap_table *table;
    struct boot_swap_state primary_slot;
    struct boot_swap_state secondary_slot;
    int rc;
    size_t i;

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(image_index),
                                    &primary_slot);
    if (rc) {
        return BOOT_SWAP_TYPE_PANIC;
    }

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(image_index),
                                    &secondary_slot);
    if (rc) {
        return BOOT_SWAP_TYPE_PANIC;
    }

    for (i = 0; i < BOOT_SWAP_TABLES_COUNT; i++) {
        table = boot_swap_tables + i;

        if (boot_magic_compatible_check(table->magic_primary_slot,
                                        primary_slot.magic) &&
            boot_magic_compatible_check(table->magic_secondary_slot,
                                        secondary_slot.magic) &&
            (table->image_ok_primary_slot == BOOT_FLAG_ANY   ||
                table->image_ok_primary_slot == primary_slot.image_ok) &&
            (table->image_ok_secondary_slot == BOOT_FLAG_ANY ||
                table->image_ok_secondary_slot == secondary_slot.image_ok) &&
            (table->copy_done_primary_slot == BOOT_FLAG_ANY  ||
                table->copy_done_primary_slot == primary_slot.copy_done)) {
            BOOT_LOG_INF("Swap type: %s",
                         table->swap_type == BOOT_SWAP_TYPE_TEST   ? "test"   :
                         table->swap_type == BOOT_SWAP_TYPE_PERM   ? "perm"   :
                         table->swap_type == BOOT_SWAP_TYPE_REVERT ? "revert" :
                         "BUG; can't happen");
            if (table->swap_type != BOOT_SWAP_TYPE_TEST &&
                    table->swap_type != BOOT_SWAP_TYPE_PERM &&
                    table->swap_type != BOOT_SWAP_TYPE_REVERT) {
                return BOOT_SWAP_TYPE_PANIC;
            }
            return table->swap_type;
        }
    }

    BOOT_LOG_INF("Swap type: none");
    return BOOT_SWAP_TYPE_NONE;
}

/*
 * This function is not used by the bootloader itself, but its required API
 * by external tooling like mcumgr.
 */
int
boot_swap_type(void)
{
    return boot_swap_type_multi(0);
}

/**
 * Marks the image in the secondary slot as pending.  On the next reboot,
 * the system will perform a one-time boot of the the secondary slot image.
 *
 * @param permanent         Whether the image should be used permanently or
 *                              only tested once:
 *                                  0=run image once, then confirm or revert.
 *                                  1=run image forever.
 *
 * @return                  0 on success; nonzero on failure.
 */
int
boot_set_pending(int permanent)
{
    const struct flash_area *fap;
    struct boot_swap_state state_secondary_slot;
    uint8_t swap_type;
    int rc;

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(0),
                                    &state_secondary_slot);
    if (rc != 0) {
        return rc;
    }

    switch (state_secondary_slot.magic) {
    case BOOT_MAGIC_GOOD:
        /* Swap already scheduled. */
        return 0;

    case BOOT_MAGIC_UNSET:
        rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &fap);
        if (rc != 0) {
            rc = BOOT_EFLASH;
        } else {
            rc = boot_write_magic(fap);
        }

        if (rc == 0 && permanent) {
            rc = boot_write_image_ok(fap);
        }

        if (rc == 0) {
            if (permanent) {
                swap_type = BOOT_SWAP_TYPE_PERM;
            } else {
                swap_type = BOOT_SWAP_TYPE_TEST;
            }
            rc = boot_write_swap_info(fap, swap_type, 0);
        }

        flash_area_close(fap);
        return rc;

    case BOOT_MAGIC_BAD:
        /* The image slot is corrupt.  There is no way to recover, so erase the
         * slot to allow future upgrades.
         */
        rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &fap);
        if (rc != 0) {
            return BOOT_EFLASH;
        }

        flash_area_erase(fap, 0, fap->fa_size);
        flash_area_close(fap);
        return BOOT_EBADIMAGE;

    default:
        assert(0);
        return BOOT_EBADIMAGE;
    }
}

/**
 * Marks All images in the primary slot as confirmed.  The system will continue
 * booting into the image in the primary slot until told to boot from a
 * different slot.
 *
 * @return                  0 on success; nonzero on failure.
 *                          if one image cannot be confirmed, failure is returned
 */
int
boot_set_confirmed(void)
{
    const struct flash_area *fap;
    struct boot_swap_state state_primary_slot;
    int rc;
    int image_index;
    for (image_index = 0; image_index < MCUBOOT_IMAGE_NUMBER; image_index++)
    {
        rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(image_index),
                &state_primary_slot);
        if (rc != 0) {
            return rc;
        }
        switch (state_primary_slot.magic) {
        case BOOT_MAGIC_GOOD:
            /* Confirm needed; proceed. */
            rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index), &fap);
            if (rc) {
                rc = BOOT_EFLASH;
                goto done;
            }

            if (state_primary_slot.copy_done == BOOT_FLAG_UNSET) {
                /* Swap never completed.  This is unexpected. */
                rc = BOOT_EBADVECT;
                goto done;
            }

            if (state_primary_slot.image_ok == BOOT_FLAG_UNSET) {
                rc = boot_write_image_ok(fap);
                if (rc)
                    goto done;
            }
            flash_area_close(fap);
            break;
        case BOOT_MAGIC_UNSET:
            /* nothing to do */
            break;
        case BOOT_MAGIC_BAD:
            /* Unexpected state. */
            return BOOT_EBADVECT;
        }
    }
done:
    flash_area_close(fap);
    return rc;
}
#endif  /* !defined(MCUBOOT_PRIMARY_ONLY)  */
