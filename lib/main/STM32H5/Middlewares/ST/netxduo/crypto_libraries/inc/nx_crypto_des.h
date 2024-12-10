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
/**   DES Encryption Standard (DES)                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_des.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX DES encryption algorithm, derived        */
/*    principally from FIPS-46. From an 8 bytes of raw input, the DES     */
/*    encryption routine produces an 8-byte encryption of the input.      */
/*    Conversely, from an 8-byte encryption, the decryption routine       */
/*    produces the original 8 bytes of input. Note that the caller must   */
/*    ensure 8 bytes of input and output are provided.                    */
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

#ifndef  _NX_CRYPTO_DES_H_
#define  _NX_CRYPTO_DES_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_cbc.h"


#define NX_CRYPTO_DES_KEY_LEN_IN_BITS    64
#define NX_CRYPTO_DES_BLOCK_SIZE_IN_BITS 64
#define NX_CRYPTO_DES_IV_LEN_IN_BITS     64

UINT _nx_crypto_method_des_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                VOID **handle,
                                VOID *crypto_metadata,
                                ULONG crypto_metadata_size);

UINT _nx_crypto_method_des_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_des_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

/* Define the DES context structure.  */

typedef struct NX_CRYPTO_DES_STRUCT
{

    ULONG nx_des_encryption_keys[32];           /* Contains the encryption keys     */
    ULONG nx_des_decryption_keys[32];           /* Contains the decryption keys     */
    NX_CRYPTO_CBC nx_crypto_cbc_context;        /* CBC metadata                     */
} NX_CRYPTO_DES;


/* Define the function prototypes for DES.  */

UINT _nx_crypto_des_key_set(NX_CRYPTO_DES * context, UCHAR key[8]);
UINT _nx_crypto_des_encrypt(NX_CRYPTO_DES * context, UCHAR source[8], UCHAR destination[8], UINT length);
UINT _nx_crypto_des_decrypt(NX_CRYPTO_DES * context, UCHAR source[8], UCHAR destination[8], UINT length);
VOID _nx_crypto_des_process_block(UCHAR source[8], UCHAR destination[8], ULONG keys[32]);

#ifdef __cplusplus
}
#endif

#endif

