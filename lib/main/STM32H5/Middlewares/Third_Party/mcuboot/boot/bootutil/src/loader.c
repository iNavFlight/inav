/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2016-2020 Linaro LTD
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

/**
 * This file provides an interface to the boot loader.  Functions defined in
 * this file should only be called while the boot loader is running.
 */

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "bootutil_priv.h"
#include "swap_priv.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/security_cnt.h"
#include "bootutil/boot_record.h"
#include "bootutil/fault_injection_hardening.h"

#ifdef MCUBOOT_ENC_IMAGES
#include "bootutil/enc_key.h"
#endif

#include "mcuboot_config/mcuboot_config.h"

#if defined(OTFDEC_EXTERNAL_FLASH_ENABLE)
#include "boot_hal_otfdec.h"
#endif
#include "boot_hal_imagevalid.h"
#include "boot_hal_hash_ref.h"
#ifdef FLOW_CONTROL
#include "boot_hal_flowcontrol.h"
extern void Error_Handler(void);
#endif

MCUBOOT_LOG_MODULE_DECLARE(mcuboot);

static struct boot_loader_state boot_data;
void boot_status_reset(struct boot_status *bs);
#if (BOOT_IMAGE_NUMBER > 1)
#define IMAGES_ITER(x) for ((x) = 0; (x) < BOOT_IMAGE_NUMBER; ++(x))
#else
#define IMAGES_ITER(x)
#endif

/*
 * This macro allows some control on the allocation of local variables.
 * When running natively on a target, we don't want to allocated huge
 * variables on the stack, so make them global instead. For the simulator
 * we want to run as many threads as there are tests, and it's safer
 * to just make those variables stack allocated.
 */
#if !defined(__BOOTSIM__)
#define TARGET_STATIC static
#else
#define TARGET_STATIC
#endif

static int
boot_read_image_headers(struct boot_loader_state *state, bool require_all,
        struct boot_status *bs)
{
    int rc;
    int i;

    for (i = 0; i < BOOT_NUM_SLOTS; i++) {
        rc = boot_read_image_header(state, i, boot_img_hdr(state, i), bs);
        if (rc != 0) {
            /* If `require_all` is set, fail on any single fail, otherwise
             * if at least the first slot's header was read successfully,
             * then the boot loader can attempt a boot.
             *
             * Failure to read any headers is a fatal error.
             */
            if (i > 0 && !require_all) {
                return 0;
            } else {
                return rc;
            }
        }
    }

    return 0;
}

#if !defined(MCUBOOT_DIRECT_XIP)
/*
 * Compute the total size of the given image.  Includes the size of
 * the TLVs.
 */
#if !defined(MCUBOOT_OVERWRITE_ONLY) ||  defined(MCUBOOT_OVERWRITE_ONLY_FAST)
static int
boot_read_image_size(struct boot_loader_state *state, int slot, uint32_t *size)
{
    const struct flash_area *fap;
    struct image_tlv_info info;
    uint32_t off;
    uint32_t protect_tlv_size;
    int area_id;
    int rc;

#if (BOOT_IMAGE_NUMBER == 1)
    (void)state;
#endif

    area_id = flash_area_id_from_multi_image_slot(BOOT_CURR_IMG(state), slot);
    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    off = BOOT_TLV_OFF(boot_img_hdr(state, slot));

    if (flash_area_read(fap, off, &info, sizeof(info))) {
        rc = BOOT_EFLASH;
        goto done;
    }

    protect_tlv_size = boot_img_hdr(state, slot)->ih_protect_tlv_size;
    if (info.it_magic == IMAGE_TLV_PROT_INFO_MAGIC) {
        if (protect_tlv_size != info.it_tlv_tot) {
            rc = BOOT_EBADIMAGE;
            goto done;
        }

        if (flash_area_read(fap, off + info.it_tlv_tot, &info, sizeof(info))) {
            rc = BOOT_EFLASH;
            goto done;
        }
    } else if (protect_tlv_size != 0) {
        rc = BOOT_EBADIMAGE;
        goto done;
    }

    if (info.it_magic != IMAGE_TLV_INFO_MAGIC) {
        rc = BOOT_EBADIMAGE;
        goto done;
    }

    *size = off + protect_tlv_size + info.it_tlv_tot;
    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}
#endif /* !MCUBOOT_OVERWRITE_ONLY */

#if !defined(MCUBOOT_RAM_LOAD)
static uint32_t
boot_write_sz(struct boot_loader_state *state)
{
    uint32_t elem_sz;
#if MCUBOOT_SWAP_USING_SCRATCH
    uint32_t align;
#endif

    /* Figure out what size to write update status update as.  The size depends
     * on what the minimum write size is for scratch area, active image slot.
     * We need to use the bigger of those 2 values.
     */
    elem_sz = flash_area_align(BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT));
#if MCUBOOT_SWAP_USING_SCRATCH
    align = flash_area_align(BOOT_SCRATCH_AREA(state));
    if (align > elem_sz) {
        elem_sz = align;
    }
#endif

    return elem_sz;
}

#ifndef MCUBOOT_USE_FLASH_AREA_GET_SECTORS
static int
boot_initialize_area(struct boot_loader_state *state, int flash_area)
{
    int num_sectors = BOOT_MAX_IMG_SECTORS;
    int rc;

    if (flash_area == FLASH_AREA_IMAGE_PRIMARY(BOOT_CURR_IMG(state))) {
        rc = flash_area_to_sectors(flash_area, &num_sectors,
                                   BOOT_IMG(state, BOOT_PRIMARY_SLOT).sectors);
        BOOT_IMG(state, BOOT_PRIMARY_SLOT).num_sectors = (size_t)num_sectors;

    } else if (flash_area == FLASH_AREA_IMAGE_SECONDARY(BOOT_CURR_IMG(state))) {
        rc = flash_area_to_sectors(flash_area, &num_sectors,
                                 BOOT_IMG(state, BOOT_SECONDARY_SLOT).sectors);
        BOOT_IMG(state, BOOT_SECONDARY_SLOT).num_sectors = (size_t)num_sectors;

#if MCUBOOT_SWAP_USING_SCRATCH
    } else if (flash_area == FLASH_AREA_IMAGE_SCRATCH) {
        rc = flash_area_to_sectors(flash_area, &num_sectors,
                                   state->scratch.sectors);
        state->scratch.num_sectors = (size_t)num_sectors;
#endif

    } else {
        return BOOT_EFLASH;
    }

    return rc;
}
#else  /* defined(MCUBOOT_USE_FLASH_AREA_GET_SECTORS) */
static int
boot_initialize_area(struct boot_loader_state *state, int flash_area)
{
    uint32_t num_sectors;
    struct flash_sector *out_sectors;
    size_t *out_num_sectors;
    int rc;

    num_sectors = BOOT_MAX_IMG_SECTORS;

    if (flash_area == FLASH_AREA_IMAGE_PRIMARY(BOOT_CURR_IMG(state))) {
        out_sectors = BOOT_IMG(state, BOOT_PRIMARY_SLOT).sectors;
        out_num_sectors = &BOOT_IMG(state, BOOT_PRIMARY_SLOT).num_sectors;
#if !defined(MCUBOOT_PRIMARY_ONLY)
    } else if (flash_area == FLASH_AREA_IMAGE_SECONDARY(BOOT_CURR_IMG(state))) {
        out_sectors = BOOT_IMG(state, BOOT_SECONDARY_SLOT).sectors;
        out_num_sectors = &BOOT_IMG(state, BOOT_SECONDARY_SLOT).num_sectors;
#if MCUBOOT_SWAP_USING_SCRATCH
    } else if (flash_area == FLASH_AREA_IMAGE_SCRATCH) {
        out_sectors = state->scratch.sectors;
        out_num_sectors = &state->scratch.num_sectors;
#endif
#endif
    } else {
        return BOOT_EFLASH;
    }

    rc = flash_area_get_sectors(flash_area, &num_sectors, out_sectors);
    if (rc != 0) {
        return rc;
    }
    *out_num_sectors = num_sectors;
    return 0;
}
#endif  /* !defined(MCUBOOT_USE_FLASH_AREA_GET_SECTORS) */

/**
 * Determines the sector layout of both image slots and the scratch area.
 * This information is necessary for calculating the number of bytes to erase
 * and copy during an image swap.  The information collected during this
 * function is used to populate the state.
 */
static int
boot_read_sectors(struct boot_loader_state *state)
{
    uint8_t image_index;
    int rc;

    image_index = BOOT_CURR_IMG(state);

    rc = boot_initialize_area(state, FLASH_AREA_IMAGE_PRIMARY(image_index));
    if (rc != 0) {
        return BOOT_EFLASH;
    }
#if !defined(MCUBOOT_PRIMARY_ONLY)
    rc = boot_initialize_area(state, FLASH_AREA_IMAGE_SECONDARY(image_index));
    if (rc != 0) {
        return BOOT_EFLASH;
    }
#endif
#if MCUBOOT_SWAP_USING_SCRATCH
    rc = boot_initialize_area(state, FLASH_AREA_IMAGE_SCRATCH);
    if (rc != 0) {
        return BOOT_EFLASH;
    }
#endif

    BOOT_WRITE_SZ(state) = boot_write_sz(state);

    return 0;
}

void
boot_status_reset(struct boot_status *bs)
{
#ifdef MCUBOOT_ENC_IMAGES
    memset(&bs->enckey, 0xff, BOOT_NUM_SLOTS * BOOT_ENC_KEY_SIZE);
#if MCUBOOT_SWAP_SAVE_ENCTLV
    memset(&bs->enctlv, 0xff, BOOT_NUM_SLOTS * BOOT_ENC_TLV_ALIGN_SIZE);
#endif
#endif /* MCUBOOT_ENC_IMAGES */

    bs->use_scratch = 0;
    bs->swap_size = 0;
    bs->source = 0;

    bs->op = BOOT_STATUS_OP_MOVE;
    bs->idx = BOOT_STATUS_IDX_0;
    bs->state = BOOT_STATUS_STATE_0;
    bs->swap_type = BOOT_SWAP_TYPE_NONE;
}

bool
boot_status_is_reset(const struct boot_status *bs)
{
    return (bs->op == BOOT_STATUS_OP_MOVE &&
            bs->idx == BOOT_STATUS_IDX_0 &&
            bs->state == BOOT_STATUS_STATE_0);
}

#if !defined(MCUBOOT_PRIMARY_ONLY)
/**
 * Writes the supplied boot status to the flash file system.  The boot status
 * contains the current state of an in-progress image copy operation.
 *
 * @param bs                    The boot status to write.
 *
 * @return                      0 on success; nonzero on failure.
 */
int
boot_write_status(const struct boot_loader_state *state, struct boot_status *bs)
{
    const struct flash_area *fap;
    uint32_t off;
    int area_id;
    int rc;
    uint8_t buf[BOOT_MAX_ALIGN];
    uint8_t align;
    uint8_t erased_val;

    /* NOTE: The first sector copied (that is the last sector on slot) contains
     *       the trailer. Since in the last step the primary slot is erased, the
     *       first two status writes go to the scratch which will be copied to
     *       the primary slot!
     */

#if MCUBOOT_SWAP_USING_SCRATCH
    if (bs->use_scratch) {
        /* Write to scratch. */
        area_id = FLASH_AREA_IMAGE_SCRATCH;
    } else {
#endif
        /* Write to the primary slot. */
        area_id = FLASH_AREA_IMAGE_PRIMARY(BOOT_CURR_IMG(state));
#if MCUBOOT_SWAP_USING_SCRATCH
    }
#endif

    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    off = boot_status_off(fap) +
          boot_status_internal_off(bs, BOOT_WRITE_SZ(state));
    align = flash_area_align(fap);
    erased_val = flash_area_erased_val(fap);
    memset(buf, erased_val, BOOT_MAX_ALIGN);
    buf[0] = bs->state;

    rc = flash_area_write(fap, off, buf, align);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}
