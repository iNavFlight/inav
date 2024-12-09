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
/*    _lx_nand_flash_page_ecc_check                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks the NAND page and ECC for errors and           */ 
/*    attempts to correct 1 bit errors.                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    page_buffer                           Page buffer                   */ 
/*    ecc_buffer                            Returned ECC buffer           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_256byte_ecc_check      Check 256 bytes and ECC       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NAND flash driver                                                   */ 
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
UINT  _lx_nand_flash_page_ecc_check(LX_NAND_FLASH *nand_flash, UCHAR *page_buffer, UCHAR *ecc_buffer)
{

UINT    bytes_checked;
UINT    status;
UINT    return_status =  LX_SUCCESS;

    
    /* Loop to check the entire NAND flash page.  */
    bytes_checked =  0;
    while (bytes_checked < nand_flash -> lx_nand_flash_bytes_per_page)
    {
    
        /* Check this 256 byte piece of the NAND page.  */
        status =  _lx_nand_flash_256byte_ecc_check(page_buffer, ecc_buffer);
        
        /* Determine if there was an error.  */
        if (status != LX_SUCCESS)
        {
        
            /* Determine if a non-correctable error is present.  */
            if (status == LX_NAND_ERROR_NOT_CORRECTED)
            {
            
                /* Always return a non-correctable error, if present.  */
                return_status =  LX_NAND_ERROR_NOT_CORRECTED;    
                break;
            }
            
            /* A single-bit error was corrected, return this status 
               if there is no LX_ERROR status already detected.  */
            else if (return_status == LX_SUCCESS)
            {
            
                /* Return a notice that the single bit error was corrected.  */
                return_status =  LX_NAND_ERROR_CORRECTED;
            }
        }
        
        /* Move to the next 256 byte portion of the page.  */
        bytes_checked =  bytes_checked + 256;
        
        /* Move the page buffer forward.  */
        page_buffer =  page_buffer + 256;
        
        /* Move the ECC buffer forward, note there are 3 bytes of ECC per page. */
        ecc_buffer =   ecc_buffer + 3;
    }
    
    /* Return status.  */
    return(return_status);
}

