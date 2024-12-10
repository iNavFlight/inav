/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Module Self Test                                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_CRYPTO_METHOD_SELF_TEST_H
#define NX_CRYPTO_METHOD_SELF_TEST_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"

#ifdef NX_CRYPTO_SELF_TEST

UINT _nx_crypto_method_self_test_aes(NX_CRYPTO_METHOD *crypto_method_aes,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_des(NX_CRYPTO_METHOD *crypto_method_des,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_3des(NX_CRYPTO_METHOD *crypto_method_3des,
                                             VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_sha(NX_CRYPTO_METHOD *crypto_method_sha,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_md5(NX_CRYPTO_METHOD *crypto_method_md5,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_rsa(NX_CRYPTO_METHOD *crypto_method,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_hmac_sha(NX_CRYPTO_METHOD *crypto_method_hmac_sha,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_hmac_md5(NX_CRYPTO_METHOD *crypto_method_hmac_md5,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_prf(NX_CRYPTO_METHOD *crypto_method_prf,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_drbg(NX_CRYPTO_METHOD *crypto_method_drbg,
                                      VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_ecdsa(NX_CRYPTO_METHOD *crypto_method_ecdsa,
                                       VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_pkcs1(NX_CRYPTO_METHOD *crypto_method_pkcs1,
                                       VOID *metadata, UINT metadata_size);
UINT _nx_crypto_method_self_test_ecdh(NX_CRYPTO_METHOD *crypto_method_ecdh,
                                      VOID *metadata, UINT metadata_size);

#endif
#endif /* NX_CRYPTO_METHOD_SELF_TEST_H  */

