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
/**   HMAC MD5 Digest Algorithm (MD5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_hmac_md5.h                                PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX HMAC MD5 algorithm, derived from         */
/*    RFC2104. From a user-specified number of input bytes and key, this  */
/*    produces a 16-byte (128-bit) digest or sometimes called a hash      */
/*    value. The resulting digest is returned in a 16-byte array supplied */
/*    by the caller.                                                      */
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

#ifndef  NX_HMAC_MD5_H
#define  NX_HMAC_MD5_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_md5.h"
#include "nx_crypto_hmac.h"

#define NX_CRYPTO_HMAC_MD5_KEY_LEN_IN_BITS      128
#define NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS NX_CRYPTO_MD5_ICV_LEN_IN_BITS

/* Define the control block structure for backward compatibility. */
#define NX_MD5_HMAC                             NX_CRYPTO_MD5_HMAC

typedef struct NX_CRYPTO_MD5_HMAC_STRUCT
{
    NX_MD5 nx_md5_hmac_context;
    NX_CRYPTO_HMAC nx_md5_hmac_metadata;
} NX_CRYPTO_MD5_HMAC;

/* Define the function prototypes for HMAC MD5.  */

UINT _nx_crypto_method_hmac_md5_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                     VOID  **handle,
                                     VOID  *crypto_metadata,
                                     ULONG crypto_metadata_size);

UINT _nx_crypto_method_hmac_md5_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_hmac_md5_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

