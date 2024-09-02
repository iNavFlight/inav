/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2021 STMicroelectronics
 */

/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are two choices:
 * MCUBOOT_USE_MBED_TLS or MCUBOOT_USE_HAL.
 * It is a compile error if there is not exactly one of these defined.
 */

#ifndef __BOOTUTIL_CRYPTO_RSA_OAEP_H_
#define __BOOTUTIL_CRYPTO_RSA_OAEP_H_

#include "mcuboot_config/mcuboot_config.h"
#include "mbedtls/asn1.h"

#if (defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_MBED_TLS)) != 1
    #error "One crypto backend must be defined: either MBEDTLS or HAL"
#endif

#include "bootutil/crypto/sha256.h"

#if defined(MCUBOOT_USE_MBED_TLS)
    #include "mbedtls/rsa.h"
    #include "mbedtls/rsa_internal.h"
    #include "mbedtls/version.h"
    #define BOOTUTIL_RSA_PKCS_V21  MBEDTLS_RSA_PKCS_V21
    #define BOOTUTIL_MD_SHA256     MBEDTLS_MD_SHA256
    #define BOOTUTIL_RSA_PRIVATE   MBEDTLS_RSA_PRIVATE
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>
    #define ST_PKA_TIMEOUT     5000      /* 5s timeout for the Public key accelerator */
    #define MPI_MAX_SIZE       MCUBOOT_SIGN_RSA_LEN/8
    #define BOOTUTIL_RSA_PKCS_V21  1
    #define BOOTUTIL_MD_SHA256     0
    #define BOOTUTIL_RSA_PRIVATE   1
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
typedef mbedtls_rsa_context bootutil_rsa_context;

static inline int bootutil_asn1_get_rsa_number( unsigned char **p,
                  const unsigned char *end,
                  mbedtls_mpi *X )
{
    return( mbedtls_asn1_get_mpi( p, end, X) );
}

static inline int bootutil_rsa_number_size( mbedtls_mpi *X )
{
    return( mbedtls_mpi_size( X ) );
}

static inline void bootutil_rsa_init(bootutil_rsa_context *ctx,
                                     int padding,
                                     int hash_id )
{
    mbedtls_rsa_init(ctx, padding, hash_id);
}

static inline void bootutil_rsa_drop(bootutil_rsa_context *ctx)
{
    mbedtls_rsa_free(ctx);
}

static inline int bootutil_rsa_public(bootutil_rsa_context *ctx,
                                      const unsigned char *input,
                                      unsigned char *output)
{
    return mbedtls_rsa_public( ctx, input, output );
}

static inline int bootutil_rsa_oaep_decrypt( bootutil_rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t output_max_len )
{
    return mbedtls_rsa_rsaes_oaep_decrypt( ctx,
                                           f_rng,
                                           p_rng,
                                           mode,
                                           label, label_len,
                                           olen,
                                           input,
                                           output,
                                           output_max_len );
}
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_HAL)
typedef struct
{
    size_t len;
    unsigned char *p;
}
rsa_integer;

typedef struct
{
    int ver;                    /*!<  Always 0.*/
    size_t len;                 /*!<  The size of \p N in Bytes. */

    rsa_integer N;              /*!<  The public modulus. */
    rsa_integer E;              /*!<  The public exponent. */
    rsa_integer D;              /*!<  The private exponent. */

    rsa_integer P;              /*!<  The first prime factor. */
    rsa_integer Q;              /*!<  The second prime factor. */
    rsa_integer Phi;            /*!<  The Euler tolient function. */

    PKA_HandleTypeDef hpka;
}
bootutil_rsa_context;

static inline int bootutil_asn1_get_rsa_number( unsigned char **p,
                  const unsigned char *end,
                  rsa_integer *X )
{
    int ret;
    size_t len;

    if( ( ret = mbedtls_asn1_get_tag( p, end, &len, MBEDTLS_ASN1_INTEGER ) ) != 0 )
        return( ret );

    /* Skip leading zeros. */
    while( len > 0 && **p == 0 )
    {
        ++( *p );
        --len;
    }

    /* retrieve N */
    X->len = len;
    X->p = *p;

    *p += len;

    return( ret );
}

static inline int bootutil_rsa_number_size( rsa_integer *X )
{
    return( X->len );
}

static inline void bootutil_rsa_init(bootutil_rsa_context *ctx,
                                     int padding,
                                     int hash_id )
{
    INPUT_VALIDATE( ctx != NULL );

    ((void) padding);
    ((void) hash_id);

    /* Enable HW peripheral clock */
    __HAL_RCC_PKA_CLK_ENABLE();

    memset( ctx, 0, sizeof( bootutil_rsa_context ) );
}

static inline void bootutil_rsa_drop(bootutil_rsa_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );
    /* De-initialize HW peripheral */
    HAL_PKA_DeInit( &ctx->hpka );
    __HAL_RCC_PKA_CLK_DISABLE();
}

