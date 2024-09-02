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

#ifndef __BOOTUTIL_CRYPTO_ECDH_P256_H_
#define __BOOTUTIL_CRYPTO_ECDH_P256_H_

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_TINYCRYPT) + \
     defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_MBED_TLS)) != 1
    #error "One crypto backend must be defined: either MBEDTLS or TINYCRYPT or HAL"
#endif

#if defined(MCUBOOT_USE_MBED_TLS)
    #include "mbedtls/ecdh.h"
    #include "mbedtls/hkdf.h"
    #define BOOTUTIL_CRYPTO_ECDH_P256_HASH_SIZE (4 * 8)
#endif /* MCUBOOT_USE_MBED_TLS */

#if defined(MCUBOOT_USE_TINYCRYPT)
    #include <tinycrypt/ecc_dh.h>
    #include <tinycrypt/constants.h>
    #define BOOTUTIL_CRYPTO_ECDH_P256_HASH_SIZE (4 * 8)
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>
    #define SIZE_32 32
    #define ST_ECP_TIMEOUT     (5000U)
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
extern "C" {
#endif
#if defined(MCUBOOT_USE_MBED_TLS)
typedef mbedtls_ecdh_context bootutil_ecdh_p256_context;
static inline void bootutil_ecdh_p256_init(bootutil_ecdh_p256_context *ctx)
{
  mbedtls_ecdh_init(ctx);  
}
static inline void bootutil_ecdh_p256_drop(bootutil_ecdh_p256_context *ctx)
{
    mbedtls_ecdh_free(ctx);
}


static inline int bootutil_ecdh_p256_shared_secret(bootutil_ecdh_p256_context *ctx, const uint8_t *pk, const uint8_t *sk, uint8_t *z)
{
    int rc = -1;
    size_t olen; /* output len */
    rc = mbedtls_ecdh_setup(ctx,MBEDTLS_ECP_DP_SECP256R1);
    if (rc) {
        return -4;
    }
    rc = mbedtls_ecp_point_read_binary( &ctx->ctx.mbed_ecdh.grp,
            &ctx->ctx.mbed_ecdh.Qp,
            pk, 2*NUM_ECC_BYTES+1);
    if (rc) {
        return -1;
    }
    if (mbedtls_ecp_check_pubkey(&ctx->ctx.mbed_ecdh.grp, &ctx->ctx.mbed_ecdh.Qp)) {
        return -11;
    }
    /* import the private key */
    rc = mbedtls_mpi_read_binary(&ctx->ctx.mbed_ecdh.d, sk, NUM_ECC_BYTES);
    if (rc) {
        return -2;
    }
    /* compute secret */
    rc = mbedtls_ecdh_calc_secret( ctx,
                                   &olen, z,
                                   NUM_ECC_BYTES,
                                   NULL,
                                   NULL);
    if (rc) {
	return -3;
    }
    return 0;
}
#endif
#if defined(MCUBOOT_USE_TINYCRYPT)
typedef uintptr_t bootutil_ecdh_p256_context;
static inline void bootutil_ecdh_p256_init(bootutil_ecdh_p256_context *ctx)
{
    (void)ctx;
}

static inline void bootutil_ecdh_p256_drop(bootutil_ecdh_p256_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_ecdh_p256_shared_secret(bootutil_ecdh_p256_context *ctx, const uint8_t *pk, const uint8_t *sk, uint8_t *z)
{
    int rc;
    (void)ctx;

    rc = uECC_valid_public_key(pk, uECC_secp256r1());
    if (rc != 0) {
        return -1;
    }

    rc = uECC_shared_secret(pk, sk, z, uECC_secp256r1());
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
       

    return 0;
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_HAL)
typedef struct
{
    PKA_HandleTypeDef hpka;
}
bootutil_ecdh_p256_context;

static inline void bootutil_ecdh_p256_init(bootutil_ecdh_p256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    /* Enable HW peripheral clock */
    __HAL_RCC_PKA_CLK_ENABLE();

    memset( ctx, 0, sizeof( bootutil_ecdh_p256_context ) );
}
static inline void bootutil_ecdh_p256_drop(bootutil_ecdh_p256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );
    HAL_PKA_DeInit(&ctx->hpka);
    __HAL_RCC_PKA_CLK_DISABLE();
}

static inline int bootutil_ecdh_p256_shared_secret(bootutil_ecdh_p256_context *ctx, const uint8_t *pk, const uint8_t *sk, uint8_t *z)
{
    PKA_ECCMulInTypeDef ECC_MulIn = {0};
    PKA_ECCMulOutTypeDef ECC_MulOut = {0};
    PKA_PointCheckInTypeDef Point_CheckIn = {0};
    PKA_MontgomeryParamInTypeDef Montgomery_ParamIn = {0};
    uint32_t Montgomery[SIZE_32] = {0};

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( pk != NULL );
    INPUT_VALIDATE_RET( sk != NULL );
    INPUT_VALIDATE_RET( z != NULL );

    /* Set HW peripheral input parameter : curve coefs */
    ECC_MulIn.modulusSize    = SIZE_32;
    ECC_MulIn.coefSign       = P_256_a_sign;
    ECC_MulIn.coefA          = P_256_absA;
    ECC_MulIn.coefB          = P_256_b;
    ECC_MulIn.modulus        = P_256_p;
    ECC_MulIn.primeOrder     = P_256_n;

    /* Set HW peripheral input parameter : public key */
    /* Store public key except the first byte that is on the uncompressed form */
    ECC_MulIn.pointX = (uint8_t *)(&pk[1]);
    ECC_MulIn.pointY = (uint8_t *)(&pk[NUM_ECC_BYTES + 1]);

    /* Store private in local buffer to insure the persistency */
    ECC_MulIn.scalarMulSize = NUM_ECC_BYTES;
    ECC_MulIn.scalarMul = (uint8_t *)sk;

    /* Initialize HW peripheral */
    ctx->hpka.Instance = PKA;
    if (HAL_PKA_Init(&ctx->hpka) != HAL_OK)
    {
        return ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    /* Reset PKA RAM */
    HAL_PKA_RAMReset(&ctx->hpka);

    /* Check if the pub key is in the curve */
    Point_CheckIn.modulusSize = ECC_MulIn.modulusSize;
    Point_CheckIn.coefSign = ECC_MulIn.coefSign;
    Point_CheckIn.coefA = ECC_MulIn.coefA;
    Point_CheckIn.coefB = ECC_MulIn.coefB;
    Point_CheckIn.modulus = ECC_MulIn.modulus;
    Point_CheckIn.pointX = ECC_MulIn.pointX;
    Point_CheckIn.pointY = ECC_MulIn.pointY;
    Point_CheckIn.pMontgomeryParam = Montgomery;

    Montgomery_ParamIn.size = SIZE_32;
    Montgomery_ParamIn.pOp1 = P_256_p;
      
    if (HAL_PKA_MontgomeryParam(&ctx->hpka, &Montgomery_ParamIn, ST_ECP_TIMEOUT) != HAL_OK)
    {
       return ERR_PLATFORM_HW_ACCEL_FAILED;
    }
      
    HAL_PKA_MontgomeryParam_GetResult(&ctx->hpka, Montgomery);
    
    if (HAL_PKA_PointCheck(&ctx->hpka, &Point_CheckIn, ST_ECP_TIMEOUT) != HAL_OK)
    {
       return ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    
    if (HAL_PKA_PointCheck_IsOnCurve(&ctx->hpka) != 1U)
    {
       return ERR_AES_INVALID_KEY_LENGTH;
    }
  
    /* compute secret */
    /* Start the ECC scalar multiplication */
    if (HAL_PKA_ECCMul(&ctx->hpka, &ECC_MulIn, ST_ECP_TIMEOUT) != HAL_OK)
    {
        return ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    ECC_MulOut.ptX = (uint8_t *)z;
    ECC_MulOut.ptY = NULL;

    HAL_PKA_ECCMul_GetResult(&ctx->hpka , &ECC_MulOut);

    return 0;
}
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_ECDH_P256_H_ */