#endif /* MCUBOOT_PRIMARY_ONLY */
#endif /* !MCUBOOT_RAM_LOAD */
#endif /* !MCUBOOT_DIRECT_XIP */

/*
 * Validate image hash/signature and optionally the security counter in a slot.
 */
static fih_int
boot_image_check(struct boot_loader_state *state, struct image_header *hdr,
                 const struct flash_area *fap, struct boot_status *bs)
{
    TARGET_STATIC uint8_t tmpbuf[BOOT_TMPBUF_SZ];
    uint8_t image_index;
#ifdef MCUBOOT_ENC_IMAGES
    int rc;
#endif
    fih_int fih_rc = FIH_FAILURE;

#if (BOOT_IMAGE_NUMBER == 1)
    (void)state;
#endif

    (void)bs;

    image_index = BOOT_CURR_IMG(state);

#ifdef MCUBOOT_ENC_IMAGES
#ifdef MCUBOOT_PRIMARY_ONLY
    if (MUST_DECRYPT_PRIMARY_ONLY(fap, image_index, hdr)) {
#else
    if (MUST_DECRYPT(fap, image_index, hdr)) { 
#endif

        rc = boot_enc_load(BOOT_CURR_ENC(state), image_index, hdr, fap, bs);
        if (rc < 0) {
            FIH_RET(fih_rc);
        }
#ifdef MCUBOOT_PRIMARY_ONLY
        /*  no secondary slot  */
        if (rc == 0 && boot_enc_set_key(BOOT_CURR_ENC(state), 0, bs)) {
             FIH_RET(fih_rc);
        }
#else
        if (rc == 0 && boot_enc_set_key(BOOT_CURR_ENC(state), 1, bs)) {
            FIH_RET(fih_rc);
        }
#endif
    }
#endif
    FIH_CALL(bootutil_img_validate, fih_rc, BOOT_CURR_ENC(state), image_index,
             hdr, fap, tmpbuf, BOOT_TMPBUF_SZ, NULL, 0, NULL);

    FIH_RET(fih_rc);
}

#if !defined(MCUBOOT_DIRECT_XIP) && !defined(MCUBOOT_RAM_LOAD)
static fih_int
split_image_check(struct image_header *app_hdr,
                  const struct flash_area *app_fap,
                  struct image_header *loader_hdr,
                  const struct flash_area *loader_fap)
{
    static void *tmpbuf;
    uint8_t loader_hash[32];
    fih_int fih_rc = FIH_FAILURE;

    if (!tmpbuf) {
        tmpbuf = malloc(BOOT_TMPBUF_SZ);
        if (!tmpbuf) {
            goto out;
        }
    }

    FIH_CALL(bootutil_img_validate, fih_rc, NULL, 0, loader_hdr, loader_fap,
             tmpbuf, BOOT_TMPBUF_SZ, NULL, 0, loader_hash);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    FIH_CALL(bootutil_img_validate, fih_rc, NULL, 0, app_hdr, app_fap,
             tmpbuf, BOOT_TMPBUF_SZ, loader_hash, 32, NULL);

out:
    FIH_RET(fih_rc);
}
#endif /* !MCUBOOT_DIRECT_XIP && !MCUBOOT_RAM_LOAD */

/*
 * Check that this is a valid header.  Valid means that the magic is
 * correct, and that the sizes/offsets are "sane".  Sane means that
 * there is no overflow on the arithmetic, and that the result fits
 * within the flash area we are in.
 */
static bool
boot_is_header_valid(const struct image_header *hdr, const struct flash_area *fap)
{
    uint32_t size;

    if (hdr->ih_magic != IMAGE_MAGIC) {
        return false;
    }

    if (!boot_u32_safe_add(&size, hdr->ih_img_size, hdr->ih_hdr_size)) {
        return false;
    }

    if (size >= fap->fa_size) {
        return false;
    }

    return true;
}

/*
 * Check that a memory area consists of a given value.
 */
static inline bool
boot_data_is_set_to(uint8_t val, void *data, size_t len)
{
    uint8_t i;
    uint8_t *p = (uint8_t *)data;
    for (i = 0; i < len; i++) {
        if (val != p[i]) {
            return false;
        }
    }
    return true;
}

static int
boot_check_header_erased(struct boot_loader_state *state, int slot)
{
    const struct flash_area *fap;
    struct image_header *hdr;
    uint8_t erased_val;
    int area_id;
    int rc;

    area_id = flash_area_id_from_multi_image_slot(BOOT_CURR_IMG(state), slot);
    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        return -1;
    }

    erased_val = flash_area_erased_val(fap);
    flash_area_close(fap);

    hdr = boot_img_hdr(state, slot);
    if (!boot_data_is_set_to(erased_val, &hdr->ih_magic, sizeof(hdr->ih_magic))) {
        return -1;
    }

    return 0;
}

#if ((BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)) || \
    defined(MCUBOOT_DIRECT_XIP) || \
    defined(MCUBOOT_RAM_LOAD) || \
    (defined(MCUBOOT_OVERWRITE_ONLY) && defined(MCUBOOT_DOWNGRADE_PREVENTION))
/**
 * Compare image version numbers not including the build number
 *
 * @param ver1           Pointer to the first image version to compare.
 * @param ver2           Pointer to the second image version to compare.
 *
 * @retval -1           If ver1 is strictly less than ver2.
 * @retval 0            If the image version numbers are equal,
 *                      (not including the build number).
 * @retval 1            If ver1 is strictly greater than ver2.
 */
static int
boot_version_cmp(const struct image_version *ver1,
                 const struct image_version *ver2)
{
    if (ver1->iv_major > ver2->iv_major) {
        return 1;
    }
    if (ver1->iv_major < ver2->iv_major) {
        return -1;
    }
    /* The major version numbers are equal, continue comparison. */
    if (ver1->iv_minor > ver2->iv_minor) {
        return 1;
    }
    if (ver1->iv_minor < ver2->iv_minor) {
        return -1;
    }
    /* The minor version numbers are equal, continue comparison. */
    if (ver1->iv_revision > ver2->iv_revision) {
        return 1;
    }
    if (ver1->iv_revision < ver2->iv_revision) {
        return -1;
    }

    return 0;
}
#endif

/*
 * Check that there is a valid image in a slot
 *
 * @returns
 *         FIH_SUCCESS                      if image was successfully validated
 *         1 (or its fih_int encoded form)  if no bootloable image was found
 *         FIH_FAILURE                      on any errors
 */
static fih_int
boot_validate_slot(struct boot_loader_state *state, int slot,
                   struct boot_status *bs)
{
    const struct flash_area *fap;
    struct image_header *hdr;
    int area_id;
    fih_int fih_rc = FIH_FAILURE;
    int rc;

    area_id = flash_area_id_from_multi_image_slot(BOOT_CURR_IMG(state), slot);
    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        FIH_RET(fih_rc);
    }

    hdr = boot_img_hdr(state, slot);
    if (boot_check_header_erased(state, slot) == 0 ||
        (hdr->ih_flags & IMAGE_F_NON_BOOTABLE)) {

#if defined(MCUBOOT_SWAP_USING_SCRATCH) || defined(MCUBOOT_SWAP_USING_MOVE)
        /*
         * This fixes an issue where an image might be erased, but a trailer
         * be left behind. It can happen if the image is in the secondary slot
         * and did not pass validation, in which case the whole slot is erased.
         * If during the erase operation, a reset occurs, parts of the slot
         * might have been erased while some did not. The concerning part is
         * the trailer because it might disable a new image from being loaded
         * through mcumgr; so we just get rid of the trailer here, if the header
         * is erased.
         */
        if (slot != BOOT_PRIMARY_SLOT) {
            swap_erase_trailer_sectors(state, fap);
        }
#endif

        /* No bootable image in slot; continue booting from the primary slot. */
        fih_rc = fih_int_encode(1);
        goto out;
    }

#if defined(MCUBOOT_OVERWRITE_ONLY) && defined(MCUBOOT_DOWNGRADE_PREVENTION)
    if (slot != BOOT_PRIMARY_SLOT) {
        /* Check if version of secondary slot is sufficient */
        rc = boot_version_cmp(
                &boot_img_hdr(state, BOOT_SECONDARY_SLOT)->ih_ver,
                &boot_img_hdr(state, BOOT_PRIMARY_SLOT)->ih_ver);
        if (rc < 0 && boot_check_header_erased(state, BOOT_PRIMARY_SLOT)) {
            BOOT_LOG_ERR("insufficient version in secondary slot");
            flash_area_erase(fap, 0, fap->fa_size);
            /* Image in the secondary slot does not satisfy version requirement.
             * Erase the image and continue booting from the primary slot.
             */
            fih_rc = fih_int_encode(1);
            goto out;
        }
    }
#endif

    /* Check header, if not valid, do not erase slot as slot may be shared
     * with other images using another magic. Addtionally, no need to perform
     * image check in this case if header is not valid.
     */
    if (!boot_is_header_valid(hdr, fap))
    {
        fih_rc = fih_int_encode(1);
        goto out;
    }

    FIH_CALL(boot_image_check, fih_rc, state, hdr, fap, bs);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        if ((slot != BOOT_PRIMARY_SLOT) || ARE_SLOTS_EQUIVALENT()) {
            flash_area_erase(fap, 0, fap->fa_size);
            /* Image is invalid, erase it to prevent further unnecessary
             * attempts to validate and boot it.
             */
        }

#if !defined(__BOOTSIM__)
        BOOT_LOG_ERR("Image in the %s slot is not valid!",
                     (slot == BOOT_PRIMARY_SLOT) ? "primary" : "secondary");
#endif
        fih_rc = fih_int_encode(1);
        goto out;
    }

out:
    flash_area_close(fap);

    FIH_RET(fih_rc);
}

#ifdef MCUBOOT_HW_ROLLBACK_PROT
/**
 * Updates the stored security counter value with the image's security counter
 * value which resides in the given slot, only if it's greater than the stored
 * value.
 *
 * @param image_index   Index of the image to determine which security
 *                      counter to update.
 * @param slot          Slot number of the image.
 * @param hdr           Pointer to the image header structure of the image
 *                      that is currently stored in the given slot.
 * @param updated       Pointer to cnt updated status flag (1: yes, 0: no)
 *
 * @return              0 on success; nonzero on failure.
 */
static int
boot_update_security_counter(uint8_t image_index, int slot,
                             struct image_header *hdr, uint32_t *updated)
{
    const struct flash_area *fap = NULL;
    uint32_t img_security_cnt;
    int rc;

    rc = flash_area_open(flash_area_id_from_multi_image_slot(image_index, slot),
                         &fap);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = bootutil_get_img_security_cnt(hdr, fap, &img_security_cnt);
    if (rc != 0) {
        goto done;
    }

    rc = boot_nv_security_counter_update(image_index, img_security_cnt, updated);
    if (rc != 0) {
        goto done;
    }

done:
    flash_area_close(fap);
    return rc;
}
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

#if !defined(MCUBOOT_DIRECT_XIP) && !defined(MCUBOOT_RAM_LOAD)
#if !defined(MCUBOOT_PRIMARY_ONLY)
/**
 * Determines which swap operation to perform, if any.  If it is determined
 * that a swap operation is required, the image in the secondary slot is checked
 * for validity.  If the image in the secondary slot is invalid, it is erased,
 * and a swap type of "none" is indicated.
 *
 * @return                      The type of swap to perform (BOOT_SWAP_TYPE...)
 */
