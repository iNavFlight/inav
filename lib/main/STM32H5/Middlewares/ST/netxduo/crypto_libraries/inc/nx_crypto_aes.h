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
/**   AES Encryption                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_aes.h                                     PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto AES module.                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            moved inverse key expansion,*/
/*                                            added using RAM tables,     */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_AES_H
#define NX_CRYPTO_AES_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"
#include "nx_crypto_cbc.h"
#include "nx_crypto_ctr.h"
#include "nx_crypto_ccm.h"
#include "nx_crypto_gcm.h"



/* Constants. */
#define NX_CRYPTO_BITS_IN_UCHAR                  ((UINT)0x8)

/* Helper macros for bit indexing (used by the division operation). */
#define NX_CRYPTO_BIT_POSITION_BYTE_INDEX(x)     (x >> 3)                 /* Divide the bit position by 8 to get the byte array index.   */
#define NX_CRYPTO_BIT_POSITION_BIT_VALUE(x)      ((UINT)0x1 << (x & 0x7)) /* The bit to set (OR with the byte) is at (bit position % 8). */

/* Word-aligned versions. */
#define NX_CRYPTO_BIT_POSITION_WORD_INDEX(x)     (x >> 4)                 /* Divide the bit position by 16 to get the word array index.   */
#define NX_CRYPTO_BIT_POSITION_WORD_BIT_VALUE(x) ((UINT)0x1 << (x & 0xF)) /* The bit to set (OR with the byte) is at (bit position % 16). */


/* AES Support */

#define NX_CRYPTO_AES_STATE_ROWS                 (4)
#define NX_CRYPTO_AES_STATE_NB_BYTES             (4)

/* AES expects key sizes in the number of 32-bit words each key takes. */
#define NX_CRYPTO_AES_KEY_SIZE_128_BITS          (4)
#define NX_CRYPTO_AES_KEY_SIZE_192_BITS          (6)
#define NX_CRYPTO_AES_KEY_SIZE_256_BITS          (8)

#define NX_CRYPTO_AES_256_KEY_LEN_IN_BITS        (256)
#define NX_CRYPTO_AES_192_KEY_LEN_IN_BITS        (192)
#define NX_CRYPTO_AES_128_KEY_LEN_IN_BITS        (128)
#define NX_CRYPTO_AES_XCBC_MAC_KEY_LEN_IN_BITS   (128)

#define NX_CRYPTO_AES_MAX_KEY_SIZE               (NX_CRYPTO_AES_KEY_SIZE_256_BITS) /* Maximum key size in bytes. */

#define NX_CRYPTO_AES_BLOCK_SIZE                 (16)                              /* The AES block size for all NetX Crypto operations, in bytes. */
#define NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS         (128)
#define NX_CRYPTO_AES_IV_LEN_IN_BITS             (128)
#define NX_CRYPTO_AES_CTR_IV_LEN_IN_BITS         (64)

#define NX_CRYPTO_AES_KEY_SCHEDULE_UNKNOWN       0
#define NX_CRYPTO_AES_KEY_SCHEDULE_ENCRYPT       1
#define NX_CRYPTO_AES_KEY_SCHEDULE_DECRYPT       2

/* Define NX_CRYPTO_AES_USE_RAM_TABLES to move tables to RAM. */
#ifdef NX_CRYPTO_AES_USE_RAM_TABLES
#define NX_CRYPTO_AES_TABLE                      static
#else
#define NX_CRYPTO_AES_TABLE                      static const
#endif

/* Define the control block structure for backward compatibility. */
#define NX_AES                                   NX_CRYPTO_AES

typedef struct NX_CRYPTO_AES_STRUCT
{
    /* UINT nb; Number of bytes per column, equal to block length / 32  - ALWAYS == 4 */

    /* AES internal state, 4 rows (32 bits each) of nb bytes */
    UINT nx_crypto_aes_state[NX_CRYPTO_AES_STATE_NB_BYTES];

    /* Number of *words* in the cipher key - can be 4 (128 bits), 6 (192 bits), or 8 (256 bits). */
    USHORT nx_crypto_aes_key_size;

    /* Number of AES rounds for the current key. */
    UCHAR nx_crypto_aes_rounds;

    /* Use the flag field to indicate the inverse key expansion is done. */
    UCHAR nx_crypto_aes_inverse_key_expanded;

    /* The key schedule is as large as the key size (max = 256 bits) with expansion, total 64 UINT words. */
    UINT nx_crypto_aes_key_schedule[NX_CRYPTO_AES_MAX_KEY_SIZE * 8];
    UINT nx_crypto_aes_decrypt_key_schedule[NX_CRYPTO_AES_MAX_KEY_SIZE * 8];

    /* Metadata for each mode. */
    union
    {
        NX_CRYPTO_CBC cbc;
        NX_CRYPTO_CTR ctr;
        NX_CRYPTO_GCM gcm;
        NX_CRYPTO_CCM ccm;
    } nx_crypto_aes_mode_context;
} NX_CRYPTO_AES;

UINT _nx_crypto_aes_encrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length);
UINT _nx_crypto_aes_decrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length);

UINT _nx_crypto_aes_key_set(NX_CRYPTO_AES *aes_ptr, UCHAR *key, UINT key_size);

UINT _nx_crypto_method_aes_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                VOID **handle,
                                VOID *crypto_metadata,
                                ULONG crypto_metadata_size);

UINT _nx_crypto_method_aes_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_aes_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_cbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_ccm_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_gcm_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_ctr_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_xcbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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


#endif /* NX_CRYPTO_AES_H_ */

