/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017-2020 Linaro LTD
 * Copyright (c) 2017-2019 JUUL Labs
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

#ifndef H_BOOTUTIL_PRIV_
#define H_BOOTUTIL_PRIV_

#include <string.h>

#include "sysflash/sysflash.h"

#include <flash_map_backend/flash_map_backend.h>

#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "bootutil/fault_injection_hardening.h"
#include "mcuboot_config/mcuboot_config.h"

#ifdef MCUBOOT_ENC_IMAGES
#include "bootutil/enc_key.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCUBOOT_HAVE_ASSERT_H
#include "mcuboot_config/mcuboot_assert.h"
#else
#include <assert.h>
#define ASSERT assert
#endif

struct flash_area;

#define BOOT_EFLASH      1
#define BOOT_EFILE       2
#define BOOT_EBADIMAGE   3
#define BOOT_EBADVECT    4
#define BOOT_EBADSTATUS  5
#define BOOT_ENOMEM      6
#define BOOT_EBADARGS    7
#define BOOT_EBADVERSION 8

#if defined(MCUBOOT_TMPBUF_SZ)
#define BOOT_TMPBUF_SZ  MCUBOOT_TMPBUF_SZ
#else
#define BOOT_TMPBUF_SZ  0x1000
#endif

/** Number of image slots in flash; currently limited to two. */
#if defined(MCUBOOT_PRIMARY_ONLY)
#define BOOT_NUM_SLOTS                  1
#else
#define BOOT_NUM_SLOTS                  2
#endif
#if (defined(MCUBOOT_OVERWRITE_ONLY) + \
     defined(MCUBOOT_SWAP_USING_MOVE) + \
     defined(MCUBOOT_DIRECT_XIP) + \
     defined(MCUBOOT_RAM_LOAD)) > 1
#error "Please enable only one of MCUBOOT_OVERWRITE_ONLY, MCUBOOT_SWAP_USING_MOVE, MCUBOOT_DIRECT_XIP or MCUBOOT_RAM_LOAD"
#endif

#if !defined(MCUBOOT_OVERWRITE_ONLY) && \
    !defined(MCUBOOT_SWAP_USING_MOVE) && \
    !defined(MCUBOOT_DIRECT_XIP) && \
    !defined(MCUBOOT_RAM_LOAD) 
#define MCUBOOT_SWAP_USING_SCRATCH 1
#endif

#define BOOT_STATUS_OP_MOVE     1
#define BOOT_STATUS_OP_SWAP     2

/*
 * Maintain state of copy progress.
 */
struct boot_status {
    uint32_t idx;         /* Which area we're operating on */
    uint8_t state;        /* Which part of the swapping process are we at */
    uint8_t op;           /* What operation are we performing? */
    uint8_t use_scratch;  /* Are status bytes ever written to scratch? */
    uint8_t swap_type;    /* The type of swap in effect */
    uint32_t swap_size;   /* Total size of swapped image */
#ifdef MCUBOOT_ENC_IMAGES
    uint8_t enckey[BOOT_NUM_SLOTS][BOOT_ENC_KEY_SIZE];
#if MCUBOOT_SWAP_SAVE_ENCTLV
    uint8_t enctlv[BOOT_NUM_SLOTS][BOOT_ENC_TLV_ALIGN_SIZE];
#endif
#endif
    int source;           /* Which slot contains swap status metadata */
};

#define BOOT_MAGIC_GOOD     1
#define BOOT_MAGIC_BAD      2
#define BOOT_MAGIC_UNSET    3
#define BOOT_MAGIC_ANY      4  /* NOTE: control only, not dependent on sector */
#define BOOT_MAGIC_NOTGOOD  5  /* NOTE: control only, not dependent on sector */

/*
 * NOTE: leave BOOT_FLAG_SET equal to one, this is written to flash!
 */
#define BOOT_FLAG_SET       1
#define BOOT_FLAG_BAD       2
#define BOOT_FLAG_UNSET     3
#define BOOT_FLAG_ANY       4  /* NOTE: control only, not dependent on sector */

#define BOOT_STATUS_IDX_0   1

#define BOOT_STATUS_STATE_0 1
#define BOOT_STATUS_STATE_1 2
#define BOOT_STATUS_STATE_2 3

/**
 * End-of-image slot structure.
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                                                               ~
 *  ~    Swap status (BOOT_MAX_IMG_SECTORS * min-write-size * 3)    ~
 *  ~                                                               ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                 Encryption key 0 (16 octets) [*]              |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                 Encryption key 1 (16 octets) [*]              |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      Swap size (4 octets)                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Swap info   |           0xff padding (7 octets)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Copy done   |           0xff padding (7 octets)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Image OK    |           0xff padding (7 octets)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       MAGIC (16 octets)                       |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * [*]: Only present if the encryption option is enabled
 *      (`MCUBOOT_ENC_IMAGES`).
 */