static int
boot_validated_swap_type(struct boot_loader_state *state,
                         struct boot_status *bs)
{
    int swap_type;
    fih_int fih_rc = FIH_FAILURE;

    swap_type = boot_swap_type_multi(BOOT_CURR_IMG(state));
    if (BOOT_IS_UPGRADE(swap_type)) {
        /* Boot loader wants to switch to the secondary slot.
         * Ensure image is valid.
         */
        FIH_CALL(boot_validate_slot, fih_rc, state, BOOT_SECONDARY_SLOT, bs);
        if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
            if (fih_eq(fih_rc, fih_int_encode(1))) {
                swap_type = BOOT_SWAP_TYPE_NONE;
            } else {
                swap_type = BOOT_SWAP_TYPE_FAIL;
            }
        }
    }

    return swap_type;
}
#endif /* MCUBOOT_PRIMARY_ONLY */

/**
 * Erases a region of flash.
 *
 * @param flash_area           The flash_area containing the region to erase.
 * @param off                   The offset within the flash area to start the
 *                                  erase.
 * @param sz                    The number of bytes to erase.
 *
 * @return                      0 on success; nonzero on failure.
 */
int
boot_erase_region(const struct flash_area *fap, uint32_t off, uint32_t sz)
{
    return flash_area_erase(fap, off, sz);
}

/**
 * Copies the contents of one flash region to another.  You must erase the
 * destination region prior to calling this function.
 *
 * @param flash_area_id_src     The ID of the source flash area.
 * @param flash_area_id_dst     The ID of the destination flash area.
 * @param off_src               The offset within the source flash area to
 *                                  copy from.
 * @param off_dst               The offset within the destination flash area to
 *                                  copy to.
 * @param sz                    The number of bytes to copy.
 *
 * @return                      0 on success; nonzero on failure.
 */
int
boot_copy_region(struct boot_loader_state *state,
                 const struct flash_area *fap_src,
                 const struct flash_area *fap_dst,
                 uint32_t off_src, uint32_t off_dst, uint32_t sz)
{
    uint32_t bytes_copied;
    int chunk_sz;
    int rc;
#if defined(MCUBOOT_ENC_IMAGES)
    uint32_t off;
    uint32_t tlv_off;
    size_t blk_off;
    struct image_header *hdr;
    uint16_t idx;
    uint32_t blk_sz;
    uint8_t image_index;
#endif
#if !defined(MCUBOOT_OVERWRITE_ONLY)
    TARGET_STATIC uint8_t buf[1024];
#else
    /* fix me must be set according to sector size of target device */
    TARGET_STATIC uint8_t buf[FLASH_AREA_IMAGE_SECTOR_SIZE];
#endif
#if !defined(MCUBOOT_ENC_IMAGES)
    (void)state;
#endif

    bytes_copied = 0;
    while (bytes_copied < sz) {
        if (sz - bytes_copied > sizeof buf) {
            chunk_sz = sizeof buf;
        } else {
            chunk_sz = sz - bytes_copied;
        }
        rc = flash_area_read(fap_src, off_src + bytes_copied, buf, chunk_sz);
        if (rc != 0) {
            return BOOT_EFLASH;
        }
#if defined(MCUBOOT_PRIMARY_ONLY)
#if defined(MCUBOOT_ENC_IMAGES)
        image_index = BOOT_CURR_IMG(state);
        off = off_src;
        hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);
#endif /* MCUBOOT_ENC_IMAGES */
#else
#ifdef MCUBOOT_ENC_IMAGES
        image_index = BOOT_CURR_IMG(state);
        if ((fap_src->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index) ||
            fap_dst->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) &&
            !(fap_src->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index) &&
              fap_dst->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index))) {
            /* assume the secondary slot as src, needs decryption */
            hdr = boot_img_hdr(state, BOOT_SECONDARY_SLOT);
#if !defined(MCUBOOT_SWAP_USING_MOVE)
            off = off_src;
            if (fap_dst->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) {
                /* might need encryption (metadata from the primary slot) */
                hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);
                off = off_dst;
            }
#else
            off = off_dst;
            if (fap_dst->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) {
                hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);
            }
#endif
#endif
#endif /* MCUBOOT_PRIMARY_ONLY */
#if defined(MCUBOOT_ENC_IMAGES)
            if (IS_ENCRYPTED(hdr)) {
                blk_sz = chunk_sz;
                idx = 0;
                if (off + bytes_copied < hdr->ih_hdr_size) {
                    /* do not decrypt header */
#if defined(MCUBOOT_PRIMARY_ONLY)
                /* remove encryption flag in primary only */
                BOOT_LOG_INF("ih_flags %x",(unsigned int)((struct image_header *)buf)->ih_flags);
                ((struct image_header *)buf)->ih_flags &=~IMAGE_F_ENCRYPTED;
                BOOT_LOG_INF("ih_flags %x",(unsigned int)((struct image_header *)buf)->ih_flags);
#endif
                blk_off = 0;
                blk_sz = chunk_sz - hdr->ih_hdr_size;
                idx = hdr->ih_hdr_size;
            } else {
                blk_off = ((off + bytes_copied) - hdr->ih_hdr_size) & 0xf;
            }
            tlv_off = BOOT_TLV_OFF(hdr);
            if (off + bytes_copied + chunk_sz > tlv_off) {
                /* do not decrypt TLVs */
                if (off + bytes_copied >= tlv_off) {
                    blk_sz = 0;
                } else {
                    blk_sz = tlv_off - (off + bytes_copied) - (chunk_sz - blk_sz);
                    }

                }
                boot_encrypt(BOOT_CURR_ENC(state), image_index, fap_src,
                         (off + bytes_copied + idx) - hdr->ih_hdr_size, blk_sz,
                         blk_off, &buf[idx]);
            }
#if !defined(MCUBOOT_PRIMARY_ONLY)
        }
#endif
#endif
#if defined(MCUBOOT_PRIMARY_ONLY)
        rc = flash_area_erase(fap_dst, off_dst + bytes_copied, chunk_sz);
        if (rc != 0) {
            return BOOT_EFLASH;
        }
#endif
        rc = flash_area_write(fap_dst, off_dst + bytes_copied, buf, chunk_sz);
        if (rc != 0) {
            return BOOT_EFLASH;
        }

        bytes_copied += chunk_sz;

        MCUBOOT_WATCHDOG_FEED();
    }

    return 0;
}

/**
 * Overwrite primary slot with the image contained in the secondary slot.
 * If a prior copy operation was interrupted by a system reset, this function
 * redos the copy.
 *
 * @param bs                    The current boot status.  This function reads
 *                                  this struct to determine if it is resuming
 *                                  an interrupted swap operation.  This
 *                                  function writes the updated status to this
 *                                  function on return.
 *
 * @return                      0 on success; nonzero on failure.
 */
#if defined(MCUBOOT_PRIMARY_ONLY)
static int
boot_copy_image(struct boot_loader_state *state, struct boot_status *bs)
{
    size_t sect_count;
    size_t sect;
    int rc;
    size_t size;
    size_t this_size;
    const struct flash_area *fap_primary_slot;
    uint8_t image_index;
    uint32_t security_counter_updated;

    (void)bs;
    /*  compute size of area  */
    sect_count = boot_img_num_sectors(state, BOOT_PRIMARY_SLOT);

    for (sect = 0, size = 0; sect < sect_count; sect++) {
        this_size = boot_img_sector_size(state, BOOT_PRIMARY_SLOT, sect);
        size += this_size;
    }

    BOOT_LOG_INF("Image upgrade primary slot -> primary slot");
    BOOT_LOG_INF("Power off not supported");
    image_index = BOOT_CURR_IMG(state);

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index),
                         &fap_primary_slot);
    assert(rc == 0);

#ifdef MCUBOOT_ENC_IMAGES
    rc = boot_enc_load(BOOT_CURR_ENC(state), image_index,
                       boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                       fap_primary_slot, bs);
    BOOT_LOG_INF("Load Encrypted Key");

    if (rc < 0) {
        return BOOT_EBADIMAGE;
    }
    if (rc == 0 && boot_enc_set_key(BOOT_CURR_ENC(state), 0, bs)) {
        return BOOT_EBADIMAGE;
    }
#endif

    BOOT_LOG_INF("Copying the primary slot to the primary slot: 0x%zx bytes",
                 size);
    rc = boot_copy_region(state, fap_primary_slot, fap_primary_slot, 0, 0, size);

#ifdef MCUBOOT_HW_ROLLBACK_PROT
    /* Update the stored security counter with the new image's security counter
     * value. Both slots hold the new image at this point, but the secondary
     * slot's image header must be passed since the image headers in the
     * boot_data structure have not been updated yet.
     */
    rc = boot_update_security_counter(BOOT_CURR_IMG(state), BOOT_PRIMARY_SLOT,
                                      boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                      &security_counter_updated);
    if (rc != 0) {
        BOOT_LOG_ERR("Security counter update failed after image upgrade.");
        return rc;
    }
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

    flash_area_close(fap_primary_slot);
    /* TODO: Perhaps verify the primary slot's signature again? */

    return 0;
}
#elif defined(MCUBOOT_OVERWRITE_ONLY) || defined(MCUBOOT_BOOTSTRAP)
static int
boot_copy_image(struct boot_loader_state *state, struct boot_status *bs)
{
    size_t sect_count;
    size_t sect;
    int rc;
    size_t size;
    size_t this_size;
    const struct flash_area *fap_primary_slot;
    const struct flash_area *fap_secondary_slot;
    uint8_t image_index;
    uint32_t security_counter_updated;

#if defined(MCUBOOT_OVERWRITE_ONLY_FAST)
    uint32_t sector;
    uint32_t trailer_sz;
    uint32_t off;
    uint32_t sz;
#endif

    (void)bs;

#if defined(MCUBOOT_OVERWRITE_ONLY_FAST)
    uint32_t src_size = 0;
    rc = boot_read_image_size(state, BOOT_SECONDARY_SLOT, &src_size);
    assert(rc == 0);
#endif

    BOOT_LOG_INF("Image upgrade secondary slot -> primary slot");
    BOOT_LOG_INF("Erasing the primary slot");

    image_index = BOOT_CURR_IMG(state);

    rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(image_index),
            &fap_primary_slot);
    assert (rc == 0);

    rc = flash_area_open(FLASH_AREA_IMAGE_SECONDARY(image_index),
            &fap_secondary_slot);
    assert (rc == 0);

    sect_count = boot_img_num_sectors(state, BOOT_PRIMARY_SLOT);
    for (sect = 0, size = 0; sect < sect_count; sect++) {
        this_size = boot_img_sector_size(state, BOOT_PRIMARY_SLOT, sect);
        rc = boot_erase_region(fap_primary_slot, size, this_size);
        assert(rc == 0);

#if defined(MCUBOOT_OVERWRITE_ONLY_FAST)
        if ((size + this_size) >= src_size) {
            size += src_size - size;
            size += BOOT_WRITE_SZ(state) - (size % BOOT_WRITE_SZ(state));
            break;
        }
#endif

        size += this_size;
    }

#if defined(MCUBOOT_OVERWRITE_ONLY_FAST)
    trailer_sz = boot_trailer_sz(BOOT_WRITE_SZ(state));
    sector = boot_img_num_sectors(state, BOOT_PRIMARY_SLOT) - 1;
    sz = 0;
    do {
        sz += boot_img_sector_size(state, BOOT_PRIMARY_SLOT, sector);
        off = boot_img_sector_off(state, BOOT_PRIMARY_SLOT, sector);
        sector--;
    } while (sz < trailer_sz);

    rc = boot_erase_region(fap_primary_slot, off, sz);
    assert(rc == 0);
