/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2016-2019 JUUL Labs
 * Copyright (c) 2017 Linaro LTD
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
#include "bootutil/bootutil_log.h"
#ifdef MCUBOOT_SIGN_EC256
/*TODO: remove this after cypress port mbedtls to abstract crypto api */
#ifdef MCUBOOT_USE_CC310
#define NUM_ECC_BYTES (256 / 8)
#endif

#include "bootutil/sign_key.h"

#include "mbedtls/oid.h"
#include "mbedtls/asn1.h"
#include "bootutil/crypto/ecdsa_p256.h"
#include "bootutil_priv.h"
#include "bootutil/fault_injection_hardening.h"

/*
 * Declaring these like this adds NULL termination.
 */
static const uint8_t ec_pubkey_oid[] = MBEDTLS_OID_EC_ALG_UNRESTRICTED;
static const uint8_t ec_secp256r1_oid[] = MBEDTLS_OID_EC_GRP_SECP256R1;

/*
 * Parse the public key used for signing.
 */
static int
bootutil_import_key(uint8_t **cp, uint8_t *end)
{
    size_t len;
    mbedtls_asn1_buf alg;
    mbedtls_asn1_buf param;
    fih_int fih_rc = FIH_FAILURE;
    if (mbedtls_asn1_get_tag(cp, end, &len,
        MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) {
        return -1;
    }
    end = *cp + len;

    /* ECParameters (RFC5480) */
    if (mbedtls_asn1_get_alg(cp, end, &alg, &param)) {
        return -2;
    }
    /* id-ecPublicKey (RFC5480) */
    if (alg.len != sizeof(ec_pubkey_oid) - 1) {
        return -3;
    }
    FIH_CALL(boot_fih_memequal, fih_rc, alg.p, ec_pubkey_oid, sizeof(ec_pubkey_oid) - 1);
    /* namedCurve (RFC5480) */
    if (param.len != sizeof(ec_secp256r1_oid) - 1) {
        return -4;
    }
    FIH_CALL(boot_fih_memequal,fih_rc, param.p, ec_secp256r1_oid, sizeof(ec_secp256r1_oid) - 1);
    /* ECPoint (RFC5480) */
    if (mbedtls_asn1_get_bitstring_null(cp, end, &len)) {
        return -6;
    }
    if (*cp + len != end) {
        return -7;
    }

    if (len != 2 * NUM_ECC_BYTES + 1) {
        return -8;
    }
    /* Is uncompressed? */
    if (*cp[0] != 0x04) {
        return -9;
    }
#if defined(MCUBOOT_USE_TINYCRYPT) || defined(MCUBOOT_USE_CC310)
    (*cp)++;
#endif
    return 0;
}

/*
 * cp points to ASN1 string containing an integer.
 * Verify the tag, and that the length is 32 bytes.
 */
static int
bootutil_read_bigint(uint8_t i[NUM_ECC_BYTES], uint8_t **cp, uint8_t *end)
{
    size_t len;

    if (mbedtls_asn1_get_tag(cp, end, &len, MBEDTLS_ASN1_INTEGER)) {
        return -3;
    }

    if (len >= NUM_ECC_BYTES) {
        memcpy(i, *cp + len - NUM_ECC_BYTES, NUM_ECC_BYTES);
    } else {
        memset(i, 0, NUM_ECC_BYTES - len);
        memcpy(i + NUM_ECC_BYTES - len, *cp, len);
    }
    *cp += len;
    return 0;
}

/*
 * Read in signature. Signature has r and s encoded as integers.
 */
static int
bootutil_decode_sig(uint8_t signature[NUM_ECC_BYTES * 2], uint8_t *cp, uint8_t *end)
{
    int rc;
    size_t len;

    rc = mbedtls_asn1_get_tag(&cp, end, &len,
                              MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    if (rc) {
        return -1;
    }
    if (cp + len > end) {
        return -2;
    }

    rc = bootutil_read_bigint(signature, &cp, end);
    if (rc) {
        return -3;
    }
    rc = bootutil_read_bigint(signature + NUM_ECC_BYTES, &cp, end);
    if (rc) {
        return -4;
    }
    return 0;
}

fih_int
bootutil_verify_sig(uint8_t *hash, uint32_t hlen, uint8_t *sig, size_t slen,
  uint8_t key_id)
{
    int rc = -1;
    bootutil_ecdsa_p256_context ctx;
    uint8_t *pubkey;
    uint8_t *end;
    uint8_t signature[2 * NUM_ECC_BYTES];
    fih_int fih_rc = FIH_FAILURE;
    pubkey = (uint8_t *)bootutil_keys[key_id].key;
    end = pubkey + *bootutil_keys[key_id].len;
    /*  initial the public  ecdsa key */
    BOOT_LOG_INF("checking public key %x %x",slen,*bootutil_keys[key_id].len );
    rc = bootutil_import_key(&pubkey, end);
    if (rc) {
        goto out;
    }

    rc = bootutil_decode_sig(signature, sig, sig + slen);
    if (rc) {
        goto out;
    }


    /*
     * This is simplified, as the hash length is also 32 bytes.
     */
    if (hlen != NUM_ECC_BYTES) {
        goto out;
    }
    BOOT_LOG_INF("verifying signature hlen %x", (unsigned int)hlen);
    bootutil_ecdsa_p256_init(&ctx);
    rc = bootutil_ecdsa_p256_verify(&ctx, pubkey,(end-pubkey), hash, signature);
    /* for more control , we need to implement FIH mechanisms within mbed tls */
    /* since we have a double signature inside mbedtls, nothing more is done */
    if (rc == 0)
      fih_rc = FIH_SUCCESS;
    bootutil_ecdsa_p256_drop(&ctx);
out:

    FIH_RET(fih_rc);
}

#endif /* MCUBOOT_SIGN_EC256 */
