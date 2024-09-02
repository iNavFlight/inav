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
/*    _lx_nand_flash_logical_sector_find                  PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find the specified logical sector in      */ 
/*    the NAND flash.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    superceded_check                      Check for page being          */ 
/*                                            superceded (can happen if   */ 
/*                                            on interruptions of page    */ 
/*                                            write)                      */ 
/*    block                                 Destination for block         */ 
/*    page                                  Destination for page          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Driver block status           */ 
/*    _lx_nand_flash_driver_extra_bytes_get Driver get extra bytes        */ 
/*    _lx_nand_flash_driver_extra_bytes_set NAND flash set extra bytes    */ 
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
UINT  _lx_nand_flash_logical_sector_find(LX_NAND_FLASH *nand_flash, ULONG logical_sector, ULONG superceded_check, ULONG *block, ULONG *page)
{

LX_NAND_PAGE_EXTRA_INFO             extra_info;
UCHAR                               block_status;
ULONG                               *block_word_ptr;
ULONG                               valid_pages;
ULONG                               mapped_pages;
ULONG                               total_blocks;
ULONG                               total_pages;
ULONG                               i, j;
ULONG                               search_start;
ULONG                               max_mapped_sector;
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
LX_NAND_SECTOR_MAPPING_CACHE_ENTRY  *sector_mapping_cache_entry_ptr = LX_NULL;
LX_NAND_SECTOR_MAPPING_CACHE_ENTRY  temp_sector_mapping_cache_entry;
#endif
UINT                                status;


    /* Initialize the return parameters.  */
    *block =  (ULONG) 0;
    *page =   (ULONG) 0;
    
    /* Determine if there are any mapped pages.  */
    if (nand_flash -> lx_nand_flash_mapped_pages == 0)
    {

        /* No mapped pages so nothing can be found!.  */
        return(LX_SECTOR_NOT_FOUND);
    }

    /* Determine if this logical sector is greater than what has previously been mapped.  */
    if (logical_sector > nand_flash -> lx_nand_flash_max_mapped_sector)
    {
    
        /* Since this logical sector request is greater than what has been mapped previously, 
           there is no point in searching any further.  */

        /* Return sector not found status.  */
        return(LX_SECTOR_NOT_FOUND);  
    }

    /* Determine if the sector mapping cache is enabled.  */
    if (nand_flash -> lx_nand_flash_sector_mapping_cache_enabled)
    {
    
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

        /* Calculate the starting index of the sector cache for this sector entry.  */
        i =  (logical_sector & LX_NAND_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NAND_SECTOR_MAPPING_CACHE_DEPTH;

        /* Build a pointer to the cache entry.  */
        sector_mapping_cache_entry_ptr =  &nand_flash -> lx_nand_flash_sector_mapping_cache[i];

        /* Determine if the sector is in the sector mapping cache - assuming the depth of the sector 
           mapping cache is LX_NAND_SECTOR_MAPPING_CACHE_DEPTH entries.  */
        if ((sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_hits++;

            /* Yes, return the cached values associated with the sector.  */
            *block =  (ULONG) sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_block;
            *page =   (ULONG) sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_page;

            /* Don't move anything since we found the entry at the top.  */

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 1) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_hits++;

            /* Yes, return the cached values associated with the sector.  */
            *block =   (ULONG) (sector_mapping_cache_entry_ptr + 1) -> lx_nand_sector_mapping_cache_block;
            *page =    (ULONG) (sector_mapping_cache_entry_ptr + 1) -> lx_nand_sector_mapping_cache_page;

            /* Just swap the first and second entry.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 2) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_hits++;

            /* Yes, return the cached value.  */
            *block =   (ULONG) (sector_mapping_cache_entry_ptr + 2) -> lx_nand_sector_mapping_cache_block;
            *page =    (ULONG) (sector_mapping_cache_entry_ptr + 2) -> lx_nand_sector_mapping_cache_page;

            /* Move the third entry to the top and the first two entries down.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_hits++;

            /* Yes, return the cached value.  */
            *block =  (ULONG) (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_block;
            *page =   (ULONG) (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_page;

            /* Move the last entry to the top and the first three entries down.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 3);
            *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }

        /* If we get here, we have a cache miss so increment the counter before we fall through to the search loop.  */
        nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_misses++;
#else

        /* Direct mapping cache is defined.  */
        
        /* See if we are still opening the flash.  */
        if (nand_flash -> lx_nand_flash_state == LX_NAND_FLASH_OPENED)
        {

            /* Flash instance is opened.  */
          
            /* Determine if this logical sector fits in the logical sector direct cache mapping.  */
            if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
            {

                /* Yes, invalidate the logical sector cache.  */
                if ((nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block == 0) &&
                    (nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page == 0))
                {
            
                    /* Entry is not mapped.  */

                    /* No, return sector not found status.  */
                    return(LX_SECTOR_NOT_FOUND);  
                }
                else
                {
            
                    /* Sector is mapped, return the block and page of the mapping.  */
                    *block =  nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block;
                    *page =   nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page;

                    /* Return a successful status.  */
                    return(LX_SUCCESS);
                }
            }
        }
        else
        {
        
            /* If we get here, we have a cache miss so increment the counter before we fall through to the search loop.  */
            nand_flash -> lx_nand_flash_diagnostic_sector_mapping_cache_misses++;
        }
#endif
    }
   
    /* Clear the maximum mapped sector.  */
    max_mapped_sector =  0;

    /* Setup the total number of mapped pages.  */
    mapped_pages =  nand_flash -> lx_nand_flash_mapped_pages;

    /* Setup pointer to internal buffer.  */
    block_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

    /* Start searching from the last found block.  */
    i =  nand_flash -> lx_nand_flash_found_block_search;

    /* Setup the starting page to look at.  */
    j =  nand_flash -> lx_nand_flash_found_page_search;

    /* Pickup the total number of blocks.  */
    total_blocks =  nand_flash -> lx_nand_flash_total_blocks;

    /* Loop through the blocks to attempt to find the mapped logical sector.  */
    while (total_blocks--) 
    {

        /* First, check to make sure this block is good.  */
        status =  _lx_nand_flash_driver_block_status_get(nand_flash, i, &block_status);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, i, 0);

            /* Return the error.  */
            return(status);
        }

        /* Is this block bad?  */
        if (block_status != LX_NAND_GOOD_BLOCK)
        {
        
            /* Yes, this block is bad.  */
            
            /* Move to the next block.  */
            i++;
            
            /* Check for wrap condition.  */
            if (i >= nand_flash -> lx_nand_flash_total_blocks)
            {
            
                /* Wrap to the first block.  */
                i =  0;
            }
            
            /* Start looking at the first page of the next block.  */
            j =  1;

            /* Continue to the next block.  */
            continue;
        }

        /* Read the extra bytes of page 0. This will tell us if the page has any valid mappings.  */
        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, i, 0, (UCHAR *) &extra_info, sizeof(extra_info));    
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, i, 0);
            
            /* Return the error... no point in continuing.  */
            return(status);
        }
        
        /* Determine if there are any valid mappings.  */
        if (((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_BLOCK_VALID) == 0) ||
            ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_BLOCK_EMPTY) != 0))
        {
        
            /* Yes, this block is either no longer valid or is empty, i.e., no mappings.  */
            
            /* Move to the next block.  */
            i++;
            
            /* Check for wrap condition.  */
            if (i >= nand_flash -> lx_nand_flash_total_blocks)
            {
            
                /* Wrap to the first block.  */
                i =  0;
            }

            /* Reset to the first page of the next block.  */
            j =  1;

            /* Continue to the next block.  */
            continue;
        }

        /* Now read page 0 of the block, which has the erase count in the first 4 bytes. */
        status =  _lx_nand_flash_driver_read(nand_flash, i, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, i, 0);

            /* Determine if the error is fatal.  */
            if (status != LX_NAND_ERROR_CORRECTED)
            {
                            
                /* Return the error.  */
                return(status);
            }
        }

        /* Determine if we have a valid logical sector mapping list.  */
        if ((block_word_ptr[1] != LX_NAND_PAGE_FREE) &&
            (block_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] == LX_NAND_PAGE_LIST_VALID))
        {
        
            /* Yes, we have a valid logical sector mapping list in page 0.  */
            
            /* Clear the valid entries counter.  */
            valid_pages =  0;
            
            /* Setup the total number of pages, less the metadata page.  */
            total_pages =  nand_flash -> lx_nand_flash_pages_per_block - 1;

            /* Now search through the sector list to find a match.  */
            while (total_pages--)
            {
            
                /* Is this entry valid?  */
                if ((block_word_ptr[j] & (LX_NAND_PAGE_VALID | LX_NAND_PAGE_MAPPING_NOT_VALID)) == LX_NAND_PAGE_VALID) 
                {
               
                    /* Increment the valid entries counter.  */
                    valid_pages++;

                    /* Determine if this is a new maximum logical sector.  */
                    if ((block_word_ptr[j] & LX_NAND_LOGICAL_SECTOR_MASK) > max_mapped_sector)
                    {
                    
                        /* Yes, a new maximum mapped sectors - remember it!  */
                        max_mapped_sector =  (block_word_ptr[j] & LX_NAND_LOGICAL_SECTOR_MASK);
                    }
                               
                    /* Do we have a valid sector match?  */
                    if ((block_word_ptr[j] & LX_NAND_LOGICAL_SECTOR_MASK) == logical_sector)
                    {
                    
                        /* Read in the actual page entry to make sure this logical sector mapping is still valid.  */
                        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, i, j, (UCHAR *) &extra_info, sizeof(extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, i, j);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Verify that the actual page still has a valid sector mapping.  */
                        if (((extra_info.lx_nand_page_extra_info_logical_sector & (LX_NAND_PAGE_VALID | LX_NAND_PAGE_MAPPING_NOT_VALID)) == LX_NAND_PAGE_VALID) &&  
                            ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) == logical_sector))
                        {

                            /* Decrement the number of mapped pages to examine.  */
                            mapped_pages--;

                            /* Determine if we care about the superceded bit.  */
                            if (superceded_check == LX_FALSE)
                            {
                                    
                                /* Prepare the return information.  */
                                *block =  i;
                                *page =   j;

                                /* Determine if the sector mapping cache is enabled.  */
                                if (nand_flash -> lx_nand_flash_sector_mapping_cache_enabled)
                                {

#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                                    /* Yes, update the cache with the logical sector mapping.  */
                            
                                    /* Move all the cache entries down so the oldest is at the bottom.  */
                                    *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
                                    *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
                                    *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr);

                                    /* Setup the new sector information in the cache.  */
                                    sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_logical_sector =  (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID);
                                    sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_block =  (USHORT) i;
                                    sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_page =   (USHORT) j;
#else

                                    /* Determine if this logical sector fits in the logical sector cache mapping.  */
                                    if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                                    {

                                        /* Yes, store the logical sector to block/page mapping in the cache.  */
                                        nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) i;
                                        nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) j;
                                    }
#endif
                                }

                                /* Move to the next page.  */
                                j++;
                                
                                /* Determine if the page has wrapped around.  */
                                if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                                {
                                
                                    /* Yes, page wrapped.  Move to page 1 of the next block.  */
                                    j =  1;
                                }

                                /* Remember the last found block and page for next search.  */
                                nand_flash -> lx_nand_flash_found_block_search =  i;
                                nand_flash -> lx_nand_flash_found_page_search =   j;

                                /* Return success!  */
                                return(LX_SUCCESS);                     
                            }

                            /* Check for the superceded bit being clear, which means the sector was superceded.  */
                            else if (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_SUPERCEDED)
                            {
                        
                                /* Prepare the return information.  */
                                *block =   i;
                                *page =    j;

                                /* No need to update the cache here, since this condition only happens during initialization.  */

                                /* Move to the next page.  */
                                j++;
                                
                                /* Determine if the page has wrapped around.  */
                                if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                                {
                                
                                    /* Yes, page wrapped.  Move to page 1 of the next block.  */
                                    j =  1;
                                }

                                /* Remember the last found block and page for next search.  */
                                nand_flash -> lx_nand_flash_found_block_search =  i;
                                nand_flash -> lx_nand_flash_found_page_search =   j;

                                /* Return success!  */
                                return(LX_SUCCESS);                     
                            }
                        }
                        else
                        {
                        
                            /* The entry is not really valid anymore, decrement the valid entries counter.  */
                            valid_pages--;
                        }
                    }
                }
                
                /* Move to the next page.  */
                j++;
                
                /* Check for wrap condition.  */
                if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                {
                
                    /* Yes, page has wrapped, reset to page 1.  */
                    j =  1;
                }
            }

            /* At this point we know that nothing matched in the mapping list. Now we need to check to see if there are any valid entries.  */
            if (valid_pages)
            {

                /* Reset the valid pages counter.  */
                valid_pages =  0;

                /* Setup the total number of pages, less the metadata page.  */
                total_pages =  nand_flash -> lx_nand_flash_pages_per_block - 1;

                /* Now search through the sector list to find a match.  */
                while (total_pages--)
                {
            
                    /* Is this entry valid?  */
                    if ((block_word_ptr[j] & (LX_NAND_PAGE_VALID | LX_NAND_PAGE_MAPPING_NOT_VALID)) == LX_NAND_PAGE_VALID)
                    {
                                  
                        /* Read in the actual page entry to make sure this logical sector mapping is still valid.  */
                        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, i, j, (UCHAR *) &extra_info, sizeof(extra_info));

                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, i, j);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Verify that the actual page still has a valid sector mapping.  */
                        if ((extra_info.lx_nand_page_extra_info_logical_sector & (LX_NAND_PAGE_VALID | LX_NAND_PAGE_MAPPING_NOT_VALID)) == LX_NAND_PAGE_VALID)
                        {
                        
                            /* Set the valid pages counter to 1 and break the loop.  */
                            valid_pages =  1;
                            
                            /* If one entry is valid, we can stop looking. */
                            break;
                        }
                    }

                    /* Move to the next page.  */
                    j++;
                
                    /* Check for wrap condition.  */
                    if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                    {
                
                        /* Yes, page has wrapped, reset to page 1.  */
                        j =  1;
                    }
                }                
            }
            
            /* Now, a final check for valid pages.  */
            if (valid_pages == 0)
            {
            
                /* There are no valid entries. We can now mark this block as not valid since there are no active 
                   mappings. This will prevent us from examining it again during the search process.  */
                extra_info.lx_nand_page_extra_info_logical_sector =  0;
                status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, i, 0, (UCHAR *) &extra_info, sizeof(extra_info));    

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, i, 0);
                }
                
                /* Reset the search page to 1.  */
                j =  1;
            }
        }
        else
        {
        
            /* No, we don't have a valid logical sector mapping list.   */
            
            /* Setup the total number of pages, less the metadata page.  */
            total_pages =  nand_flash -> lx_nand_flash_pages_per_block - 1;

            /* Remember the start of the search.  */
            search_start =  j;

            /* Now search through the sector list to find a match.  */
            while (total_pages--)
            {

                /* Read in the actual page entry to look for the logical sector mapping.  */
                status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, i, j, (UCHAR *) &extra_info, sizeof(extra_info));

                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, i, j);

                    /* Return the error.  */
                    return(status);
                }

                /* Is there a logical page mapping?  */
                if (extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
                {
                
                    /* Since the mapping is done sequentially in the block, we know nothing
                       else exists after this point.  */
              
                    /* Determine if the search started at the beginning of the block.  */
                    if (search_start == 1)
                    {
                 
                        /* Yes, we started at the beginning of the block.  We are now done with this block. */
                        break;
                    }
                    else
                    {
              
                        /* Setup the new total to the search start.  */
                        total_pages =  (search_start - 1);
                    
                        /* Clear search start.  */
                        search_start =  1;
                                
                        /* Start search over.  */
                        j =  1;
                        continue;
                    }
                }
                
                /* Is this page mapping valid?  */
                if ((extra_info.lx_nand_page_extra_info_logical_sector & (LX_NAND_PAGE_VALID | LX_NAND_PAGE_MAPPING_NOT_VALID)) == LX_NAND_PAGE_VALID)
                {
                
                    /* Yes, this page mapping is valid.  */
                    
                    /* Decrease the number of pages to examine.  */
                    mapped_pages--;


                    /* Determine if this is a new maximum logical sector.  */
                    if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) > max_mapped_sector)
                    {
                    
                        /* Yes, a new maximum mapped sectors - remember it!  */
                        max_mapped_sector =  (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK);
                    }

                    /* Do we have a valid sector match?  */
                    if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) == logical_sector)
                    {

                        /* Determine if we care about the superceded bit.  */
                        if (superceded_check == LX_FALSE)
                        {
                                    
                            /* Prepare the return information.  */
                            *block =  i;
                            *page =   j;

                            /* Determine if the sector mapping cache is enabled.  */
                            if (nand_flash -> lx_nand_flash_sector_mapping_cache_enabled)
                            {

                                /* Yes, update the cache with the logical sector mapping.  */
                          
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                                /* Move all the cache entries down so the oldest is at the bottom.  */
                                *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
                                *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
                                *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr);

                                /* Setup the new sector information in the cache.  */
                                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_logical_sector =  (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID);
                                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_block =  (USHORT) i;
                                sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_page =   (USHORT) j;
#else

                                /* Determine if this logical sector fits in the logical sector cache mapping.  */
                                if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                                {

                                    /* Yes, store the logical sector to block/page mapping in the cache.  */
                                    nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) i;
                                    nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) j;
                                }
