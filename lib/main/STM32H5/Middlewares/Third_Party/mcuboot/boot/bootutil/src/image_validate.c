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

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>

#include <flash_map_backend/flash_map_backend.h>
#include "bootutil/bootutil_log.h"
#include "bootutil/image.h"
#include "bootutil/crypto/sha256.h"
#include "bootutil/sign_key.h"
#include "bootutil/security_cnt.h"
#include "bootutil/fault_injection_hardening.h"

#include "mcuboot_config/mcuboot_config.h"

#ifdef MCUBOOT_ENC_IMAGES
#include "bootutil/enc_key.h"
#endif
#if !defined(MCUBOOT_USE_HAL)
#if defined(MCUBOOT_SIGN_RSA)
#include "mbedtls/rsa.h"
#endif
#if defined(MCUBOOT_SIGN_EC) || defined(MCUBOOT_SIGN_EC256)
#include "mbedtls/ecdsa.h"
#endif
#endif /* not MCUBOOT_USE_HAL */
#if defined(MCUBOOT_ENC_IMAGES) || defined(MCUBOOT_SIGN_RSA) || \
    defined(MCUBOOT_SIGN_EC) || defined(MCUBOOT_SIGN_EC256)
#include "mbedtls/asn1.h"
#endif

#include "bootutil_priv.h"

#if defined(MCUBOOT_USE_HASH_REF)
#include "boot_hal_hash_ref.h"
#include "boot_hal_imagevalid.h"
#endif

/*
 * Compute SHA256 over the image.
 */
static int
bootutil_img_hash(struct enc_key_data *enc_state, int image_index,
                  struct image_header *hdr, const struct flash_area *fap,
                  uint8_t *tmp_buf, uint32_t tmp_buf_sz, uint8_t *hash_result,
                  uint8_t *seed, int seed_len)
{
    bootutil_sha256_context sha256_ctx;
    uint32_t blk_sz;
    uint32_t size;
    uint32_t off;
    int rc;
#if defined(MCUBOOT_ENC_IMAGES)
    uint16_t hdr_size;
    uint32_t blk_off;
    uint32_t tlv_off;
#endif

#if (BOOT_IMAGE_NUMBER == 1) || !defined(MCUBOOT_ENC_IMAGES) || \
    defined(MCUBOOT_RAM_LOAD)
    (void)enc_state;
    (void)image_index;
#ifdef MCUBOOT_RAM_LOAD
    (void)blk_sz;
    (void)off;
    (void)rc;
#endif
#endif


#if defined(MCUBOOT_ENC_IMAGES) && !defined(MCUBOOT_PRIMARY_ONLY)
    /* Encrypted images only exist in the secondary slot  */
    if (MUST_DECRYPT(fap, image_index, hdr) &&
            !boot_enc_valid(enc_state, image_index, fap)) {
        return -1;
    }
#endif

    bootutil_sha256_init(&sha256_ctx);

    /* in some cases (split image) the hash is seeded with data from
     * the loader image */
    if (seed && (seed_len > 0)) {
        bootutil_sha256_update(&sha256_ctx, seed, seed_len);
    }

    /* Hash is computed over image header and image itself. */
#if defined(MCUBOOT_ENC_IMAGES)
    size = hdr_size = hdr->ih_hdr_size;
#else
    size = hdr->ih_hdr_size;
#endif
    size += hdr->ih_img_size;
#if defined(MCUBOOT_ENC_IMAGES)
    tlv_off = size;
#endif

    /* If protected TLVs are present they are also hashed. */
    size += hdr->ih_protect_tlv_size;

#ifdef MCUBOOT_RAM_LOAD
    bootutil_sha256_update(&sha256_ctx,(void*)(hdr->ih_load_addr), size);
#else
    for (off = 0; off < size; off += blk_sz) {
        blk_sz = size - off;
        if (blk_sz > tmp_buf_sz) {
            blk_sz = tmp_buf_sz;
        }
#ifdef MCUBOOT_ENC_IMAGES
        /* The only data that is encrypted in an image is the payload;
         * both header and TLVs (when protected) are not.
         */
        if ((off < hdr_size) && ((off + blk_sz) > hdr_size)) {
            /* read only the header */
            blk_sz = hdr_size - off;
        }
        if ((off < tlv_off) && ((off + blk_sz) > tlv_off)) {
            /* read only up to the end of the image payload */
            blk_sz = tlv_off - off;
        }
#endif
        rc = flash_area_read(fap, off, tmp_buf, blk_sz);
        if (rc) {
            bootutil_sha256_drop(&sha256_ctx);
            return rc;
        }

#ifdef MCUBOOT_ENC_IMAGES
#ifdef MCUBOOT_PRIMARY_ONLY
    if (MUST_DECRYPT_PRIMARY_ONLY(fap, image_index, hdr)) {
#else
    if (MUST_DECRYPT(fap, image_index, hdr)) {
#endif
            /* Only payload is encrypted (area between header and TLVs) */
            if (off >= hdr_size && off < tlv_off) {
                blk_off = (off - hdr_size) & 0xf;
                boot_encrypt(enc_state, image_index, fap, off - hdr_size,
                        blk_sz, blk_off, tmp_buf);
            }
#ifdef MCUBOOT_PRIMARY_ONLY
            else {
                /* header must not be flagged decrypted to compute the correct hash */
                struct image_header *header= (struct image_header *)tmp_buf;
                BOOT_LOG_INF("Controlling an encrypted primary image");
                /* double check that header content is in buffer */
                if (blk_sz == hdr_size)
                    header->ih_flags &=~IMAGE_F_ENCRYPTED;
                else {
                    BOOT_LOG_INF("Header does not fit in a block size: not supported with MCUBOOT_PRIMARY_ONLY");
                    return -1;
                }
            }
#endif

        }
#endif
        bootutil_sha256_update(&sha256_ctx, tmp_buf, blk_sz);
    }
#endif /* MCUBOOT_RAM_LOAD */
    bootutil_sha256_finish(&sha256_ctx, hash_result);
    bootutil_sha256_drop(&sha256_ctx);

    return 0;
}

/*
 * Currently, we only support being able to verify one type of
 * signature, because there is a single verification function that we
 * call.  List the type of TLV we are expecting.  If we aren't
 * configured for any signature, don't define this macro.
 */
#if (defined(MCUBOOT_SIGN_RSA)      + \
     defined(MCUBOOT_SIGN_EC)       + \
     defined(MCUBOOT_SIGN_EC256)    + \
     defined(MCUBOOT_SIGN_ED25519)) > 1
