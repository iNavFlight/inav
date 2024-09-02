/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017-2018 Linaro LTD
 * Copyright (c) 2017-2019 JUUL Labs
 * Copyright (c) 2020 Arm Limited
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

#include "mcuboot_config/mcuboot_config.h"

#ifdef MCUBOOT_SIGN_RSA
#include "bootutil/sign_key.h"
#include "bootutil/crypto/rsa_oaep.h"
#include "mbedtls/asn1.h"

#include "bootutil_priv.h"
#include "bootutil/fault_injection_hardening.h"

#if defined(MCUBOOT_DOUBLE_SIGN_VERIF)
#include "boot_hal_imagevalid.h"
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */


/*
 * Constants for this particular constrained implementation of
 * RSA-PSS.  In particular, we support RSA 2048, with a SHA256 hash,
 * and a 32-byte salt.  A signature with different parameters will be
 * rejected as invalid.
 */

/* The size, in octets, of the message. */
#define PSS_EMLEN (MCUBOOT_SIGN_RSA_LEN / 8)

/* The size of the hash function.  For SHA256, this is 32 bytes. */
#define PSS_HLEN 32

/* Size of the salt, should be fixed. */
#define PSS_SLEN 32

/* The length of the mask: emLen - hLen - 1. */
#define PSS_MASK_LEN (PSS_EMLEN - PSS_HLEN - 1)

#define PSS_HASH_OFFSET PSS_MASK_LEN

/* For the mask itself, how many bytes should be all zeros. */
#define PSS_MASK_ZERO_COUNT (PSS_MASK_LEN - PSS_SLEN - 1)
#define PSS_MASK_ONE_POS   PSS_MASK_ZERO_COUNT

/* Where the salt starts. */
#define PSS_MASK_SALT_POS   (PSS_MASK_ONE_POS + 1)

static const uint8_t pss_zeros[8] = {0};

/*
 * Parse the public key used for signing. Simple RSA format.
 */
