/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2016-2018 JUUL Labs
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

#ifdef MCUBOOT_SIGN_EC
#include "bootutil/sign_key.h"

#include "mbedtls/ecdsa.h"
#include "mbedtls/oid.h"
#include "mbedtls/asn1.h"

#include "bootutil_priv.h"

/*
 * Declaring these like this adds NULL termination.
 */
static const uint8_t ec_pubkey_oid[] = MBEDTLS_OID_EC_ALG_UNRESTRICTED;
static const uint8_t ec_secp224r1_oid[] = MBEDTLS_OID_EC_GRP_SECP224R1;

/*
 * Parse the public key used for signing.
 */
static int
bootutil_parse_eckey(mbedtls_ecdsa_context *ctx, uint8_t **p, uint8_t *end)
{
    size_t len;
    mbedtls_asn1_buf alg;
    mbedtls_asn1_buf param;

    if (mbedtls_asn1_get_tag(p, end, &len,
        MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) {
        return -1;
    }
    end = *p + len;

    if (mbedtls_asn1_get_alg(p, end, &alg, &param)) {
        return -2;
    }
    if (alg.len != sizeof(ec_pubkey_oid) - 1 ||
      boot_fih_memequal(alg.p, ec_pubkey_oid, sizeof(ec_pubkey_oid) - 1)) {
        return -3;
    }
    if (param.len != sizeof(ec_secp224r1_oid) - 1||
      boot_fih_memequal(param.p, ec_secp224r1_oid, sizeof(ec_secp224r1_oid) - 1)) {
        return -4;
    }

    if (mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP224R1)) {
        return -5;
    }

    if (mbedtls_asn1_get_bitstring_null(p, end, &len)) {
        return -6;
    }
    if (*p + len != end) {
        return -7;
    }

    if (mbedtls_ecp_point_read_binary(&ctx->grp, &ctx->Q, *p, end - *p)) {
        return -8;
    }

    if (mbedtls_ecp_check_pubkey(&ctx->grp, &ctx->Q)) {
        return -9;
    }
    return 0;
}

static int
bootutil_cmp_sig(mbedtls_ecdsa_context *ctx, uint8_t *hash, uint32_t hlen,
  uint8_t *sig, size_t slen)
{
    return mbedtls_ecdsa_read_signature(ctx, hash, hlen, sig, slen);
}

int
bootutil_verify_sig(uint8_t *hash, uint32_t hlen, uint8_t *sig, size_t slen,
  uint8_t key_id)
{
    int rc;
    uint8_t *cp;
    uint8_t *end;
    mbedtls_ecdsa_context ctx;

    mbedtls_ecdsa_init(&ctx);

    cp = (uint8_t *)bootutil_keys[key_id].key;
    end = cp + *bootutil_keys[key_id].len;

    rc = bootutil_parse_eckey(&ctx, &cp, end);
    if (rc) {
        return -1;
    }

    while (sig[slen - 1] == '\0') {
        slen--;
    }
    rc = bootutil_cmp_sig(&ctx, hash, hlen, sig, slen);
    mbedtls_ecdsa_free(&ctx);

    return rc;
}
#endif /* MCUBOOT_SIGN_EC */