extern const uint32_t boot_img_magic[4];

struct boot_swap_state {
    uint8_t magic;      /* One of the BOOT_MAGIC_[...] values. */
    uint8_t swap_type;  /* One of the BOOT_SWAP_TYPE_[...] values. */
    uint8_t copy_done;  /* One of the BOOT_FLAG_[...] values. */
    uint8_t image_ok;   /* One of the BOOT_FLAG_[...] values. */
    uint8_t image_num;  /* Boot status belongs to this image */
};

#ifdef MCUBOOT_IMAGE_NUMBER
#define BOOT_IMAGE_NUMBER          MCUBOOT_IMAGE_NUMBER
#else
#define BOOT_IMAGE_NUMBER          1
#endif

_Static_assert(BOOT_IMAGE_NUMBER > 0, "Invalid value for BOOT_IMAGE_NUMBER");

#if !defined(MCUBOOT_DIRECT_XIP) && !defined(MCUBOOT_RAM_LOAD)
#define ARE_SLOTS_EQUIVALENT()    0
#else
#define ARE_SLOTS_EQUIVALENT()    1

#if (BOOT_IMAGE_NUMBER != 1)
#error "The MCUBOOT_DIRECT_XIP and MCUBOOT_RAM_LOAD mode only supports single-image boot (MCUBOOT_IMAGE_NUMBER=1)."
#endif
#ifdef MCUBOOT_ENC_IMAGES
#error "Image encryption (MCUBOOT_ENC_IMAGES) is not supported when MCUBOOT_DIRECT_XIP or MCUBOOT_RAM_LOAD mode is selected."
#endif
#endif /* MCUBOOT_DIRECT_XIP || MCUBOOT_RAM_LOAD */

#define BOOT_MAX_IMG_SECTORS       MCUBOOT_MAX_IMG_SECTORS

/*
 * Extract the swap type and image number from image trailers's swap_info
 * filed.
 */
#define BOOT_GET_SWAP_TYPE(swap_info)    ((swap_info) & 0x0F)
#define BOOT_GET_IMAGE_NUM(swap_info)    ((swap_info) >> 4)

/* Construct the swap_info field from swap type and image number */
#define BOOT_SET_SWAP_INFO(swap_info, image, type)  {                          \
                                                    assert((image) < 0xF);     \
                                                    assert((type)  < 0xF);     \
                                                    (swap_info) = (image) << 4 \
                                                                | (type);      \
                                                    }

#define BOOT_LOG_IMAGE_INFO(slot, hdr)                                    \
    BOOT_LOG_INF("%-9s slot: version=%u.%u.%u+%u",                        \
                 ((slot) == BOOT_PRIMARY_SLOT) ? "Primary" : "Secondary", \
                 (hdr)->ih_ver.iv_major,                                  \
                 (hdr)->ih_ver.iv_minor,                                  \
                 (hdr)->ih_ver.iv_revision,                               \
                 (hdr)->ih_ver.iv_build_num)

/*
 * The current flashmap API does not check the amount of space allocated when
 * loading sector data from the flash device, allowing for smaller counts here
 * would most surely incur in overruns.
 *
 * TODO: make flashmap API receive the current sector array size.
 */
/*#if !defined(MCUBOOT_PRIMARY_ONLY)
#if BOOT_MAX_IMG_SECTORS < 32
#error "Too few sectors, please increase BOOT_MAX_IMG_SECTORS to at least 32"
#endif
#endif*/

#if MCUBOOT_SWAP_USING_MOVE
#define BOOT_STATUS_MOVE_STATE_COUNT    1
#define BOOT_STATUS_SWAP_STATE_COUNT    2
#define BOOT_STATUS_STATE_COUNT         (BOOT_STATUS_MOVE_STATE_COUNT + BOOT_STATUS_SWAP_STATE_COUNT)
#else
#define BOOT_STATUS_STATE_COUNT         3
#endif

/** Maximum number of image sectors supported by the bootloader. */
#define BOOT_STATUS_MAX_ENTRIES         BOOT_MAX_IMG_SECTORS

#define BOOT_PRIMARY_SLOT               0
#define BOOT_SECONDARY_SLOT             1

#define BOOT_STATUS_SOURCE_NONE         0
#define BOOT_STATUS_SOURCE_SCRATCH      1
#define BOOT_STATUS_SOURCE_PRIMARY_SLOT 2

