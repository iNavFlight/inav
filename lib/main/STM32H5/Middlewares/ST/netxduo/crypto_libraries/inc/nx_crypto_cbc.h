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
/**   CBC Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_cbc.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto CBC module.                                             */
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

#ifndef NX_CRYPTO_CBC_H
#define NX_CRYPTO_CBC_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"

#ifndef NX_CRYPTO_CBC_MAX_BLOCK_SIZE
#define NX_CRYPTO_CBC_MAX_BLOCK_SIZE 16
#endif /* NX_CRYPTO_CBC_MAX_BLOCK_SIZE */

typedef struct NX_CRYPTO_CBC_STRUCT
{

    /* Initial Vector for next round. */
    UCHAR nx_crypto_cbc_last_block[NX_CRYPTO_CBC_MAX_BLOCK_SIZE];
} NX_CRYPTO_CBC;

NX_CRYPTO_KEEP UINT _nx_crypto_cbc_encrypt(VOID *crypto_metadata, NX_CRYPTO_CBC *cbc_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UCHAR block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_cbc_decrypt(VOID *crypto_metadata, NX_CRYPTO_CBC *cbc_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UCHAR block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_cbc_encrypt_init(NX_CRYPTO_CBC *cbc_metadata, UCHAR *iv, UINT iv_len);

#define _nx_crypto_cbc_decrypt_init _nx_crypto_cbc_encrypt_init

#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_CBC_H */

