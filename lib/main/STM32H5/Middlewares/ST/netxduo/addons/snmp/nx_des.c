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

#include "nx_api.h"
#include "nx_des.h"


/* Define macros for the DES transform function.  */


/* Define the eight S-box data structures used in the permutation. Keep them static, since there is no 
   reason to have these symbols referenced outside this file. */

static ULONG sb1[64] =
{

    0x01010400UL, 0x00000000UL, 0x00010000UL, 0x01010404UL, 0x01010004UL, 0x00010404UL, 0x00000004UL, 0x00010000UL,
    0x00000400UL, 0x01010400UL, 0x01010404UL, 0x00000400UL, 0x01000404UL, 0x01010004UL, 0x01000000UL, 0x00000004UL,
    0x00000404UL, 0x01000400UL, 0x01000400UL, 0x00010400UL, 0x00010400UL, 0x01010000UL, 0x01010000UL, 0x01000404UL,
    0x00010004UL, 0x01000004UL, 0x01000004UL, 0x00010004UL, 0x00000000UL, 0x00000404UL, 0x00010404UL, 0x01000000UL,
    0x00010000UL, 0x01010404UL, 0x00000004UL, 0x01010000UL, 0x01010400UL, 0x01000000UL, 0x01000000UL, 0x00000400UL,
    0x01010004UL, 0x00010000UL, 0x00010400UL, 0x01000004UL, 0x00000400UL, 0x00000004UL, 0x01000404UL, 0x00010404UL,
    0x01010404UL, 0x00010004UL, 0x01010000UL, 0x01000404UL, 0x01000004UL, 0x00000404UL, 0x00010404UL, 0x01010400UL,
    0x00000404UL, 0x01000400UL, 0x01000400UL, 0x00000000UL, 0x00010004UL, 0x00010400UL, 0x00000000UL, 0x01010004UL
};

static ULONG sb2[64] =
{

    0x80108020UL, 0x80008000UL, 0x00008000UL, 0x00108020UL, 0x00100000UL, 0x00000020UL, 0x80100020UL, 0x80008020UL,
    0x80000020UL, 0x80108020UL, 0x80108000UL, 0x80000000UL, 0x80008000UL, 0x00100000UL, 0x00000020UL, 0x80100020UL,
    0x00108000UL, 0x00100020UL, 0x80008020UL, 0x00000000UL, 0x80000000UL, 0x00008000UL, 0x00108020UL, 0x80100000UL,
    0x00100020UL, 0x80000020UL, 0x00000000UL, 0x00108000UL, 0x00008020UL, 0x80108000UL, 0x80100000UL, 0x00008020UL,
    0x00000000UL, 0x00108020UL, 0x80100020UL, 0x00100000UL, 0x80008020UL, 0x80100000UL, 0x80108000UL, 0x00008000UL,
    0x80100000UL, 0x80008000UL, 0x00000020UL, 0x80108020UL, 0x00108020UL, 0x00000020UL, 0x00008000UL, 0x80000000UL,
    0x00008020UL, 0x80108000UL, 0x00100000UL, 0x80000020UL, 0x00100020UL, 0x80008020UL, 0x80000020UL, 0x00100020UL,
    0x00108000UL, 0x00000000UL, 0x80008000UL, 0x00008020UL, 0x80000000UL, 0x80100020UL, 0x80108020UL, 0x00108000UL
};

static ULONG sb3[64] =
{
  
    0x00000208UL, 0x08020200UL, 0x00000000UL, 0x08020008UL, 0x08000200UL, 0x00000000UL, 0x00020208UL, 0x08000200UL,
    0x00020008UL, 0x08000008UL, 0x08000008UL, 0x00020000UL, 0x08020208UL, 0x00020008UL, 0x08020000UL, 0x00000208UL,
    0x08000000UL, 0x00000008UL, 0x08020200UL, 0x00000200UL, 0x00020200UL, 0x08020000UL, 0x08020008UL, 0x00020208UL,
    0x08000208UL, 0x00020200UL, 0x00020000UL, 0x08000208UL, 0x00000008UL, 0x08020208UL, 0x00000200UL, 0x08000000UL,
    0x08020200UL, 0x08000000UL, 0x00020008UL, 0x00000208UL, 0x00020000UL, 0x08020200UL, 0x08000200UL, 0x00000000UL,
    0x00000200UL, 0x00020008UL, 0x08020208UL, 0x08000200UL, 0x08000008UL, 0x00000200UL, 0x00000000UL, 0x08020008UL,
    0x08000208UL, 0x00020000UL, 0x08000000UL, 0x08020208UL, 0x00000008UL, 0x00020208UL, 0x00020200UL, 0x08000008UL,
    0x08020000UL, 0x08000208UL, 0x00000208UL, 0x08020000UL, 0x00020208UL, 0x00000008UL, 0x08020008UL, 0x00020200UL
};