#define BOOT_MAGIC_SZ (sizeof boot_img_magic)

/**
 * Compatibility shim for flash sector type.
 *
 * This can be deleted when flash_area_to_sectors() is removed.
 */
#ifdef MCUBOOT_USE_FLASH_AREA_GET_SECTORS
typedef struct flash_sector boot_sector_t;
#else
typedef struct flash_area boot_sector_t;
#endif

/** Private state maintained during boot. */
struct boot_loader_state {
    struct {
        struct image_header hdr;
        const struct flash_area *area;
        boot_sector_t *sectors;
        size_t num_sectors;
    } imgs[BOOT_IMAGE_NUMBER][BOOT_NUM_SLOTS];

#if MCUBOOT_SWAP_USING_SCRATCH
    struct {
        const struct flash_area *area;
        boot_sector_t *sectors;
        size_t num_sectors;
    } scratch;
#endif

    uint8_t swap_type[BOOT_IMAGE_NUMBER];
    uint32_t write_sz;

#if defined(MCUBOOT_ENC_IMAGES)
    struct enc_key_data enc[BOOT_IMAGE_NUMBER][BOOT_NUM_SLOTS];
#endif

#if (BOOT_IMAGE_NUMBER > 1)
    uint8_t curr_img_idx;
#endif
};

fih_int bootutil_verify_sig(uint8_t *hash, uint32_t hlen, uint8_t *sig,
                            size_t slen, uint8_t key_id);

fih_int boot_fih_memequal(const void *s1, const void *s2, size_t n);

int boot_magic_compatible_check(uint8_t tbl_val, uint8_t val);
uint32_t boot_status_sz(uint32_t min_write_sz);
uint32_t boot_trailer_sz(uint32_t min_write_sz);
int boot_status_entries(int image_index, const struct flash_area *fap);
uint32_t boot_status_off(const struct flash_area *fap);
uint32_t boot_magic_off(const struct flash_area *fap);
uint32_t boot_swap_info_off(const struct flash_area *fap);
int boot_read_swap_state(const struct flash_area *fap,
                         struct boot_swap_state *state);
int boot_read_swap_state_by_id(int flash_area_id,
                               struct boot_swap_state *state);
int boot_write_magic(const struct flash_area *fap);
int boot_write_status(const struct boot_loader_state *state, struct boot_status *bs);
int boot_write_copy_done(const struct flash_area *fap);
int boot_write_image_ok(const struct flash_area *fap);
int boot_write_swap_info(const struct flash_area *fap, uint8_t swap_type,
                         uint8_t image_num);
int boot_write_swap_size(const struct flash_area *fap, uint32_t swap_size);
int boot_read_swap_size(int image_index, uint32_t *swap_size);
int boot_slots_compatible(struct boot_loader_state *state);
uint32_t boot_status_internal_off(const struct boot_status *bs, int elem_sz);
int boot_read_image_header(struct boot_loader_state *state, int slot,
                           struct image_header *out_hdr, struct boot_status *bs);
int boot_copy_region(struct boot_loader_state *state,
                     const struct flash_area *fap_src,
                     const struct flash_area *fap_dst,
                     uint32_t off_src, uint32_t off_dst, uint32_t sz);
int boot_erase_region(const struct flash_area *fap, uint32_t off, uint32_t sz);
bool boot_status_is_reset(const struct boot_status *bs);

#ifdef MCUBOOT_ENC_IMAGES
int boot_write_enc_key(const struct flash_area *fap, uint8_t slot,
                       const struct boot_status *bs);
int boot_read_enc_key(int image_index, uint8_t slot, struct boot_status *bs);
#endif

/**
 * Checks that a buffer is erased according to what the erase value for the
 * flash device provided in `flash_area` is.
 *
 * @returns true if the buffer is erased; false if any of the bytes is not
 * erased, or when buffer is NULL, or when len == 0.
 */
#ifdef MCUBOOT_USE_MCE
bool bootutil_buffer_is_erased(uint32_t off,
                               const struct flash_area *area,
                               const void *buffer, size_t len);
#else /* not MCUBOOT_USE_MCE */
bool bootutil_buffer_is_erased(const struct flash_area *area,
                               const void *buffer, size_t len);
#endif /* MCUBOOT_USE_MCE */

/**
 * Safe (non-overflowing) uint32_t addition.  Returns true, and stores
 * the result in *dest if it can be done without overflow.  Otherwise,
 * returns false.
 */
