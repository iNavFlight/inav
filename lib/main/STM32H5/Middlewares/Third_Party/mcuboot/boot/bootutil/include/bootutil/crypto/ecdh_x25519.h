/*
 * This module provides a thin abstraction over some of the crypto
 * primitives to make it easier to swap out the used crypto library.
 *
 * At this point, there are two choices: MCUBOOT_USE_MBED_TLS, or
 * MCUBOOT_USE_TINYCRYPT.  It is a compile error there is not exactly
 * one of these defined.
 */

#ifndef __BOOTUTIL_CRYPTO_ECDH_X25519_H_
#define __BOOTUTIL_CRYPTO_ECDH_X25519_H_

#include "mcuboot_config/mcuboot_config.h"

#if (defined(MCUBOOT_USE_MBED_TLS) + \
     defined(MCUBOOT_USE_TINYCRYPT)) != 1
    #error "One crypto backend must be defined: either MBED_TLS or TINYCRYPT"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MCUBOOT_USE_TINYCRYPT) || defined(MCUBOOT_USE_MBED_TLS)
extern int X25519(uint8_t out_shared_key[32], const uint8_t private_key[32],
                  const uint8_t peer_public_value[32]);

typedef uintptr_t bootutil_ecdh_x25519_context;
static inline void bootutil_ecdh_x25519_init(bootutil_ecdh_x25519_context *ctx)
{
    (void)ctx;
}

static inline void bootutil_ecdh_x25519_drop(bootutil_ecdh_x25519_context *ctx)
{
    (void)ctx;
}

static inline int bootutil_ecdh_x25519_shared_secret(bootutil_ecdh_x25519_context *ctx, const uint8_t *pk, const uint8_t *sk, uint8_t *z)
{
    int rc;
    (void)ctx;

    rc = X25519(z, sk, pk);
    if (rc != 0) {
        return -1;
    }

    return 0;
}
#endif /* MCUBOOT_USE_TINYCRYPT */

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_CRYPTO_ECDH_X25519_H_ */