static int
bootutil_parse_rsakey(bootutil_rsa_context *ctx, uint8_t **p, uint8_t *end)
{
    int rc;
    size_t len;

    if ((rc = mbedtls_asn1_get_tag(p, end, &len,
          MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) != 0) {
        return -1;
    }

    if (*p + len != end) {
        return -2;
    }

    /* retrieve N and E (public key) */
    if ((rc = bootutil_asn1_get_rsa_number(p, end, &ctx->N)) != 0 ||
      (rc = bootutil_asn1_get_rsa_number(p, end, &ctx->E)) != 0) {
        return -3;
    }
    ctx->len = bootutil_rsa_number_size(&ctx->N);

    if (*p != end) {
        return -4;
    }

#if defined(MCUBOOT_USE_MBED_TLS)
    /* The mbedtls version is more than 2.6.1 */
#if MBEDTLS_VERSION_NUMBER > 0x02060100
    rc = mbedtls_rsa_import(ctx, &ctx->N, NULL, NULL, NULL, &ctx->E);
    if (rc != 0) {
        return -5;
    }
#endif

    rc = mbedtls_rsa_check_pubkey(ctx);
    if (rc != 0) {
        return -6;
    }
#endif /* MCUBOOT_USE_MBED_TLS */

    return rc;
}

/*
 * Compute the RSA-PSS mask-generation function, MGF1.  Assumptions
 * are that the mask length will be less than 256 * PSS_HLEN, and
 * therefore we never need to increment anything other than the low
 * byte of the counter.
 *
 * This is described in PKCS#1, B.2.1.
 */
static void
pss_mgf1(uint8_t *mask, const uint8_t *hash)
{
    bootutil_sha256_context ctx;
    uint8_t counter[4] = { 0, 0, 0, 0 };
    uint8_t htmp[PSS_HLEN];
    int count = PSS_MASK_LEN;
    int bytes;

    while (count > 0) {
        bootutil_sha256_init(&ctx);
        bootutil_sha256_update(&ctx, hash, PSS_HLEN);
        bootutil_sha256_update(&ctx, counter, 4);
        bootutil_sha256_finish(&ctx, htmp);

        counter[3]++;

        bytes = PSS_HLEN;
        if (bytes > count)
            bytes = count;

        memcpy(mask, htmp, bytes);
        mask += bytes;
        count -= bytes;
    }

    bootutil_sha256_drop(&ctx);
}

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
 * Validate an RSA signature, using RSA-PSS, as described in PKCS #1
 * v2.2, section 9.1.2, with many parameters required to have fixed
 * values.
 */
static fih_int
bootutil_cmp_rsasig(bootutil_rsa_context *ctx, uint8_t *hash, uint32_t hlen,
  uint8_t *sig)
{
    bootutil_sha256_context shactx;
    uint8_t em[MBEDTLS_MPI_MAX_SIZE];
    uint8_t db_mask[PSS_MASK_LEN];
    uint8_t h2[PSS_HLEN];
    int i;
    int rc = 0;
    fih_int fih_rc = FIH_FAILURE;

    if (ctx->len != PSS_EMLEN || PSS_EMLEN > MBEDTLS_MPI_MAX_SIZE) {
        rc = -1;
        goto out;
    }

    if (hlen != PSS_HLEN) {
        rc = -1;
        goto out;
    }

    if (bootutil_rsa_public(ctx, sig, em)) {
       rc = -1;
       goto out;
   }

    /*
     * PKCS #1 v2.2, 9.1.2 EMSA-PSS-Verify
     *
     * emBits is 2048
     * emLen = ceil(emBits/8) = 256
     *
     * The salt length is not known at the beginning.
     */

    /* Step 1.  The message is constrained by the address space of a
     * 32-bit processor, which is far less than the 2^61-1 limit of
     * SHA-256.
     */

    /* Step 2.  mHash is passed in as 'hash', with hLen the hlen
     * argument. */

    /* Step 3.  if emLen < hLen + sLen + 2, inconsistent and stop.
     * The salt length is not known at this point.
     */

    /* Step 4.  If the rightmost octet of EM does have the value
     * 0xbc, output inconsistent and stop.
     */
    if (em[PSS_EMLEN - 1] != 0xbc) {
        rc = -1;
        goto out;
    }

    /* Step 5.  Let maskedDB be the leftmost emLen - hLen - 1 octets
     * of EM, and H be the next hLen octets.
     *
     * maskedDB is then the first 256 - 32 - 1 = 0-222
     * H is 32 bytes 223-254
     */

    /* Step 6.  If the leftmost 8emLen - emBits bits of the leftmost
     * octet in maskedDB are not all equal to zero, output
     * inconsistent and stop.
     *
     * 8emLen - emBits is zero, so there is nothing to test here.
     */

    /* Step 7.  let dbMask = MGF(H, emLen - hLen - 1). */
    pss_mgf1(db_mask, &em[PSS_HASH_OFFSET]);

    /* Step 8.  let DB = maskedDB xor dbMask.
     * To avoid needing an additional buffer, store the 'db' in the
     * same buffer as db_mask.  From now, to the end of this function,
     * db_mask refers to the unmasked 'db'. */
    for (i = 0; i < PSS_MASK_LEN; i++) {
        db_mask[i] ^= em[i];
    }

    /* Step 9.  Set the leftmost 8emLen - emBits bits of the leftmost
     * octet in DB to zero.
     * pycrypto seems to always make the emBits 2047, so we need to
     * clear the top bit. */
    db_mask[0] &= 0x7F;

    /* Step 10.  If the emLen - hLen - sLen - 2 leftmost octets of DB
     * are not zero or if the octet at position emLen - hLen - sLen -
     * 1 (the leftmost position is "position 1") does not have
     * hexadecimal value 0x01, output "inconsistent" and stop. */
    for (i = 0; i < PSS_MASK_ZERO_COUNT; i++) {
        if (db_mask[i] != 0) {
            rc = -1;
            goto out;
        }
    }

    if (db_mask[PSS_MASK_ONE_POS] != 1) {
        rc = -1;
        goto out;
    }

    /* Step 11. Let salt be the last sLen octets of DB */

    /* Step 12.  Let M' = 0x00 00 00 00 00 00 00 00 || mHash || salt; */

    /* Step 13.  Let H' = Hash(M') */
    bootutil_sha256_init(&shactx);
    bootutil_sha256_update(&shactx, pss_zeros, 8);
    bootutil_sha256_update(&shactx, hash, PSS_HLEN);
    bootutil_sha256_update(&shactx, &db_mask[PSS_MASK_SALT_POS], PSS_SLEN);
    bootutil_sha256_finish(&shactx, h2);
    bootutil_sha256_drop(&shactx);

    /* Step 14.  If H = H', output "consistent".  Otherwise, output
     * "inconsistent". */
    FIH_CALL(boot_fih_memequal, fih_rc, h2, &em[PSS_HASH_OFFSET], PSS_HLEN);

#if defined(MCUBOOT_DOUBLE_SIGN_VERIF)
    /* Double the signature verification (using another way) to resist to basic HW attacks.
     * The second verification is applicable to final signature check on primary slot images
     * only (condition: ImageValidEnable).
     * It is performed in 2 steps:
     * 1- save signature status in global variable ImageValidStatus[]
     *    Input value of comparison function is XORed with IMAGE_VALID to avoid
     *    value 0 for success: IMAGE_VALID for success.
     * 2- verify saved signature status later in boot process
     */
    if (ImageValidEnable == 1)
    {
        /* Check ImageValidIndex is in expected range MCUBOOT_IMAGE_NUMBER */
        if (ImageValidIndex >= MCUBOOT_IMAGE_NUMBER)
        {
          fih_rc = fih_int_encode(-1);
          FIH_RET(fih_rc);
        }
        em[PSS_HASH_OFFSET] ^= IMAGE_VALID;
        ImageValidStatus[ImageValidIndex++] = boot_secure_memequal(h2, &em[PSS_HASH_OFFSET], PSS_HLEN);
    }
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */

out:
    if (rc) {
        fih_rc = fih_int_encode(rc);
    }
    FIH_RET(fih_rc);
}

fih_int
bootutil_verify_sig(uint8_t *hash, uint32_t hlen, uint8_t *sig, size_t slen,
  uint8_t key_id)
{
    bootutil_rsa_context ctx;

    int rc;
    fih_int fih_rc = FIH_FAILURE;
    uint8_t *cp;
    uint8_t *end;

    bootutil_rsa_init(&ctx, 0, 0);

    cp = (uint8_t *)bootutil_keys[key_id].key;
    end = cp + *bootutil_keys[key_id].len;

    rc = bootutil_parse_rsakey(&ctx, &cp, end);


    if (rc || slen != ctx.len) {
        bootutil_rsa_drop(&ctx);
        goto out;
    }
    FIH_CALL(bootutil_cmp_rsasig, fih_rc, &ctx, hash, hlen, sig);

out:
    bootutil_rsa_drop(&ctx);


    FIH_RET(fih_rc);
}
#endif /* MCUBOOT_SIGN_RSA */

