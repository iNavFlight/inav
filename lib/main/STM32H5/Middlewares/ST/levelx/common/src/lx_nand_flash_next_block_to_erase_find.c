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
/*    _lx_nand_flash_next_block_to_erase_find             PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the next block to erase in the NAND flash.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    return_erase_block                    Returned block to erase       */ 
/*    return_erase_count                    Returned erase count of block */ 
/*    return_mapped_pages                   Returned number of mapped     */ 
/*                                            pages                       */ 
/*    return_obsolete_pages                 Returned number of obsolete   */ 
/*                                            pages                       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _lx_nand_flash_driver_extra_bytes_get Driver block extra bytes get  */  
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Driver block status get       */ 
/*    _lx_nand_flash_driver_read            Driver page read              */ 
/*    _lx_nand_flash_system_error           Internal system error handler */ 
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
UINT  _lx_nand_flash_next_block_to_erase_find(LX_NAND_FLASH *nand_flash, ULONG *return_erase_block, ULONG *return_erase_count, ULONG *return_mapped_pages, ULONG *return_obsolete_pages)
{

LX_NAND_PAGE_EXTRA_INFO extra_info;
UCHAR                   block_status;
ULONG                   *block_word_ptr;
ULONG                   block;
ULONG                   page;
ULONG                   mapped_pages;
ULONG                   erase_count;
ULONG                   obsolete_pages;
ULONG                   min_block_erase = 0;
ULONG                   min_block_erase_count = 0;
ULONG                   min_block_obsolete_count = 0;
ULONG                   min_block_mapped_count = 0;
ULONG                   max_obsolete_pages = 0;
ULONG                   max_obsolete_block = 0;
ULONG                   max_obsolete_erase_count = 0;
ULONG                   max_obsolete_mapped_count = 0;
ULONG                   min_system_block_erase_count = 0;
ULONG                   max_system_block_erase_count = 0;
ULONG                   erase_count_threshold = 0;
UINT                    status;


    /* Initialize the minimum erase count.  */
    min_block_erase_count =  LX_ALL_ONES;
    
    /* Initialize the system minimum and maximum erase counts.  */
    min_system_block_erase_count =  LX_ALL_ONES;
    max_system_block_erase_count =  0;
        
    /* Initialize the maximum obsolete page count.  */
    max_obsolete_pages =  0;
        
    /* Calculate the erase count threshold.  */
    if (nand_flash -> lx_nand_flash_free_pages >= nand_flash -> lx_nand_flash_pages_per_block)
    {

        /* Calculate erase count threshold by adding constant to the current minimum.  */
        erase_count_threshold =  nand_flash -> lx_nand_flash_minimum_erase_count + LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA;
    }
    else
    {
      
        /* When the number of free pages is low, simply pick the block that has the most number of obsolete sectors.  */
        erase_count_threshold =  LX_ALL_ONES;
    }
    
    /* Set the block word pointer to the internal buffer.  */
    block_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;
    
    /* Loop through the blocks to attempt to find the block to erase.  */
    for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
    {

        /* First, check to see if this block is good. */
        status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);
            
            /* Return the error.  */
            return(status);
        }

        /* Determine if this block is bad.  */
        if (block_status != LX_NAND_GOOD_BLOCK)
        {

            /* Block is bad, move to the next block.  */
            continue;        
        }

        /* Read page 0 of the block, the first word has the erase count.  */
        status =  _lx_nand_flash_driver_read(nand_flash, block, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);

            /* Determine if the error is fatal.  */
            if (status != LX_NAND_ERROR_CORRECTED)
            {
                            
                /* Return the error.  */
                return(status);
            }
        }

        /* Pickup the erase count from the first word of the block.  */
        erase_count =  block_word_ptr[0];

        /* Update the system minimum and maximum erase counts.  */
        if (erase_count < min_system_block_erase_count)
            min_system_block_erase_count =  erase_count;
        if (erase_count > max_system_block_erase_count)
            max_system_block_erase_count =  erase_count;

        /* Compute the number of obsolete and mapped pages for this block.  */
        obsolete_pages =  0;
        mapped_pages =    0;
        
        /* Loop through the pages for this block.  */
        for (page = 1; page < nand_flash -> lx_nand_flash_pages_per_block; page++)
        {
        
            /* Read the logical sector mapping for this page.  */
            status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, block, page, (UCHAR *) &extra_info, sizeof(extra_info));

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, page);

                /* Return the error.  */
                return(status);
            }
            
            /* Is this entry free?  */
            if (extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
            {
              
                /* Get out of the loop, we are done!  */
                break;
            }
            
            /* Is this entry obsolete?  */
            else if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_VALID) == 0)
            {
                
                /* Increment the number of obsolete pages.  */
                obsolete_pages++;    
            }
            else 
            {
                
                /* Increment the number of mapped pages.  */
                mapped_pages++;
            }
        }

        /* Determine if we have a block with a new maximum number of obsolete pages.  */
        if ((obsolete_pages > max_obsolete_pages) && (erase_count <= erase_count_threshold))
        {
        
            /* Update the new maximum obsolete pages and related information.  */
            max_obsolete_pages =        obsolete_pages;
            max_obsolete_block =        block;
            max_obsolete_erase_count =  erase_count;
            max_obsolete_mapped_count = mapped_pages;
        }
        else if ((max_obsolete_pages) && (obsolete_pages == max_obsolete_pages) && (erase_count <= erase_count_threshold))
        {
        
            /* Another block has the same number of obsolete pages.  Does this new block have a smaller erase
               count?  */
            if (erase_count < max_obsolete_erase_count)
            {
            
                /* Yes, choose the block with the smaller erase count.  */
                max_obsolete_pages =        obsolete_pages;
                max_obsolete_block =        block;
                max_obsolete_erase_count =  erase_count;
                max_obsolete_mapped_count = mapped_pages;
            }
        }
        
        /* Determine if we have a new minimum erase count.  */
        if (erase_count < min_block_erase_count)
        {
            
            /* Update the new minimum erase count and related information.  */
            min_block_erase_count =     erase_count;
            min_block_erase =           block;
            min_block_obsolete_count =  obsolete_pages;
            min_block_mapped_count =    mapped_pages;
        }
    }

    /* Determine if we can erase the block with the most obsolete pages.  */
    if (max_obsolete_pages)
    {
    
        /* Erase the block with the most obsolete pages.  */
        *return_erase_block =       max_obsolete_block;
        *return_erase_count =       max_obsolete_erase_count;
        *return_obsolete_pages =    max_obsolete_pages;
        *return_mapped_pages =      max_obsolete_mapped_count;
    }
    else
    {
      
        /* Otherwise, erase the block with the smallest erase count.  */
        *return_erase_block =       min_block_erase;
        *return_erase_count =       min_block_erase_count;
        *return_obsolete_pages =    min_block_obsolete_count;
        *return_mapped_pages =      min_block_mapped_count;
    }

    /* Update the overall minimum and maximum erase count.  */
    nand_flash -> lx_nand_flash_minimum_erase_count =  min_system_block_erase_count;
    nand_flash -> lx_nand_flash_maximum_erase_count =  max_system_block_erase_count;
    
    /* Return success.  */
    return(LX_SUCCESS);
}

