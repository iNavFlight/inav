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
/**   CCM Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ccm.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto CCM module.                                             */
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

#ifndef NX_CRYPTO_CCM_H
#define NX_CRYPTO_CCM_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"

#define NX_CRYPTO_CCM_BLOCK_SIZE 16

typedef struct NX_CRYPTO_CCM_STRUCT
{

    /* Internal context of CCM mode. */
    USHORT nx_crypto_ccm_icv_length;
    UCHAR nx_crypto_ccm_reserved[2];
    UCHAR nx_crypto_ccm_A[NX_CRYPTO_CCM_BLOCK_SIZE];
    UCHAR nx_crypto_ccm_X[NX_CRYPTO_CCM_BLOCK_SIZE];

    /* Pointer of additional data. */
    VOID *nx_crypto_ccm_additional_data;

    /* Length of additional data. */
    UINT nx_crypto_ccm_additional_data_len;
} NX_CRYPTO_CCM;

NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_init(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                VOID *additional_data, UINT additional_len,
                                                UINT length, UCHAR *iv, USHORT icv_len, USHORT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_update(UINT op, VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length, UINT block_size);

NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *icv, UINT block_size);
NX_CRYPTO_KEEP UINT _nx_crypto_ccm_decrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *icv, UINT block_size);

#define _nx_crypto_ccm_decrypt_init         _nx_crypto_ccm_encrypt_init
#define _nx_crypto_ccm_decrypt_update       _nx_crypto_ccm_encrypt_update


#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_CCM_H */