static ULONG sb4[64] =
{

    0x00802001UL, 0x00002081UL, 0x00002081UL, 0x00000080UL, 0x00802080UL, 0x00800081UL, 0x00800001UL, 0x00002001UL,
    0x00000000UL, 0x00802000UL, 0x00802000UL, 0x00802081UL, 0x00000081UL, 0x00000000UL, 0x00800080UL, 0x00800001UL,
    0x00000001UL, 0x00002000UL, 0x00800000UL, 0x00802001UL, 0x00000080UL, 0x00800000UL, 0x00002001UL, 0x00002080UL,
    0x00800081UL, 0x00000001UL, 0x00002080UL, 0x00800080UL, 0x00002000UL, 0x00802080UL, 0x00802081UL, 0x00000081UL,
    0x00800080UL, 0x00800001UL, 0x00802000UL, 0x00802081UL, 0x00000081UL, 0x00000000UL, 0x00000000UL, 0x00802000UL,
    0x00002080UL, 0x00800080UL, 0x00800081UL, 0x00000001UL, 0x00802001UL, 0x00002081UL, 0x00002081UL, 0x00000080UL,
    0x00802081UL, 0x00000081UL, 0x00000001UL, 0x00002000UL, 0x00800001UL, 0x00002001UL, 0x00802080UL, 0x00800081UL,
    0x00002001UL, 0x00002080UL, 0x00800000UL, 0x00802001UL, 0x00000080UL, 0x00800000UL, 0x00002000UL, 0x00802080UL
};

static ULONG sb5[64] =
{

    0x00000100UL, 0x02080100UL, 0x02080000UL, 0x42000100UL, 0x00080000UL, 0x00000100UL, 0x40000000UL, 0x02080000UL,
    0x40080100UL, 0x00080000UL, 0x02000100UL, 0x40080100UL, 0x42000100UL, 0x42080000UL, 0x00080100UL, 0x40000000UL,
    0x02000000UL, 0x40080000UL, 0x40080000UL, 0x00000000UL, 0x40000100UL, 0x42080100UL, 0x42080100UL, 0x02000100UL,
    0x42080000UL, 0x40000100UL, 0x00000000UL, 0x42000000UL, 0x02080100UL, 0x02000000UL, 0x42000000UL, 0x00080100UL,
    0x00080000UL, 0x42000100UL, 0x00000100UL, 0x02000000UL, 0x40000000UL, 0x02080000UL, 0x42000100UL, 0x40080100UL,
    0x02000100UL, 0x40000000UL, 0x42080000UL, 0x02080100UL, 0x40080100UL, 0x00000100UL, 0x02000000UL, 0x42080000UL,
    0x42080100UL, 0x00080100UL, 0x42000000UL, 0x42080100UL, 0x02080000UL, 0x00000000UL, 0x40080000UL, 0x42000000UL,
    0x00080100UL, 0x02000100UL, 0x40000100UL, 0x00080000UL, 0x00000000UL, 0x40080000UL, 0x02080100UL, 0x40000100UL
};

static ULONG sb6[64] =
{

    0x20000010UL, 0x20400000UL, 0x00004000UL, 0x20404010UL, 0x20400000UL, 0x00000010UL, 0x20404010UL, 0x00400000UL,
    0x20004000UL, 0x00404010UL, 0x00400000UL, 0x20000010UL, 0x00400010UL, 0x20004000UL, 0x20000000UL, 0x00004010UL,
    0x00000000UL, 0x00400010UL, 0x20004010UL, 0x00004000UL, 0x00404000UL, 0x20004010UL, 0x00000010UL, 0x20400010UL,
    0x20400010UL, 0x00000000UL, 0x00404010UL, 0x20404000UL, 0x00004010UL, 0x00404000UL, 0x20404000UL, 0x20000000UL,
    0x20004000UL, 0x00000010UL, 0x20400010UL, 0x00404000UL, 0x20404010UL, 0x00400000UL, 0x00004010UL, 0x20000010UL,
    0x00400000UL, 0x20004000UL, 0x20000000UL, 0x00004010UL, 0x20000010UL, 0x20404010UL, 0x00404000UL, 0x20400000UL,
    0x00404010UL, 0x20404000UL, 0x00000000UL, 0x20400010UL, 0x00000010UL, 0x00004000UL, 0x20400000UL, 0x00404010UL,
    0x00004000UL, 0x00400010UL, 0x20004010UL, 0x00000000UL, 0x20404000UL, 0x20000000UL, 0x00400010UL, 0x20004010UL
};

