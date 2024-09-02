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

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto_phash.h"
#include "nx_crypto_hmac_sha1.h"
#include "nx_crypto_hmac_md5.h"

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_tls_prf_1.h                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the TLS Pseudo-Random Function (PRF) as described */
/*    in RFCs 2246 and 4346. This PRF is used for all key generation in   */
/*    TLS versions 1.0 and 1.1.                                           */
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

/* Define the control block structure for backward compatibility. */
#define NX_SECURE_TLS_PRF_1                     NX_CRYPTO_TLS_PRF_1

typedef struct NX_CRYPTO_TLS_PRF_1_STRUCT
{
    NX_CRYPTO_PHASH nx_secure_tls_prf_phash_info;
    UCHAR nx_secure_tls_prf_label_seed_buffer[80]; /* phash_seed = label(13 bytes) || prf_seed(64 bytes) */
    UCHAR nx_secure_tls_prf_temp_A_buffer[100]; /* The temp_A buffer needs to be large enough to holdthe lable(13 bytes) || prf_seed(64 bytes) || hash_size(20 bytes for SHA1/MD5) */
    UCHAR nx_secure_tls_prf_temp_hmac_output_buffer[20]; /* The temp buffer for the output buffer of hmac(secret, A(i) + seed) */
    UCHAR nx_secure_tls_prf_hmac_metadata_area[sizeof(NX_CRYPTO_SHA1_HMAC) + sizeof(NX_CRYPTO_MD5_HMAC)]; /* metadata buffer for the hmac function */
} NX_CRYPTO_TLS_PRF_1;

UINT _nx_crypto_method_prf_1_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                  UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                  VOID **handle,
                                  VOID *crypto_metadata,
                                  ULONG crypto_metadata_size);

UINT _nx_crypto_method_prf_1_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_prf_1_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                       VOID *handle, /* Crypto handler */
                                       struct NX_CRYPTO_METHOD_STRUCT *method,
                                       UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                       UCHAR *input, ULONG input_length_in_byte,
                                       UCHAR *iv_ptr,
                                       UCHAR *output, ULONG output_length_in_byte,
                                       VOID *crypto_metadata, ULONG crypto_metadata_size,
                                       VOID *packet_ptr,
                                       VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

#ifdef __cplusplus
}
#endif