#endif

#ifdef MCUBOOT_ENC_IMAGES
    if (IS_ENCRYPTED(boot_img_hdr(state, BOOT_SECONDARY_SLOT))) {
        rc = boot_enc_load(BOOT_CURR_ENC(state), image_index,
                boot_img_hdr(state, BOOT_SECONDARY_SLOT),
                fap_secondary_slot, bs);

        if (rc < 0) {
            return BOOT_EBADIMAGE;
        }
        if (rc == 0 && boot_enc_set_key(BOOT_CURR_ENC(state), 1, bs)) {
            return BOOT_EBADIMAGE;
        }
    }
#endif

#if !defined(MCUBOOT_OVERWRITE_ONLY)
    /* Confirm image after copy, as no revert possible */
    rc = swap_set_image_ok(BOOT_CURR_IMG(state));
    if (rc != 0) {
      return rc;
    }
#endif /* !defined(MCUBOOT_OVERWRITE_ONLY) */

    BOOT_LOG_INF("Copying the secondary slot to the primary slot: 0x%x bytes",
                 size);
    rc = boot_copy_region(state, fap_secondary_slot, fap_primary_slot, 0, 0, size);
    if (rc != 0) {
        return rc;
    }

#if defined(MCUBOOT_OVERWRITE_ONLY_FAST)
    rc = boot_write_magic(fap_primary_slot);
    if (rc != 0) {
        return rc;
    }
#endif

#ifdef MCUBOOT_HW_ROLLBACK_PROT
    /* Update the stored security counter with the new image's security counter
     * value. Both slots hold the new image at this point, but the secondary
     * slot's image header must be passed since the image headers in the
     * boot_data structure have not been updated yet.
     */
    rc = boot_update_security_counter(BOOT_CURR_IMG(state), BOOT_PRIMARY_SLOT,
                                boot_img_hdr(state, BOOT_SECONDARY_SLOT),
                                &security_counter_updated);
    if (rc != 0) {
        BOOT_LOG_ERR("Security counter update failed after image upgrade.");
        return rc;
    }
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

    /*
     * Erases complete secondary slot. A new image download can be performed
     * without remaining data from previously installed image.
     */
    BOOT_LOG_DBG("erasing secondary slot");
    rc = flash_area_erase(fap_secondary_slot, 0, fap_secondary_slot->fa_size);
    assert(rc == 0);

    flash_area_close(fap_primary_slot);
    flash_area_close(fap_secondary_slot);

    /* TODO: Perhaps verify the primary slot's signature again? */

    return 0;
}
#endif

#if !defined(MCUBOOT_OVERWRITE_ONLY)
/**
 * Swaps the two images in flash.  If a prior copy operation was interrupted
 * by a system reset, this function completes that operation.
 *
 * @param bs                    The current boot status.  This function reads
 *                                  this struct to determine if it is resuming
 *                                  an interrupted swap operation.  This
 *                                  function writes the updated status to this
 *                                  function on return.
 *
 * @return                      0 on success; nonzero on failure.
 */
static int
boot_swap_image(struct boot_loader_state *state, struct boot_status *bs)
{
    struct image_header *hdr;
#ifdef MCUBOOT_ENC_IMAGES
    const struct flash_area *fap;
    uint8_t slot;
    uint8_t i;
#endif
    uint32_t size;
    uint32_t copy_size;
    uint8_t image_index;
    int rc;

    /* FIXME: just do this if asked by user? */

    size = copy_size = 0;
    image_index = BOOT_CURR_IMG(state);

    if (boot_status_is_reset(bs)) {
        /*
         * No swap ever happened, so need to find the largest image which
         * will be used to determine the amount of sectors to swap.
         */
        hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);
        if (hdr->ih_magic == IMAGE_MAGIC) {
            rc = boot_read_image_size(state, BOOT_PRIMARY_SLOT, &copy_size);
            assert(rc == 0);
        }

#ifdef MCUBOOT_ENC_IMAGES
        if (IS_ENCRYPTED(hdr)) {
            fap = BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT);
            rc = boot_enc_load(BOOT_CURR_ENC(state), image_index, hdr, fap, bs);
            assert(rc >= 0);

            if (rc == 0) {
                rc = boot_enc_set_key(BOOT_CURR_ENC(state), 0, bs);
                assert(rc == 0);
            } else {
                rc = 0;
            }
        } else {
            memset(bs->enckey[0], 0xff, BOOT_ENC_KEY_SIZE);
        }
#endif

        hdr = boot_img_hdr(state, BOOT_SECONDARY_SLOT);
        if (hdr->ih_magic == IMAGE_MAGIC) {
            rc = boot_read_image_size(state, BOOT_SECONDARY_SLOT, &size);
            assert(rc == 0);
        }

#ifdef MCUBOOT_ENC_IMAGES
        hdr = boot_img_hdr(state, BOOT_SECONDARY_SLOT);
        if (IS_ENCRYPTED(hdr)) {
            fap = BOOT_IMG_AREA(state, BOOT_SECONDARY_SLOT);
            rc = boot_enc_load(BOOT_CURR_ENC(state), image_index, hdr, fap, bs);
            assert(rc >= 0);

            if (rc == 0) {
                rc = boot_enc_set_key(BOOT_CURR_ENC(state), 1, bs);
                assert(rc == 0);
            } else {
                rc = 0;
            }
        } else {
            memset(bs->enckey[1], 0xff, BOOT_ENC_KEY_SIZE);
        }
#endif

        if (size > copy_size) {
            copy_size = size;
        }

        bs->swap_size = copy_size;
    } else {
        /*
         * If a swap was under way, the swap_size should already be present
         * in the trailer...
         */
        rc = boot_read_swap_size(image_index, &bs->swap_size);
        assert(rc == 0);

        copy_size = bs->swap_size;

#ifdef MCUBOOT_ENC_IMAGES
        for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
            rc = boot_read_enc_key(image_index, slot, bs);
            assert(rc == 0);

            for (i = 0; i < BOOT_ENC_KEY_SIZE; i++) {
                if (bs->enckey[slot][i] != 0xff) {
                    break;
                }
            }

            if (i != BOOT_ENC_KEY_SIZE) {
                boot_enc_set_key(BOOT_CURR_ENC(state), slot, bs);
            }
        }
#endif
    }
    (void)rc;
    swap_run(state, bs, copy_size);

#ifdef MCUBOOT_VALIDATE_PRIMARY_SLOT
    extern int boot_status_fails;
    if (boot_status_fails > 0) {
        BOOT_LOG_WRN("%d status write fails performing the swap",
                     boot_status_fails);
    }
#endif

    return 0;
}
#endif

#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
/**
 * Check the image dependency whether it is satisfied and modify
 * the swap type if necessary.
 *
 * @param dep               Image dependency which has to be verified.
 *
 * @return                  0 on success; nonzero on failure.
 */
static int
boot_verify_slot_dependency(struct boot_loader_state *state,
                            struct image_dependency *dep)
{
    struct image_version *dep_version;
    size_t dep_slot;
    int rc;
    uint8_t swap_type;

    /* Determine the source of the image which is the subject of
     * the dependency and get it's version. */
    swap_type = state->swap_type[dep->image_id];
    dep_slot = BOOT_IS_UPGRADE(swap_type) ? BOOT_SECONDARY_SLOT
                                          : BOOT_PRIMARY_SLOT;
    dep_version = &state->imgs[dep->image_id][dep_slot].hdr.ih_ver;

    rc = boot_version_cmp(dep_version, &dep->image_min_version);
    if (rc < 0) {
        /* Dependency not satisfied.
         * Modify the swap type to decrease the version number of the image
         * (which will be located in the primary slot after the boot process),
         * consequently the number of unsatisfied dependencies will be
         * decreased or remain the same.
         */
        switch (BOOT_SWAP_TYPE(state)) {
        case BOOT_SWAP_TYPE_TEST:
        case BOOT_SWAP_TYPE_PERM:
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
            break;
        case BOOT_SWAP_TYPE_NONE:
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_REVERT;
            break;
        default:
            break;
        }
    } else {
        /* Dependency satisfied. */
        rc = 0;
    }

    return rc;
}

/**
 * Read all dependency TLVs of an image from the flash and verify
 * one after another to see if they are all satisfied.
 *
 * @param slot              Image slot number.
 *
 * @return                  0 on success; nonzero on failure.
 */
