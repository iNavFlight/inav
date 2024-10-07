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
/*    _lx_nand_flash_block_reclaim                        PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reclaims one block from the NAND flash.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_block_erase     Driver erase block            */ 
/*    _lx_nand_flash_driver_block_status_set                              */ 
/*                                          Set block status to bad       */ 
/*    _lx_nand_flash_driver_extra_bytes_get NAND flash get extra bytes    */ 
/*    _lx_nand_flash_driver_extra_bytes_set NAND flash set extra bytes    */ 
/*    _lx_nand_flash_driver_write           Driver flash page write       */ 
/*    _lx_nand_flash_driver_read            Driver flash page read        */ 
/*    _lx_nand_flash_block_full_update      Update page 0 with list of    */ 
/*                                            mapped pages                */ 
/*    _lx_nand_flash_next_block_to_erase_find                             */ 
/*                                          Find next block to erase      */ 
/*    _lx_nand_flash_physical_page_allocate Allocate new page             */ 
/*    _lx_nand_flash_sector_mapping_cache_invalidate                      */ 
/*                                          Invalidate cache entry        */ 
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
/*  08-02-2021     Bhupendra Naphade        Modified comment(s), updated  */
/*                                            obselete page count check,  */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            removed multiple write      */
/*                                            to page 0,                  */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_block_reclaim(LX_NAND_FLASH *nand_flash)
{

LX_NAND_PAGE_EXTRA_INFO old_extra_info;
LX_NAND_PAGE_EXTRA_INFO new_extra_info;
ULONG                   *page_word_ptr;
ULONG                   i;
ULONG                   erase_block;
ULONG                   erase_count;
ULONG                   mapped_pages;
ULONG                   obsolete_pages;
ULONG                   free_pages;
ULONG                   logical_sector;
ULONG                   new_block;
ULONG                   new_page;
ULONG                   new_erase_count;
UINT                    status;


    /* Increment the reclaim attempts counter.  */
    nand_flash -> lx_nand_flash_diagnostic_block_reclaim_attempts++;

    /* Determine the next block to erase.  */
    _lx_nand_flash_next_block_to_erase_find(nand_flash, &erase_block, &erase_count, &mapped_pages, &obsolete_pages);

    /* Setup pointer to page memory.  */
    page_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

    /* Determine if the search pointer is set for this block.  */
    if (nand_flash -> lx_nand_flash_free_block_search == erase_block)
    {
    
        /* Ensure the search block is not the block we are trying to free.  */
        nand_flash -> lx_nand_flash_free_block_search =  erase_block + 1;
     
        /* Check for wrap condition.  */
        if (nand_flash -> lx_nand_flash_free_block_search >= nand_flash -> lx_nand_flash_total_blocks)
            nand_flash -> lx_nand_flash_free_block_search =  0;
    }

    /* Determine if this block is completely obsolete.  */
    if (obsolete_pages == nand_flash -> lx_nand_flash_pages_per_block - 1)
    {

        /* Read page 0 of the block, which has the erase count in the first 4 bytes. */
        status =  _lx_nand_flash_driver_read(nand_flash, erase_block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
        }

        /* Write the erased started indication.  */            
        page_word_ptr[0] =  LX_BLOCK_ERASE_STARTED;
        status =  _lx_nand_flash_driver_write(nand_flash, erase_block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

        /* Check for an error from flash driver.   */
        if (status)
        {

            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
        }
       
        /* Erase the entire block.  */
        status =  _lx_nand_flash_driver_block_erase(nand_flash, erase_block, erase_count+1);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
            
            /* Attempt to mark this block as bad.  */
            status =  _lx_nand_flash_driver_block_status_set(nand_flash, erase_block, LX_NAND_BAD_BLOCK);

            /* Check for error in setting the block status.  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
            }
            
            /* Increment the bad block count.  */
            nand_flash -> lx_nand_flash_bad_blocks++;
        
            /* Update number of obsolete pages but not free pages since this block is now bad.  */
            nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
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
            }

            /* Set the buffer to all ones.   */
            for (i = 0; i < nand_flash -> lx_nand_flash_words_per_page; i++)
            {
        
                /* Set word to all ones.  */
                page_word_ptr[i] =  LX_ALL_ONES;
            }

            /* Now store the erase count.  */
            page_word_ptr[0] =  (erase_count);
        
            /* Write the erase count for the block.  */
            status =  _lx_nand_flash_driver_write(nand_flash, erase_block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);

                /* Attempt to mark this block as bad.  */
                status =  _lx_nand_flash_driver_block_status_set(nand_flash, erase_block, LX_NAND_BAD_BLOCK);

                /* Check for error in setting the block status.  */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
                }
            
                /* Increment the bad block count.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
        
                /* Update number of obsolete pages but not free pages since this block is now bad.  */
                nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
            }
            else
            {
        
                /* Update parameters of this flash.  */
                nand_flash -> lx_nand_flash_free_pages =      nand_flash -> lx_nand_flash_free_pages + obsolete_pages;
                nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
            }
        }
    }
    else 
    {
    
        /* Calculate the number of free pages in this block.  */
        free_pages =  nand_flash -> lx_nand_flash_pages_per_block - (obsolete_pages + mapped_pages);
        
        /* Determine if there are enough free pages outside of this block to reclaim this block.  */
        if (mapped_pages <= (nand_flash -> lx_nand_flash_free_pages - free_pages))
        {
        
            /* Now search through the list to find mapped logical sectors to move.  */
            for (i = 1; i < nand_flash -> lx_nand_flash_pages_per_block; i++)
            {

                /* Read the logical sector mapping for this page.  */
                status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, erase_block, i, (UCHAR *) &old_extra_info, sizeof(old_extra_info));

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, erase_block, i);
                    
                    /* Return the error.  */
                    return(status);
                }

                /* Determine if the entry hasn't been used.  */
                if (old_extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
                {
                
                    /* Since allocations are done sequentially in the block, we know nothing
                       else exists after this point.  */
                    break;
                }
            
                /* Is this entry mapped?  */
                if (old_extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_VALID)
                {
                
                    /* Pickup the logical sector associated with this mapped page.  */
                    logical_sector =  old_extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK;

                    /* Invalidate the old logical sector mapping cache entry.  */
                    _lx_nand_flash_sector_mapping_cache_invalidate(nand_flash, logical_sector);
                      
                    /* Allocate a new page for this write.  */
                    _lx_nand_flash_physical_page_allocate(nand_flash, &new_block, &new_page, &new_erase_count);

                    /* Check to see if the new block is the same as the block we are trying to reclaim.  */
                    if (new_block == erase_block)
                    {

                        /* Yes, the new page was found in the block to be erased. Simply move the search pointer
                           to the block after the erase block and search for another page from there.  */
                        nand_flash -> lx_nand_flash_free_block_search =  erase_block + 1;

                        /* Check for wrap condition.  */
                        if (nand_flash -> lx_nand_flash_free_block_search >= nand_flash -> lx_nand_flash_total_blocks)
                            nand_flash -> lx_nand_flash_free_block_search =  0;

                        /* Allocate new page again. */
                        _lx_nand_flash_physical_page_allocate(nand_flash, &new_block, &new_page, &new_erase_count);

                        /* Check again for the new page inside of the block to erase. This should be impossible, since
                           we check previously if there are enough free pages outside of this block needed to reclaim
                           this block.  */
                        if (new_block == erase_block)
                        {

                            /* System error, a new page is not available outside of the erase block. 
                               Clear the new page so we fall through to the error handling. */
                            new_page = 0;
                        }
                    }
                    
                    /* Determine if the new page allocation was successful.  */
                    if (new_page)
                    {
    
                        /* Yes, we were able to allocate a new page for the logical sector.  */

                        /* Read the existing page into the internal buffer.  */
                        status =  _lx_nand_flash_driver_read(nand_flash, erase_block, i, page_word_ptr, nand_flash -> lx_nand_flash_words_per_page);
        
                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, erase_block, i);
                            
                            /* Determine if the error is fatal.  */
                            if (status != LX_NAND_ERROR_CORRECTED)
                            {
                            
                                /* Return the error.  */
                                return(status);
                            }
                        }

                        /* Write the logical sector data to the new page.  */
                        status =  _lx_nand_flash_driver_write(nand_flash, new_block, new_page, page_word_ptr, nand_flash -> lx_nand_flash_words_per_page);

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Now deprecate the old logical sector mapping.  */
            
                        /* Clear bit 30, which indicates this page is being superceded.  */
                        old_extra_info.lx_nand_page_extra_info_logical_sector =  old_extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_SUPERCEDED);
            
                        /* Write the value back to the flash to clear bit 30.  */
                        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, erase_block, i, (UCHAR *) &old_extra_info, sizeof(old_extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, erase_block, i);

                            /* Return the error.  */
                            return(status);
                        }
                
                        /* Now build the new mapping entry - with the not valid bit set initially.  */
                        new_extra_info.lx_nand_page_extra_info_logical_sector =  ((ULONG) LX_NAND_PAGE_VALID) | ((ULONG) LX_NAND_PAGE_SUPERCEDED) | ((ULONG) LX_NAND_PAGE_MAPPING_NOT_VALID) | logical_sector;
            
                        /* Write out the new mapping entry.  */
                        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, new_block, new_page, (UCHAR *) &new_extra_info, sizeof(new_extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Now clear the not valid bit to make this sector mapping valid.  This is done because the writing of the extra bytes itself can 
                           be interrupted and we need to make sure this can be detected when the flash is opened again.  */
                        new_extra_info.lx_nand_page_extra_info_logical_sector =   new_extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_MAPPING_NOT_VALID);
            
                        /* Clear the not valid bit.  */
                        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, new_block, new_page, (UCHAR *) &new_extra_info, sizeof(new_extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

                            /* Return the error.  */
                            return(status);
                        }

