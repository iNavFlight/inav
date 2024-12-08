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
  Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
  rights reserved.

  License to copy and use this software is granted provided that it
  is identified as the ""RSA Data Security, Inc. MD5 Message-Digest
  Algorithm"" in all material mentioning or referencing this software
  or this function.

  License is also granted to make and use derivative works provided
  that such works are identified as ""derived from the RSA Data
  Security, Inc. MD5 Message-Digest Algorithm"" in all material
  mentioning or referencing the derived work.

  RSA Data Security, Inc. makes no representations concerning either
  the merchantability of this software or the suitability of this
  software for any particular purpose. It is provided ""as is""
  without express or implied warranty of any kind.

  These notices must be retained in any copies of any part of this
  documentation and/or software.
*/
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** NetX Component                                                        */
/**                                                                       */
/**   MD5 Digest Algorithm (MD5)                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    nx_md5.h                                            PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX MD5 algorithm, derived principally from  */ 
/*    RFC1321. From a user-specified number of input bytes, this routine  */ 
/*    produces a 16-byte (128-bit) digest or sometimes called a hash      */ 
/*    value. The resulting digest is returned in a 16-byte array supplied */ 
/*    by the caller.                                                      */ 
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef  NX_MD5_H
#define  NX_MD5_H

#include "nx_api.h"


/* Define the MD5 context structure.  */

typedef struct NX_MD5_STRUCT
{

    ULONG       nx_md5_states[4];                       /* Contains each state (A,B,C,D)    */
    ULONG       nx_md5_bit_count[2];                    /* Contains the 64-bit total bit    */ 
                                                        /*   count, where index 0 holds the */ 
                                                        /*   least significant bit count and*/ 
                                                        /*   index 1 contains the most      */ 
                                                        /*   significant portion of the bit */ 
                                                        /*   count                          */ 
    UCHAR       nx_md5_buffer[64];                      /* Working buffer for MD5 algorithm */
                                                        /*   where partial buffers are      */ 
                                                        /*   accumulated until a full block */ 
                                                        /*   can be processed               */ 
} NX_MD5;


/* Define the function prototypes for MD5.  */

UINT        _nx_md5_initialize(NX_MD5 *context);
UINT        _nx_md5_update(NX_MD5 *context, UCHAR *input_ptr, UINT input_length);
UINT        _nx_md5_digest_calculate(NX_MD5 *context, UCHAR digest[16]);
VOID        _nx_md5_process_buffer(NX_MD5 *context, UCHAR buffer[64]);

#endif
