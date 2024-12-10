/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2021 STMicroelectronics
 */

/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are two choices: MCUBOOT_USE_MBED_TLS, or
 * MCUBOOT_USE_TINYCRYPT.  It is a compile error there is not exactly
 * one of these defined.
 */

#ifndef __BOOTUTIL_CRYPTO_ECDSA_P256_H_
#define __BOOTUTIL_CRYPTO_ECDSA_P256_H_

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_TINYCRYPT) + \
     defined(MCUBOOT_USE_CC310) + \
     defined(MCUBOOT_USE_HAL) + \
     defined(MCUBOOT_USE_MBED_TLS)) != 1
    #error "One crypto backend must be defined: either CC310 or TINYCRYPT"
#endif

#if defined(MCUBOOT_USE_TINYCRYPT)
    #include <tinycrypt/ecc_dsa.h>
    #include <tinycrypt/constants.h>
    #define BOOTUTIL_CRYPTO_ECDSA_P256_HASH_SIZE (4 * 8)
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_CC310)
    #include <cc310_glue.h>
    #define BOOTUTIL_CRYPTO_ECDSA_P256_HASH_SIZE (4 * 8)
#endif /* MCUBOOT_USE_CC310 */

#if defined(MCUBOOT_USE_HAL)
    #include <cryptoboot_hal.h>
    #define ST_ECDSA_TIMEOUT     (5000U)
