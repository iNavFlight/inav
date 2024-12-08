/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017-2019 Linaro LTD
 * Copyright (c) 2017-2019 JUUL Labs
 * Copyright (c) 2021 STMicroelectronics
 */

/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are several choices:
 * MCUBOOT_USE_MBED_TLS, MCUBOOT_USE_TINYCRYPT, MCUBOOT_USE_CC310 or
 * MCUBOOT_USE_HAL.
 * It is a compile error there is not exactly one of these defined.
 */

#ifndef __BOOTUTIL_CRYPTO_SHA256_H_
#define __BOOTUTIL_CRYPTO_SHA256_H_

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_MBED_TLS) + \
     defined(MCUBOOT_USE_TINYCRYPT) + \
     defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_CC310)) != 1
    #error "One crypto backend must be defined: either CC310, MBED_TLS, TINYCRYPT, or HAL"
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
    #include <mbedtls/sha256.h>
    #define BOOTUTIL_CRYPTO_SHA256_BLOCK_SIZE (64)
    #define BOOTUTIL_CRYPTO_SHA256_DIGEST_SIZE (32)
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
    #include <tinycrypt/sha256.h>
    #include <tinycrypt/constants.h>
    #define BOOTUTIL_CRYPTO_SHA256_BLOCK_SIZE TC_SHA256_BLOCK_SIZE
    #define BOOTUTIL_CRYPTO_SHA256_DIGEST_SIZE TC_SHA256_DIGEST_SIZE
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_CC310)
    #include <cc310_glue.h>
    #define BOOTUTIL_CRYPTO_SHA256_BLOCK_SIZE (64)
    #define BOOTUTIL_CRYPTO_SHA256_DIGEST_SIZE (32)
#endif /* MCUBOOT_USE_CC310 */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>
    #define ST_SHA256_BLOCK_SIZE  ((size_t)  64)  /* HW handles 512 bits, ie 64 bytes */
    #define ST_SHA256_EXTRA_BYTES ((size_t)  4)   /* One supplementary word on first block */

    #define ST_SHA256_TIMEOUT     ((uint32_t) 5000)
    #define BOOTUTIL_CRYPTO_SHA256_DIGEST_SIZE (32)
#endif /* MCUBOOT_USE_HAL */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
typedef mbedtls_sha256_context bootutil_sha256_context;

static inline void bootutil_sha256_init(bootutil_sha256_context *ctx)
{
    mbedtls_sha256_init(ctx);
    (void)mbedtls_sha256_starts_ret(ctx, 0);
}

static inline void bootutil_sha256_drop(bootutil_sha256_context *ctx)
{
    /* XXX: config defines MBEDTLS_PLATFORM_NO_STD_FUNCTIONS so no need to free */
    /* (void)mbedtls_sha256_free(ctx); */
    (void)ctx;
}

static inline int bootutil_sha256_update(bootutil_sha256_context *ctx,
                                         const void *data,
                                         uint32_t data_len)
{
    return mbedtls_sha256_update_ret(ctx, data, data_len);
}

static inline int bootutil_sha256_finish(bootutil_sha256_context *ctx,
                                          uint8_t *output)
{
    return mbedtls_sha256_finish_ret(ctx, output);
}
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
typedef struct tc_sha256_state_struct bootutil_sha256_context;
static inline void bootutil_sha256_init(bootutil_sha256_context *ctx)
{
    tc_sha256_init(ctx);
}

static inline void bootutil_sha256_drop(bootutil_sha256_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_sha256_update(bootutil_sha256_context *ctx,
                                         const void *data,
                                         uint32_t data_len)
{
    return tc_sha256_update(ctx, data, data_len);
}

static inline int bootutil_sha256_finish(bootutil_sha256_context *ctx,
                                          uint8_t *output)
{
    return tc_sha256_final(output, ctx);
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_CC310)
static inline void bootutil_sha256_init(bootutil_sha256_context *ctx)
{
    cc310_sha256_init(ctx);
}

static inline void bootutil_sha256_drop(bootutil_sha256_context *ctx)
{
    (void)ctx;
    nrf_cc310_disable();
}

static inline int bootutil_sha256_update(bootutil_sha256_context *ctx,
                                          const void *data,
                                          uint32_t data_len)
{
    cc310_sha256_update(ctx, data, data_len);
    return 0;
}

static inline int bootutil_sha256_finish(bootutil_sha256_context *ctx,
                                          uint8_t *output)
{
    cc310_sha256_finalize(ctx, output);
    return 0;
}
#endif /* MCUBOOT_USE_CC310 */

#if defined(MCUBOOT_USE_HAL)
/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    HASH_HandleTypeDef hhash;                       /*!< Handle of HASH HAL */
    uint8_t sbuf[ST_SHA256_BLOCK_SIZE + ST_SHA256_EXTRA_BYTES];
                                                    /*!< Buffer to store input data
                                                        (first block with its extra bytes,
                                                         intermediate blocks,
                                                         or last input block) */
    uint8_t sbuf_len;                               /*!< Number of bytes stored in sbuf */
    uint8_t first;                                  /*!< Extra-bytes on first computed block */
}
bootutil_sha256_context;

