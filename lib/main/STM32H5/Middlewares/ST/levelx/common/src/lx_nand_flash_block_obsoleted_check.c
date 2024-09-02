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
/*    _lx_nand_flash_block_obsoleted_check                PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks to see if the specified block is completely    */ 
/*    obsoleted. If so, the block is reclaimed (erased) and made          */ 
/*    available.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number to check         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Driver block status           */ 
/*    _lx_nand_flash_driver_block_erase     Driver block erase            */ 
/*    _lx_nand_flash_driver_extra_bytes_get Driver get extra bytes        */ 
/*    _lx_nand_flash_driver_block_status_set                              */ 
/*                                          Driver set extra bytes        */ 
/*    _lx_nand_flash_driver_read            Driver page read              */ 
/*    _lx_nand_flash_driver_write           Driver page write             */ 
/*    _lx_nand_flash_block_reclaim          Reclaim one block             */ 
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
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            removed multiple write      */
/*                                            to page 0,                  */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _lx_nand_flash_block_obsoleted_check(LX_NAND_FLASH *nand_flash, ULONG block)
{

LX_NAND_PAGE_EXTRA_INFO             extra_info;
UCHAR                               block_status;
ULONG                               *page_word_ptr;
ULONG                               erase_count;
ULONG                               j;
UINT                                status;


    /* First, check to make sure this block is good.  */
    status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

    /* Check for an error from flash driver.   */
    if (status)
    {
        
        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Simply return.  */
        return;
    }

    /* Is this block bad?  */
    if (block_status != LX_NAND_GOOD_BLOCK)
    {
        
        /* Yes, this block is bad.  */

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Simply return.  */
        return;
    }

    /* Read the extra bytes of page 0. This will tell us if the page has any valid mappings.  */
    status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, block, 0, (UCHAR *) &extra_info, sizeof(extra_info));    
        
    /* Check for an error from flash driver.   */
    if (status)
    {
        
        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);
            
        /* Simply return.  */
        return;
    }
        
    /* Determine if the block is full, since it must be full first before it can be
       completely obsoleted.  */
    if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_BLOCK_FULL) != 0)
    {
        
        /* This block is not full, therefore it cannot be obsoleted.  */
        
        /* Simply return.  */
        return;
    }

    /* Setup pointer to internal buffer.  */
    page_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

    /* Now read page 0 of the block, which has the erase count in the first 4 bytes. */
    status =  _lx_nand_flash_driver_read(nand_flash, block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
    /* Check for an error from flash driver.   */
    if (status)
    {
        
        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Determine if the error is fatal.  */
        if (status != LX_NAND_ERROR_CORRECTED)
        {
                            
            /* Simply return.  */
            return;
        }
    }

    /* Determine if we have a valid logical sector mapping list.  */
    if ((page_word_ptr[1] != LX_NAND_PAGE_FREE) &&
        (page_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] == LX_NAND_PAGE_LIST_VALID))
    {
        
        /* Yes, we have a valid logical sector mapping list in page 0.  */
                      
        /* Traverse the list to look for the logical sector.  */
        for (j = 1; j < nand_flash -> lx_nand_flash_pages_per_block; j++)
        {
            
            /* Is this entry valid?  */
            if (page_word_ptr[j] & LX_NAND_PAGE_VALID)
            {
                
                /* Read in the actual page entry to make sure this logical sector mapping is still valid.  */
                status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, block, j, (UCHAR *) &extra_info, sizeof(extra_info));

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, block, j);

                    /* Simply return.  */
                    return;
                }

                /* Determine if the actual page is still valid. */
                if (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_VALID)
                {

                    /* If so, we found a valid page so the block isn't completely obsoleted.  */
                    
                    /* Simply return.  */
                    return;
                }
            }
        }

        /* Pickup the erase count.  */
        erase_count =  page_word_ptr[0];

        /* Determine if we need to reclaim blocks to balance out the erase counts.  */
        if ((erase_count == nand_flash -> lx_nand_flash_maximum_erase_count) &&
            ((nand_flash -> lx_nand_flash_maximum_erase_count - nand_flash -> lx_nand_flash_minimum_erase_count) >= LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA))
        {

            /* Attempt to reclaim one block.  */
            _lx_nand_flash_block_reclaim(nand_flash);   
        }
        else
        {

            /* At this point we can reclaim the block.  */

            /* Write the erased started indication.  */            
            page_word_ptr[0] =  LX_BLOCK_ERASE_STARTED;
            status =  _lx_nand_flash_driver_write(nand_flash, block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

            /* Check for an error from flash driver.   */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);
            }

            /* Erase the entire block.  */
            status =  _lx_nand_flash_driver_block_erase(nand_flash, block, erase_count+1);

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);

                /* Attempt to mark this block as bad.  */
                status =  _lx_nand_flash_driver_block_status_set(nand_flash, block, LX_NAND_BAD_BLOCK);

                /* Check for error in setting the block status.  */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, block, 0);
                }
            
                /* Increment the bad block count.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
        
                /* Update number of obsolete pages but not free pages since this block is now bad.  */
                nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - (nand_flash -> lx_nand_flash_pages_per_block - 1);
            }
            else
            {
        
                /* Increment the erase count.  */
                erase_count++;            
                
                /* Determine if the new erase count exceeds the maximum.  */
                if (erase_count > ((ULONG) LX_BLOCK_ERASE_COUNT_MAX))
                {
                
                    /* Yes, erase count is in overflow. Stay at the maximum count.  */
                    erase_count =  ((ULONG) LX_BLOCK_ERASE_COUNT_MAX);
                }

                /* Determine if we need to update the maximum erase count.  */
                if (erase_count > nand_flash -> lx_nand_flash_maximum_erase_count)
                {
        
                    /* Yes, a new maximum is present.  */
                    nand_flash -> lx_nand_flash_maximum_erase_count =  erase_count;
                
                    /* Determine if the search pointer is on this block.  */
                    if (nand_flash -> lx_nand_flash_free_block_search == block)
                    {
                
                        /* Move to next block.  */
                        nand_flash -> lx_nand_flash_free_block_search++;
                    
                        /* Check for wrap condition.  */
                        if (nand_flash -> lx_nand_flash_free_block_search >= nand_flash -> lx_nand_flash_total_blocks)
                        {
                        
                            /* Reset to the beginning block.  */
                            nand_flash -> lx_nand_flash_free_block_search =  0;
                        }
                    }
                }
        
                /* Set the buffer to all ones.   */
                for (j = 0; j < nand_flash -> lx_nand_flash_words_per_page; j++)
                {
        
                    /* Set word to all ones.  */
                    page_word_ptr[j] =  LX_ALL_ONES;
                }

                /* Now store the erase count.  */
                page_word_ptr[0] =  (erase_count);
        
                /* Write the erase count for the block.  */
                status =  _lx_nand_flash_driver_write(nand_flash, block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, block, 0);

                    /* Attempt to mark this block as bad.  */
                    status =  _lx_nand_flash_driver_block_status_set(nand_flash, block, LX_NAND_BAD_BLOCK);

                    /* Check for error in setting the block status.  */
                    if (status)
                    {
        
                        /* Call system error handler.  */
                        _lx_nand_flash_system_error(nand_flash, status, block, 0);

                        /* Simply return.  */
                        return;
                    }
            
                    /* Increment the bad block count.  */
                    nand_flash -> lx_nand_flash_bad_blocks++;
        
                    /* Update number of obsolete pages but not free pages since this block is now bad.  */
                    nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - (nand_flash -> lx_nand_flash_pages_per_block - 1);
               }
               else
               {

                    /* Update parameters of this flash.  */
                    nand_flash -> lx_nand_flash_free_pages =      nand_flash -> lx_nand_flash_free_pages + (nand_flash -> lx_nand_flash_pages_per_block - 1);
                    nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - (nand_flash -> lx_nand_flash_pages_per_block - 1);
                }
            }
        }
    }
}