static int
boot_verify_slot_dependencies(struct boot_loader_state *state, uint32_t slot)
{
    const struct flash_area *fap;
    struct image_tlv_iter it;
    struct image_dependency dep;
    uint32_t off;
    uint16_t len;
    int area_id;
    int rc;

    area_id = flash_area_id_from_multi_image_slot(BOOT_CURR_IMG(state), slot);
    rc = flash_area_open(area_id, &fap);
    if (rc != 0) {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = bootutil_tlv_iter_begin(&it, boot_img_hdr(state, slot), fap,
            IMAGE_TLV_DEPENDENCY, true);
    if (rc != 0) {
        goto done;
    }

    while (true) {
        rc = bootutil_tlv_iter_next(&it, &off, &len, NULL);
        if (rc < 0) {
            return -1;
        } else if (rc > 0) {
            rc = 0;
            break;
        }

        if (len != sizeof(dep)) {
            rc = BOOT_EBADIMAGE;
            goto done;
        }

        rc = flash_area_read(fap, off, &dep, len);
        if (rc != 0) {
            rc = BOOT_EFLASH;
            goto done;
        }

        if (dep.image_id >= BOOT_IMAGE_NUMBER) {
            rc = BOOT_EBADARGS;
            goto done;
        }

        /* Verify dependency and modify the swap type if not satisfied. */
        rc = boot_verify_slot_dependency(state, &dep);
        if (rc != 0) {
            /* Dependency not satisfied. */
            goto done;
        }
    }

done:
    flash_area_close(fap);
    return rc;
}

/**
 * Iterate over all the images and verify whether the image dependencies in the
 * TLV area are all satisfied and update the related swap type if necessary.
 */
static int
boot_verify_dependencies(struct boot_loader_state *state)
{
    int rc = -1;
    uint8_t slot;

    BOOT_CURR_IMG(state) = 0;
    while (BOOT_CURR_IMG(state) < BOOT_IMAGE_NUMBER) {
        if (BOOT_SWAP_TYPE(state) != BOOT_SWAP_TYPE_NONE &&
            BOOT_SWAP_TYPE(state) != BOOT_SWAP_TYPE_FAIL) {
            slot = BOOT_SECONDARY_SLOT;
        } else {
            slot = BOOT_PRIMARY_SLOT;
        }

        rc = boot_verify_slot_dependencies(state, slot);
        if (rc == 0) {
            /* All dependencies've been satisfied, continue with next image. */
            BOOT_CURR_IMG(state)++;
        } else {
            /* Cannot upgrade due to non-met dependencies, so disable all
             * image upgrades.
             */
            for (int idx = 0; idx < BOOT_IMAGE_NUMBER; idx++) {
                BOOT_CURR_IMG(state) = idx;
                BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
            }
            break;
        }
    }
    return rc;
}
#endif /* (BOOT_IMAGE_NUMBER > 1) */

/**
 * Performs a clean (not aborted) image update.
 *
 * @param bs                    The current boot status.
 *
 * @return                      0 on success; nonzero on failure.
 */
static int
boot_perform_update(struct boot_loader_state *state, struct boot_status *bs)
{
    int rc;
#ifndef MCUBOOT_OVERWRITE_ONLY
    uint8_t swap_type;
    uint32_t security_counter_updated;
#endif

#if defined(OTFDEC_EXTERNAL_FLASH_ENABLE)
    /* At this point decision is taken to perform clean (not aborted) image
     * update. The otfdec symmetric key (for OSPI flash execution) located in
     * internal flash must be invalidated (it will be replaced by the new one)
     */
    if (IS_OTFDEC_ENCRYPTED(boot_img_hdr(state, BOOT_SECONDARY_SLOT))) {
        if (otfdec_invalidate_key() != 0) {
            return BOOT_EFLASH;
        }
    }
#endif /* OTFDEC_EXTERNAL_FLASH_ENABLE */

    /* At this point there are no aborted swaps. */
#if defined(MCUBOOT_OVERWRITE_ONLY)
    rc = boot_copy_image(state, bs);
#elif defined(MCUBOOT_BOOTSTRAP)
    /* Check if the image update was triggered by a bad image in the
     * primary slot (the validity of the image in the secondary slot had
     * already been checked).
     */
    fih_int fih_rc = FIH_FAILURE;
    rc = boot_check_header_erased(state, BOOT_PRIMARY_SLOT);
    FIH_CALL(boot_validate_slot, fih_rc, state, BOOT_PRIMARY_SLOT, bs);
    if (rc == 0 || fih_not_eq(fih_rc, FIH_SUCCESS)) {
        rc = boot_copy_image(state, bs);
    } else {
        rc = boot_swap_image(state, bs);
    }
#else
        rc = boot_swap_image(state, bs);
#endif
    assert(rc == 0);

#ifndef MCUBOOT_OVERWRITE_ONLY
    /* The following state needs image_ok be explicitly set after the
     * swap was finished to avoid a new revert.
     */
    swap_type = BOOT_SWAP_TYPE(state);
    if (swap_type == BOOT_SWAP_TYPE_REVERT ||
            swap_type == BOOT_SWAP_TYPE_PERM) {
        rc = swap_set_image_ok(BOOT_CURR_IMG(state));
        if (rc != 0) {
            BOOT_SWAP_TYPE(state) = swap_type = BOOT_SWAP_TYPE_PANIC;
        }
    }

#ifdef MCUBOOT_HW_ROLLBACK_PROT
    if (swap_type == BOOT_SWAP_TYPE_PERM) {
        /* Update the stored security counter with the new image's security
         * counter value. The primary slot holds the new image at this point,
         * but the secondary slot's image header must be passed since image
         * headers in the boot_data structure have not been updated yet.
         *
         * In case of a permanent image swap mcuboot will never attempt to
         * revert the images on the next reboot. Therefore, the security
         * counter must be increased right after the image upgrade.
         */
#ifdef MCUBOOT_PRIMARY_ONLY
        rc = boot_update_security_counter(
                                    BOOT_CURR_IMG(state),
                                    BOOT_PRIMARY_SLOT,
                                    boot_img_hdr(state, BOOT_PRIMARY_SLOT)
                                    &security_counter_updated);
#else
        rc = boot_update_security_counter(
                                    BOOT_CURR_IMG(state),
                                    BOOT_PRIMARY_SLOT,
                                    boot_img_hdr(state, BOOT_SECONDARY_SLOT),
                                    &security_counter_updated);
#endif
        if (rc != 0) {
            BOOT_LOG_ERR("Security counter update failed after "
                         "image upgrade.");
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
        }
    }
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

    if (BOOT_IS_UPGRADE(swap_type)) {
        rc = swap_set_copy_done(BOOT_CURR_IMG(state));
        if (rc != 0) {
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
        }
    }
#endif /* !MCUBOOT_OVERWRITE_ONLY */

    return rc;
}

/**
 * Completes a previously aborted image swap.
 *
 * @param bs                    The current boot status.
 *
 * @return                      0 on success; nonzero on failure.
 */
#if !defined(MCUBOOT_OVERWRITE_ONLY)
static int
boot_complete_partial_swap(struct boot_loader_state *state,
        struct boot_status *bs)
{
    int rc;

    /* Determine the type of swap operation being resumed from the
     * `swap-type` trailer field.
     */
    rc = boot_swap_image(state, bs);
    assert(rc == 0);

    BOOT_SWAP_TYPE(state) = bs->swap_type;

    /* The following states need image_ok be explicitly set after the
     * swap was finished to avoid a new revert.
     */
    if (bs->swap_type == BOOT_SWAP_TYPE_REVERT ||
        bs->swap_type == BOOT_SWAP_TYPE_PERM) {
        rc = swap_set_image_ok(BOOT_CURR_IMG(state));
        if (rc != 0) {
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
        }
    }

    if (BOOT_IS_UPGRADE(bs->swap_type)) {
        rc = swap_set_copy_done(BOOT_CURR_IMG(state));
        if (rc != 0) {
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
        }
    }

    if (BOOT_SWAP_TYPE(state) == BOOT_SWAP_TYPE_PANIC) {
        BOOT_LOG_ERR("panic!");
        assert(0);

        /* Loop forever... */
        while (1) {}
    }

    return rc;
}
#endif /* !MCUBOOT_OVERWRITE_ONLY */

#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
/**
 * Review the validity of previously determined swap types of other images.
 *
 * @param aborted_swap          The current image upgrade is a
 *                              partial/aborted swap.
 */
static void
boot_review_image_swap_types(struct boot_loader_state *state,
                             bool aborted_swap)
{
    /* In that case if we rebooted in the middle of an image upgrade process, we
     * must review the validity of swap types, that were previously determined
     * for other images. The image_ok flag had not been set before the reboot
     * for any of the updated images (only the copy_done flag) and thus falsely
     * the REVERT swap type has been determined for the previous images that had
     * been updated before the reboot.
     *
     * There are two separate scenarios that we have to deal with:
     *
     * 1. The reboot has happened during swapping an image:
     *      The current image upgrade has been determined as a
     *      partial/aborted swap.
     * 2. The reboot has happened between two separate image upgrades:
     *      In this scenario we must check the swap type of the current image.
     *      In those cases if it is NONE or REVERT we cannot certainly determine
     *      the fact of a reboot. In a consistent state images must move in the
     *      same direction or stay in place, e.g. in practice REVERT and TEST
     *      swap types cannot be present at the same time. If the swap type of
     *      the current image is either TEST, PERM or FAIL we must review the
     *      already determined swap types of other images and set each false
     *      REVERT swap types to NONE (these images had been successfully
     *      updated before the system rebooted between two separate image
     *      upgrades).
     */

    if (BOOT_CURR_IMG(state) == 0) {
        /* Nothing to do */
        return;
    }

    if (!aborted_swap) {
        if ((BOOT_SWAP_TYPE(state) == BOOT_SWAP_TYPE_NONE) ||
            (BOOT_SWAP_TYPE(state) == BOOT_SWAP_TYPE_REVERT)) {
            /* Nothing to do */
            return;
        }
    }

    for (uint8_t i = 0; i < BOOT_CURR_IMG(state); i++) {
        if (state->swap_type[i] == BOOT_SWAP_TYPE_REVERT) {
            state->swap_type[i] = BOOT_SWAP_TYPE_NONE;
        }
    }
}
#endif

/**
 * Prepare image to be updated if required.
 *
 * Prepare image to be updated if required with completing an image swap
 * operation if one was aborted and/or determining the type of the
 * swap operation. In case of any error set the swap type to NONE.
 *
 * @param state                 TODO
 * @param bs                    Pointer where the read and possibly updated
 *                              boot status can be written to.
 */
static void
boot_prepare_image_for_update(struct boot_loader_state *state,
                              struct boot_status *bs)
{
    int rc;
#if !defined(MCUBOOT_PRIMARY_ONLY)
    fih_int fih_rc = FIH_FAILURE;
#endif
    /* Determine the sector layout of the image slots and scratch area. */
    rc = boot_read_sectors(state);
    if (rc != 0) {
        BOOT_LOG_WRN("Failed reading sectors; BOOT_MAX_IMG_SECTORS=%d"
                     " - too small?", BOOT_MAX_IMG_SECTORS);
        /* Unable to determine sector layout, continue with next image
         * if there is one.
         */
        BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
        return;
    }

    /* Attempt to read an image header from each slot. */
    rc = boot_read_image_headers(state, false, NULL);
    if (rc != 0) {
        /* Continue with next image if there is one. */
        BOOT_LOG_WRN("Failed reading image headers; Image=%u",
                BOOT_CURR_IMG(state));
        BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
        return;
    }
#if defined(MCUBOOT_PRIMARY_ONLY)
    /* if any header is flag encoded update is required */
    if (boot_img_hdr(state,0)->ih_magic == IMAGE_MAGIC) {
        if (IS_PRIMARY_ONLY(BOOT_IMG_HDR(state)) && IS_ENCRYPTED(BOOT_IMG_HDR(state))) {
            BOOT_LOG_WRN("Image %d is encrypted ",
                         BOOT_CURR_IMG(state));

            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PERM;
        } else {
            BOOT_LOG_WRN("Image %d is installed ",
                         BOOT_CURR_IMG(state));

            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
        }
    }
    /* no other check require, installation is done on image itself */
    /* an interruption during installation requires a new download */
    return;
#else
    /* If the current image's slots aren't compatible, no swap is possible.
     * Just boot into primary slot.
     */
    if (boot_slots_compatible(state)) {
        boot_status_reset(bs);

#ifndef MCUBOOT_OVERWRITE_ONLY
        rc = swap_read_status(state, bs);
        if (rc != 0) {
            BOOT_LOG_WRN("Failed reading boot status; Image=%u",
                    BOOT_CURR_IMG(state));
            /* Continue with next image if there is one. */
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
            return;
        }
#endif

#ifdef MCUBOOT_SWAP_USING_MOVE
        /*
         * Must re-read image headers because the boot status might
         * have been updated in the previous function call.
         */
        rc = boot_read_image_headers(state, !boot_status_is_reset(bs), bs);
#ifdef MCUBOOT_BOOTSTRAP
        /* When bootstrapping it's OK to not have image magic in the primary slot */
        if (rc != 0 && (BOOT_CURR_IMG(state) != BOOT_PRIMARY_SLOT ||
                boot_check_header_erased(state, BOOT_PRIMARY_SLOT) != 0)) {
#else
        if (rc != 0) {
#endif

            /* Continue with next image if there is one. */
            BOOT_LOG_WRN("Failed reading image headers; Image=%u",
                    BOOT_CURR_IMG(state));
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
            return;
        }
#endif

        /* Determine if we rebooted in the middle of an image swap
         * operation. If a partial swap was detected, complete it.
         */
        if (!boot_status_is_reset(bs)) {

#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
            boot_review_image_swap_types(state, true);
#endif

#ifdef MCUBOOT_OVERWRITE_ONLY
            /* Should never arrive here, overwrite-only mode has
             * no swap state.
             */
            assert(0);
#else
            /* Determine the type of swap operation being resumed from the
             * `swap-type` trailer field.
             */
            rc = boot_complete_partial_swap(state, bs);
            assert(rc == 0);
#endif
            /* Attempt to read an image header from each slot. Ensure that
             * image headers in slots are aligned with headers in boot_data.
             */
            rc = boot_read_image_headers(state, false, bs);
            assert(rc == 0);

            /* Swap has finished set to NONE */
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
        } else {
            /* There was no partial swap, determine swap type. */
            if (bs->swap_type == BOOT_SWAP_TYPE_NONE) {
                BOOT_SWAP_TYPE(state) = boot_validated_swap_type(state, bs);
            } else {
                FIH_CALL(boot_validate_slot, fih_rc,
                         state, BOOT_SECONDARY_SLOT, bs);
                if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
                    BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_FAIL;
                } else {
                    BOOT_SWAP_TYPE(state) = bs->swap_type;
                }
            }

#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
            boot_review_image_swap_types(state, false);
#endif

/* Below code introduces additional primary slot validation and is not needed if
 * initial image in secondary slot is complete (inc. header and magic
 * installation). It is then removed.
 */
#if 0
#ifdef MCUBOOT_BOOTSTRAP
            if (BOOT_SWAP_TYPE(state) == BOOT_SWAP_TYPE_NONE) {
                /* Header checks are done first because they are
                 * inexpensive. Since overwrite-only copies starting from
                 * offset 0, if interrupted, it might leave a valid header
                 * magic, so also run validation on the primary slot to be
                 * sure it's not OK.
                 */
                rc = boot_check_header_erased(state, BOOT_PRIMARY_SLOT);
                FIH_CALL(boot_validate_slot, fih_rc,
                         state, BOOT_PRIMARY_SLOT, bs);

                if (rc == 0 || fih_not_eq(fih_rc, FIH_SUCCESS)) {

                    rc = (boot_img_hdr(state, BOOT_SECONDARY_SLOT)->ih_magic == IMAGE_MAGIC) ? 1: 0;
                    FIH_CALL(boot_validate_slot, fih_rc,
                             state, BOOT_SECONDARY_SLOT, bs);

                    if (rc == 1 && fih_eq(fih_rc, FIH_SUCCESS)) {
                        /* Set swap type to REVERT to overwrite the primary
                         * slot with the image contained in secondary slot
                         * and to trigger the explicit setting of the
                         * image_ok flag.
                         */
                        BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_REVERT;
                    }
                }
            }
#endif
#endif
        }
    } else {
        /* In that case if slots are not compatible. */
        BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_NONE;
    }
#endif
}

fih_int
context_boot_go(struct boot_loader_state *state, struct boot_rsp *rsp)
{
    size_t slot;
    struct boot_status bs;
    int rc = -1;
    fih_int fih_rc = FIH_FAILURE;
    int fa_id;
    int image_index;
#if defined (MCUBOOT_HW_ROLLBACK_PROT) && !defined(MCUBOOT_OVERWRITE_ONLY)
    struct boot_swap_state swap_state;
#endif /* MCUBOOT_HW_ROLLBACK_PROT */
#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
    bool has_upgrade;
#endif /* BOOT_IMAGE_NUMBER > 1 */
    uint32_t security_counter_updated;

    /* The array of slot sectors are defined here (as opposed to file scope) so
     * that they don't get allocated for non-boot-loader apps.  This is
     * necessary because the gcc option "-fdata-sections" doesn't seem to have
     * any effect in older gcc versions (e.g., 4.8.4).
     */
    TARGET_STATIC boot_sector_t primary_slot_sectors[BOOT_IMAGE_NUMBER][BOOT_MAX_IMG_SECTORS];
#if !defined(MCUBOOT_PRIMARY_ONLY)
    TARGET_STATIC boot_sector_t secondary_slot_sectors[BOOT_IMAGE_NUMBER][BOOT_MAX_IMG_SECTORS];
#endif
#if MCUBOOT_SWAP_USING_SCRATCH
    TARGET_STATIC boot_sector_t scratch_sectors[BOOT_MAX_IMG_SECTORS];
#endif

    memset(state, 0, sizeof(struct boot_loader_state));
#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
    has_upgrade = false;
#endif /* (BOOT_IMAGE_NUMBER > 1) */



    /* Iterate over all the images. By the end of the loop the swap type has
     * to be determined for each image and all aborted swaps have to be
     * completed.
     */
    IMAGES_ITER(BOOT_CURR_IMG(state)) {

#if defined(MCUBOOT_ENC_IMAGES) && ((BOOT_IMAGE_NUMBER > 1) || defined(MCUBOOT_ENC_SECURITY_CNT))
        /* The keys used for encryption may no longer be valid (could belong to
         * another images). Therefore, mark them as invalid to force their reload
         * by boot_enc_load().
         */
        boot_enc_zeroize(BOOT_CURR_ENC(state));
#endif

        image_index = BOOT_CURR_IMG(state);

        BOOT_IMG(state, BOOT_PRIMARY_SLOT).sectors =
            primary_slot_sectors[image_index];
#if !defined(MCUBOOT_PRIMARY_ONLY)
        BOOT_IMG(state, BOOT_SECONDARY_SLOT).sectors =
            secondary_slot_sectors[image_index];
#endif
#if MCUBOOT_SWAP_USING_SCRATCH
        state->scratch.sectors = scratch_sectors;
#endif

        /* Open primary and secondary image areas for the duration
         * of this call.
         */
        for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
            fa_id = flash_area_id_from_multi_image_slot(image_index, slot);
            rc = flash_area_open(fa_id, &BOOT_IMG_AREA(state, slot));
            assert(rc == 0);
        }
#if MCUBOOT_SWAP_USING_SCRATCH
        rc = flash_area_open(FLASH_AREA_IMAGE_SCRATCH,
                             &BOOT_SCRATCH_AREA(state));
        assert(rc == 0);
#endif

        /* Determine swap type and complete swap if it has been aborted. */
        boot_prepare_image_for_update(state, &bs);
#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
        if (BOOT_IS_UPGRADE(BOOT_SWAP_TYPE(state))) {
            has_upgrade = true;
        }
#endif
    }

#if defined(MCUBOOT_HW_ROLLBACK_PROT) && !defined(MCUBOOT_OVERWRITE_ONLY)
    /* Iterate over all the images. At this point there are no aborted swaps
     * and the swap types are determined for each image. By the end of the loop,
     * the security counters will be updated.
     * If a security counter update is performed, the system must reset,
     * because swap type has to be re-evaluated.
     */
    IMAGES_ITER(BOOT_CURR_IMG(state)) {
        /* Update the stored security counter with the active image's security
         * counter value. It will only be updated if the new security counter is
         * greater than the stored value, and if the active image is confirmed:
         * image has marked itself "OK" (the image_ok flag has been set).
         * This way a "revert" can be performed when it's necessary.
         */
        /* Get active image trailer state: primary slot confirmed? */
        rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(BOOT_CURR_IMG(state)),
                                        &swap_state);
        assert(rc == 0);

        /* Check if primary slot contains a confirmed image */
        if ((swap_state.image_ok != BOOT_FLAG_UNSET) &&
            (boot_is_header_valid(boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                  BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT)))) {
            /* Image already confirmed. */
            rc = boot_update_security_counter(
                                    BOOT_CURR_IMG(state),
                                    BOOT_PRIMARY_SLOT,
                                    boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                    &security_counter_updated);
            if (rc != 0) {
                BOOT_LOG_ERR("Security counter update failed after image "
                             "validation.");
                goto out;
            }

            /* If security cnt has changed, then reset to start again mcuboot
             * process from start with up to date NVCounters */
            if (security_counter_updated) {
                BOOT_LOG_INF("Security counter updated. Reset device");
                NVIC_SystemReset();
            }

        }
    }
