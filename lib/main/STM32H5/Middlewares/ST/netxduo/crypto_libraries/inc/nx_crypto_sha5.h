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
/**   SHA-512 Digest Algorithm (SHA5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_sha5.h                                    PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX SHA512 component, derived primarily      */
/*    from NIST FIPS PUB 180-4 (Crypto Hash Standard).                    */
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
/*  08-02-2021     Timothy Stapko           Modified comment(s), and      */
/*                                            used ULONG64_DEFINED macro, */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/



#ifndef SRC_NX_CRYPTO_SHA5_H_
#define SRC_NX_CRYPTO_SHA5_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


#include "nx_crypto.h"

#ifndef ULONG64_DEFINED
#define ULONG64_DEFINED
#define ULONG64                              unsigned long long
#endif /* ULONG64 */


#define NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES    128
#define NX_CRYPTO_SHA384_ICV_LEN_IN_BITS        384
#define NX_CRYPTO_SHA512_ICV_LEN_IN_BITS        512
#define NX_CRYPTO_SHA512_224_ICV_LEN_IN_BITS    224
#define NX_CRYPTO_SHA512_256_ICV_LEN_IN_BITS    256

/* Define the control block structure for backward compatibility. */
#define NX_SHA512                               NX_CRYPTO_SHA512

typedef struct NX_CRYPTO_SHA512_STRUCT
{

    ULONG64 nx_sha512_states[8];                        /* Contains each state (A,B,C,D,E,F,G,H).   */
    ULONG64 nx_sha512_bit_count[2];                     /* Contains the 128-bit total bit            */
                                                        /*   count, where index 0 holds the         */
                                                        /*   least significant bit count and        */
                                                        /*   index 1 contains the most              */
                                                        /*   significant portion of the bit         */
                                                        /*   count.                                 */
    UCHAR nx_sha512_buffer[NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES];
                                                        /* Working buffer for SHA512 algorithm      */
                                                        /*   where partial buffers are              */
                                                        /*   accumulated until a full block         */
                                                        /*   can be processed.                      */
    ULONG64 nx_sha512_word_array[128];                  /* Working 64 word array.                   */
} NX_CRYPTO_SHA512;


UINT _nx_crypto_sha512_initialize(NX_CRYPTO_SHA512 *context, UINT algorithm);
UINT _nx_crypto_sha512_update(NX_CRYPTO_SHA512 *context, UCHAR *input_ptr, UINT input_length);
UINT _nx_crypto_sha512_digest_calculate(NX_CRYPTO_SHA512 *context, UCHAR *digest, UINT algorithm);
VOID _nx_crypto_sha512_process_buffer(NX_CRYPTO_SHA512 *context, UCHAR *buffer);

UINT _nx_crypto_method_sha512_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                   UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                   VOID  **handle,
                                   VOID  *crypto_metadata,
                                   ULONG crypto_metadata_size);

UINT _nx_crypto_method_sha512_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_sha512_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

#endif /* SRC_NX_CRYPTO_SHA5_H_ */