/**
 * @brief       Call the PKA modular exponentiation : output = input^e mod n
 * @param[in]   input        Input of the modexp
 * @param[in]   ctx          RSA context
 * @param[in]   is_private   public (0) or private (1) exponentiation
 * @param[in]   is_protected normal (0) or protected (1) exponentiation
 * @param[out]  output       Output of the ModExp (with length of the modulus)
 * @retval      0                                       Ok
 * @retval      MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED    Error in the HW
 */
static int rsa_pka_modexp( bootutil_rsa_context *ctx,
                           int is_private,
                           int is_protected,
                           const unsigned char *input,
                           unsigned char *output )
{
    int ret = 0;
    size_t elen;
    size_t nlen;
    PKA_ModExpInTypeDef in = {0};

    __ALIGN_BEGIN uint8_t e_binary[MPI_MAX_SIZE] = {0} __ALIGN_END;
    __ALIGN_BEGIN uint8_t n_binary[MPI_MAX_SIZE] = {0} __ALIGN_END;

#if 0
    /* parameters for exponentiation in protected mode */
    size_t philen;
    PKA_ModExpProtectModeInTypeDef in_protected = {0};
#endif

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( input != NULL );
    INPUT_VALIDATE_RET( output != NULL );

    if ( is_private )
    {
        elen = ctx->D.len;
    }
    else
    {
        elen = ctx->E.len;
    }

    /* exponent aligned on 4 bytes (driver requirement)*/
    elen = ((elen + 3)/4)*4;

    if ( is_private )
    {
        memcpy(e_binary + elen - ctx->D.len ,ctx->D.p, ctx->D.len);
    }
    else
    {
        memcpy(e_binary + elen - ctx->E.len ,ctx->E.p, ctx->E.len);
    }

    nlen = ctx->N.len;
    memcpy(n_binary, ctx->N.p, nlen);

#if 0
    if ( is_protected )
    {
        philen = mbedtls_mpi_size( &ctx->Phi );

        /* first phi computation */
        if ( 0 == philen )
        {
            MBEDTLS_MPI_CHK( rsa_deduce_phi( &ctx->P, &ctx->Q, &ctx->Phi ) );
            philen = mbedtls_mpi_size( &ctx->Phi );
        }

        phi_binary = mbedtls_calloc( 1, philen );
        MBEDTLS_MPI_CHK( ( phi_binary == NULL ) ? MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED : 0 );
        MBEDTLS_MPI_CHK( mbedtls_mpi_write_binary( &ctx->Phi, phi_binary, philen ) );

        in_protected.expSize = elen;           /* Exponent length */
        in_protected.OpSize  = nlen;           /* modulus length */
        in_protected.pOp1    = input;
        in_protected.pExp    = e_binary;       /* Exponent */
        in_protected.pMod    = n_binary;       /* modulus */
        in_protected.pPhi    = phi_binary;     /* Euler tolient function */
    }
    else
#endif
    /* exponention in normal mode */
    {
        in.expSize = elen;           /* Exponent length */
        in.OpSize  = nlen;           /* modulus length */
        in.pOp1    = input;
        in.pExp    = e_binary;       /* Exponent */
        in.pMod    = n_binary;       /* modulus */
    }

    /* Initialize HW peripheral */
    ctx->hpka.Instance = PKA;
    CHK( ( HAL_PKA_Init( &ctx->hpka ) != HAL_OK ) ? ERR_PLATFORM_HW_ACCEL_FAILED : 0 );

    /* Reset PKA RAM */
    HAL_PKA_RAMReset(&ctx->hpka);

#if 0
    if ( is_protected )
    {
        /* output = input ^ e_binary mod n (protected mode) */
        CHK( ( HAL_PKA_ModExpProtectMode( &ctx->hpka, &in_protected, ST_PKA_TIMEOUT ) != HAL_OK ) ? ERR_PLATFORM_HW_ACCEL_FAILED : 0 );
    }
    else
#endif
    {
         /* output = input ^ e_binary mod n (normal mode) */
        CHK( ( HAL_PKA_ModExp( &ctx->hpka, &in, ST_PKA_TIMEOUT ) != HAL_OK ) ? ERR_PLATFORM_HW_ACCEL_FAILED : 0 );
    }

    HAL_PKA_ModExp_GetResult( &ctx->hpka, (uint8_t *)output );

cleanup:

#if 0
    if (phi_binary != NULL)
    {
        mbedtls_platform_zeroize( phi_binary, philen );
        mbedtls_free( phi_binary );
    }
#endif

    return ret;
}

static inline int bootutil_rsa_public(bootutil_rsa_context *ctx,
                                      const unsigned char *input,
                                      unsigned char *output)
{
    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( input != NULL );
    INPUT_VALIDATE_RET( output != NULL );

    return rsa_pka_modexp( ctx,
                            0 /* public */,
                            0 /* unprotected mode */,
                            input, output );
}

/**
 * Generate and apply the MGF1 operation (from PKCS#1 v2.1) to a buffer.
 *
 * \param dst       buffer to mask
 * \param dlen      length of destination buffer
 * \param src       source of the mask generation
 * \param slen      length of the source buffer
 * \param md_ctx    message digest context to use
 */