#endif /* MCUBOOT_HW_ROLLBACK_PROT && !MCUBOOT_OVERWRITE_ONLY */

#if (BOOT_IMAGE_NUMBER > 1) && !defined(MCUBOOT_PRIMARY_ONLY)
    if (has_upgrade) {
        /* Iterate over all the images and verify whether the image dependencies
         * are all satisfied and update swap type if necessary.
         */
        rc = boot_verify_dependencies(state);
        if (rc != 0) {
            /*
             * It was impossible to upgrade because the expected dependency version
             * was not available. Here we already changed the swap_type so that
             * instead of asserting the bootloader, we continue and no upgrade is
             * performed.
             */
            rc = 0;
        }
    }
#endif

    /* Iterate over all the images. At this point there are no aborted swaps
     * and the swap types are determined for each image. By the end of the loop
     * all required update operations will have been finished.
     */
    IMAGES_ITER(BOOT_CURR_IMG(state)) {

#if (BOOT_IMAGE_NUMBER > 1) || defined(MCUBOOT_ENC_SECURITY_CNT)
#ifdef MCUBOOT_ENC_IMAGES
        /* The keys used for encryption may no longer be valid (could belong to
         * another images). Therefore, mark them as invalid to force their reload
         * by boot_enc_load().
         */
        boot_enc_zeroize(BOOT_CURR_ENC(state));
#endif /* MCUBOOT_ENC_IMAGES */

        /* Indicate that swap is not aborted */
        boot_status_reset(&bs);
#endif /* (BOOT_IMAGE_NUMBER > 1) */

        /* Set the previously determined swap type */
        bs.swap_type = BOOT_SWAP_TYPE(state);

        switch (BOOT_SWAP_TYPE(state)) {
        case BOOT_SWAP_TYPE_NONE:
            break;

        case BOOT_SWAP_TYPE_TEST:          /* fallthrough */
        case BOOT_SWAP_TYPE_PERM:          /* fallthrough */
        case BOOT_SWAP_TYPE_REVERT:
            rc = boot_perform_update(state, &bs);
            assert(rc == 0);
            break;

        case BOOT_SWAP_TYPE_FAIL:
            /* The image in secondary slot was invalid and is now erased. Ensure
             * we don't try to boot into it again on the next reboot. Do this by
             * pretending we just reverted back to primary slot.
             */
#ifndef MCUBOOT_OVERWRITE_ONLY
            /* image_ok needs to be explicitly set to avoid a new revert. */
            rc = swap_set_image_ok(BOOT_CURR_IMG(state));
            if (rc != 0) {
                BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
            }
#endif /* !MCUBOOT_OVERWRITE_ONLY */
            break;

        default:
            BOOT_SWAP_TYPE(state) = BOOT_SWAP_TYPE_PANIC;
        }
#ifndef MCUBOOT_PRIMARY_ONLY
        if (BOOT_SWAP_TYPE(state) == BOOT_SWAP_TYPE_PANIC) {
            BOOT_LOG_ERR("panic!");
            assert(0);

            /* Loop forever... */
            FIH_PANIC;
        }
#endif
    }

    /* Iterate over all the images. At this point all required update operations
     * have finished. By the end of the loop each image in the primary slot will
     * have been re-validated.
     */
    BOOT_LOG_INF("Starting validation of primary slot(s)");
    IMAGES_ITER(BOOT_CURR_IMG(state)) {
        if (BOOT_SWAP_TYPE(state) != BOOT_SWAP_TYPE_NONE) {
            /* Attempt to read an image header from each slot. Ensure that image
             * headers in slots are aligned with headers in boot_data.
             */
            rc = boot_read_image_headers(state, false, &bs);
            if (rc != 0) {
                goto out;
            }
            /* Since headers were reloaded, it can be assumed we just performed
             * a swap or overwrite. Now the header info that should be used to
             * provide the data for the bootstrap, which previously was at
             * secondary slot, was updated to primary slot.
             */
        }
#ifdef MCUBOOT_VALIDATE_PRIMARY_SLOT
#if defined(MCUBOOT_DOUBLE_SIGN_VERIF) || defined(MCUBOOT_USE_HASH_REF)
        /* Enable image validation double check */
        ImageValidEnable = 1;

#endif /* MCUBOOT_DOUBLE_SIGN_VERIF || MCUBOOT_USE_HASH_REF */
        FIH_CALL(boot_validate_slot, fih_rc, state, BOOT_PRIMARY_SLOT, NULL);
        if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
            goto out;
        }
