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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_table_self_test.h                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all services for module self-test.                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef NX_SECURE_CRYPTO_TABLE_SELF_TEST_H
#define NX_SECURE_CRYPTO_TABLE_SELF_TEST_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_secure_tls.h"

UINT _nx_secure_crypto_table_self_test(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                       VOID *metadata, UINT metadata_size);

#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
/* This structure is used for self test function lookup table. */
typedef struct NX_SECURE_CRYPTO_SELF_TEST_STRUCT
{

    /* Algorithm ID. */
    UINT nx_crypto_algorithm;

    /* Function pointer to the self test function for the corresponding algorithm. */
    UINT (*self_test_function)(NX_CRYPTO_METHOD *crypto_method, VOID *metadata, UINT metadata_size);

} NX_SECURE_CRYPTO_SELF_TEST;

UINT _nx_secure_crypto_method_self_test_aes(NX_CRYPTO_METHOD *crypto_method_aes,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_des(NX_CRYPTO_METHOD *crypto_method_des,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_3des(NX_CRYPTO_METHOD *crypto_method_3des,
                                             VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_sha(NX_CRYPTO_METHOD *crypto_method_sha,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_md5(NX_CRYPTO_METHOD *crypto_method_md5,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_rsa(NX_CRYPTO_METHOD *crypto_method,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_hmac_sha(NX_CRYPTO_METHOD *crypto_method_hmac_sha,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_hmac_md5(NX_CRYPTO_METHOD *crypto_method_hmac_md5,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_prf(NX_CRYPTO_METHOD *crypto_method_prf,
                                            VOID *metadata, UINT metadata_size);
#endif
#endif /* NX_SECURE_CRYPTO_TABLE_SELF_TEST_H */