#if  defined(MCUBOOT_DOUBLE_SIGN_VERIF)
    #include "boot_hal_imagevalid.h"
    #include "bootutil_priv.h"
    #define PKA_ECDSA_VERIF_OUT_SIGNATURE_R    ((PKA_ECDSA_SIGNATURE_ADDRESS - PKA_RAM_OFFSET)>>2)   /*!< Output result */
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
extern "C" {
#endif
#if defined(MCUBOOT_USE_MBED_TLS)
typedef mbedtls_ecp_keypair bootutil_ecdsa_p256_context;
static inline void bootutil_ecdsa_p256_init(bootutil_ecdsa_p256_context *ctx)
{
    mbedtls_ecp_keypair_init(ctx);
}
#define BITS_TO_BYTES(bits) (((bits) + 7) / 8)

static inline int bootutil_ecdsa_p256_verify(bootutil_ecdsa_p256_context *ctx, const uint8_t *pk, int len, const uint8_t *hash, const uint8_t *sig)
{
    int rc=-1;
    mbedtls_mpi r, s;
    size_t curve_bytes;

    rc = mbedtls_ecp_group_load(&ctx->grp,  MBEDTLS_ECP_DP_SECP256R1);
    if (rc)
        return -1;
    /*  size is hardcoded required  */
    rc = mbedtls_ecp_point_read_binary( &ctx->grp, &ctx->Q,
			pk, 65);
    if (rc) return -4;
    /* Check that the point is on the curve. */
	rc = mbedtls_ecp_check_pubkey( &ctx->grp, &ctx->Q );
    if (rc) return -5;
    /* set signature   */
	mbedtls_mpi_init( &r );
	mbedtls_mpi_init( &s );
    curve_bytes = BITS_TO_BYTES( ctx->grp.pbits );
    rc = mbedtls_mpi_read_binary(&r,sig, curve_bytes);
    if (rc) goto out;
    rc = mbedtls_mpi_read_binary(&s,sig + curve_bytes, curve_bytes);
    if (rc) goto out;
    rc = mbedtls_ecdsa_verify( &ctx->grp, hash , NUM_ECC_BYTES,
			&ctx->Q, &r, &s );
out:
    mbedtls_mpi_free( &r );
    mbedtls_mpi_free( &s );
    return rc;
}
static inline void bootutil_ecdsa_p256_drop(bootutil_ecdsa_p256_context *ctx)
{
    mbedtls_ecp_keypair_free(ctx);
}


#endif
#if defined(MCUBOOT_USE_TINYCRYPT)
typedef mbedtls_ecp_keypair bootutil_ecdsa_p256_context;
static inline void bootutil_ecdsa_p256_init(bootutil_ecdsa_p256_context *ctx)
{
    mbedtls_ecp_keypair_init(ctx);
    
}

static inline void bootutil_ecdsa_p256_drop(bootutil_ecdsa_p256_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_ecdsa_p256_verify(bootutil_ecdsa_p256_context *ctx, const uint8_t *pk, const uint8_t *hash, const uint8_t *sig)
{
    int rc;
    rc = mbedtls_ecp_group_load(&ctx->grp,  MBEDTLS_ECP_DP_SECP256R1)
    if (rc)
        return -1;
    mbedtls_ecp_point_read_binary( &ecp.grp, &ecp.Q,
			pk, (end - pubkey));
    
        ;rc = uECC_verify(pk, hash, BOOTUTIL_CRYPTO_ECDSA_P256_HASH_SIZE, sig, uECC_secp256r1());
    if (rc != TC_CRYPTO_SUCCESS) {
        return -1;
    }
    return 0;
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#if defined(MCUBOOT_USE_CC310)
typedef uintptr_t bootutil_ecdsa_p256_context;
static inline void bootutil_ecdsa_p256_init(bootutil_ecdsa_p256_context *ctx)
{
    (void)ctx;
}

static inline void bootutil_ecdsa_p256_drop(bootutil_ecdsa_p256_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_ecdsa_p256_verify(bootutil_ecdsa_p256_context *ctx, uint8_t *pk, uint8_t *hash, uint8_t *sig)
{
    (void)ctx;
    return cc310_ecdsa_verify_secp256r1(hash, pk, sig, BOOTUTIL_CRYPTO_ECDSA_P256_HASH_SIZE);
}
#endif /* MCUBOOT_USE_CC310 */

#if defined(MCUBOOT_USE_HAL)
typedef struct
{
    PKA_HandleTypeDef hpka;
}
bootutil_ecdsa_p256_context;

static inline void bootutil_ecdsa_p256_init(bootutil_ecdsa_p256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );

    /* Enable HW peripheral clock */
    __HAL_RCC_PKA_CLK_ENABLE();

    memset( ctx, 0, sizeof( bootutil_ecdsa_p256_context ) );
}

#if  defined(MCUBOOT_DOUBLE_SIGN_VERIF)
/**
  * @brief  Check PKA signature with a constant time execution.
  * @param  hpka PKA handle
  * @param  in Input information
  * @retval IMAGE_VALID if equal, IMAGE_INVALID otherwise.
  */
static int CheckPKASignature(PKA_HandleTypeDef *hpka, PKA_ECDSAVerifInTypeDef *in)
{
  __IO uint8_t result = 0;
  uint32_t i;
  uint32_t j;
  uint8_t* p_sign_PKA = (uint8_t*) &hpka->Instance->RAM[PKA_ECDSA_VERIF_OUT_SIGNATURE_R];
  uint8_t* pSign = (uint8_t*)in->RSign;
  uint32_t Size =  in->primeOrderSize;

  /* Signature comparison LSB vs MSB */
  for (i = 0U, j = Size - 1U; i < Size; i++, j--)
  {
    result |= pSign[i] ^ IMAGE_VALID ^ p_sign_PKA[j];
  }

  /* Loop fully executed ==> no basic HW attack */
  /* Any other unexpected result */
  if ( (i != Size) || (result != IMAGE_VALID) )
  {
    result = IMAGE_INVALID;
  }

  return result;
}
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */

static inline int bootutil_ecdsa_p256_verify(bootutil_ecdsa_p256_context *ctx, const uint8_t *pk, int len, const uint8_t *hash, const uint8_t *sig)
{
    PKA_ECDSAVerifInTypeDef ECDSA_VerifyIn = {0};

    INPUT_VALIDATE_RET( ctx != NULL );
    INPUT_VALIDATE_RET( pk != NULL );
    INPUT_VALIDATE_RET( hash != NULL );
    INPUT_VALIDATE_RET( sig != NULL );
    INPUT_VALIDATE_RET( len == (2*NUM_ECC_BYTES+1) );

    /* Set HW peripheral input parameter : curve coefs */
    ECDSA_VerifyIn.primeOrderSize =  sizeof(P_256_n);
    ECDSA_VerifyIn.modulusSize =     sizeof(P_256_p);
    ECDSA_VerifyIn.modulus =         P_256_p;
    ECDSA_VerifyIn.coefSign =        P_256_a_sign;
    ECDSA_VerifyIn.coef =            P_256_absA;
    ECDSA_VerifyIn.basePointX =      P_256_Gx;
    ECDSA_VerifyIn.basePointY =      P_256_Gy;
    ECDSA_VerifyIn.primeOrder =      P_256_n;

    /* Set HW peripheral input parameter : hash that was signed */
    ECDSA_VerifyIn.hash = hash;

   /* Set HW peripheral input parameter : public key */
   /* Store public key except the first byte that is on the uncompressed form */
   ECDSA_VerifyIn.pPubKeyCurvePtX = (uint8_t *)(&pk[1]);
   ECDSA_VerifyIn.pPubKeyCurvePtY = (uint8_t *)(&pk[NUM_ECC_BYTES + 1]);

   ECDSA_VerifyIn.RSign = &sig[0];
   ECDSA_VerifyIn.SSign = &sig[NUM_ECC_BYTES];

   /* Initialize HW peripheral */
   ctx->hpka.Instance = PKA;
   if (HAL_PKA_Init(&ctx->hpka) != HAL_OK)
   {
       return ERR_PLATFORM_HW_ACCEL_FAILED;
   }

   /* Reset PKA RAM */
   HAL_PKA_RAMReset(&ctx->hpka);

   if (HAL_PKA_ECDSAVerif(&ctx->hpka, &ECDSA_VerifyIn, ST_ECDSA_TIMEOUT) != HAL_OK)
   {
       return ERR_PLATFORM_HW_ACCEL_FAILED;
   }

   /* Check the result */
   if (HAL_PKA_ECDSAVerif_IsValidSignature(&ctx->hpka) != 1U)
   {
       return ERR_ECP_VERIFY_FAILED;
   }
#if defined(MCUBOOT_DOUBLE_SIGN_VERIF)
    /* Double the signature verification (using another way) to resist to basic HW attacks.
     * The second verification is applicable to final signature check on primary slot images
     * only (condition: ImageValidEnable).
     * It is performed in 2 steps:
     * 1- save signature status in global variable ImageValidStatus[]
     *    Return value of HAL api (0 failed, 1 passed) is mul with IMAGE_VALID to avoid
     *    value 1 for success: IMAGE_VALID for success.
     * 2- verify saved signature status later in boot process
     */
    if (ImageValidEnable == 1)
    {
        /* Check ImageValidIndex is in expected range MCUBOOT_IMAGE_NUMBER */
        if (ImageValidIndex >= MCUBOOT_IMAGE_NUMBER)
        {
            return ERR_ECP_VERIFY_FAILED;
        }
        ImageValidStatus[ImageValidIndex++] = CheckPKASignature(&ctx->hpka, &ECDSA_VerifyIn);
    }
#endif /* MCUBOOT_DOUBLE_SIGN_VERIF */
   return 0;
}
static inline void bootutil_ecdsa_p256_drop(bootutil_ecdsa_p256_context *ctx)
{
    INPUT_VALIDATE( ctx != NULL );
    HAL_PKA_DeInit(&ctx->hpka);
    __HAL_RCC_PKA_CLK_DISABLE();
}
#endif /* MCUBOOT_USE_HAL */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_ECDSA_P256_H_ */