#error "Only a single signature type is supported!"
#endif

#if defined(MCUBOOT_SIGN_RSA)
#    if MCUBOOT_SIGN_RSA_LEN == 2048
#        define EXPECTED_SIG_TLV IMAGE_TLV_RSA2048_PSS
#    elif MCUBOOT_SIGN_RSA_LEN == 3072
#        define EXPECTED_SIG_TLV IMAGE_TLV_RSA3072_PSS
#    else
#        error "Unsupported RSA signature length"
#    endif
#    define SIG_BUF_SIZE (MCUBOOT_SIGN_RSA_LEN / 8)
#    define EXPECTED_SIG_LEN(x) ((x) == SIG_BUF_SIZE) /* 2048 bits */
#elif defined(MCUBOOT_SIGN_EC)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ECDSA224
#    define SIG_BUF_SIZE 128
#    define EXPECTED_SIG_LEN(x)  ((x) <= 64) /* (tbc) 56 bytes for sign + 8 bytes for asn1 */
#elif defined(MCUBOOT_SIGN_EC256)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ECDSA256
#    define SIG_BUF_SIZE 128
#    define EXPECTED_SIG_LEN(x) ((x) <= 72) /* (tbc) 64 bytes for sign + 8 bytes for asn1 */
#elif defined(MCUBOOT_SIGN_ED25519)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ED25519
#    define SIG_BUF_SIZE 64
#    define EXPECTED_SIG_LEN(x) ((x) == SIG_BUF_SIZE)
#else
#    define SIG_BUF_SIZE 32 /* no signing, sha256 digest only */
#endif

