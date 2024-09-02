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
/**   GCM Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_gcm.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto GCM module.                                             */
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

#ifndef NX_CRYPTO_GCM_H
#define NX_CRYPTO_GCM_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"


#define NX_CRYPTO_GCM_BLOCK_SIZE 16
#define NX_CRYPTO_GCM_BLOCK_SIZE_BITS 128
#define NX_CRYPTO_GCM_BLOCK_SIZE_INT 4
#define NX_CRYPTO_GCM_BLOCK_SIZE_SHIFT 4

typedef struct NX_CRYPTO_GCM_STRUCT
{

    /* Total length of input. */
    ULONG nx_crypto_gcm_input_total_length;

    /* Internal context of GCM mode. */
    UCHAR nx_crypto_gcm_hkey[NX_CRYPTO_GCM_BLOCK_SIZE];
    UCHAR nx_crypto_gcm_j0[NX_CRYPTO_GCM_BLOCK_SIZE];
    UCHAR nx_crypto_gcm_s[NX_CRYPTO_GCM_BLOCK_SIZE];
    UCHAR nx_crypto_gcm_counter[NX_CRYPTO_GCM_BLOCK_SIZE];

    /* Pointer of additional data. */
    VOID *nx_crypto_gcm_additional_data;

    /* Length of additional data. */
    UINT nx_crypto_gcm_additional_data_len;
} NX_CRYPTO_GCM;

NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_init(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                VOID *additional_data, UINT additional_len,
                                                UCHAR *iv, UINT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_update(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length,
                                                  UINT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *output, UINT icv_len, UINT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_gcm_decrypt_update(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length,
                                                  UINT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_gcm_decrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *input, UINT icv_len, UINT block_size);

#define _nx_crypto_gcm_decrypt_init _nx_crypto_gcm_encrypt_init

#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_GCM_H */

