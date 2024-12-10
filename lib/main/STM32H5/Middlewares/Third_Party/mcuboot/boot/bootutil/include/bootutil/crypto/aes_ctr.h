/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2021 STMicroelectronics
 */

/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are three choices:
 * MCUBOOT_USE_MBED_TLS, MCUBOOT_USE_TINYCRYPT or MCUBOOT_USE_HAL.
 * It is a compile error there is not exactly one of these defined.
 */

#ifndef __BOOTUTIL_CRYPTO_AES_CTR_H_
#define __BOOTUTIL_CRYPTO_AES_CTR_H_

#include <string.h>

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_MBED_TLS) + \
     defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_TINYCRYPT)) != 1
    #error "One crypto backend must be defined: either MBEDTLS or TINYCRYPT or HAL"
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
    #include <mbedtls/aes.h>
    #define BOOTUTIL_CRYPTO_AES_CTR_KEY_SIZE (16)
    #define BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE (16)
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
    #include <string.h>
    #include <tinycrypt/aes.h>
    #include <tinycrypt/ctr_mode.h>
    #include <tinycrypt/constants.h>
    #define BOOTUTIL_CRYPTO_AES_CTR_KEY_SIZE TC_AES_KEY_SIZE
    #define BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE TC_AES_BLOCK_SIZE
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>
    #define BOOTUTIL_CRYPTO_AES_CTR_KEY_SIZE (16)
    #define BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE (16)
    #define ST_AES_TIMEOUT     0xFFU   /* 255 ms timeout for the crypto processor */
    #define ST_AES_NO_ALGO     0xFFFFU /* any algo is programmed */
#endif /* MCUBOOT_USE_HAL */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
typedef mbedtls_aes_context bootutil_aes_ctr_context;
static inline void bootutil_aes_ctr_init(bootutil_aes_ctr_context *ctx)
{
    (void)mbedtls_aes_init(ctx);
}

static inline void bootutil_aes_ctr_drop(bootutil_aes_ctr_context *ctx)
{
    /* XXX: config defines MBEDTLS_PLATFORM_NO_STD_FUNCTIONS so no need to free */
    (void)mbedtls_aes_free(ctx); 
}

static inline int bootutil_aes_ctr_set_key(bootutil_aes_ctr_context *ctx, const uint8_t *k)
{
    return mbedtls_aes_setkey_enc(ctx, k, BOOTUTIL_CRYPTO_AES_CTR_KEY_SIZE * 8);
}

static inline int bootutil_aes_ctr_encrypt(bootutil_aes_ctr_context *ctx, uint8_t *counter, const uint8_t *m, uint32_t mlen, size_t blk_off, uint8_t *c)
{
    uint8_t stream_block[BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE];
    int rc;
    rc = mbedtls_aes_crypt_ctr(ctx, mlen, &blk_off, counter, stream_block, m, c);
    memset(stream_block, 0, BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE);
    return rc;
}

static inline int bootutil_aes_ctr_decrypt(bootutil_aes_ctr_context *ctx, uint8_t *counter, const uint8_t *c, uint32_t clen, size_t blk_off, uint8_t *m)
{
    uint8_t stream_block[BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE];
    int rc;
    rc = mbedtls_aes_crypt_ctr(ctx, clen, &blk_off, counter, stream_block, c, m);
    memset(stream_block, 0, BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE);
    return rc;
}
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
typedef struct tc_aes_key_sched_struct bootutil_aes_ctr_context;
static inline void bootutil_aes_ctr_init(bootutil_aes_ctr_context *ctx)
{
    (void)ctx;
}