#ifdef EXPECTED_SIG_TLV
#if !defined(MCUBOOT_HW_KEY)
static int
bootutil_find_key(int image_index, uint8_t *keyhash, uint8_t keyhash_len)
{
    bootutil_sha256_context sha256_ctx;
    const struct bootutil_key *key;
    uint8_t hash[32];

    if (keyhash_len > 32) {
        return -1;
    }

    key = &bootutil_keys[image_index];
    bootutil_sha256_init(&sha256_ctx);
    bootutil_sha256_update(&sha256_ctx, key->key, *key->len);
    bootutil_sha256_finish(&sha256_ctx, hash);
    if (!memcmp(hash, keyhash, keyhash_len)) {
        bootutil_sha256_drop(&sha256_ctx);
        return (int)image_index;
    }
    bootutil_sha256_drop(&sha256_ctx);
    return -1;
}
#else
extern unsigned int pub_key_len;
static int
bootutil_find_key(uint8_t image_index, uint8_t *key, uint16_t key_len)
{
    bootutil_sha256_context sha256_ctx;
    uint8_t hash[32];
    uint8_t key_hash[32];
    size_t key_hash_size = sizeof(key_hash);
    int rc;
    fih_int fih_rc;

    bootutil_sha256_init(&sha256_ctx);
    bootutil_sha256_update(&sha256_ctx, key, key_len);
    bootutil_sha256_finish(&sha256_ctx, hash);
    bootutil_sha256_drop(&sha256_ctx);

    rc = boot_retrieve_public_key_hash(image_index, key_hash, &key_hash_size);
    if (rc) {
        return rc;
    }

    /* Adding hardening to avoid this potential attack:
     *  - Image is signed with an arbitrary key and the corresponding public
     *    key is added as a TLV field.
     * - During public key validation (comparing against key-hash read from
     *   HW) a fault is injected to accept the public key as valid one.
     */
    FIH_CALL(boot_fih_memequal, fih_rc, hash, key_hash, key_hash_size);
    if (fih_eq(fih_rc, FIH_SUCCESS)) {
        bootutil_keys[0].key = key;
        pub_key_len = key_len;
        return 0;
    }

    return -1;
}
#endif /* !MCUBOOT_HW_KEY */
#endif

#ifdef MCUBOOT_HW_ROLLBACK_PROT
/**
 * Reads the value of an image's security counter.
 *
 * @param hdr           Pointer to the image header structure.
 * @param fap           Pointer to a description structure of the image's
 *                      flash area.
 * @param security_cnt  Pointer to store the security counter value.
 *
 * @return              0 on success; nonzero on failure.
 */
int32_t
bootutil_get_img_security_cnt(struct image_header *hdr,
                              const struct flash_area *fap,
                              uint32_t *img_security_cnt)
{
    struct image_tlv_iter it;
    uint32_t off;
    uint16_t len;
    int32_t rc;

    if ((hdr == NULL) ||
        (fap == NULL) ||
        (img_security_cnt == NULL)) {
        /* Invalid parameter. */
        return BOOT_EBADARGS;
    }

    /* The security counter TLV is in the protected part of the TLV area. */
    if (hdr->ih_protect_tlv_size == 0) {
        return BOOT_EBADIMAGE;
    }

    rc = bootutil_tlv_iter_begin(&it, hdr, fap, IMAGE_TLV_SEC_CNT, true);
    if (rc) {
        return rc;
    }

    /* Traverse through the protected TLV area to find
     * the security counter TLV.
     */

    rc = bootutil_tlv_iter_next(&it, &off, &len, NULL);
    if (rc != 0) {
        /* Security counter TLV has not been found. */
        return -1;
    }

    if (len != sizeof(*img_security_cnt)) {
        /* Security counter is not valid. */
        return BOOT_EBADIMAGE;
    }

    rc = LOAD_IMAGE_DATA(hdr, fap, off, img_security_cnt, len);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}
#endif /* MCUBOOT_HW_ROLLBACK_PROT */

#if defined(MCUBOOT_DOUBLE_SIGN_VERIF)
static uint32_t boot_secure_memequal(const void *s1, const void *s2, size_t n)
{
    size_t i;
    uint8_t *s1_p = (uint8_t*) s1;
    uint8_t *s2_p = (uint8_t*) s2;
    uint32_t ret = 0;

    for (i = 0; i < n; i++) {
        ret |= (s1_p[i] ^ s2_p[i]);
    }

    return ret;
}
#endif

/*
 * Verify the integrity of the image.
 * Return non-zero if image could not be validated/does not validate.
 */
