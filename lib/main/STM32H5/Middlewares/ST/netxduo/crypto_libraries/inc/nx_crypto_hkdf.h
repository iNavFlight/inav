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
/**  HMAC-based Extract-and-Expand Key Derivation Function (HKDF)         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_hkdf.h                                     PORTABLE C     */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX HKDF algorithm, derived from RFC 5869.   */
/*    From user-specified input, the HKDF generates a block of data       */
/*    suitable for use as key material for various cryptographic          */
/*    protocols such as TLS 1.3.                                          */
/*                                                                        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
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

#ifndef  NX_CRYPTO_HKDF_H
#define  NX_CRYPTO_HKDF_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


#include "nx_crypto.h"
#include "nx_crypto_sha2.h"
#include "nx_crypto_hmac_sha5.h"

typedef struct NX_CRYPTO_HKDF_STRUCT
{
    /* Pointer to salt value for HKDF-extract operation. */
    UCHAR *nx_crypto_hkdf_salt;
    NX_CRYPTO_KEY_SIZE nx_crypto_hkdf_salt_length;

    /* Pointer to Input Keying Material (IKM) for HKDF-extract. */
    UCHAR *nx_crypto_hkdf_ikm;
    UINT nx_crypto_hkdf_ikm_length;

    /* Application-specific "info" used in the HKDF-expand operation. */
    UCHAR *nx_crypto_hkdf_info;
    UINT   nx_crypto_hkdf_info_size;

    /* Buffer to store Pseudo-Random Key (PRK) output from HKDF-extract.
       The buffer must be as large as the largest HMAC hash output
       (e.g. SHA-512 output length). */
    UCHAR nx_crypto_hkdf_prk[64];
    UINT nx_crypto_hkdf_prk_size; /* Actual output size (hash length). */

    /* The HMAC method to use (generic HMAC wrapper). */
    NX_CRYPTO_METHOD *nx_crypto_hmac_method;

    /* The hash method to be used (e.g. SHA-256, SHA-384). */
    NX_CRYPTO_METHOD *nx_crypto_hash_method;

    /* Temporary space for HKDF-expand intermediary (T). It must be large enough
     * to hold the previous T concatenated with "info" and a single octet counter.
     * Length > 64 + 50 + 1. Must be 4-byte aligned for hmac metadata below. */
    UCHAR nx_crypto_hkdf_temp_T[120];

    /* Workspace for the HMAC operations. */
    UCHAR nx_crypto_hmac_metadata[sizeof(NX_CRYPTO_SHA512_HMAC)];

    /* Output from HMAC operations. */
    UCHAR *nx_crypto_hmac_output;
    UINT nx_crypto_hmac_output_size;
} NX_CRYPTO_HKDF;

extern NX_CRYPTO_METHOD crypto_method_hmac_md5;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha384;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha512;

UINT _nx_crypto_hkdf_extract(NX_CRYPTO_HKDF *hkdf);
UINT _nx_crypto_hkdf_expand(NX_CRYPTO_HKDF *hkdf, UCHAR *output, UINT desired_length);

/* Define the function prototypes for HKDF.  */

UINT _nx_crypto_method_hkdf_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID  **handle,
                                 VOID  *crypto_metadata,
                                 ULONG crypto_metadata_size);

UINT _nx_crypto_method_hkdf_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_hkdf_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                      VOID *handle, /* Crypto handler */
                                      struct NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key,
                                      NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      UCHAR *input,
                                      ULONG input_length_in_byte,
                                      UCHAR *iv_ptr,
                                      UCHAR *output,
                                      ULONG output_length_in_byte,
                                      VOID *crypto_metadata,
                                      ULONG crypto_metadata_size,
                                      VOID *packet_ptr,
                                      VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

#ifdef __cplusplus
}
#endif

#endif