static inline void bootutil_aes_ctr_drop(bootutil_aes_ctr_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_aes_ctr_set_key(bootutil_aes_ctr_context *ctx, const uint8_t *k)
{
    int rc;
    rc = tc_aes128_set_encrypt_key(ctx, k);
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    return 0;
}

static int _bootutil_aes_ctr_crypt(bootutil_aes_ctr_context *ctx, uint8_t *counter, const uint8_t *in, uint32_t inlen, uint32_t blk_off, uint8_t *out)
{
    uint8_t buf[16];
    uint32_t buflen;
    int rc;
    if (blk_off == 0) {
        rc = tc_ctr_mode(out, inlen, in, inlen, counter, ctx);
        if (rc != TC_CRYPTO_SUCCESS) {
            return -1;
        }
    } else if (blk_off < 16) {
        buflen = ((inlen + blk_off <= 16) ? inlen : (16 - blk_off));
        inlen -= buflen;
        memcpy(&buf[blk_off], &in[0], buflen);
        rc = tc_ctr_mode(buf, 16, buf, 16, counter, ctx);
        if (rc != TC_CRYPTO_SUCCESS) {
            return -1;
        }
        memcpy(&out[0], &buf[blk_off], buflen);
        memset(&buf[0], 0, 16);
        if (inlen > 0) {
            rc = tc_ctr_mode(&out[buflen], inlen, &in[buflen], inlen, counter, ctx);
        }
        if (rc != TC_CRYPTO_SUCCESS) {
            return -1;
        }
    } else {
        return -1;
    }
    return 0;
}

static inline int bootutil_aes_ctr_encrypt(bootutil_aes_ctr_context *ctx, uint8_t *counter, const uint8_t *m, uint32_t mlen, uint32_t blk_off, uint8_t *c)
{
    return _bootutil_aes_ctr_crypt(ctx, counter, m, mlen, blk_off, c);
}

static inline int bootutil_aes_ctr_decrypt(bootutil_aes_ctr_context *ctx, uint8_t *counter, const uint8_t *c, uint32_t clen, uint32_t blk_off, uint8_t *m)
{
    return _bootutil_aes_ctr_crypt(ctx, counter, c, clen, blk_off, m);
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
/**
 * \brief          AES context structure
 */
typedef struct
{
    /* Encryption/Decryption key */
    uint32_t aes_key[8];
    CRYP_HandleTypeDef hcryp_aes;   /* AES context */
}
bootutil_aes_ctr_context;

/* Private macro -------------------------------------------------------------*/
/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
do {                                                    \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
} while( 0 )
#endif

static int aes_setkey( bootutil_aes_ctr_context *ctx,
                       const unsigned char *key,
                       unsigned int keybits )
{
    unsigned int i;
    int ret = 0;

    INPUT_VALIDATE_RET( ctx != NULL );

    switch(keybits)
    {
        case 128:
            ctx->hcryp_aes.Init.KeySize       = CRYP_KEYSIZE_128B;
            break;
        case 256:
            ctx->hcryp_aes.Init.KeySize       = CRYP_KEYSIZE_256B;
            break;
        default:
            ret = ERR_AES_INVALID_KEY_LENGTH;
            goto exit;
    }

    /* Format and fill AES key  */
    for( i=0; i < (keybits/32); i++)
        GET_UINT32_BE( ctx->aes_key[i], key,4*i );

    ctx->hcryp_aes.Init.DataType      = CRYP_BYTE_SWAP;
    ctx->hcryp_aes.Init.pKey          = ctx->aes_key;
    ctx->hcryp_aes.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;


    /* Set the common CRYP parameters */
    ctx->hcryp_aes.Instance = SAES;
    ctx->hcryp_aes.Init.KeyMode = CRYP_KEYMODE_NORMAL;
    ctx->hcryp_aes.Init.KeySelect = CRYP_KEYSEL_NORMAL;
    ctx->hcryp_aes.Init.Algorithm     = CRYP_AES_ECB;
    
    /* Enable SAES clock */
    __HAL_RCC_SAES_CLK_ENABLE();

    if (HAL_CRYP_Init(&ctx->hcryp_aes) != HAL_OK)
    {
        ret = ERR_PLATFORM_HW_ACCEL_FAILED;
    }

exit:
    return ret;
}

static inline void bootutil_aes_ctr_init(bootutil_aes_ctr_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    memset( ctx, 0, sizeof( bootutil_aes_ctr_context ) );

    ctx->hcryp_aes.Init.Algorithm  = ST_AES_NO_ALGO;
}

static inline void bootutil_aes_ctr_drop(bootutil_aes_ctr_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );
    if (ctx->hcryp_aes.Instance != NULL)
    {
      HAL_CRYP_DeInit(&ctx->hcryp_aes);
    }
    /* Disable SAES clock */
    __HAL_RCC_SAES_CLK_DISABLE();
}

static inline int bootutil_aes_ctr_set_key(bootutil_aes_ctr_context *ctx, const uint8_t *k)
{
    return aes_setkey( ctx, k, BOOTUTIL_CRYPTO_AES_CTR_KEY_SIZE * 8 );
}

/*
 * AES-ECB block encryption
 */
static int aes_encrypt_ecb(bootutil_aes_ctr_context *ctx,
                    const unsigned char input[16],
                    unsigned char output[16])
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( input != NULL );
    INPUT_VALIDATE_RET( output != NULL );

    /* Set the Algo if not configured till now */
    if (CRYP_AES_ECB != ctx->hcryp_aes.Init.Algorithm)
    {
        ctx->hcryp_aes.Init.Algorithm  = CRYP_AES_ECB;

        /* Configure the CRYP  */
        if (HAL_CRYP_SetConfig(&ctx->hcryp_aes, &ctx->hcryp_aes.Init) != HAL_OK)
            return ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    /* AES encryption */
    if (HAL_CRYP_Encrypt(&ctx->hcryp_aes,
                         (uint32_t *)input, 16,
                         (uint32_t *)output,
                         ST_AES_TIMEOUT) != HAL_OK)
    {
        return ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    return (0);
}

/*
 * AES-CTR buffer encryption/decryption
 */
static int aes_crypt_ctr(bootutil_aes_ctr_context *ctx,
                          size_t length,
                          size_t *nc_off,
                          unsigned char nonce_counter[16],
                          unsigned char stream_block[16],
                          const unsigned char *input,
                          unsigned char *output)
{
    int ret = 0;
    int c, i;
    size_t n;

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( nc_off != NULL );
    INPUT_VALIDATE_RET( nonce_counter != NULL );
    INPUT_VALIDATE_RET( stream_block != NULL );
    INPUT_VALIDATE_RET( input != NULL );
    INPUT_VALIDATE_RET( output != NULL );

    n = *nc_off;

    while (length--) {
        if (n == 0) {
            if ( ( ret = aes_encrypt_ecb(ctx, nonce_counter, stream_block) ) != 0) {
                return ret;
            }

            for (i = 16; i > 0; i--)
                if (++nonce_counter[i - 1] != 0) {
                    break;
                }
        }
        c = *input++;
        *output++ = (unsigned char)(c ^ stream_block[n]);

        n = (n + 1) & 0x0F;
    }

    *nc_off = n;

    return (ret);
}

static inline int bootutil_aes_ctr_encrypt(bootutil_aes_ctr_context *ctx,
                                           uint8_t *counter,
                                           const uint8_t *m, uint32_t mlen,
                                           size_t blk_off,
                                           uint8_t *c)
{
    uint8_t stream_block[BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE];
    int rc;
    rc = aes_crypt_ctr(ctx, mlen, &blk_off, counter, stream_block, m, c);
    memset(stream_block, 0, BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE);
    return rc;
}

static inline int bootutil_aes_ctr_decrypt(bootutil_aes_ctr_context *ctx,
                                           uint8_t *counter,
                                           const uint8_t *c, uint32_t clen,
                                           size_t blk_off,
                                           uint8_t *m)
{
    uint8_t stream_block[BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE];
    int rc;
    rc = aes_crypt_ctr(ctx, clen, &blk_off, counter, stream_block, c, m);
    memset(stream_block, 0, BOOTUTIL_CRYPTO_AES_CTR_BLOCK_SIZE);
    return rc;
}
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_AES_CTR_H_ */
