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
/*    _lx_nand_flash_sector_write                         PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes a logical sector to the NAND flash page.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    buffer                                Pointer to buffer to write    */ 
/*                                            (the size is number of      */ 
/*                                             bytes in a page)           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_read            Driver flash sector read      */ 
/*    _lx_nand_flash_driver_write           Driver flash sector write     */ 
/*    _lx_nand_flash_driver_extra_bytes_get Get extra bytes from spare    */ 
/*    _lx_nand_flash_driver_extra_bytes_set Set extra bytes in spare      */ 
/*    _lx_nand_flash_block_full_update      Update page 0 with list of    */ 
/*                                            mapped pages                */ 
/*    _lx_nand_flash_block_obsoleted_check  Check for block obsoleted     */ 
/*    _lx_nand_flash_block_reclaim          Reclaim one flash block       */ 
/*    _lx_nand_flash_logical_sector_find    Find logical sector           */ 
/*    _lx_nand_flash_physical_page_allocate Allocate new page             */ 
/*    _lx_nand_flash_sector_mapping_cache_invalidate                      */ 
/*                                          Invalidate cache entry        */ 
/*    _lx_nand_flash_system_error           Internal system error handler */ 
/*    tx_mutex_get                          Get thread protection         */ 
/*    tx_mutex_put                          Release thread protection     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
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
UINT  _lx_nand_flash_sector_write(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer)
{

LX_NAND_PAGE_EXTRA_INFO             old_extra_info;
LX_NAND_PAGE_EXTRA_INFO             new_extra_info;
ULONG                               old_block;
ULONG                               old_page;
ULONG                               new_block;
ULONG                               new_page;
ULONG                               new_erase_count;
ULONG                               i;
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
LX_NAND_SECTOR_MAPPING_CACHE_ENTRY  *sector_mapping_cache_entry_ptr;
#endif
UINT                                status;
ULONG                               *block_word_ptr;


#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Determine if there are less than two block's worth of free pages.  */
    i =  0;
    while (nand_flash -> lx_nand_flash_free_pages <= nand_flash -> lx_nand_flash_pages_per_block)
    {
     
        /* Attempt to reclaim one block.  */
        _lx_nand_flash_block_reclaim(nand_flash);

        /* Increment the block count.  */
        i++;

        /* Have we exceeded the number of blocks in the system?  */
        if (i >= nand_flash -> lx_nand_flash_total_blocks)
        { 
          
            /* Yes, break out of the loop.  */
            break;
        }
    }

    /* Increment the number of write requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_write_requests++;

    /* See if we can find the logical sector in the current mapping.  */
    _lx_nand_flash_logical_sector_find(nand_flash, logical_sector, LX_FALSE, &old_block, &old_page);
    
    /* Allocate a new page for this write.  */
    _lx_nand_flash_physical_page_allocate(nand_flash, &new_block, &new_page, &new_erase_count);

    /* Determine if the new page allocation was successful.  */
    if (new_page)
    {
    
        /* Yes, we were able to allocate a new page.  */

        /* Determine if this is the new maximum mapped sector.  */
        if (logical_sector > nand_flash -> lx_nand_flash_max_mapped_sector)
        {
            
            /* Remember this maximum mapped sector.  */
            nand_flash -> lx_nand_flash_max_mapped_sector =  logical_sector;
        }

        /* Update the number of free pages.  */
        nand_flash -> lx_nand_flash_free_pages--;

        /* Write the logical sector data to the new page.  */
        status =  _lx_nand_flash_driver_write(nand_flash, new_block, new_page, buffer, nand_flash -> lx_nand_flash_words_per_page);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Determine if there was an old mapping.  */
        if (old_page)
        {

            /* Read the extra info of the old page.  */
            status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, old_block, old_page, (UCHAR *) &old_extra_info, sizeof(old_extra_info));    
        
            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, old_block, old_page);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }
        
            /* Clear bit 30, which indicates this sector is superceded.  */
            old_extra_info.lx_nand_page_extra_info_logical_sector =  old_extra_info.lx_nand_page_extra_info_logical_sector & (~((ULONG) LX_NAND_PAGE_SUPERCEDED));

            /* Write the extra info to old page.  */
            status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, old_block, old_page, (UCHAR *) &old_extra_info, sizeof(old_extra_info));    
        
            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, old_block, old_page);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }
        }

        /* Now build the new mapping entry  - with the not valid bit set initially.  */
        new_extra_info.lx_nand_page_extra_info_logical_sector =  ((ULONG) LX_NAND_PAGE_VALID) | ((ULONG) LX_NAND_PAGE_SUPERCEDED) |  ((ULONG) LX_NAND_PAGE_MAPPING_NOT_VALID) | logical_sector;
            
        /* Write out the new mapping entry.  */
        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, new_block, new_page, (UCHAR *) &new_extra_info, sizeof(new_extra_info));    

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Now clear the not valid bit to make this sector mapping valid.  This is done because the writing of the extra bytes itself can 
           be interrupted and we need to make sure this can be detected when the flash is opened again.  */
        new_extra_info.lx_nand_page_extra_info_logical_sector =  new_extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_MAPPING_NOT_VALID);
            
        /* Clear the not valid bit.  */
        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, new_block, new_page, (UCHAR *) &new_extra_info, sizeof(new_extra_info));    

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, new_block, new_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }


        /* Increment the number of mapped sectors.  */
        nand_flash -> lx_nand_flash_mapped_pages++;
        
        /* Was there a previously mapped sector?  */
        if (old_page)
        {
        
            /* Now clear bit 31, which indicates this sector is now obsoleted.  */
            old_extra_info.lx_nand_page_extra_info_logical_sector =  old_extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_VALID);
            
            /* Write the value back to the flash to clear bit 31.  */
            status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, old_block, old_page, (UCHAR *) &old_extra_info, sizeof(old_extra_info));    
            
            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, old_block, old_page);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Setup pointer to internal buffer.  */
            block_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

            /* Now read page 0 of the block, which has the erase count in the first 4 bytes. */
            status =  _lx_nand_flash_driver_read(nand_flash, old_block, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
            
            /* Check for an error from flash driver.   */
            if (status)
            {
            
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, old_block, 0);

                /* Determine if the error is fatal.  */
                if (status != LX_NAND_ERROR_CORRECTED)
                {
                                
#ifdef LX_THREAD_SAFE_ENABLE

                    /* Release the thread safe mutex.  */
                    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                    /* Return the error.  */
                    return(status);
                }
            }

