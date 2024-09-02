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
/**   ECJPAKE                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ecjpake.h                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the symbols, structures and operations for        */
/*    Elliptic Curve J-PAKE.                                              */
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

#ifndef NX_CRYPTO_ECJPAKE_H
#define NX_CRYPTO_ECJPAKE_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


#include "nx_crypto.h"
#include "nx_crypto_ec.h"

#define NX_CRYPTO_ECJPAKE_CLIENT_ID           "client"
#define NX_CRYPTO_ECJPAKE_SERVER_ID           "server"

#ifndef NX_CRYPTO_ECJPAKE_SCRATCH_BUFFER_SIZE
#define NX_CRYPTO_ECJPAKE_SCRATCH_BUFFER_SIZE 4096
#endif /* NX_CRYPTO_ECJPAKE_SCRATCH_BUFFER_SIZE */

typedef struct
{
    UCHAR *nx_crypto_ecjpake_zkp_x;
    UINT   nx_crypto_ecjpake_zkp_x_len;
    UCHAR *nx_crypto_ecjpake_zkp_v;
    UINT   nx_crypto_ecjpake_zkp_v_len;
    UCHAR *nx_crypto_ecjpake_zkp_r;
    UINT   nx_crypto_ecjpake_zkp_r_len;
} NX_CRYPTO_ECJPAKE_ZKP;

typedef struct
{
    NX_CRYPTO_EC         *nx_crypto_ecjpake_curve;

    NX_CRYPTO_EC_POINT    nx_crypto_ecjpake_public_x1;   /* x1 and x2 are always local public keys */
    NX_CRYPTO_EC_POINT    nx_crypto_ecjpake_public_x2;
    NX_CRYPTO_EC_POINT    nx_crypto_ecjpake_public_x3;   /* x3 and x4 are always peer public keys */
    NX_CRYPTO_EC_POINT    nx_crypto_ecjpake_public_x4;

    NX_CRYPTO_HUGE_NUMBER nx_crypto_ecjpake_private_x2;

    NX_CRYPTO_METHOD     *nx_crypto_ecjpake_hash_method;
    VOID                 *nx_crypto_ecjpake_hash_metadata;
    ULONG                 nx_crypto_ecjpake_hash_metadata_size;

    UCHAR                *nx_crypto_ecjpake_psk;
    UINT                  nx_crypto_ecjpake_psk_length;

    UCHAR                 nx_crypto_ecjpake_scratch_buffer[NX_CRYPTO_ECJPAKE_SCRATCH_BUFFER_SIZE];
    HN_UBASE             *nx_crypto_ecjpake_scratch_ptr;
} NX_CRYPTO_ECJPAKE;

VOID _nx_crypto_ecjpake_init(NX_CRYPTO_ECJPAKE *ecjpake,
                             NX_CRYPTO_EC *curve,
                             NX_CRYPTO_METHOD *hash_method,
                             VOID *hash_metadata,
                             ULONG hash_metadata_size,
                             HN_UBASE **scratch_pptr);

UINT _nx_crypto_ecjpake_hello_generate(NX_CRYPTO_ECJPAKE *ecjpake,
                                       CHAR *id, UINT id_len,
                                       UCHAR *output, ULONG output_length,
                                       ULONG *actual_size,
                                       HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_hello_process(NX_CRYPTO_ECJPAKE *ecjpake,
                                      CHAR *id, UINT id_len,
                                      UCHAR *input, UINT input_length,
                                      HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_key_exchange_generate(NX_CRYPTO_ECJPAKE *ecjpake,
                                              UCHAR *shared_secret,
                                              UINT shared_secret_len,
                                              CHAR *id, UINT id_len,
                                              UCHAR *output, ULONG output_length,
                                              ULONG *actual_size,
                                              HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_key_exchange_process(NX_CRYPTO_ECJPAKE *ecjpake,
                                             UCHAR *shared_secret,
                                             UINT shared_secret_len,
                                             CHAR *id, UINT id_len,
                                             UCHAR *input, UINT input_length,
                                             UCHAR *pms,
                                             HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_schnorr_zkp_hash(NX_CRYPTO_METHOD *hash_method,
                                         VOID *hash_metadata,
                                         NX_CRYPTO_EC *curve,
                                         NX_CRYPTO_EC_POINT *g,
                                         NX_CRYPTO_EC_POINT *v,
                                         NX_CRYPTO_EC_POINT *x,
                                         NX_CRYPTO_HUGE_NUMBER *h,
                                         CHAR *id,
                                         UINT id_len,
                                         HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_schnorr_zkp_generate(NX_CRYPTO_METHOD *hash_method,
                                             VOID *hash_metadata,
                                             NX_CRYPTO_EC *curve,
                                             NX_CRYPTO_EC_POINT *g,
                                             NX_CRYPTO_EC_POINT *v,
                                             NX_CRYPTO_EC_POINT *public_key,
                                             CHAR *id,
                                             UINT id_len,
                                             NX_CRYPTO_HUGE_NUMBER *private_key,
                                             NX_CRYPTO_HUGE_NUMBER *r,
                                             HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_schnorr_zkp_verify(NX_CRYPTO_METHOD *hash_method,
                                           VOID *hash_metadata,
                                           NX_CRYPTO_EC *curve,
                                           NX_CRYPTO_EC_POINT *g,
                                           NX_CRYPTO_EC_POINT *v,
                                           NX_CRYPTO_EC_POINT *public_key,
                                           CHAR *id,
                                           UINT id_len,
                                           NX_CRYPTO_HUGE_NUMBER *r,
                                           HN_UBASE *scratch);

VOID _nx_crypto_ecjpake_public_key_generate(NX_CRYPTO_EC *curve,
                                            NX_CRYPTO_EC_POINT *x1,
                                            NX_CRYPTO_EC_POINT *x3,
                                            NX_CRYPTO_EC_POINT *x4,
                                            NX_CRYPTO_HUGE_NUMBER *x2,
                                            NX_CRYPTO_HUGE_NUMBER *s,
                                            NX_CRYPTO_EC_POINT *g,
                                            NX_CRYPTO_EC_POINT *public_key,
                                            NX_CRYPTO_HUGE_NUMBER *private_key,
                                            HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_pre_master_secret_generate(NX_CRYPTO_METHOD *hash_method,
                                                   VOID *hash_metadata,
                                                   NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_EC_POINT *x4,
                                                   NX_CRYPTO_HUGE_NUMBER *s,
                                                   NX_CRYPTO_EC_POINT *public_key,
                                                   NX_CRYPTO_HUGE_NUMBER *x2,
                                                   UCHAR *pms,
                                                   HN_UBASE *scratch);

UINT _nx_crypto_ecjpake_key_encryption_key_generate(NX_CRYPTO_METHOD *hash_method,
                                                    VOID *hash_metadata,
                                                    UCHAR *key_expansion,
                                                    UINT key_expansion_len,
                                                    UCHAR *key_encryption_key);

UINT _nx_crypto_method_ecjpake_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                    UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                    VOID **handle,
                                    VOID *crypto_metadata,
                                    ULONG crypto_metadata_size);

UINT _nx_crypto_method_ecjpake_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_ecjpake_operation(UINT op,
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

#endif /* NX_CRYPTO_ECJPAKE_H */

