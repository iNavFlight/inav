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
/**   HMAC Mode                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_hmac.h                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto HMAC module.                                            */
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

#ifndef NX_CRYPTO_HMAC_H
#define NX_CRYPTO_HMAC_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"

#ifndef NX_CRYPTO_HMAC_MAX_PAD_SIZE
#define NX_CRYPTO_HMAC_MAX_PAD_SIZE  (128)
#endif

typedef struct NX_CRYPTO_HMAC_STRUCT
{
    VOID  *context;
    UCHAR  k_ipad[NX_CRYPTO_HMAC_MAX_PAD_SIZE];
    UCHAR  k_opad[NX_CRYPTO_HMAC_MAX_PAD_SIZE];
    UINT   algorithm;
    UINT   block_size;
    UINT   output_length;
    UINT   (*crypto_initialize)(VOID *, UINT);
    UINT   (*crypto_update)(VOID *, UCHAR *, UINT);
    UINT   (*crypto_digest_calculate)(VOID *, UCHAR *, UINT);
    NX_CRYPTO_METHOD *hash_method;
    VOID *hash_context;
} NX_CRYPTO_HMAC;

UINT _nx_crypto_hmac(NX_CRYPTO_HMAC *crypto_matadata,
                     UCHAR *input_ptr, UINT input_length,
                     UCHAR *key_ptr, UINT key_length,
                     UCHAR *digest_ptr, UINT digest_length);

UINT _nx_crypto_hmac_initialize(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *key_ptr, UINT key_length);

UINT _nx_crypto_hmac_update(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *input_ptr, UINT input_length);

UINT _nx_crypto_hmac_digest_calculate(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *digest_ptr, UINT digest_length);

VOID _nx_crypto_hmac_metadata_set(NX_CRYPTO_HMAC *hmac_metadata,
                                  VOID *context,
                                  UINT algorithm, UINT block_size, UINT output_length,
                                  UINT (*crypto_initialize)(VOID *, UINT),
                                  UINT (*crypto_update)(VOID *, UCHAR *, UINT),
                                  UINT (*crypto_digest_calculate)(VOID *, UCHAR *, UINT));

UINT _nx_crypto_hmac_hash_initialize(VOID *context, UINT algorithm);
UINT   _nx_crypto_hmac_hash_update(VOID *context, UCHAR *input, UINT input_length);
UINT   _nx_crypto_hmac_hash_digest_calculate(VOID *context, UCHAR *digest, UINT algorithm);



UINT  _nx_crypto_method_hmac_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                        VOID  **handle,
                                                        VOID  *crypto_metadata,
                                                        ULONG crypto_metadata_size);

UINT  _nx_crypto_method_hmac_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_hmac_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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


#endif /* NX_CRYPTO_HAMC_H */