#endif
                            }

                            /* Move to the next page.  */
                            j++;
                                
                            /* Determine if the page has wrapped around.  */
                            if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                            {
                                
                                /* Yes, page wrapped - move to page 1.  */
                                j =  1;
                            }

                            /* Remember the last found block and page for next search.  */
                            nand_flash -> lx_nand_flash_found_block_search =  i;
                            nand_flash -> lx_nand_flash_found_page_search =   j;

                            /* Return success!  */
                            return(LX_SUCCESS);                     
                        }

                        /* Check for the superceded bit being clear, which means the sector was superceded.  */
                        else if (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_SUPERCEDED)
                        {
                        
                            /* Prepare the return information.  */
                            *block =   i;
                            *page =    j;

                            /* No need to update the cache here, since this condition only happens during initialization.  */

                            /* Move to the next page.  */
                            j++;
                                
                            /* Determine if the page has wrapped around.  */
                            if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                            {
                                
                                /* Yes, page wrapped - move to page 1.  */
                                j =  1;
                            }

                            /* Remember the last found block and page for next search.  */
                            nand_flash -> lx_nand_flash_found_block_search =  i;
                            nand_flash -> lx_nand_flash_found_page_search =   j;

                            /* Return success!  */
                            return(LX_SUCCESS);                     
                        }
                    }
                }
                
                /* Move to next page.  */
                j++;
                
                /* Determine if we have a wrap-around condition. */
                if (j >= nand_flash -> lx_nand_flash_pages_per_block)
                {
                
                    /* Yes, page wrapped - move to page 1.  */
                    j =  1;
                }
            }
        }

        /* Determine if there are any more mapped sectors.  */
        if (mapped_pages == 0)
            break;

        /* Move to the next block.  */
        i++;
            
        /* Check for wrap condition.  */
        if (i >= nand_flash -> lx_nand_flash_total_blocks)
        {
            
            /* Wrap to the first block.  */
            i =  0;
        }

        /* Start at the first page in the next block.  */
        j =  1;
    }

    /* Determine if we should update the maximum mapped sector.  */
    if (nand_flash -> lx_nand_flash_max_mapped_sector != 0xFFFFFFFF)
    {
    
        /* Yes, update the maximum mapped sector.  */
        nand_flash -> lx_nand_flash_max_mapped_sector =  max_mapped_sector;
    }

    /* Return sector not found status.  */
    return(LX_SECTOR_NOT_FOUND);  
}