#ifndef LX_NAND_FLASH_MAPPING_LIST_UPDATE_DISABLE

            /* Determine if there is a valid mapping list.  */
            if ((block_word_ptr[1] != LX_NAND_PAGE_FREE) &&
                (block_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] == LX_NAND_PAGE_LIST_VALID))
            {

                /* Mark the entry as invalid.  */
                block_word_ptr[old_page] &= ~LX_NAND_PAGE_VALID;

                /* Invalidate the page.  */
                status =  _lx_nand_flash_driver_write(nand_flash, old_block, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
            }
#endif

            /* Increment the number of obsolete pages.  */
            nand_flash -> lx_nand_flash_obsolete_pages++;

            /* Decrement the number of mapped pages.  */
            nand_flash -> lx_nand_flash_mapped_pages--;
            
            /* Invalidate the old sector mapping cache entry.  */
            _lx_nand_flash_sector_mapping_cache_invalidate(nand_flash, logical_sector);
            
            /* Call routine to see if this block is completely obsoleted.  If so, 
               we can reclaim it immediately.  */
            _lx_nand_flash_block_obsoleted_check(nand_flash, old_block);
        }

        /* Read the extra info of the old page.  */
        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, new_block, new_page, (UCHAR *) &new_extra_info, sizeof(new_extra_info));    

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, old_block, old_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Check to see if the logical sector is still mapped to the same block/page.  */
        if (((new_extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) == logical_sector) &&
            (new_extra_info.lx_nand_page_extra_info_logical_sector & ((ULONG) LX_NAND_PAGE_VALID)))
        {

            /* Determine if the new page is the last page of the block.  */
            if (new_page == (nand_flash -> lx_nand_flash_pages_per_block - 1))
            {
        
            
                /* Yes, we need to update page 0 of the block with the list of mapped 
                   pages for this block.  */
                _lx_nand_flash_block_full_update(nand_flash, new_block, new_erase_count);
            }

            /* Determine if the logical sector mapping cache is enabled.  */
            if (nand_flash -> lx_nand_flash_sector_mapping_cache_enabled)
            {
        
                /* Yes, sector mapping cache is enabled, place this logical sector mapping information in the cache.  */

#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
                
                /* Calculate the starting index of the sector mapping cache for this sector entry.  */
                i =  (logical_sector & LX_NAND_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NAND_SECTOR_MAPPING_CACHE_DEPTH;

                /* Build a pointer to the cache entry.  */
                sector_mapping_cache_entry_ptr =  &nand_flash -> lx_nand_flash_sector_mapping_cache[i];

                /* Move all the cache entries down so the oldest is at the bottom.  */
                *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
                *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
                *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr);
           
                /* Setup the new logical sector information in the cache.  */
                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_logical_sector =  (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID);
                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_block =           (USHORT) new_block;
                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_page =            (USHORT) new_page;
#else

                /* Determine if this logical sector fits in the logical sector cache mapping.  */
                if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                {

                    /* Yes, store the logical sector to block/page mapping in the cache.  */
                    nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) new_block;
                    nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) new_page;
                }
#endif
            }
        }

        /* Indicate the write was successful.  */
        status =  LX_SUCCESS;        
    }
    else
    {
    
        /* Indicate the write was unsuccessful.  */
        status =  LX_NO_SECTORS;
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return the completion status.  */
    return(status);
}


