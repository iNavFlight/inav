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
/**   CTR Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ctr.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto CTR module.                                             */
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

#ifndef NX_CRYPTO_CTR_H
#define NX_CRYPTO_CTR_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"

#define NX_CRYPTO_CTR_BLOCK_SIZE 16

typedef struct NX_CRYPTO_CTR_STRUCT
{

    /* Counter block. */
    UCHAR nx_crypto_ctr_counter_block[NX_CRYPTO_CTR_BLOCK_SIZE];
} NX_CRYPTO_CTR;

/* The CTR mode is symmetric - encryption and decryption are the same operation. */
NX_CRYPTO_KEEP UINT _nx_crypto_ctr_encrypt(VOID *crypto_metadata, NX_CRYPTO_CTR *ctr_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UINT block_size);
NX_CRYPTO_KEEP UINT _nx_crypto_ctr_encrypt_init(NX_CRYPTO_CTR *ctr_metadata, UCHAR *iv, UINT iv_len,
                                                UCHAR *nonce, UINT nonce_len);
#define _nx_crypto_ctr_decrypt   _nx_crypto_ctr_encrypt


#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_CTR_H */