fih_int
bootutil_img_validate(struct enc_key_data *enc_state, int image_index,
                      struct image_header *hdr, const struct flash_area *fap,
                      uint8_t *tmp_buf, uint32_t tmp_buf_sz, uint8_t *seed,
                      int seed_len, uint8_t *out_hash)
{
    uint32_t off;
    uint16_t len;
    uint16_t type;
    int sha256_valid = 0;
#if defined(MCUBOOT_ENCRYPT_RSA) || defined(MCUBOOT_ENCRYPT_KW) || defined(MCUBOOT_ENCRYPT_EC256)
    uint8_t tlv_enc = 0;
#endif
#ifdef EXPECTED_SIG_TLV
    fih_int valid_signature = FIH_FAILURE;
    int key_id = -1;
#ifdef MCUBOOT_HW_KEY
    /* Few extra bytes for encoding and for public exponent. */
    uint8_t key_buf[SIG_BUF_SIZE + 24];
#endif
#endif /* EXPECTED_SIG_TLV */
    struct image_tlv_iter it;
    uint8_t buf[SIG_BUF_SIZE];
    uint8_t hash[32];
#ifdef MCUBOOT_USE_HASH_REF
    uint8_t hash_ref[32];
#endif /* MCUBOOT_USE_HASH_REF */
    int rc = 0;
    fih_int fih_rc = FIH_FAILURE;
#ifdef MCUBOOT_HW_ROLLBACK_PROT
    fih_int security_cnt = fih_int_encode(INT_MAX);
    uint32_t img_security_cnt = 0;
    fih_int security_counter_valid = FIH_FAILURE;
#endif

    rc = bootutil_img_hash(enc_state, image_index, hdr, fap, tmp_buf,
            tmp_buf_sz, hash, seed, seed_len);
    if (rc) {
        goto out;
    }

    if (out_hash) {
        memcpy(out_hash, hash, 32);
    }

    rc = bootutil_tlv_iter_begin(&it, hdr, fap, IMAGE_TLV_ANY, false);
    if (rc) {
        goto out;
    }

    /*
     * Traverse through all of the TLVs, performing any checks we know
     * and are able to do.
     */
    while (true) {
        rc = bootutil_tlv_iter_next(&it, &off, &len, &type);
        if (rc < 0) {
            goto out;
        } else if (rc > 0) {
            break;
        }

        if (type == IMAGE_TLV_SHA256) {
            /*
             * Verify the SHA256 image hash.  This must always be
             * present.
             */
            if (len != sizeof(hash)) {
                rc = -1;
                goto out;
            }
            rc = LOAD_IMAGE_DATA(hdr, fap, off, buf, sizeof(hash));
            if (rc) {
                goto out;
            }

            FIH_CALL(boot_fih_memequal, fih_rc, hash, buf, sizeof(hash));
            if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
                goto out;
            }

            sha256_valid = 1;
#ifdef EXPECTED_SIG_TLV
#ifndef MCUBOOT_HW_KEY
        } else if (type == IMAGE_TLV_KEYHASH) {
            /*
             * Determine which key we should be checking.
             */
            if (len > 32) {
                rc = -1;
                goto out;
            }
            rc = LOAD_IMAGE_DATA(hdr, fap, off, buf, len);
            if (rc) {
                goto out;
            }
            key_id = bootutil_find_key(image_index, buf, len);
            /* The key must be found */
            if (key_id < 0 || key_id >= bootutil_key_cnt)
            {
                rc = -1;
                goto out;
            }
#else
        } else if (type == IMAGE_TLV_PUBKEY) {
            /*
             * Determine which key we should be checking.
             */
            if ((len > sizeof(key_buf)) || (len > sizeof(pubkey))) {
                rc = -1;
                goto out;
            }
            rc = LOAD_IMAGE_DATA(hdr, fap, off, key_buf, len);
            if (rc) {
                goto out;
            }
            key_id = bootutil_find_key(image_index, key_buf, len);
            /* The key must be found */
            if (key_id < 0 || key_id >= bootutil_key_cnt)
            {
                rc = -1;
                goto out;
            }
#endif /* !MCUBOOT_HW_KEY */
        } else if (type == EXPECTED_SIG_TLV) {
            /* Check signature length*/
            if (!EXPECTED_SIG_LEN(len) || len > sizeof(buf)) {
                rc = -1;
                goto out;
            }
#if defined(MCUBOOT_USE_HASH_REF)
            if (ImageValidEnable == 1) {
                /*
                 * Compare SHA256 of image with SHA256 reference. If matching,
                 * the signature verification can be bypassed.
                 */
                rc = boot_hash_ref_get(hash_ref, sizeof(hash_ref), image_index);
                if (rc == 0) {

                    FIH_CALL(boot_fih_memequal, valid_signature, hash, hash_ref, sizeof(hash));
                    if (fih_eq(valid_signature, FIH_SUCCESS)) {

                        BOOT_LOG_INF("hash ref OK");

#if defined(MCUBOOT_DOUBLE_SIGN_VERIF)
                        /* Double the hash ref verification (using another way) to resist to basic HW attacks.
                         * The second verification is applicable to final hash ref check on primary slot images
                         * only (condition: ImageValidEnable).
                         * It is performed in 2 steps:
                         * 1- save hash ref verification status in global variable ImageValidStatus[]
                         * 2- verify saved hash ref verification status later in boot process
                         */

                        /* Check ImageValidIndex is in expected range MCUBOOT_IMAGE_NUMBER */
                        if (ImageValidIndex >= MCUBOOT_IMAGE_NUMBER)
                        {
                            rc = -1;
                            goto out;
                        }

                        hash_ref[0] ^= IMAGE_VALID;
                        ImageValidStatus[ImageValidIndex++] = boot_secure_memequal(hash, hash_ref, sizeof(hash));
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */

                        /* Bypass signature verification */
                        continue;
                    }
                }
            }
#endif /* MCUBOOT_USE_HASH_REF */

            /* Ignore this signature if it is out of bounds. */
            if (key_id < 0 || key_id >= bootutil_key_cnt) {
                key_id = -1;
                continue;
            }
            rc = LOAD_IMAGE_DATA(hdr, fap, off, buf, len);
            if (rc) {
                goto out;
            }
            BOOT_LOG_INF("verify sig key id %d", key_id);
            FIH_CALL(bootutil_verify_sig, valid_signature, hash, sizeof(hash),
                                                           buf, len, key_id);
            BOOT_LOG_INF("signature %s",
            fih_eq(valid_signature,FIH_SUCCESS) ? "OK" : "KO");
            key_id = -1;

            /* Exit if signature is invalid. */
            if (fih_not_eq(valid_signature, FIH_SUCCESS)) {
                rc = -1;
                goto out;
            }

#if defined(MCUBOOT_USE_HASH_REF)
            if (ImageValidEnable == 1) {
                /* Prepare SHA256 as reference for next boot. The storage of the
                   reference will be performed later in boot process after double
                   verification. */
                rc = boot_hash_ref_set(hash, sizeof(hash), image_index);
                if (rc) {
                    goto out;
                }
            }
#endif /* MCUBOOT_USE_HASH_REF */
#endif /* EXPECTED_SIG_TLV */
#ifdef MCUBOOT_HW_ROLLBACK_PROT
        } else if (type == IMAGE_TLV_SEC_CNT) {
            /* check that TLV is within protected area */
            /* tlv off is set to next tlv */
            if (it.tlv_off > it.prot_end)
            {
                rc= -1;
                goto out;
            }
            /*
             * Verify the image's security counter.
             * This must always be present.
             */
            if (len != sizeof(img_security_cnt)) {
                /* Security counter is not valid. */
                rc = -1;
                goto out;
            }

            rc = LOAD_IMAGE_DATA(hdr, fap, off, &img_security_cnt, len);
            if (rc) {
                goto out;
            }

            FIH_CALL(boot_nv_security_counter_get, fih_rc, image_index,
                                                           &security_cnt);
            if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
                goto out;
            }
            BOOT_LOG_INF("verify counter  %d %x %x", image_index, (unsigned int)img_security_cnt, security_cnt.val );
            /* Compare the new image's security counter value against the
             * stored security counter value.
             */
            fih_rc = fih_int_encode_zero_equality(img_security_cnt <
                                   fih_int_decode(security_cnt));
            if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
                goto out;
            }

            /* The image's security counter has been successfully verified. */
            security_counter_valid = fih_rc;
            BOOT_LOG_INF("counter  %d : ok", image_index );
