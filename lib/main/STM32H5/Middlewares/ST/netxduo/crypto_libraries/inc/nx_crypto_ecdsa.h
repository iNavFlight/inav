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
/**   Elliptic Curve Digital Signature Algorithm (ECDSA)                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_ecdsa.h                                    PORTABLE C     */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX ECDSA module.                                                  */
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

#ifndef  NX_ECDSA_H
#define  NX_ECDSA_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_huge_number.h"
#include "nx_crypto_ec.h"


#ifndef NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE
#define NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE 3016
#endif /* NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE */


/* ECDSA signature structure. */
typedef struct NX_CRYPTO_ECDSA
{
    NX_CRYPTO_EC *nx_crypto_ecdsa_curve;
    NX_CRYPTO_METHOD *nx_crypto_ecdsa_hash_method;
    HN_UBASE      nx_crypto_ecdsa_scratch_buffer[NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE >> HN_SIZE_SHIFT];
} NX_CRYPTO_ECDSA;

/* Define the function prototypes for ECDSA.  */

UINT _nx_crypto_ecdsa_sign(NX_CRYPTO_EC *curve,
                           UCHAR *hash,
                           UINT hash_length,
                           UCHAR *private_key,
                           UINT private_key_length,
                           UCHAR *signature,
                           ULONG signature_length,
                           ULONG *actual_signature_length,
                           HN_UBASE *scratch);

UINT _nx_crypto_ecdsa_verify(NX_CRYPTO_EC *curve,
                             UCHAR *hash,
                             UINT hash_length,
                             UCHAR *public_key,
                             UINT public_key_length,
                             UCHAR *signature,
                             UINT signature_length,
                             HN_UBASE *scratch);

UINT _nx_crypto_method_ecdsa_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                  UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                  VOID  **handle,
                                  VOID  *crypto_metadata,
                                  ULONG crypto_metadata_size);

UINT _nx_crypto_method_ecdsa_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_ecdsa_operation(UINT op,
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