static inline void bootutil_sha256_init(bootutil_sha256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    /* Enable HASH clock */
    __HAL_RCC_HASH_CLK_ENABLE();

    memset( ctx, 0, sizeof( bootutil_sha256_context ) );

#if defined(MCUBOOT_USE_HASH_HAL_V1)
    ctx->hhash.Init.DataType = HASH_DATATYPE_8B;
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
    ctx->hhash.Instance = HASH;
    ctx->hhash.Init.DataType = HASH_BYTE_SWAP;
    ctx->hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA256;
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
    HAL_HASH_Init(&ctx->hhash);

    /* first block on 17 words */
    ctx->first = ST_SHA256_EXTRA_BYTES;
}

static inline void bootutil_sha256_drop(bootutil_sha256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    HAL_HASH_DeInit(&ctx->hhash);

    /* Disable HASH clock */
    __HAL_RCC_HASH_CLK_DISABLE();
}

static inline int bootutil_sha256_update(bootutil_sha256_context *ctx,
                                          const void *data,
                                          uint32_t data_len)
{
    size_t currentlen = data_len;

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( data_len == 0 || data != NULL );

    if (currentlen < (ST_SHA256_BLOCK_SIZE + ctx->first - ctx->sbuf_len))
    {
        /* only store input data in context buffer */
        memcpy(ctx->sbuf + ctx->sbuf_len, data, currentlen);
        ctx->sbuf_len += currentlen;
    }
    else
    {
        /* fill context buffer until ST_SHA256_BLOCK_SIZE bytes, and process it */
        memcpy(ctx->sbuf + ctx->sbuf_len, (unsigned char *)data, (ST_SHA256_BLOCK_SIZE + ctx->first - ctx->sbuf_len));
        currentlen -= (ST_SHA256_BLOCK_SIZE + ctx->first - ctx->sbuf_len);

#if defined(MCUBOOT_USE_HASH_HAL_V1)
        if (HAL_HASHEx_SHA256_Accmlt(&ctx->hhash,
                                     (uint8_t *)(ctx->sbuf),
                                     ST_SHA256_BLOCK_SIZE + ctx->first) != 0)
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
        if (HAL_HASH_Accumulate(&ctx->hhash,
                                (uint8_t *)(ctx->sbuf),
                                ST_SHA256_BLOCK_SIZE + ctx->first,
                                ST_SHA256_TIMEOUT) != HAL_OK)
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
        {
            return ERR_PLATFORM_HW_ACCEL_FAILED;
        }

        /* Process following input data with size multiple of ST_SHA256_BLOCK_SIZE bytes */
        size_t iter = currentlen / ST_SHA256_BLOCK_SIZE;
        if (iter != 0)
        {

#if defined(MCUBOOT_USE_HASH_HAL_V1)
            if (HAL_HASHEx_SHA256_Accmlt(&ctx->hhash,
                                         (uint8_t *)((unsigned char *)data + ST_SHA256_BLOCK_SIZE + ctx->first - ctx->sbuf_len),
                                         (iter * ST_SHA256_BLOCK_SIZE)) != 0)
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
            if (HAL_HASH_Accumulate(&ctx->hhash,
                                    (uint8_t *)((unsigned char *)data + ST_SHA256_BLOCK_SIZE + ctx->first - ctx->sbuf_len),
                                    (iter * ST_SHA256_BLOCK_SIZE),
                                    ST_SHA256_TIMEOUT) != HAL_OK)
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
            {
                return ERR_PLATFORM_HW_ACCEL_FAILED;
            }
        }

        /* following blocks on 16 words */
        ctx->first = 0;

        /* Store only the remaining input data up to (ST_SHA256_BLOCK_SIZE - 1) bytes */
        ctx->sbuf_len = currentlen % ST_SHA256_BLOCK_SIZE;
        if (ctx->sbuf_len != 0)
        {
            memcpy(ctx->sbuf, (unsigned char *)data + data_len - ctx->sbuf_len, ctx->sbuf_len);
        }
    }
    return 0;
}

static inline int bootutil_sha256_finish(bootutil_sha256_context *ctx,
                                          uint8_t *output)
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( output != NULL );

#if defined(MCUBOOT_USE_HASH_HAL_V1)
    if (HAL_HASHEx_SHA256_Accmlt_End(&ctx->hhash, ctx->sbuf, ctx->sbuf_len, output, ST_SHA256_TIMEOUT) != 0)
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
    if (HAL_HASH_AccumulateLast(&ctx->hhash, ctx->sbuf, ctx->sbuf_len, output, ST_SHA256_TIMEOUT) != HAL_OK)
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
    {
        return ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    ctx->sbuf_len = 0;
    return 0;
}
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_SHA256_H_ */