static ULONG sb7[64] =
{

    0x00200000UL, 0x04200002UL, 0x04000802UL, 0x00000000UL, 0x00000800UL, 0x04000802UL, 0x00200802UL, 0x04200800UL,
    0x04200802UL, 0x00200000UL, 0x00000000UL, 0x04000002UL, 0x00000002UL, 0x04000000UL, 0x04200002UL, 0x00000802UL,
    0x04000800UL, 0x00200802UL, 0x00200002UL, 0x04000800UL, 0x04000002UL, 0x04200000UL, 0x04200800UL, 0x00200002UL,
    0x04200000UL, 0x00000800UL, 0x00000802UL, 0x04200802UL, 0x00200800UL, 0x00000002UL, 0x04000000UL, 0x00200800UL,
    0x04000000UL, 0x00200800UL, 0x00200000UL, 0x04000802UL, 0x04000802UL, 0x04200002UL, 0x04200002UL, 0x00000002UL,
    0x00200002UL, 0x04000000UL, 0x04000800UL, 0x00200000UL, 0x04200800UL, 0x00000802UL, 0x00200802UL, 0x04200800UL,
    0x00000802UL, 0x04000002UL, 0x04200802UL, 0x04200000UL, 0x00200800UL, 0x00000000UL, 0x00000002UL, 0x04200802UL,
    0x00000000UL, 0x00200802UL, 0x04200000UL, 0x00000800UL, 0x04000002UL, 0x04000800UL, 0x00000800UL, 0x00200002UL
};

static ULONG sb8[64] =
{

    0x10001040UL, 0x00001000UL, 0x00040000UL, 0x10041040UL, 0x10000000UL, 0x10001040UL, 0x00000040UL, 0x10000000UL,
    0x00040040UL, 0x10040000UL, 0x10041040UL, 0x00041000UL, 0x10041000UL, 0x00041040UL, 0x00001000UL, 0x00000040UL,
    0x10040000UL, 0x10000040UL, 0x10001000UL, 0x00001040UL, 0x00041000UL, 0x00040040UL, 0x10040040UL, 0x10041000UL,
    0x00001040UL, 0x00000000UL, 0x00000000UL, 0x10040040UL, 0x10000040UL, 0x10001000UL, 0x00041040UL, 0x00040000UL,
    0x00041040UL, 0x00040000UL, 0x10041000UL, 0x00001000UL, 0x00000040UL, 0x10040040UL, 0x00001000UL, 0x00041040UL,
    0x10001000UL, 0x00000040UL, 0x10000040UL, 0x10040000UL, 0x10040040UL, 0x10000000UL, 0x00040000UL, 0x10001040UL,
    0x00000000UL, 0x10041040UL, 0x00040040UL, 0x10000040UL, 0x10040000UL, 0x10001000UL, 0x10001040UL, 0x00000000UL,
    0x10041040UL, 0x00041000UL, 0x00041000UL, 0x00001040UL, 0x00001040UL, 0x00040040UL, 0x10000000UL, 0x10041000UL
};


/* Define the left half bit swap table.  */

static ULONG left_half_bit_swap[16] =
{

    0x00000000UL, 0x00000001UL, 0x00000100UL, 0x00000101UL,
    0x00010000UL, 0x00010001UL, 0x00010100UL, 0x00010101UL,
    0x01000000UL, 0x01000001UL, 0x01000100UL, 0x01000101UL,
    0x01010000UL, 0x01010001UL, 0x01010100UL, 0x01010101UL
};

/* Define the right half bit swap table.  */

