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
/**   Transport Layer Security (TLS)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef SRC_NX_SECURE_PHASH_H_
#define SRC_NX_SECURE_PHASH_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_hmac_sha2.h"
#include "nx_crypto_hmac_sha5.h"

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_phash.h                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the TLS P-HASH function described in RFCs 2246,   */
/*    4346, and 5246. It is used in the TLS PRF function as a wrapper to  */
/*    various hash routines to generate arbitrary-length data.            */
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
typedef struct NX_CRYPTO_PHASH_STRUCT
{
    UCHAR *nx_crypto_phash_secret; /* secret */
    NX_CRYPTO_KEY_SIZE nx_crypto_phash_secret_length;
    UCHAR *nx_crypto_phash_seed; /* seed */
    UINT nx_crypto_phash_seed_length;
    UCHAR *nx_crypto_phash_temp_A; /* the buffer for A(i) */
    UINT nx_crypto_phash_temp_A_size;
    NX_CRYPTO_METHOD *nx_crypto_hmac_method; /* hmac method */
    UCHAR *nx_crypto_hmac_metadata; /* hash_metadata */
    UINT nx_crypto_hmac_metadata_size;
    UCHAR *nx_crypto_hmac_output;
    UINT nx_crypto_hmac_output_size;
} NX_CRYPTO_PHASH;

extern NX_CRYPTO_METHOD crypto_method_hmac_md5;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha384;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha512;

UINT _nx_crypto_phash(NX_CRYPTO_PHASH *phash, UCHAR *output, UINT desired_length);

#ifdef __cplusplus
}
#endif

#endif