#if defined(MCUBOOT_DOUBLE_SIGN_VERIF) || defined(MCUBOOT_USE_HASH_REF)
        /* Disable image validation double check */
        ImageValidEnable = 0;
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF || MCUBOOT_USE_HASH_REF */
#else
        /* Even if we're not re-validating the primary slot, we could be booting
         * onto an empty flash chip. At least do a basic sanity check that
         * the magic number on the image is OK.
         */
        if (BOOT_IMG(state, BOOT_PRIMARY_SLOT).hdr.ih_magic != IMAGE_MAGIC) {
            BOOT_LOG_ERR("bad image magic 0x%lx; Image=%u", (unsigned long)
                         &boot_img_hdr(state,BOOT_PRIMARY_SLOT)->ih_magic,
                         BOOT_CURR_IMG(state));
            rc = BOOT_EBADIMAGE;
            goto out;
        }
#endif /* MCUBOOT_VALIDATE_PRIMARY_SLOT */

#if defined(OTFDEC_EXTERNAL_FLASH_ENABLE)
        /* If not already valid in internal flash, the otfdec symmetric key (for
         * OSPI flash execution) is read from image TLV, decrypted, and stored
         * in internal flash. It will be used at boot time.
         */
        if ((IS_OTFDEC_ENCRYPTED(boot_img_hdr(state, BOOT_PRIMARY_SLOT)))
            && (otfdec_is_key_valid() == 0)) {

            image_index = BOOT_CURR_IMG(state);

            rc = boot_enc_load(BOOT_CURR_ENC(state), image_index,
                    boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                    BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT), &bs);

            if (rc != 0) {
                rc = BOOT_EBADIMAGE;
                goto out;
            }
            if (otfdec_write_key(bs.enckey[0]) != 0) {
                rc = BOOT_EFLASH;
                goto out;
            }
        }
#endif /* OTFDEC_EXTERNAL_FLASH_ENABLE */

#if defined(MCUBOOT_HW_ROLLBACK_PROT) && defined(MCUBOOT_OVERWRITE_ONLY)
        /* Update the stored security counter with the active image's security
         * counter value. It will only be updated if the new security counter is
         * greater than the stored value.
         */
        rc = boot_update_security_counter(
                                BOOT_CURR_IMG(state),
                                BOOT_PRIMARY_SLOT,
                                boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                &security_counter_updated);
        if (rc != 0) {
            BOOT_LOG_ERR("Security counter update failed after image "
                         "validation.");
            goto out;
        }
#endif /* MCUBOOT_HW_ROLLBACK_PROT && MCUBOOT_OVERWRITE_ONLY*/

#ifdef MCUBOOT_MEASURED_BOOT
        rc = boot_save_boot_status(BOOT_CURR_IMG(state),
                                   boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                   BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT));
        if (rc != 0) {
            BOOT_LOG_ERR("Failed to add Image %u data to shared memory area",
                         BOOT_CURR_IMG(state));
            goto out;
        }
#endif /* MCUBOOT_MEASURED_BOOT */

#ifdef MCUBOOT_DATA_SHARING
        rc = boot_save_shared_data(boot_img_hdr(state, BOOT_PRIMARY_SLOT),
                                   BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT));
        if (rc != 0) {
            BOOT_LOG_ERR("Failed to add data to shared memory area.");
            goto out;
        }
#endif /* MCUBOOT_DATA_SHARING */
    }

#if (BOOT_IMAGE_NUMBER > 1)
    /* Always boot from the primary slot of Image 0. */
    BOOT_CURR_IMG(state) = 0;
#endif

    rsp->br_flash_dev_id = BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT)->fa_device_id;
    rsp->br_image_off = boot_img_slot_off(state, BOOT_PRIMARY_SLOT);
    rsp->br_hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);


    /*
     * Since the boot_status struct stores plaintext encryption keys, reset
     * them here to avoid the possibility of jumping into an image that could
     * easily recover them.
     */
    memset(&bs, 0, sizeof(struct boot_status));

    rsp->br_flash_dev_id = BOOT_IMG_AREA(state, BOOT_PRIMARY_SLOT)->fa_device_id;
    rsp->br_image_off = boot_img_slot_off(state, BOOT_PRIMARY_SLOT);
    rsp->br_hdr = boot_img_hdr(state, BOOT_PRIMARY_SLOT);

    fih_rc = FIH_SUCCESS;

    IMAGES_ITER(BOOT_CURR_IMG(state)) {
#if MCUBOOT_SWAP_USING_SCRATCH
        flash_area_close(BOOT_SCRATCH_AREA(state));
#endif
        for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
            flash_area_close(BOOT_IMG_AREA(state, BOOT_NUM_SLOTS - 1 - slot));
        }
    }
out:
    if (rc) {
        fih_rc = fih_int_encode(rc);
    }

    FIH_RET(fih_rc);
}

fih_int
split_go(int loader_slot, int split_slot, void **entry)
{
    boot_sector_t *sectors;
    uintptr_t entry_val;
    int loader_flash_id;
    int split_flash_id;
    int rc;
    fih_int fih_rc = FIH_FAILURE;

    sectors = malloc(BOOT_MAX_IMG_SECTORS * 2 * sizeof *sectors);
    if (sectors == NULL) {
        FIH_RET(FIH_FAILURE);
    }
    BOOT_IMG(&boot_data, loader_slot).sectors = sectors + 0;
    BOOT_IMG(&boot_data, split_slot).sectors = sectors + BOOT_MAX_IMG_SECTORS;

    loader_flash_id = flash_area_id_from_image_slot(loader_slot);
    rc = flash_area_open(loader_flash_id,
                         &BOOT_IMG_AREA(&boot_data, loader_slot));
    assert(rc == 0);
    split_flash_id = flash_area_id_from_image_slot(split_slot);
    rc = flash_area_open(split_flash_id,
                         &BOOT_IMG_AREA(&boot_data, split_slot));
    assert(rc == 0);

    /* Determine the sector layout of the image slots and scratch area. */
    rc = boot_read_sectors(&boot_data);
    if (rc != 0) {
        rc = SPLIT_GO_ERR;
        goto done;
    }

    rc = boot_read_image_headers(&boot_data, true, NULL);
    if (rc != 0) {
        goto done;
    }

    /* Don't check the bootable image flag because we could really call a
     * bootable or non-bootable image.  Just validate that the image check
     * passes which is distinct from the normal check.
     */
    FIH_CALL(split_image_check, fih_rc,
             boot_img_hdr(&boot_data, split_slot),
             BOOT_IMG_AREA(&boot_data, split_slot),
             boot_img_hdr(&boot_data, loader_slot),
             BOOT_IMG_AREA(&boot_data, loader_slot));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto done;
    }

    entry_val = boot_img_slot_off(&boot_data, split_slot) +
                boot_img_hdr(&boot_data, split_slot)->ih_hdr_size;
    *entry = (void *) entry_val;
    rc = SPLIT_GO_OK;

done:
    flash_area_close(BOOT_IMG_AREA(&boot_data, split_slot));
    flash_area_close(BOOT_IMG_AREA(&boot_data, loader_slot));
    free(sectors);

    if (rc) {
        fih_rc = fih_int_encode(rc);
    }

    FIH_RET(fih_rc);
}

#else /* MCUBOOT_DIRECT_XIP || MCUBOOT_RAM_LOAD */

/**
 * Iterates over all slots and determines which contain a firmware image.
 *
 * @param state          Boot loader status information.
 * @param slot_usage     Pointer to an array, which aim is to carry information
 *                       about slots that contain an image. After return the
 *                       corresponding array elements are set to a non-zero
 *                       value if the given slots are in use (contain a firmware
 *                       image), otherwise they are set to zero.
 * @param slot_cnt       The number of slots, which can contain firmware images.
 *                       (Equal to or smaller than the size of the
 *                       slot_usage array.)
 *
 * @return               The number of found images.
 */
static uint32_t
boot_get_slot_usage(struct boot_loader_state *state, uint8_t slot_usage[],
                    uint32_t slot_cnt)
{
    struct image_header *hdr = NULL;
    uint32_t image_cnt = 0;
    uint32_t slot;

    memset(slot_usage, 0, slot_cnt);

    for (slot = 0; slot < slot_cnt; slot++) {
        hdr = boot_img_hdr(state, slot);

        if (boot_is_header_valid(hdr, BOOT_IMG_AREA(state, slot))) {
            slot_usage[slot] = 1;
            image_cnt++;
            BOOT_LOG_IMAGE_INFO(slot, hdr);
        } else {
            BOOT_LOG_INF("%s slot: Image not found", (slot == BOOT_PRIMARY_SLOT)
                         ? "Primary" : "Secondary");
        }
    }

    return image_cnt;
}

#ifdef MCUBOOT_DIRECT_XIP_REVERT
/**
 * Checks whether the image in the given slot was previously selected to run.
 * Erases the image if it was selected but its execution failed, otherwise marks
 * it as selected if it has not been before.
 *
 * @param state     Image metadata from the image trailer. This function fills
 *                  this struct with the data read from the image trailer.
 * @param slot      Image slot number.
 *
 * @return          0 on success; nonzero on failure.
 */
static int
boot_select_or_erase(struct boot_swap_state *state, uint32_t slot)
{
    const struct flash_area *fap;
    int fa_id;
    int rc;

    fa_id = flash_area_id_from_image_slot(slot);
    rc = flash_area_open(fa_id, &fap);
    assert(rc == 0);

    memset(state, 0, sizeof(struct boot_swap_state));
    rc = boot_read_swap_state(fap, state);
    assert(rc == 0);

    if (state->magic != BOOT_MAGIC_GOOD ||
        (state->copy_done == BOOT_FLAG_SET &&
         state->image_ok  != BOOT_FLAG_SET)) {
        /*
         * A reboot happened without the image being confirmed at
         * runtime or its trailer is corrupted/invalid. Erase the image
         * to prevent it from being selected again on the next reboot.
         */
        BOOT_LOG_DBG("Erasing faulty image in the %s slot.",
                     (slot == BOOT_PRIMARY_SLOT) ? "primary" : "secondary");
        rc = flash_area_erase(fap, 0, fap->fa_size);
        assert(rc == 0);

        flash_area_close(fap);
        rc = -1;
    } else {
        if (state->copy_done != BOOT_FLAG_SET) {
            if (state->copy_done == BOOT_FLAG_BAD) {
                BOOT_LOG_DBG("The copy_done flag had an unexpected value. Its "
                             "value was neither 'set' nor 'unset', but 'bad'.");
            }
            /*
             * Set the copy_done flag, indicating that the image has been
             * selected to boot. It can be set in advance, before even
             * validating the image, because in case the validation fails, the
             * entire image slot will be erased (including the trailer).
             */
            rc = boot_write_copy_done(fap);
            if (rc != 0) {
                BOOT_LOG_WRN("Failed to set copy_done flag of the image in "
                             "the %s slot.", (slot == BOOT_PRIMARY_SLOT) ?
                             "primary" : "secondary");
                rc = 0;
            }
        }
        flash_area_close(fap);
    }

    return rc;
}
#endif /* MCUBOOT_DIRECT_XIP_REVERT */

#ifdef MCUBOOT_RAM_LOAD

#if !defined(IMAGE_EXECUTABLE_RAM_START) || !defined(IMAGE_EXECUTABLE_RAM_SIZE)
#error "Platform MUST define executable RAM bounds in case of RAM_LOAD"
#endif