static int mgf_mask( unsigned char *dst, size_t dlen, unsigned char *src,
                      size_t slen, bootutil_sha256_context *md_ctx )
{
    unsigned char mask[32];
    unsigned char counter[4];
    unsigned char *p;
    unsigned int hlen;
    size_t i, use_len;
    int ret = 0;

    memset( mask, 0, 32 );
    memset( counter, 0, 4 );

    hlen = 32;

    /* Generate and apply dbMask */
    p = dst;

    while( dlen > 0 )
    {
        use_len = hlen;
        if( dlen < hlen )
            use_len = dlen;

        if( ( ret = bootutil_sha256_update( md_ctx, src, slen ) ) != 0 )
            goto exit;
        if( ( ret = bootutil_sha256_update( md_ctx, counter, 4 ) ) != 0 )
            goto exit;
        if( ( ret = bootutil_sha256_finish( md_ctx, mask ) ) != 0 )
            goto exit;

        for( i = 0; i < use_len; ++i )
            *p++ ^= mask[i];

        counter[3]++;

        dlen -= use_len;
    }

exit:
    //mbedtls_platform_zeroize( mask, sizeof( mask ) );

    return( ret );
}


static inline int bootutil_rsa_oaep_decrypt( bootutil_rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t output_max_len )
{
    int ret;

    size_t ilen, i, pad_len;
    unsigned char *p, bad, pad_done;
    unsigned char buf[MPI_MAX_SIZE];
    unsigned char lhash[32];
    unsigned int hlen;
    bootutil_sha256_context md_ctx;

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( output_max_len == 0 || output != NULL );
    INPUT_VALIDATE_RET( input != NULL );
    INPUT_VALIDATE_RET( olen != NULL );

    ((void) f_rng);
    ((void) p_rng);

    /*
     * Parameters sanity checks
     */
    ilen = ctx->N.len;

    if( ilen < 16 || ilen > sizeof( buf ) )
        return( ERR_BAD_INPUT_DATA );

    hlen = 32;

    // checking for integer underflow
    if( 2 * hlen + 2 > ilen )
        return( ERR_BAD_INPUT_DATA );

    /*
     * RSA operation
     */
    ret = rsa_pka_modexp(ctx,
                         1 /* private */,
                         0 /* unprotected mode */,
                         input, buf );

    if( ret != 0 )
        goto cleanup;

    /*
     * Unmask data and generate lHash
     */
    bootutil_sha256_init( &md_ctx );

    /* seed: Apply seedMask to maskedSeed */
    if( ( ret = mgf_mask( buf + 1, hlen, buf + hlen + 1, ilen - hlen - 1,
                          &md_ctx ) ) != 0 ||
    /* DB: Apply dbMask to maskedDB */
        ( ret = mgf_mask( buf + hlen + 1, ilen - hlen - 1, buf + 1, hlen,
                          &md_ctx ) ) != 0 )
    {
        bootutil_sha256_drop( &md_ctx );
        goto cleanup;
    }

    bootutil_sha256_drop( &md_ctx );

    /* Generate lHash */
    bootutil_sha256_init( &md_ctx );
    if( (ret = bootutil_sha256_update( &md_ctx, label, label_len )) != 0 )
        goto cleanup;
    if( (ret = bootutil_sha256_finish( &md_ctx, lhash )) != 0 )
        goto cleanup;

    /*
     * Check contents, in "constant-time"
     */
    p = buf;
    bad = 0;

    bad |= *p++; /* First byte must be 0 */

    p += hlen; /* Skip seed */

    /* Check lHash */
    for( i = 0; i < hlen; i++ )
        bad |= lhash[i] ^ *p++;

    /* Get zero-padding len, but always read till end of buffer
     * (minus one, for the 01 byte) */
    pad_len = 0;
    pad_done = 0;
    for( i = 0; i < ilen - 2 * hlen - 2; i++ )
    {
        pad_done |= p[i];
        pad_len += ((pad_done | (unsigned char)-pad_done) >> 7) ^ 1;
    }

    p += pad_len;
    bad |= *p++ ^ 0x01;

    /*
     * The only information "leaked" is whether the padding was correct or not
     * (eg, no data is copied if it was not correct). This meets the
     * recommendations in PKCS#1 v2.2: an opponent cannot distinguish between
     * the different error conditions.
     */
    if( bad != 0 )
    {
        //ret = MBEDTLS_ERR_RSA_INVALID_PADDING;
        ret = -3;
        goto cleanup;
    }

    if( ilen - ( p - buf ) > output_max_len )
    {
        //ret = MBEDTLS_ERR_RSA_OUTPUT_TOO_LARGE;
        ret = -4;
        goto cleanup;
    }

    *olen = ilen - (p - buf);
    if( *olen != 0 )
        memcpy( output, p, *olen );
    ret = 0;

cleanup:
    //mbedtls_platform_zeroize( buf, sizeof( buf ) );
    //mbedtls_platform_zeroize( lhash, sizeof( lhash ) );

    return( ret );
}

#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_RSA_OAEP_H_ */