static ULONG right_half_bit_swap[16] =
{
  0x00000000UL, 0x01000000UL, 0x00010000UL, 0x01010000UL,
  0x00000100UL, 0x01000100UL, 0x00010100UL, 0x01010100UL,
  0x00000001UL, 0x01000001UL, 0x00010001UL, 0x01010001UL,
  0x00000101UL, 0x01000101UL, 0x00010101UL, 0x01010101UL
};


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_des_key_set                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the 32 encryption keys as well as the 32      */ 
/*    decryption keys for the DES algorithm. It must be called before     */ 
/*    either _nx_des_encrypt or _nx_des_decrypt can be called.            */ 
/*    destination.                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               DES context pointer           */ 
/*    key                                   8-byte (64-bit) key           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
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
UINT  _nx_des_key_set(NX_DES *context, UCHAR key[8])
{

ULONG   left, right, temp;
ULONG   *encrypt_keys_ptr;
ULONG   *decrypt_keys_ptr;
UINT    round;
    

    /* Determine if the context is non-null.  */
    if (context == NX_NULL)
        return(NX_PTR_ERROR);

    /* First, convert the 8-byte raw key into two ULONG halves, in an endian neutral fashion.  */
    left =  (((ULONG) key[0]) << 24) | (((ULONG) key[1]) << 16) | (((ULONG) key[2]) << 8) | ((ULONG) key[3]);
    right =  (((ULONG) key[4]) << 24) | (((ULONG) key[5]) << 16) | (((ULONG) key[6]) << 8) | ((ULONG) key[7]);

    /* Perform permutation on the key halves.  */
    temp =  ((right >> 4) ^ left) & 0x0F0F0F0FUL;
    left =  left ^ temp;
    right =  right ^ (temp << 4);
    temp =  (right ^ left) & 0x10101010;
    left =  left ^ temp;
    right =  right ^ temp;

    left =  (left_half_bit_swap[(left) & 0xf] << 3) | (left_half_bit_swap[(left >> 8) & 0xf] << 2) | 
            (left_half_bit_swap[(left >> 16) & 0xf] << 1) | (left_half_bit_swap[(left >> 24) & 0xf]) | 
            (left_half_bit_swap[(left >> 5) & 0xf] << 7) | (left_half_bit_swap[(left >> 13) & 0xf] << 6) | 
            (left_half_bit_swap[(left >> 21) & 0xf] << 5) | (left_half_bit_swap[(left >> 29) & 0xf] << 4);
    left =  left & 0x0fffffff;

    right = (right_half_bit_swap[(right >> 1) & 0xf] << 3) | (right_half_bit_swap[(right >> 9) & 0xf] << 2) | 
            (right_half_bit_swap[(right >> 17) & 0xf] << 1) | (right_half_bit_swap[(right >> 25) & 0xf]) | 
            (right_half_bit_swap[(right >> 4) & 0xf] << 7) | (right_half_bit_swap[(right >> 12) & 0xf] << 6) | 
            (right_half_bit_swap[(right >> 20) & 0xf] << 5) | (right_half_bit_swap[(right >> 28) & 0xf] << 4);
    right = right & 0x0fffffff;

    /* Setup encryption keys pointer.  */
    encrypt_keys_ptr =  context -> nx_des_encryption_keys;
 
    /* Calculate the encryption keys.  */
    for (round = 0; round < 16; round++)
    {

        /* Modify the left and right portions of the keys.  */
        if ((round < 2) || (round == 8) || (round == 15))
        {

            left =   ((left << 1) | (left >> 27)) & 0x0FFFFFFFUL;
            right =  ((right << 1) | (right >> 27)) & 0x0FFFFFFFUL;
        }
        else
        {

            left =   ((left << 2) | (left >> 26)) & 0x0FFFFFFFUL;
            right =  ((right << 2) | (right >> 26)) & 0x0FFFFFFFUL;
        }

        /* Setup the key.  */
        *encrypt_keys_ptr++ =  ((left << 4)   & 0x24000000UL) | ((left << 28)  & 0x10000000UL) | 
                               ((left << 14)  & 0x08000000UL) | ((left << 18)  & 0x02080000UL) | 
                               ((left << 6)   & 0x01000000UL) | ((left << 9)   & 0x00200000UL) |
                               ((left >> 1)   & 0x00100000UL) | ((left << 10)  & 0x00040000UL) | 
                               ((left << 2)   & 0x00020000UL) | ((left >> 10)  & 0x00010000UL) | 
                               ((right >> 13) & 0x00002000UL) | ((right >> 4)  & 0x00001000UL) | 
                               ((right << 6)  & 0x00000800UL) | ((right >> 1)  & 0x00000400UL) | 
                               ((right >> 14) & 0x00000200UL) | (right         & 0x00000100UL) | 
                               ((right >> 5)  & 0x00000020UL) | ((right >> 10) & 0x00000010UL) | 
                               ((right >> 3)  & 0x00000008UL) | ((right >> 18) & 0x00000004UL) | 
                               ((right >> 26) & 0x00000002UL) | ((right >> 24) & 0x00000001UL);

        /* Setup the next key.  */
        *encrypt_keys_ptr++ =  ((left << 15)  & 0x20000000UL) | ((left << 17)  & 0x10000000UL) | 
                               ((left << 10)  & 0x08000000UL) | ((left << 22)  & 0x04000000UL) | 
                               ((left >> 2)   & 0x02000000UL) | ((left << 1)   & 0x01000000UL) | 
                               ((left << 16)  & 0x00200000UL) | ((left << 11)  & 0x00100000UL) | 
                               ((left << 3)   & 0x00080000UL) | ((left >> 6)   & 0x00040000UL) | 
                               ((left << 15)  & 0x00020000UL) | ((left >> 4)   & 0x00010000UL) | 
                               ((right >> 2)  & 0x00002000UL) | ((right << 8)  & 0x00001000UL) | 
                               ((right >> 14) & 0x00000808UL) | ((right >> 9)  & 0x00000400UL) | 
                               ((right)       & 0x00000200UL) | ((right << 7)  & 0x00000100UL) | 
                               ((right >> 7)  & 0x00000020UL) | ((right >> 3)  & 0x00000011UL) | 
                               ((right << 2)  & 0x00000004UL) | ((right >> 21) & 0x00000002UL);
    }

    /* Reposition the encryption key pointer.  */
    encrypt_keys_ptr =  encrypt_keys_ptr - 2;

    /* Setup decryption pointer.  */
    decrypt_keys_ptr =  context -> nx_des_decryption_keys;

    /* Now setup decryption keys.  */
    for (round = 0; round < 16; round++)
    {

        /* Copy the reverse of the encryption keys.  */
        *decrypt_keys_ptr++ =  *encrypt_keys_ptr;
        *decrypt_keys_ptr++ =  *(encrypt_keys_ptr + 1);

        /* Adjust the encryption keys pointer.  */
        encrypt_keys_ptr =  encrypt_keys_ptr - 2;
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_des_encrypt                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses the DES algorithm to encrypt 8-bytes (64-bits).  */ 
/*    The result is 8 encrypted bytes. Note that the caller must make     */ 
/*    sure the source and destination are 8-bytes in size!                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               DES context pointer           */ 
/*    source                                8-byte source                 */
/*    destination                           8-byte destination            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_des_process_block                 Encrypt 8-bytes of source     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
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
UINT  _nx_des_encrypt(NX_DES *context, UCHAR source[8], UCHAR destination[8])
{

    /* Determine if the context is non-null.  */
    if (context == NX_NULL)
        return(NX_PTR_ERROR);

    /* Encrypt the block by supplying the encryption key set.  */
    _nx_des_process_block(source, destination, context -> nx_des_encryption_keys);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_des_decrypt                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses the DES algorithm to decrypt 8-bytes (64-bits).  */ 
/*    The result is 8 original source bytes. Note that the caller must    */ 
/*    make sure the source and destination are 8-bytes in size!           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               DES context pointer           */ 
/*    source                                8-byte source                 */
/*    destination                           8-byte destination            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_des_process_block                 Decrypt 8-bytes of source     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
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
UINT  _nx_des_decrypt(NX_DES *context, UCHAR source[8], UCHAR destination[8])
{

    /* Determine if the context is non-null.  */
    if (context == NX_NULL)
        return(NX_PTR_ERROR);

    /* Decrypt the block by supplying the decryption key set.  */
    _nx_des_process_block(source, destination, context -> nx_des_decryption_keys);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_des_process_block                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses the DES algorithm to decrypt 8-bytes (64-bits).  */ 
/*    The result is 8 original source bytes. Note that the caller must    */ 
/*    make sure the source and destination are 8-bytes in size!           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    source                                8-byte source                 */
/*    destination                           8-byte destination            */ 
/*    keys                                  Pointer to either the encrypt */ 
/*                                            or decrypt keys             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
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
VOID  _nx_des_process_block(UCHAR source[8], UCHAR destination[8], ULONG keys[32])
{

ULONG   left, right, temp;
ULONG   *key_ptr;
UINT    round;
    

    /* First, convert the 8-byte source into two ULONG halves, in an endian neutral fashion.  */
    left =  (((ULONG) source[0]) << 24) | (((ULONG) source[1]) << 16) | (((ULONG) source[2]) << 8) | ((ULONG) source[3]);
    right =  (((ULONG) source[4]) << 24) | (((ULONG) source[5]) << 16) | (((ULONG) source[6]) << 8) | ((ULONG) source[7]);

    /* Compute the initial permutation.  */
    temp =   ((left >> 4) ^ right) & 0x0F0F0F0FUL;
    right =  right ^ temp;
    left =   left ^ (temp << 4);
    temp =   ((left >> 16) ^ right) & 0x0000FFFFUL;
    right =  right ^ temp;
    left =   left ^ (temp << 16);
    temp =   ((right >> 2) ^ left) & 0x33333333UL;
    left =   left ^ temp;
    right =  right ^ (temp << 2);
    temp =   ((right >> 8) ^ left) & 0x00FF00FFUL;
    left =   left ^ temp;
    right =  right ^ (temp << 8);
    right =  ((right << 1) | (right >> 31)) & 0xFFFFFFFFUL;
    temp =   (left ^ right) & 0xAAAAAAAAUL;
    right =  right ^ temp;
    left =   left ^ temp;
    left =   ((left << 1) | (left >> 31)) & 0xFFFFFFFFUL;

    /* Setup pointer to input keys.  */
    key_ptr =  keys;

    /* Now process the 16 rounds of the DES computation. There are two rounds per
       loop.  */
    for (round = 0; round < 8; round++)
    {

        /* Calculate the left half.  */
        temp =  *key_ptr++ ^ right;
        left =  left ^ sb8[temp & 0x3F] ^ sb6[(temp >> 8) & 0x3F] ^ sb4[(temp >> 16) & 0x3F] ^ sb2[(temp >> 24) & 0x3F];

        temp =  *key_ptr++ ^ ((right << 28) | (right >> 4));
        left =  left ^ sb7[temp & 0x3F] ^ sb5[(temp >> 8) & 0x3F] ^ sb3[(temp >> 16) & 0x3F] ^ sb1[(temp >> 24) & 0x3F];

        /* Calculate the right half.  */
        temp =  *key_ptr++ ^ left;
        right =  right ^ sb8[temp & 0x3F] ^ sb6[(temp >> 8) & 0x3F] ^ sb4[(temp >> 16) & 0x3F] ^ sb2[(temp >> 24) & 0x3F];

        temp =  *key_ptr++ ^ ((left << 28) | (left >> 4));
        right =  right ^ sb7[temp & 0x3F] ^ sb5[(temp >> 8) & 0x3F] ^ sb3[(temp >> 16) & 0x3F] ^ sb1[(temp >> 24) & 0x3F];
    }

    /* Now compute the final permutation.  */
    right =  ((right << 31) | (right >> 1)) & 0xFFFFFFFFUL;
    temp =   (right ^ left) & 0xAAAAAAAAUL;
    right =  right ^ temp;
    left =   left ^ temp;
    left =   ((left << 31) | (left >> 1)) & 0xFFFFFFFFUL;
    temp =   ((left >> 8) ^ right) & 0x00FF00FFUL;
    right =  right ^ temp;
    left =   left ^  (temp << 8);
    temp =   ((left >> 2) ^ right) & 0x33333333UL;
    right =  right ^ temp;
    left =   left ^ (temp << 2);
    temp =   ((right >> 16) ^ left) & 0x0000FFFFUL;
    left =   left ^ temp;
    right =  right ^ (temp << 16);
    temp =   ((right >> 4) ^ left) & 0x0F0F0F0FUL;
    left =   left ^ temp;
    right =  right ^ (temp << 4);

    /* Finally, build the output.  */
    destination[0] =  (UCHAR) (right >> 24);
    destination[1] =  (UCHAR) (right >> 16);
    destination[2] =  (UCHAR) (right >> 8);
    destination[3] =  (UCHAR) (right);
    destination[4] =  (UCHAR) (left >> 24);
    destination[5] =  (UCHAR) (left >> 16);
    destination[6] =  (UCHAR) (left >> 8);
    destination[7] =  (UCHAR) (left);
}

