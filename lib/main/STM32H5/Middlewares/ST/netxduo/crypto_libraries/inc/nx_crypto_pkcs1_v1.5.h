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

#ifndef SRC_NX_SECURE_PKCS1_V1_5_H_
#define SRC_NX_SECURE_PKCS1_V1_5_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_rsa.h"

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_pkcs1_v1.5.h                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines PKCS#1v1.5 signature encoding function used by    */
/*    TLS and other protocols for encoding encrypted signatures.          */
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

typedef struct NX_CRYPTO_PKCS1_STRUCT
{
    NX_CRYPTO_METHOD  *public_cipher_method;
    VOID              *public_cipher_metadata;
    UINT               public_cipher_metadata_size;
    NX_CRYPTO_METHOD  *hash_method;
    VOID              *hash_metadata;
    UINT               hash_metadata_size;
    UCHAR             *modulus;
    UINT               modulus_size;
    /*Note: Can be optimized.  */
    UCHAR              scratch_buffer[NX_CRYPTO_MAX_RSA_MODULUS_SIZE >> 2];
}NX_CRYPTO_PKCS1;

typedef struct NX_CRYPTO_PKCS1_OPTIONS_STRUCT
{
    NX_CRYPTO_METHOD  *public_cipher_method;
    VOID              *public_cipher_metadata;
    UINT               public_cipher_metadata_size;
    NX_CRYPTO_METHOD  *hash_method;
    VOID              *hash_metadata;
    UINT               hash_metadata_size;
}NX_CRYPTO_PKCS1_OPTIONS;

UINT _nx_crypto_pkcs1_v1_5_sign(UCHAR *input, UINT input_length,
                                UCHAR *private_key, UINT private_key_size,
                                UCHAR *metadata_area,
                                UCHAR *output, UINT output_size);

UINT _nx_crypto_pkcs1_v1_5_verify(UCHAR *message, UINT message_length,
                                  UCHAR *signature, UINT signature_length,
                                  UCHAR *public_key, UINT public_key_size,
                                  UCHAR *metadata_area);

UINT _nx_crypto_pkcs1_v1_5_encode(UCHAR *input, UINT input_length,
                                  NX_CRYPTO_METHOD *hash_method,
                                  UCHAR *metadata_area, UINT metadata_size,
                                  UCHAR *output, UINT expected_output_length);

UINT  _nx_crypto_method_pkcs1_v1_5_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                        VOID  **handle,
                                        VOID  *crypto_metadata,
                                        ULONG crypto_metadata_size);

UINT _nx_crypto_method_pkcs1_v1_5_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_pkcs1_v1_5_operation(UINT op,
                                            VOID *handle,
                                            struct NX_CRYPTO_METHOD_STRUCT *method,
                                            UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                            UCHAR *input, ULONG input_length_in_byte,
                                            UCHAR *iv_ptr,
                                            UCHAR *output, ULONG output_length_in_byte,
                                            VOID *crypto_metadata, ULONG crypto_metadata_size,
                                            VOID *packet_ptr,
                                            VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));

#ifdef __cplusplus
}
#endif

#endif

