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
/*    _lx_nand_flash_driver_block_erase                   PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver to erase a block.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    erase_count                           Erase count for this block    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_block_erase)    Driver erase block            */ 
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
UINT  _lx_nand_flash_driver_block_erase(LX_NAND_FLASH *nand_flash, ULONG block, ULONG erase_count)
{

ULONG   cache_index;
ULONG   i;
UINT    status;


    /* Determine if the block status cache is enabled.  */
    if (nand_flash -> lx_nand_flash_block_status_cache != LX_NULL)
    {
    
        /* Save the block status value in the cache.  */
        nand_flash -> lx_nand_flash_block_status_cache[block] =  0xFF;       
    }
    
    /* Determine if the page extra byte cache is enabled.  */
    if (nand_flash -> lx_nand_flash_page_extra_bytes_cache != LX_NULL)
    {
    
        /* Calculate the cache index.  */
        cache_index =  (block * nand_flash -> lx_nand_flash_pages_per_block);

        /* Loop to clear the entries in the page extra bytes cache.  */
        for (i = 0; i < nand_flash -> lx_nand_flash_pages_per_block; i++)
        {
        
            /* Clear each cache entry.  */
            nand_flash -> lx_nand_flash_page_extra_bytes_cache[cache_index+i].lx_nand_page_extra_info_logical_sector =  (ULONG) 0xFFFFFFFF;
        }
    }

    /* Determine if the page 0 cache is enabled.  */
    if (nand_flash -> lx_nand_flash_page_0_cache != LX_NULL)
    {
        
        /* Yes, the page 0 cache is enabled.  */

        /* Build index to page 0 cache.  */
        cache_index =  block * (nand_flash -> lx_nand_flash_pages_per_block + 1);

        /* Clear the associated page 0 cache.  */
        for (i = 0; i < (nand_flash -> lx_nand_flash_pages_per_block + 1); i++)
        {

            /* Clear each cache entry.  */
            nand_flash -> lx_nand_flash_page_0_cache[cache_index+i] = (ULONG) 0xFFFFFFFF;
        }   
    }    

    /* Increment the block erases count.  */
    nand_flash -> lx_nand_flash_diagnostic_block_erases++;

    /* Call driver erase block function.  */
    status =  (nand_flash -> lx_nand_flash_driver_block_erase)(block, erase_count);

    /* Return status.  */
    return(status);
}


