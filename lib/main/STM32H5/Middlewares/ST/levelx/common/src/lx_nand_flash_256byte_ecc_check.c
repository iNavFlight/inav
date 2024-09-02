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
/*    _lx_nand_flash_256byte_ecc_check                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks 256 bytes of a NAND flash and ECC and          */ 
/*    attempts to correct any single bit errors.                          */ 
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
/*    _lx_nand_flash_256byte_ecc_compute    Compute ECC for 256 bytes     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _lx_nand_flash_page_ecc_check         NAND page check               */ 
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
UINT  _lx_nand_flash_256byte_ecc_check(UCHAR *page_buffer, UCHAR *ecc_buffer)
{

INT     i, j;
UCHAR   mask; 
UCHAR   new_ecc_buffer[3];
UCHAR   ecc_errors[3];
INT     error_count;
USHORT  *data;
USHORT  byte;
USHORT  bit;
ULONG   correction_code;


    /* Clear the error count.  */
    ecc_errors[0] = 0;
    ecc_errors[1] = 0;
    ecc_errors[2] = 0;

    /* Calculate a new ECC for the 256 byte buffer.  */
    _lx_nand_flash_256byte_ecc_compute(page_buffer, new_ecc_buffer);

    /* Clear error count.  */
    error_count =  0;

    /* Loop through the ECC bytes to determine if there is an error in the page.  */
    for (i = 0; i < 3; i++) 
    {
    
        /* Check for differences in the ECCs.  */
        ecc_errors[i] =  new_ecc_buffer[i] ^ ecc_buffer[i];
        
        /* Are there any errors?  */
        if (ecc_errors[i])
        {
        
            /* Accumulate the count of set bits.  */
            mask = 1;
            for (j = 0; j < 8; j++)
            {
            
                /* Is this bit set?  */
                if (ecc_errors[i] & mask)
                {
                    
                    /* Yes, increment the count.  */
                    error_count++;
                }
                
                /* Move mask to next bit.  */
                mask = (UCHAR) ((mask << 1) & 0xFF);
            }
        }
    }

    /* Determine if there are any errors.  */
    if (error_count == 0)
    {

        /* Everything is okay, return success.  */
        return(LX_SUCCESS);
    }

    /* Was a correctable error discovered?  */
    else if (error_count == 11)  
    {

        /* Initialize bit and byte offset values.  */
        bit = 0;
        byte = 0;

        /* Setup the data pointer.  */
        data =  (USHORT *) page_buffer;

        /* Calculate the 32-bit correction code.  */
        correction_code = (ULONG) (ecc_errors[2] << 16) | (ULONG)(ecc_errors[1] << 8) | (ULONG)ecc_errors[0];

        /* Unpack the correction code.  */
        byte = (USHORT) ((byte | ((correction_code >> (21+2)) & 1) << 6) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (19+2)) & 1) << 5) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (17+2)) & 1) << 4) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (15+2)) & 1) << 3) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (13+2)) & 1) << 2) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (11+2)) & 1) << 1) & 0xFFFF);
        byte = (USHORT) ((byte | ((correction_code >> (9+2))  & 1) << 0) & 0xFFFF);
        bit  = (USHORT) ((bit | ((correction_code >> (7+2))  & 1) << 3) & 0xFFFF);
        bit  = (USHORT) ((bit | ((correction_code >> (5+2))  & 1) << 2) & 0xFFFF);
        bit  = (USHORT) ((bit | ((correction_code >> (3+2))  & 1) << 1) & 0xFFFF);
        bit  = (USHORT) ((bit | ((correction_code >> (1+2))  & 1) << 0) & 0xFFFF);

        /* Fix the error.  */
        data[byte] = (USHORT) ((data[byte] ^ (1 << bit)) & 0xFFFF);
        
        /* Return an error corrected status.  */
      return(LX_NAND_ERROR_CORRECTED);
    }
    
    /* Otherwise, an unrecoverable ECC or data error is present.  */
    else 
    {

        /* Return an error.  */
        return(LX_NAND_ERROR_NOT_CORRECTED);
    }
}