#ifdef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                        /* Determine if this logical sector fits in the logical sector direct cache mapping.  */
                        if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                        {

                            /* Remember the mapping for this logical sector.  */
                            nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) new_block;
                            nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) new_page;
                        }
#endif

                        /* Now clear bit 31, which indicates this sector is now obsoleted.  */
                        old_extra_info.lx_nand_page_extra_info_logical_sector =  old_extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_VALID);
            
                        /* Write the value back to the flash to clear bit 31.  */
                        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, erase_block, i, (UCHAR *) &old_extra_info, sizeof(old_extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, erase_block, i);

                            /* Return the error.  */
                            return(status);
                        }                       

                        /* Increment the number of moved pages.  */
                        nand_flash -> lx_nand_flash_diagnostic_moved_pages++;
                        
                        /* Determine if the new page is the last page of the block.  */
                        if (new_page == (nand_flash -> lx_nand_flash_pages_per_block - 1))
                        {
                   
                            /* Yes, we need to update page 0 of the block with the list of mapped 
                               pages for this block.  */
                            _lx_nand_flash_block_full_update(nand_flash, new_block, new_erase_count);
                        }                       
                    }
                    else
                    {

                        /* Call system error handler - the allocation should always succeed at this point.  */
                        _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_ALLOCATION_FAILED, erase_block, i);

                        /* System error... break!  */
                        break;
                    }
                        
                    /* Decrement the number of mapped pages.  */
                    mapped_pages--;
                            
                    /* Determine if we are done.  */
                    if (mapped_pages == 0)
                        break;
               }
            }

            /* Read page 0 of the block, which has the erase count in the first 4 bytes. */
            status =  _lx_nand_flash_driver_read(nand_flash, erase_block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
            }

            /* Write the erased started indication.  */            
            page_word_ptr[0] =  LX_BLOCK_ERASE_STARTED;
            status =  _lx_nand_flash_driver_write(nand_flash, erase_block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

            /* Check for an error from flash driver.   */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
            }
        
            /* Erase the entire block.  */
            status =  _lx_nand_flash_driver_block_erase(nand_flash, erase_block, erase_count+1);

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);

                /* Attempt to mark this block as bad.  */
                status =  _lx_nand_flash_driver_block_status_set(nand_flash, erase_block, LX_NAND_BAD_BLOCK);

                /* Check for error in setting the block status.  */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);
                }
            
                /* Increment the bad block count.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
        
                /* Update number of obsolete pages but not free pages since this block is now bad.  */
                nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
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
                }
        
                /* Set the buffer to all ones.   */
                for (i = 0; i < nand_flash -> lx_nand_flash_words_per_page; i++)
                {
        
                    /* Set word to all ones.  */
                    page_word_ptr[i] =  LX_ALL_ONES;
                }

                /* Now store the erase count.  */
                page_word_ptr[0] =  (erase_count);
        
                /* Write the erase count for the block.  */
                status =  _lx_nand_flash_driver_write(nand_flash, erase_block, 0, page_word_ptr, LX_NAND_ERASE_COUNT_WRITE_SIZE);

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);

                    /* Attempt to mark this block as bad.  */
                    status =  _lx_nand_flash_driver_block_status_set(nand_flash, erase_block, LX_NAND_BAD_BLOCK);

                    /* Check for error in setting the block status.  */
                    if (status)
                    {
        
                        /* Call system error handler.  */
                        _lx_nand_flash_system_error(nand_flash, status, erase_block, 0);

                        /* Return the error.  */
                        return(status);
                    }
            
                    /* Increment the bad block count.  */
                    nand_flash -> lx_nand_flash_bad_blocks++;
        
                    /* Update number of obsolete pages but not free pages since this block is now bad.  */
                    nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
                }
                else
                {

                    /* Update parameters of this flash.  */
                    nand_flash -> lx_nand_flash_free_pages =      nand_flash -> lx_nand_flash_free_pages + obsolete_pages;
                    nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages - obsolete_pages;
                }
            }
        }
    }

    /* Return status.  */
    return(LX_SUCCESS);
}

