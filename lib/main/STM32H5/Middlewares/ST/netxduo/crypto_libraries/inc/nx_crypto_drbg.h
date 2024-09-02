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
/**   Deterministic Random Bit Generator (DRBG)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_drbg.h                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto DRBG module.                                            */
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

#ifndef NX_CRYPTO_DRBG_H
#define NX_CRYPTO_DRBG_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"

/* Constants. */
#define NX_CRYPTO_DRBG_BLOCK_LENGTH_AES (16)
#define NX_CRYPTO_DRBG_MAX_BLOCK_LENGTH (16)
#define NX_CRYPTO_DRBG_MAX_KEY_LENGTH   (32)
#define NX_CRYPTO_DRBG_MAX_SEEDLEN      (48)

#ifndef NX_CRYPTO_DRBG_BLOCK_LENGTH
#define NX_CRYPTO_DRBG_BLOCK_LENGTH     (NX_CRYPTO_DRBG_BLOCK_LENGTH_AES)
#endif

#define NX_CRYPTO_DRBG_DF_INPUT_OFFSET  (NX_CRYPTO_DRBG_BLOCK_LENGTH + 8)

#ifndef NX_CRYPTO_DRBG_SEED_BUFFER_LEN
#define NX_CRYPTO_DRBG_SEED_BUFFER_LEN  (256)
#endif

#ifndef NX_CRYPTO_DRBG_MAX_ENTROPY_LEN
#define NX_CRYPTO_DRBG_MAX_ENTROPY_LEN  (125)
#endif

#ifndef NX_CRYPTO_DRBG_MAX_SEED_LIFE
#define NX_CRYPTO_DRBG_MAX_SEED_LIFE    (100000)
#endif

#ifndef NX_CRYPTO_DRBG_MUTEX_GET
#define NX_CRYPTO_DRBG_MUTEX_GET
#endif

#ifndef NX_CRYPTO_DRBG_MUTEX_PUT
#define NX_CRYPTO_DRBG_MUTEX_PUT
#endif

#ifndef NX_CRYPTO_DRBG_USE_DF
#define NX_CRYPTO_DRBG_USE_DF (1)
#endif

#ifndef NX_CRYPTO_DRBG_PREDICTION_RESISTANCE
#define NX_CRYPTO_DRBG_PREDICTION_RESISTANCE (1)
#endif

#ifndef NX_CRYPTO_DRBG_CTR_CRYPTO_METHOD
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_128;
#define NX_CRYPTO_DRBG_CTR_CRYPTO_METHOD &crypto_method_aes_cbc_128
#endif

#ifndef NX_CRYPTO_DRBG_CTR_CRYPTO_METADATA
#define NX_CRYPTO_DRBG_CTR_CRYPTO_METADATA _nx_crypto_ctr_metadata
#define NX_CRYPTO_DRBG_CTR_METADATA_SIZE (sizeof(NX_CRYPTO_AES))
#endif

#ifndef NX_CRYPTO_DRBG_ENTROPY_INPUT_FUNC
#define NX_CRYPTO_DRBG_ENTROPY_INPUT_FUNC _nx_crypto_drbg_rnd_entropy_input
#endif



/* DRBG control structure. */
typedef struct NX_CRYPTO_DRBG_STRUCT
{
    /* Crypto method and metadata used in the DRBG. */
    NX_CRYPTO_METHOD *nx_crypto_drbg_crypto_method;
    VOID *nx_crypto_drbg_crypto_metadata;

    UINT (*nx_crypto_drbg_get_entropy)(UCHAR *entropy, UINT *entropy_len, UINT entropy_max_len);

    UINT  nx_crypto_drbg_use_df;
    UINT  nx_crypto_drbg_prediction_resistance;
    UINT  nx_crypto_drbg_security_strength;

    UINT  nx_crypto_drbg_instantiated;

    /* DRBG working state. */
    UCHAR nx_crypto_drbg_key[NX_CRYPTO_DRBG_MAX_KEY_LENGTH];
    UCHAR nx_crypto_drbg_v[NX_CRYPTO_DRBG_MAX_BLOCK_LENGTH];

    /* A counter that indicates the number of requests for pseudorandom bits since instantiation or reseeding. */
    UINT  nx_crypto_drgb_reseed_counter;

    UINT  nx_crypto_drbg_seedlen;

    UCHAR nx_crypto_drbg_buffer[NX_CRYPTO_DRBG_SEED_BUFFER_LEN];
} NX_CRYPTO_DRBG;

/* DRBG control structure. */
typedef struct NX_CRYPTO_DRBG_OPTIONS_STRUCT
{
    /* Crypto method and metadata used in the DRBG. */
    NX_CRYPTO_METHOD *crypto_method;
    VOID *crypto_metadata;

    UINT (*entropy_input)(UCHAR *entropy, UINT *entropy_len, UINT entropy_max_len);

    UINT  use_df;
    UINT  prediction_resistance;
    UINT  security_strength;
} NX_CRYPTO_DRBG_OPTIONS;


/* Function prototypes */


UINT _nx_crypto_drbg_instantiate(NX_CRYPTO_DRBG *drbg_ptr,
                                 UCHAR *nonce,
                                 UINT nonce_len,
                                 UCHAR *personalization_string,
                                 UINT personalization_string_len);

UINT _nx_crypto_drbg_reseed(NX_CRYPTO_DRBG *drbg_ptr,
                            UCHAR *additional_input,
                            UINT additional_input_len);

UINT _nx_crypto_drbg_generate(NX_CRYPTO_DRBG *drbg_ptr,
                              UCHAR *output, UINT output_length_in_byte,
                              UCHAR *additional_input,
                              UINT additional_input_len);

UINT _nx_crypto_method_drbg_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID  **handle,
                                 VOID  *crypto_metadata,
                                 ULONG crypto_metadata_size);

UINT _nx_crypto_method_drbg_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_drbg_operation(UINT op,
                                      VOID *handle,
                                      struct NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      UCHAR *input, ULONG input_length_in_byte,
                                      UCHAR *iv_ptr,
                                      UCHAR *output, ULONG output_length_in_byte,
                                      VOID *crypto_metadata, ULONG crypto_metadata_size,
                                      VOID *packet_ptr,
                                      VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));

UINT _nx_crypto_drbg(UINT bits, UCHAR *result);

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_DRBG_H */

