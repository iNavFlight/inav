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
/** NetX Component                                                        */
/**                                                                       */
/**   DES Encryption Standard (DES)                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    nx_des.h                                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX DES encryption algorithm, derived        */ 
/*    principally from FIPS-46. From an 8 bytes of raw input, the DES     */ 
/*    encryption routine produces an 8-byte encryption of the input.      */ 
/*    Conversely, from an 8-byte encryption, the decryption routine       */ 
/*    produces the original 8 bytes of input. Note that the caller must   */ 
/*    ensure 8 bytes of input and output are provided.                    */ 
/*                                                                        */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included.                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef  NX_DES_H
#define  NX_DES_H

/* Define the DES context structure.  */

typedef struct NX_DES_STRUCT
{

    ULONG       nx_des_encryption_keys[32];             /* Contains the encryption keys     */ 
    ULONG       nx_des_decryption_keys[32];             /* Contains the decryption keys     */ 
} NX_DES;


/* Define the function prototypes for DES.  */

UINT        _nx_des_key_set(NX_DES *context, UCHAR key[8]);
UINT        _nx_des_encrypt(NX_DES *context, UCHAR source[8], UCHAR destination[8]);
UINT        _nx_des_decrypt(NX_DES *context, UCHAR source[8], UCHAR destination[8]);
VOID        _nx_des_process_block(UCHAR source[8], UCHAR destination[8], ULONG keys[32]);

#endif

