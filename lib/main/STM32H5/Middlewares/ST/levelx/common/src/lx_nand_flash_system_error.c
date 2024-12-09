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
/*    _lx_nand_flash_system_error                         PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles system errors in the NAND flash.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    error_code                            System error code             */ 
/*    block                                 Block where error occurred    */ 
/*    page                                  Page where error occurred     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_system_error)   Driver system error handler   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Internal LevelX                                                     */ 
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
VOID  _lx_nand_flash_system_error(LX_NAND_FLASH *nand_flash, UINT error_code, ULONG block, ULONG page)
{

    /* Increment the system error counter.  */
    nand_flash -> lx_nand_flash_diagnostic_system_errors++;
    
    /* Save the most recent system error code.  */
    nand_flash -> lx_nand_flash_diagnostic_system_error =  error_code;
    
    /* Determine if the system error is a NAND page corrected error.  */
    if (error_code == LX_NAND_ERROR_CORRECTED)
    {
    
        /* Yes, increment error correction information.  */
        nand_flash -> lx_nand_flash_page_corrections++;
        
        /* Remember the last block/page of corrected error.  */
        nand_flash -> lx_nand_flash_last_block_correction =  block;
        nand_flash -> lx_nand_flash_last_page_correction =   page;
    }
    
    /* Determine if the driver has setup a system error handler.  */
    if (nand_flash -> lx_nand_flash_driver_system_error)
    {
    
        /* Yes, call the driver's system error handler.  */
        (nand_flash -> lx_nand_flash_driver_system_error)(error_code, block, page);
    }
}

