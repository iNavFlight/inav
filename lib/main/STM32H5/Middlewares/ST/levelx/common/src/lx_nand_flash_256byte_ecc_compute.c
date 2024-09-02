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
/** LevelX Component                                                      */ 
/**                                                                       */
/**   NAND Flash                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define LX_SOURCE_CODE


/* Disable ThreadX error checking.  */

#ifndef LX_DISABLE_ERROR_CHECKING
#define LX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include "lx_api.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _lx_nand_flash_256byte_ecc_compute                  PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function computes the ECC for 256 bytes of a NAND flash page.  */ 
/*    The resulting ECC code is returned in 3 bytes.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    page_buffer                           Page buffer                   */ 
/*    ecc_buffer                            Returned ECC buffer           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _lx_nand_flash_page_ecc_compute       NAND page ECC compute         */ 
/*    _lx_nand_flash_256byte_ecc_check      Check 256 bytes and ECC       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_256byte_ecc_compute(UCHAR *page_buffer, UCHAR *ecc_buffer)
{

USHORT      i, j;
USHORT      *data;
USHORT      bits, mask;
USHORT      bit_parity;
USHORT      even_bit_parity;
USHORT      odd_bit_parity;
USHORT      even_byte_parity;
USHORT      odd_byte_parity;


    /* Initialize local variables.  */
    bit_parity =       0;
    even_bit_parity =  0;
    odd_bit_parity =   0;
    even_byte_parity = 0;
    odd_byte_parity =  0;

    /* Initialize the return ECC code area.  */
    ecc_buffer[0]=  0;
    ecc_buffer[1]=  0;
    ecc_buffer[2]=  0;
    
    /* Setup a 16-bit pointer to the buffer area.  */
    data =  (USHORT *) page_buffer;
    
    /* Loop through the 256 byte buffer, 16 bits at a time.  */
    for (i = 0; i < 128; i++) 
    {

        /* Compute the ECC value.  */
        bit_parity = bit_parity ^ data[i];
        
        /* Now count the bits in the current data word.  */
        bits =  0;
        mask =  1;
        for (j = 0; j < 16; j++)
        {

            /* Is the bit set?  */        
            if (data[i] & mask)
            {
            
                /* Yes, increment the bit count.  */
                bits++;
            }

            /* Move the mask to the next bit.  */
            mask = (USHORT) ((mask << 1) & 0xFFFF);
        }
        
        /* Determine if the number of bits is odd.  */
        if ((bits & 1) == 1) 
        {
            
            /* Odd number of bits.  Adjust the odd/even byte parity.  */
            even_byte_parity = (USHORT) ((even_byte_parity ^ (0xffff - i)) & 0xFFFF);
            odd_byte_parity = odd_byte_parity ^ i;
        }
    }

    /* Now look for bits set in the bit parity.  */
    for (i = 0; i < 16; i++) 
    {

        /* Is the bit set?  */
        if (bit_parity & 1) 
        {

            /* Yes, adjust the odd even byte parity.  */
            even_bit_parity = (USHORT) ((even_bit_parity ^ (15 - i)) & 0xFFFF);
            odd_bit_parity =   odd_bit_parity ^ i;
        }

        /* Look at next bit position.  */
        bit_parity =  bit_parity >> 1;
    }
    
    /* At this point, we need to pack the 22 ECC bits into the 3 byte return area.  */
    
    /* Pack bit 21.  */
    ecc_buffer[(21+2)/8] = ((UCHAR)(ecc_buffer[(21+2)/8] | ((odd_byte_parity >> 6) & 1) << (21+2)%8) & 0xFF);
    
    /* Pack bit 20.  */
    ecc_buffer[(20+2)/8] = ((UCHAR)(ecc_buffer[(20+2)/8] | ((even_byte_parity >> 6) & 1) << (20+2)%8) & 0xFF);
    
    /* Pack bit 19.  */
    ecc_buffer[(19+2)/8] = ((UCHAR)(ecc_buffer[(19+2)/8] | ((odd_byte_parity >> 5) & 1) << (19+2)%8) & 0xFF);

    /* Pack bit 18.  */
    ecc_buffer[(18+2)/8] = ((UCHAR)(ecc_buffer[(18+2)/8] | ((even_byte_parity >> 5) & 1) << (18+2)%8) & 0xFF);
    
    /* Pack bit 17.  */
    ecc_buffer[(17+2)/8] = ((UCHAR)(ecc_buffer[(17+2)/8] | ((odd_byte_parity >> 4) & 1) << (17+2)%8) & 0xFF);

    /* Pack bit 16.  */
    ecc_buffer[(16+2)/8] = ((UCHAR)(ecc_buffer[(16+2)/8] | ((even_byte_parity >> 4) & 1) << (16+2)%8) & 0xFF);

    /* Pack bit 15.  */
    ecc_buffer[(15+2)/8] = ((UCHAR)(ecc_buffer[(15+2)/8] | ((odd_byte_parity >> 3) & 1) << (15+2)%8) & 0xFF);

    /* Pack bit 14.  */
    ecc_buffer[(14+2)/8] = ((UCHAR)(ecc_buffer[(14+2)/8] | ((even_byte_parity >> 3) & 1) << (14+2)%8) & 0xFF);

    /* Pack bit 13.  */
    ecc_buffer[(13+2)/8] = ((UCHAR)(ecc_buffer[(13+2)/8] | ((odd_byte_parity >> 2) & 1) << (13+2)%8) & 0xFF);
    
    /* Pack bit 12.  */
    ecc_buffer[(12+2)/8] = ((UCHAR)(ecc_buffer[(12+2)/8] | ((even_byte_parity >> 2) & 1) << (12+2)%8) & 0xFF);

    /* Pack bit 11.  */
    ecc_buffer[(11+2)/8] = ((UCHAR)(ecc_buffer[(11+2)/8] | ((odd_byte_parity >> 1) & 1) << (11+2)%8) & 0xFF);

    /* Pack bit 10.  */
    ecc_buffer[(10+2)/8] = ((UCHAR)(ecc_buffer[(10+2)/8] | ((even_byte_parity >> 1) & 1) << (10+2)%8) & 0xFF);

    /* Pack bit 9.  */    
    ecc_buffer[(9+2)/8] = ((UCHAR)(ecc_buffer[(9+2)/8] | ((odd_byte_parity >> 0) & 1) << (9+2)%8) & 0xFF);

    /* Pack bit 8.  */    
    ecc_buffer[(8+2)/8] = ((UCHAR)(ecc_buffer[(8+2)/8] | ((even_byte_parity >> 0) & 1) << (8+2)%8) & 0xFF);

    /* Pack bit 7.  */
    ecc_buffer[(7+2)/8] = ((UCHAR)(ecc_buffer[(7+2)/8] | ((odd_bit_parity >> 3) & 1) << (7+2)%8) & 0xFF);
    
    /* Pack bit 6.  */
    ecc_buffer[(6+2)/8] = ((UCHAR)(ecc_buffer[(6+2)/8] | ((even_bit_parity >> 3) & 1) << (6+2)%8) & 0xFF);
    
    /* Pack bit 5.  */
    ecc_buffer[(5+2)/8] = ((UCHAR)(ecc_buffer[(5+2)/8] | ((odd_bit_parity >> 2) & 1) << (5+2)%8) & 0xFF);

    /* Pack bit 4.  */
    ecc_buffer[(4+2)/8] = ((UCHAR)(ecc_buffer[(4+2)/8] | ((even_bit_parity >> 2) & 1) << (4+2)%8) & 0xFF);

    /* Pack bit 3.  */
    ecc_buffer[(3+2)/8] = ((UCHAR)(ecc_buffer[(3+2)/8] | ((odd_bit_parity >> 1) & 1) << (3+2)%8) & 0xFF);

    /* Pack bit 2.  */
    ecc_buffer[(2+2)/8] = ((UCHAR)(ecc_buffer[(2+2)/8] | ((even_bit_parity >> 1) & 1) << (2+2)%8) & 0xFF);

    /* Pack bit 1.  */            
    ecc_buffer[(1+2)/8] = ((UCHAR)(ecc_buffer[(1+2)/8] | ((odd_bit_parity >> 0) & 1) << (1+2)%8) & 0xFF);

    /* Pack bit 0.  */    
    ecc_buffer[(0+2)/8] = ((UCHAR)(ecc_buffer[(0+2)/8] | ((even_bit_parity >> 0) & 1) << (0+2)%8) & 0xFF);

    /* Return success!  */
    return(LX_SUCCESS);
}