#endif /* MCUBOOT_HW_ROLLBACK_PROT */
        }
        else
            /* unexpected TLV , injection of assembly pattern , possible */
        {
            rc = -1;
#if defined(MCUBOOT_ENCRYPT_RSA)
            if (type == IMAGE_TLV_ENC_RSA2048)
            {
                /* Check tlv length */
                if ((len == 256)
#if !defined(MCUBOOT_PRIMARY_ONLY)
                    /* Check image is encrypted */
                    && ((hdr->ih_flags & IMAGE_F_ENCRYPTED) == IMAGE_F_ENCRYPTED)
#endif /* !defined(MCUBOOT_PRIMARY_ONLY) */
                    /* Only one non protected TLV allowed */
                    && (tlv_enc == 0))
                {
                    tlv_enc = 1;
                    rc = 0;
                }
            }
#elif defined(MCUBOOT_ENCRYPT_KW)
            if (type == IMAGE_TLV_ENC_KW128)
            {
                /* Check tlv length */
                if ((len == TBC)
#if !defined(MCUBOOT_PRIMARY_ONLY)
                    /* Check image is encrypted */
                    && ((hdr->ih_flags & IMAGE_F_ENCRYPTED) == IMAGE_F_ENCRYPTED)
#endif /* !defined(MCUBOOT_PRIMARY_ONLY) */
                    /* Only one non protected TLV allowed */
                    && (tlv_enc == 0))
                {
                    tlv_enc = 1;
                    rc = 0;
                }
            }
#elif defined(MCUBOOT_ENCRYPT_EC256)
            if (type == IMAGE_TLV_ENC_EC256)
            {
                /* Check tlv length */
                if ((len == 113)
#if !defined(MCUBOOT_PRIMARY_ONLY)
                    /* Check image is encrypted */
                    && ((hdr->ih_flags & IMAGE_F_ENCRYPTED) == IMAGE_F_ENCRYPTED)
#endif /* !defined(MCUBOOT_PRIMARY_ONLY) */
                    /* Only one non protected TLV allowed */
                    && (tlv_enc == 0))
                {
                    tlv_enc = 1;
                    rc = 0;
                }
            }
#endif
            if (type == IMAGE_TLV_DEPENDENCY)
            {
                /* check that TLV is within protected area */
                /* tlv off is set to next tlv */
                if (it.tlv_off <= it.prot_end)
                {
                    rc = 0;
                }
            }
            if (type == IMAGE_TLV_BOOT_RECORD)
            {
                /* check that TLV is within protected area */
                /* tlv off is set to next tlv */
                if (it.tlv_off > it.prot_end)
                {
                    goto out;
                }
                rc = 0;
            }
            if (rc)
            {

                BOOT_LOG_INF("unexpected TLV %x ", type);
                goto out;
            }
        }
    }

    rc = !sha256_valid;
    if (rc) {
        goto out;
    }
