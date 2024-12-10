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
/*
   Copyright (C) The Internet Society (2001).  All Rights Reserved.

   This document and translations of it may be copied and furnished to
   others, and derivative works that comment on or otherwise explain it
   or assist in its implementation may be prepared, copied, published
   and distributed, in whole or in part, without restriction of any
   kind, provided that the above copyright notice and this paragraph are
   included on all such copies and derivative works.  However, this
   document itself may not be modified in any way, such as by removing
   the copyright notice or references to the Internet Society or other
   Internet organizations, except as needed for the purpose of
   developing Internet standards in which case the procedures for
   copyrights defined in the Internet Standards process must be
   followed, or as required to translate it into languages other than
   English.

   The limited permissions granted above are perpetual and will not be
   revoked by the Internet Society or its successors or assigns.

   This document and the information contained herein is provided on an
   ""AS IS"" basis and THE INTERNET SOCIETY AND THE INTERNET ENGINEERING
   TASK FORCE DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
   BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE INFORMATION
   HEREIN WILL NOT INFRINGE ANY RIGHTS OR ANY IMPLIED WARRANTIES OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
*/
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   SHA1 Digest Algorithm (SHA1)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_sha1.h                                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX SHA1 algorithm, derived principally from */
/*    RFC3174.                                                            */
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

#ifndef  NX_CRYPTO_SHA1_H
#define  NX_CRYPTO_SHA1_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"

#define NX_CRYPTO_SHA1_BLOCK_SIZE_IN_BYTES  64
#define NX_CRYPTO_SHA1_ICV_LEN_IN_BITS      160

/* Define the control block structure for backward compatibility. */
#define NX_SHA1                                 NX_CRYPTO_SHA1

/* Define the SHA1 context structure.  */

typedef struct NX_CRYPTO_SHA1_STRUCT
{

    ULONG nx_sha1_states[5];                            /* Contains each state (A,B,C,D).   */
    ULONG nx_sha1_bit_count[2];                         /* Contains the 64-bit total bit    */
                                                        /*   count, where index 0 holds the */
                                                        /*   least significant bit count and*/
                                                        /*   index 1 contains the most      */
                                                        /*   significant portion of the bit */
                                                        /*   count.                         */
    UCHAR nx_sha1_buffer[64];                           /* Working buffer for SHA1 algorithm*/
                                                        /*   where partial buffers are      */
                                                        /*   accumulated until a full block */
                                                        /*   can be processed.              */
    ULONG nx_sha1_word_array[80];                       /* Working 80 word array.           */
} NX_CRYPTO_SHA1;


/* Define the function prototypes for SHA1.  */

UINT _nx_crypto_sha1_initialize(NX_CRYPTO_SHA1 *context, UINT algorithm);
UINT _nx_crypto_sha1_update(NX_CRYPTO_SHA1 *context, UCHAR *input_ptr, UINT input_length);
UINT _nx_crypto_sha1_digest_calculate(NX_CRYPTO_SHA1 * context, UCHAR digest[20], UINT algorithm);
VOID _nx_crypto_sha1_process_buffer(NX_CRYPTO_SHA1 * context, UCHAR buffer[64]);

UINT _nx_crypto_method_sha1_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID  **handle,
                                 VOID  *crypto_metadata,
                                 ULONG crypto_metadata_size);

UINT _nx_crypto_method_sha1_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_sha1_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                      VOID *handle, /* Crypto handler */
                                      struct NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      UCHAR *input, ULONG input_length_in_byte,
                                      UCHAR *iv_ptr,
                                      UCHAR *output, ULONG output_length_in_byte,
                                      VOID *crypto_metadata, ULONG crypto_metadata_size,
                                      VOID *packet_ptr,
                                      VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));
#endif


#ifdef __cplusplus
}
#endif
