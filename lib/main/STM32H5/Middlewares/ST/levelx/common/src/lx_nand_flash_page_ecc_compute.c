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
/*    _lx_nand_flash_page_ecc_compute                     PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function computes the ECC for a NAND flash page.               */ 
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
/*    _lx_nand_flash_256byte_ecc_compute    Compute ECC for 256 bytes     */ 
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
UINT  _lx_nand_flash_page_ecc_compute(LX_NAND_FLASH *nand_flash, UCHAR *page_buffer, UCHAR *ecc_buffer)
{

UINT    bytes_computed;

    
    /* Loop to compute the ECC over the entire NAND flash page.  */
    bytes_computed =  0;
    while (bytes_computed < nand_flash -> lx_nand_flash_bytes_per_page)
    {
    
        /* Compute the ECC for this 256 byte piece of the page.  */
        _lx_nand_flash_256byte_ecc_compute(page_buffer, ecc_buffer);
        
        /* Move to the next 256 byte portion of the page.  */
        bytes_computed =  bytes_computed + 256;
        
        /* Move the page buffer forward.  */
        page_buffer =  page_buffer + 256;
        
        /* Move the ECC buffer forward, note there are 3 bytes of ECC per page. */
        ecc_buffer =   ecc_buffer + 3;
    }
    
    /* Return success.  */
    return(LX_SUCCESS);
}

