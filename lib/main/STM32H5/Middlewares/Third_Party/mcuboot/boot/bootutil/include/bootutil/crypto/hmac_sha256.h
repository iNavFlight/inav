/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are two choices: MCUBOOT_USE_MBED_TLS, or
 * MCUBOOT_USE_TINYCRYPT.  It is a compile error there is not exactly
 * one of these defined.
 *
 * Copyright (c) 2023 STMicroelectronics. All rights reserved.
 *
 */

#ifndef __BOOTUTIL_CRYPTO_HMAC_SHA256_H_
#define __BOOTUTIL_CRYPTO_HMAC_SHA256_H_

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_TINYCRYPT) + \
     defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_MBED_TLS)) != 1
    #error "One crypto backend must be defined: either MBEDTLS or TINYCRYPT"
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
     #include "mbedtls/md.h"
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
    #include <tinycrypt/sha256.h>
    #include <tinycrypt/utils.h>
    #include <tinycrypt/constants.h>
    #include <tinycrypt/hmac.h>
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>

    #define ST_HMAC_SHA256_TIMEOUT     ((uint32_t) 5000)
    #define BOOTUTIL_CRYPTO_HMAC_SHA256_DIGEST_SIZE (32)
    #define BOOTUTIL_CRYPTO_HMAC_SHA256_MSG_SIZE (64)
#endif /* MCUBOOT_USE_HAL */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#if defined(MCUBOOT_USE_MBED_TLS)
typedef struct mbedtls_md_context_t bootutil_hmac_sha256_context;
static inline void  bootutil_hmac_sha256_init(bootutil_hmac_sha256_context *ctx)
{
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(ctx);
    mbedtls_md_setup(ctx, mbedtls_md_info_from_type(md_type) , 1);
}
static inline void bootutil_hmac_sha256_drop(bootutil_hmac_sha256_context *ctx)
{
    mbedtls_md_free(ctx);
}
static inline int bootutil_hmac_sha256_set_key(bootutil_hmac_sha256_context *ctx, const uint8_t *key, unsigned int key_size)
{
    return mbedtls_md_hmac_starts(ctx, key, key_size);
}

static inline int bootutil_hmac_sha256_update(bootutil_hmac_sha256_context *ctx, const void *data, unsigned int data_length)
{
    return mbedtls_md_hmac_update(ctx, (const unsigned char *) data, data_length);
}
static inline int bootutil_hmac_sha256_finish(bootutil_hmac_sha256_context *ctx, uint8_t *tag, unsigned int taglen)
{
    return mbedtls_md_hmac_finish(ctx, tag);
}
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
typedef struct tc_hmac_state_struct bootutil_hmac_sha256_context;
static inline void bootutil_hmac_sha256_init(bootutil_hmac_sha256_context *ctx)
{
    (void)ctx;
}

static inline void bootutil_hmac_sha256_drop(bootutil_hmac_sha256_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_hmac_sha256_set_key(bootutil_hmac_sha256_context *ctx, const uint8_t *key, unsigned int key_size)
{
    int rc;
    rc = tc_hmac_set_key(ctx, key, key_size);
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    rc = tc_hmac_init(ctx);
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    return 0;
}


static inline int bootutil_hmac_sha256_update(bootutil_hmac_sha256_context *ctx, const void *data, unsigned int data_length)
{
    int rc;
    rc = tc_hmac_update(ctx, data, data_length);
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    return 0;
}

static inline int bootutil_hmac_sha256_finish(bootutil_hmac_sha256_context *ctx, uint8_t *tag, unsigned int taglen)
{
    int rc;
    rc = tc_hmac_final(tag, taglen, ctx);
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    return 0;
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
/**
 * \brief          HMAC SHA-256 context structure
 */
typedef struct
{
    HASH_HandleTypeDef hhash;                       /*!< Handle of HASH HAL */
    uint8_t sbuf[BOOTUTIL_CRYPTO_HMAC_SHA256_MSG_SIZE];
                                                    /*!< Buffer to store input data */
    uint8_t sbuf_len;                               /*!< Number of bytes stored in sbuf */
}
bootutil_hmac_sha256_context;
static inline void  bootutil_hmac_sha256_init(bootutil_hmac_sha256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    memset( ctx, 0, sizeof( bootutil_hmac_sha256_context ) );
}
static inline void bootutil_hmac_sha256_drop(bootutil_hmac_sha256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    HAL_HASH_DeInit(&ctx->hhash);

    /* Disable HASH clock */
    __HAL_RCC_HASH_CLK_DISABLE();
}
static inline int bootutil_hmac_sha256_set_key(bootutil_hmac_sha256_context *ctx, /*const*/ uint8_t *key, unsigned int key_size)
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( key != NULL );
    INPUT_VALIDATE_RET( key_size != 0 );

    /* Enable HASH clock */
    __HAL_RCC_HASH_CLK_ENABLE();

#if defined(MCUBOOT_USE_HASH_HAL_V1)
    ctx->hhash.Init.DataType =  HASH_DATATYPE_8B;
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
    ctx->hhash.Instance = HASH;
    ctx->hhash.Init.DataType = HASH_BYTE_SWAP;
    ctx->hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA256;
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
    ctx->hhash.Init.pKey = key;
    ctx->hhash.Init.KeySize = key_size;

    HAL_HASH_Init(&ctx->hhash);

    return 0;
}

static inline int bootutil_hmac_sha256_update(bootutil_hmac_sha256_context *ctx, const void *data, unsigned int data_length)
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( data != NULL );

    if (ctx->sbuf_len + data_length > BOOTUTIL_CRYPTO_HMAC_SHA256_MSG_SIZE)
        return -1;

    memcpy(ctx->sbuf + ctx->sbuf_len, data, data_length);
    ctx->sbuf_len += data_length;

    return 0;
}
static inline int bootutil_hmac_sha256_finish(bootutil_hmac_sha256_context *ctx, uint8_t *tag, unsigned int taglen)
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( tag != NULL );
    INPUT_VALIDATE_RET( taglen == BOOTUTIL_CRYPTO_HMAC_SHA256_DIGEST_SIZE );

    (void)taglen;

#if defined(MCUBOOT_USE_HASH_HAL_V1)
    if (HAL_HMACEx_SHA256_Start(&ctx->hhash,
                                ctx->sbuf,
                                ctx->sbuf_len,
                                tag,
                                ST_HMAC_SHA256_TIMEOUT) != HAL_OK)
#else /* not MCUBOOT_USE_HASH_HAL_V1 */
    if (HAL_HASH_HMAC_Start(&ctx->hhash,
                            ctx->sbuf,
                            ctx->sbuf_len,
                            tag,
                            ST_HMAC_SHA256_TIMEOUT) != HAL_OK)
#endif /* MCUBOOT_USE_HASH_HAL_V1 */
    {
        return ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    return 0;
}
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_HMAC_SHA256_H_ */