/**
 * Verifies that the image in a slot will be loaded within the predefined bounds
 * that are allowed to be used by executable images.
 *
 * @param img_dst         The address to which the image is going to be copied.
 * @param img_sz          The size of the image.
 *
 * @return                0 on success; nonzero on failure.
 */
static int
boot_verify_ram_load_address(uint32_t img_dst, uint32_t img_sz)
{
    uint32_t img_end_addr;

    if (img_dst < IMAGE_EXECUTABLE_RAM_START) {
        return BOOT_EBADIMAGE;
    }

    if (!boot_u32_safe_add(&img_end_addr, img_dst, img_sz)) {
        return BOOT_EBADIMAGE;
    }

    if (img_end_addr > (IMAGE_EXECUTABLE_RAM_START +
                        IMAGE_EXECUTABLE_RAM_SIZE)) {
        return BOOT_EBADIMAGE;
    }

    return 0;
}

/**
 * Copies an image from a slot in the flash to an SRAM address.
 *
 * @param  slot     The flash slot of the image to be copied to SRAM.
 * @param  img_dst  The address at which the image needs to be copied to
 *                  SRAM.
 * @param  img_sz   The size of the image that needs to be copied to SRAM.
 *
 * @return          0 on success; nonzero on failure.
 */
static int
boot_copy_image_to_sram(int slot, uint32_t img_dst, uint32_t img_sz)
{
    int rc;
    const struct flash_area *fap_src = NULL;

    rc = flash_area_open(flash_area_id_from_image_slot(slot), &fap_src);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    /* Direct copy from flash to its new location in SRAM. */
    rc = flash_area_read(fap_src, 0, (void *)img_dst, img_sz);
    if (rc != 0) {
        BOOT_LOG_INF("Error whilst copying image from Flash to SRAM: %d", rc);
    }

    flash_area_close(fap_src);

    return rc;
}

/**
 * Copies an image from a slot in the flash to an SRAM address. The load
 * address and image size is extracted from the image header.
 *
 * @param  state    Boot loader status information.
 * @param  slot     The flash slot of the image to be copied to SRAM.
 * @param  hdr      Pointer to the image header structure of the image
 * @param  img_dst  Pointer to the address at which the image needs to be
 *                  copied to SRAM.
 * @param  img_sz   Pointer to the size of the image that needs to be
 *                  copied to SRAM.
 *
 * @return          0 on success; nonzero on failure.
 */
static int
boot_load_image_to_sram(struct boot_loader_state *state, uint32_t slot,
                        struct image_header *hdr, uint32_t *img_dst,
                        uint32_t *img_sz)
{
    int rc;

    if (hdr->ih_flags & IMAGE_F_RAM_LOAD) {

        *img_dst = hdr->ih_load_addr;

        rc = boot_read_image_size(state, slot, img_sz);
        if (rc != 0) {
            return rc;
        }

        rc = boot_verify_ram_load_address(*img_dst, *img_sz);
        if (rc != 0) {
            BOOT_LOG_INF("Image RAM load address 0x%lx is invalid.", *img_dst);
            return rc;
        }

        /* Copy image to the load address from where it currently resides in
         * flash.
         */
        rc = boot_copy_image_to_sram(slot, *img_dst, *img_sz);
        if (rc != 0) {
            BOOT_LOG_INF("RAM loading to 0x%lx is failed.", *img_dst);
        } else {
            BOOT_LOG_INF("RAM loading to 0x%lx is succeeded.", *img_dst);
        }
    } else {
        /* Only images that support IMAGE_F_RAM_LOAD are allowed if
         * MCUBOOT_RAM_LOAD is set.
         */
        rc = BOOT_EBADIMAGE;
    }

    return rc;
}

/**
 * Removes an image from SRAM, by overwriting it with zeros.
 *
 * @param  img_dst  The address of the image that needs to be removed from
 *                  SRAM.
 * @param  img_sz   The size of the image that needs to be removed from
 *                  SRAM.
 *
 * @return          0 on success; nonzero on failure.
 */
static inline int
boot_remove_image_from_sram(uint32_t img_dst, uint32_t img_sz)
{
    BOOT_LOG_INF("Removing image from SRAM at address 0x%lx", img_dst);
    memset((void*)img_dst, 0, img_sz);

    return 0;
}
#endif /* MCUBOOT_RAM_LOAD */

fih_int
context_boot_go(struct boot_loader_state *state, struct boot_rsp *rsp)
{
    struct image_header *hdr = NULL;
    struct image_header *selected_image_header = NULL;
    uint8_t slot_usage[BOOT_NUM_SLOTS];
    uint32_t selected_slot;
    uint32_t slot;
    uint32_t img_cnt;
    uint32_t i;
    int fa_id;
    int rc;
#ifdef MCUBOOT_RAM_LOAD
    uint32_t img_dst;
    uint32_t img_sz;
    uint32_t img_loaded = 0;
#endif /* MCUBOOT_RAM_LOAD */
#ifdef MCUBOOT_DIRECT_XIP_REVERT
    struct boot_swap_state slot_state;
#endif /* MCUBOOT_DIRECT_XIP_REVERT */
    fih_int fih_rc = FIH_FAILURE;

    memset(state, 0, sizeof(struct boot_loader_state));

    /* Open primary and secondary image areas for the duration
     * of this call.
     */
    for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
        fa_id = flash_area_id_from_image_slot(slot);
        rc = flash_area_open(fa_id, &BOOT_IMG_AREA(state, slot));
        assert(rc == 0);
    }

    /* Attempt to read an image header from each slot. */
    rc = boot_read_image_headers(state, false, NULL);
    if (rc != 0) {
        BOOT_LOG_WRN("Failed reading image headers.");
        goto out;
    }

    img_cnt = boot_get_slot_usage(state, slot_usage,
                                  sizeof(slot_usage)/sizeof(slot_usage[0]));

    if (img_cnt) {
        /* Select the newest and valid image. */
        for (i = 0; i < img_cnt; i++) {
            selected_slot = 0;

            /* Iterate over all the slots that are in use (contain an image)
             * and select the one that holds the newest image.
             */
            for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
                if (slot_usage[slot]) {
                    hdr = boot_img_hdr(state, slot);
                    if (selected_image_header != NULL) {
                        rc = boot_version_cmp(&hdr->ih_ver,
                                              &selected_image_header->ih_ver);
                        if (rc < 1) {
                            /* The version of the image being examined wasn't
                             * greater than the currently selected image's
                             * version.
                             */
                            continue;
                        }
                    }
                    selected_slot = slot;
                    selected_image_header = hdr;
                }
            }

#ifdef MCUBOOT_DIRECT_XIP_REVERT
            rc = boot_select_or_erase(&slot_state, selected_slot);
            if (rc != 0) {
                /* The selected image slot has been erased. */
                slot_usage[selected_slot] = 0;
                selected_image_header = NULL;
                continue;
            }
#endif /* MCUBOOT_DIRECT_XIP_REVERT */

#ifdef MCUBOOT_RAM_LOAD
            /* Image is first loaded to RAM and authenticated there in order to
             * prevent TOCTOU attack during image copy. This could be applied
             * when loading images from external (untrusted) flash to internal
             * (trusted) RAM and image is authenticated before copying.
             */
            rc = boot_load_image_to_sram(state, selected_slot,
                                         selected_image_header, &img_dst,
                                         &img_sz);
            if (rc != 0 ) {
                /* Image loading failed try the next one. */
                slot_usage[selected_slot] = 0;
                selected_image_header = NULL;
                continue;
            } else {
                img_loaded = 1;
            }
#endif /* MCUBOOT_RAM_LOAD */
            FIH_CALL(boot_validate_slot, fih_rc, state, selected_slot, NULL);
            if (fih_eq(fih_rc, FIH_SUCCESS)) {
                /* If a valid image is found then there is no reason to check
                 * the rest of the images, as each of them has a smaller version
                 * number.
                 */
                break;
            }
#ifdef MCUBOOT_RAM_LOAD
            else if (img_loaded) {
                /* If an image is found to be invalid then it is removed from
                 * RAM to prevent it being a shellcode vector.
                 */
                boot_remove_image_from_sram(img_dst, img_sz);
                img_loaded = 0;
            }
#endif /* MCUBOOT_RAM_LOAD */
            /* The selected image is invalid, mark its slot as "unused"
             * and start over.
             */
            slot_usage[selected_slot] = 0;
            selected_image_header = NULL;
        }

        if (fih_not_eq(fih_rc, FIH_SUCCESS) || (selected_image_header == NULL)) {
            /* If there was no valid image at all */
            goto out;
        }

#ifdef MCUBOOT_HW_ROLLBACK_PROT
        /* Update the stored security counter with the newer (active) image's
         * security counter value.
         */
#ifdef MCUBOOT_DIRECT_XIP_REVERT
        /* When the 'revert' mechanism is enabled in direct-xip mode, the
         * security counter can be increased only after reboot, if the image
         * has been confirmed at runtime (the image_ok flag has been set).
         * This way a 'revert' can be performed when it's necessary.
         */
        if (slot_state.image_ok == BOOT_FLAG_SET) {
#endif
            rc = boot_update_security_counter(0, selected_slot,
                                              selected_image_header);
            if (rc != 0) {
                BOOT_LOG_ERR("Security counter update failed after image "
                             "validation.");
                goto out;
            }
#ifdef MCUBOOT_DIRECT_XIP_REVERT
        }
#endif
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

#ifdef MCUBOOT_MEASURED_BOOT
        rc = boot_save_boot_status(0, selected_image_header,
                                   BOOT_IMG_AREA(state, selected_slot));
        if (rc != 0) {
            BOOT_LOG_ERR("Failed to add image data to shared area");
        }
#endif /* MCUBOOT_MEASURED_BOOT */

#ifdef MCUBOOT_DATA_SHARING
        rc = boot_save_shared_data(selected_image_header,
                                   BOOT_IMG_AREA(state, selected_slot));
        if (rc != 0) {
            BOOT_LOG_ERR("Failed to add data to shared memory area.");
        }
#endif /* MCUBOOT_DATA_SHARING */

        BOOT_LOG_INF("Booting image from the %s slot",
                     (selected_slot == BOOT_PRIMARY_SLOT) ?
                     "primary" : "secondary");

        rsp->br_flash_dev_id =
            BOOT_IMG_AREA(state, selected_slot)->fa_device_id;
        rsp->br_image_off = boot_img_slot_off(state, selected_slot);
        rsp->br_hdr = selected_image_header;
    } else {
        /* No candidate image available */
        rc = BOOT_EBADIMAGE;
        goto out;
    }

out:
   for (slot = 0; slot < BOOT_NUM_SLOTS; slot++) {
       flash_area_close(BOOT_IMG_AREA(state, BOOT_NUM_SLOTS - 1 - slot));
   }

   if (rc) {
       fih_rc = fih_int_encode(rc);
   }

   FIH_RET(fih_rc);
}
#endif /* MCUBOOT_DIRECT_XIP || MCUBOOT_RAM_LOAD */

/**
 * Prepares the booting process. This function moves images around in flash as
 * appropriate, and tells you what address to boot from.
 *
 * @param rsp                   On success, indicates how booting should occur.
 *
 * @return                      FIH_SUCCESS on success; nonzero on failure.
 */
fih_int
boot_go(struct boot_rsp *rsp)
{
    fih_int fih_rc = FIH_FAILURE;
#ifdef FLOW_CONTROL
    /* Check Flow control state */
    FLOW_CONTROL_CHECK(uFlowProtectValue, FLOW_CTRL_STAGE_2);
#endif
    FIH_CALL(context_boot_go, fih_rc, &boot_data, rsp);
    FIH_RET(fih_rc);
}