static inline bool boot_u32_safe_add(uint32_t *dest, uint32_t a, uint32_t b)
{
    /*
     * "a + b <= UINT32_MAX", subtract 'b' from both sides to avoid
     * the overflow.
     */
    if (a > UINT32_MAX - b) {
        return false;
    } else {
        *dest = a + b;
        return true;
    }
}

/**
 * Safe (non-overflowing) uint16_t addition.  Returns true, and stores
 * the result in *dest if it can be done without overflow.  Otherwise,
 * returns false.
 */
static inline bool boot_u16_safe_add(uint16_t *dest, uint16_t a, uint16_t b)
{
    uint32_t tmp = a + b;
    if (tmp > UINT16_MAX) {
        return false;
    } else {
        *dest = tmp;
        return true;
    }
}

/*
 * Accessors for the contents of struct boot_loader_state.
 */

/* These are macros so they can be used as lvalues. */
#if (BOOT_IMAGE_NUMBER > 1)
#define BOOT_CURR_IMG(state) ((state)->curr_img_idx)
#else
#define BOOT_CURR_IMG(state) 0
#endif
#ifdef MCUBOOT_ENC_IMAGES
#define BOOT_CURR_ENC(state) ((state)->enc[BOOT_CURR_IMG(state)])
#else
#define BOOT_CURR_ENC(state) NULL
#endif
#define BOOT_IMG(state, slot) ((state)->imgs[BOOT_CURR_IMG(state)][(slot)])
#define BOOT_IMG_AREA(state, slot) (BOOT_IMG(state, slot).area)
#define BOOT_WRITE_SZ(state) ((state)->write_sz)
#define BOOT_SWAP_TYPE(state) ((state)->swap_type[BOOT_CURR_IMG(state)])
#define BOOT_TLV_OFF(hdr) ((hdr)->ih_hdr_size + (hdr)->ih_img_size)

#define BOOT_IS_UPGRADE(swap_type)             \
    (((swap_type) == BOOT_SWAP_TYPE_TEST) ||   \
     ((swap_type) == BOOT_SWAP_TYPE_REVERT) || \
     ((swap_type) == BOOT_SWAP_TYPE_PERM))
/* use for MCUBOOT_PRIMARY_ONLY */
#define BOOT_IMG_HDR(state) (&BOOT_IMG(state,0).hdr)

static inline struct image_header*
boot_img_hdr(struct boot_loader_state *state, size_t slot)
{
    return &BOOT_IMG(state, slot).hdr;
}

static inline size_t
boot_img_num_sectors(const struct boot_loader_state *state, size_t slot)
{
    return BOOT_IMG(state, slot).num_sectors;
}

/*
 * Offset of the slot from the beginning of the flash device.
 */
static inline uint32_t
boot_img_slot_off(struct boot_loader_state *state, size_t slot)
{
    return BOOT_IMG(state, slot).area->fa_off;
}

#ifndef MCUBOOT_USE_FLASH_AREA_GET_SECTORS

static inline size_t
boot_img_sector_size(const struct boot_loader_state *state,
                     size_t slot, size_t sector)
{
    return BOOT_IMG(state, slot).sectors[sector].fa_size;
}

/*
 * Offset of the sector from the beginning of the image, NOT the flash
 * device.
 */
static inline uint32_t
boot_img_sector_off(const struct boot_loader_state *state, size_t slot,
                    size_t sector)
{
    return BOOT_IMG(state, slot).sectors[sector].fa_off -
           BOOT_IMG(state, slot).sectors[0].fa_off;
}

#else  /* defined(MCUBOOT_USE_FLASH_AREA_GET_SECTORS) */

static inline size_t
boot_img_sector_size(const struct boot_loader_state *state,
                     size_t slot, size_t sector)
{
    return BOOT_IMG(state, slot).sectors[sector].fs_size;
}

static inline uint32_t
boot_img_sector_off(const struct boot_loader_state *state, size_t slot,
                    size_t sector)
{
    return BOOT_IMG(state, slot).sectors[sector].fs_off -
           BOOT_IMG(state, slot).sectors[0].fs_off;
}

#endif  /* !defined(MCUBOOT_USE_FLASH_AREA_GET_SECTORS) */

#ifdef MCUBOOT_RAM_LOAD
#define LOAD_IMAGE_DATA(hdr, fap, start, output, size)       \
    (memcpy((output),(void*)((hdr)->ih_load_addr + (start)), \
    (size)), 0)
#else
#define LOAD_IMAGE_DATA(hdr, fap, start, output, size)       \
    (flash_area_read((fap), (start), (output), (size)))
#endif /* MCUBOOT_RAM_LOAD */

#ifdef __cplusplus
}
#endif

#endif