#ifdef EXPECTED_SIG_TLV
    fih_rc = fih_int_encode_zero_equality(fih_not_eq(valid_signature,
                                                     FIH_SUCCESS));
#endif
#ifdef MCUBOOT_HW_ROLLBACK_PROT
    if (fih_not_eq(security_counter_valid, FIH_SUCCESS)) {
        rc = -1;
        goto out;
    }
#endif

    /* Check pattern in slot, after image payload */
#if !defined(MCUBOOT_PRIMARY_ONLY)
    if (fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(image_index))
    {
        off = it.tlv_end;
       /* Check that tlv_end is not overlapping trailer */
        if (off > boot_status_off(fap))
        {
            rc = -1;
            goto out;
        }

        /* read flash per byte, until next doubleword */
        if (off % 8)
        {
            uint32_t end0 = (((off / 8) + 1) * 8);
            while (off < end0)
            {
                uint8_t data8;
                rc = flash_area_read(fap, off, &data8, sizeof(data8));
                if (rc)
                {
                    BOOT_LOG_INF("read failed %x ", (unsigned int)off);
                    rc = -1;
                    goto out;
                }
                if (data8 != 0xff)
                {
                    BOOT_LOG_INF("data wrong at %x", (unsigned int)off);
                    rc = -1;
                    goto out;
                }
                off += sizeof(data8);
            }
        }
        /* read flash per doubleword */
#if defined(MCUBOOT_OVERWRITE_ONLY)
        /* check pattern till magic at end of slot */
        uint32_t end = boot_magic_off(fap);
#else
        /* check pattern till trailer */
        uint32_t end = boot_status_off(fap);
#endif /* MCUBOOT_OVERWRITE_ONLY */
        while (off < end)
        {
            uint64_t data64;
            rc = flash_area_read(fap, off, &data64, sizeof(data64));
            if (rc)
            {
                BOOT_LOG_INF("read failed %x ", (unsigned int)off);
                rc = -1;
                goto out;
            }
            if (data64 != 0xffffffffffffffff)
            {
                BOOT_LOG_INF("data wrong at %x", (unsigned int)off);
                rc = -1;
                goto out;
            }
            off += sizeof(data64);
        }
    }
#endif /* !defined(MCUBOOT_PRIMARY_ONLY) */
out:

    if (rc) {

        fih_rc = fih_int_encode(rc);

    }
    FIH_RET(fih_rc);
}
