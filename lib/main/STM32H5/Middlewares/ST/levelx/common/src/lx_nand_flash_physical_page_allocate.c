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
/*    _lx_nand_flash_physical_page_allocate               PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates a free page for mapping to a                */ 
/*    logical sector.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Destination for block         */ 
/*    page                                  Destination for page          */ 
/*    erase_count                           Destination for erase count   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_extra_bytes_get Driver get extra bytes        */ 
/*    _lx_nand_flash_driver_extra_bytes_set Driver set extra bytes        */ 
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Driver block status get       */ 
/*    _lx_nand_flash_driver_page_erased_verify                            */ 
/*                                          Driver verify page erased     */ 
/*    _lx_nand_flash_driver_read            Driver page read              */ 
/*    _lx_nand_flash_block_full_update      Update page 0 with list of    */ 
/*                                            mapped pages                */ 
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
UINT  _lx_nand_flash_physical_page_allocate(LX_NAND_FLASH *nand_flash, ULONG *block, ULONG *page, ULONG *erase_count)
{

LX_NAND_PAGE_EXTRA_INFO extra_info;
UCHAR                   block_status;
ULONG                   search_block;
ULONG                   search_page;
ULONG                   *word_ptr;
ULONG                   i, j;
UINT                    status;


    /* Increment the number of page allocation requests.  */
    nand_flash -> lx_nand_flash_diagnostic_page_allocates++;

    /* Initialize the return parameters.  */
    *block =        (ULONG) 0;
    *page =         (ULONG) 0;
    *erase_count =  nand_flash -> lx_nand_flash_minimum_erase_count;
  
    /* Determine if there are any free pages.  */
    if (nand_flash -> lx_nand_flash_free_pages == 0)
    {

        /* Increment the number of failed allocations.  */
        nand_flash -> lx_nand_flash_diagnostic_page_allocate_errors++;

        /* No free pages, return .  */
        return(LX_NO_PAGES);
    }

    /* Pickup the search for a free page at the specified block.  */
    search_block =  nand_flash -> lx_nand_flash_free_block_search;

    /* Loop through the blocks to find a free page.  */
    for (i = 0; i < nand_flash -> lx_nand_flash_total_blocks; i++)
    {

        /* First, check to see if this block is good. */
        status =  _lx_nand_flash_driver_block_status_get(nand_flash, search_block, &block_status);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, search_block, 0);

            /* Return the error.  */
            return(status);
        }

        /* Determine if this block is bad.  */
        if (block_status != LX_NAND_GOOD_BLOCK)
        {

            /* Move to next search block.  */
            search_block++;
        
            /* Have we wrapped around yet?  */
            if (search_block >= nand_flash -> lx_nand_flash_total_blocks)
            {
         
                /* Reset the search block to the beginning.   */
                search_block =  0;
            }
          
            /* Block is bad, move to the next block.  */
            continue;        
        }

        /* Read the extra bytes of page 0. This will tell us if the page has any free entries.  */
        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, search_block, 0, (UCHAR *) &extra_info, sizeof(extra_info));    
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, search_block, 0);
            
            /* Return the error... no point in continuing.  */
            return(status);
        }
        
        /* Determine if there are any free entries.  */
        if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_BLOCK_FULL) == 0)
        {

            /* This block is full... go to the next block.  */
            search_block++;
        
            /* Have we wrapped around yet?  */
            if (search_block >= nand_flash -> lx_nand_flash_total_blocks)
            {
         
                /* Reset the search block to the beginning.   */
                search_block =  0;
            }
          
            /* Block is bad, move to the next block.  */
            continue;        
        }

        /* Now check to see if page 0 has been written to.  When the block is full, page 0 is written with the mapping of pages
           to logical sectors in this block - only when the block becomes full.  If the block has additional pages free, page 0 is all 0xFFs.  */
        status =  _lx_nand_flash_driver_read(nand_flash, search_block, 0, nand_flash -> lx_nand_flash_page_buffer, nand_flash -> lx_nand_flash_words_per_page);

        /* Check for an error... an error here indicates the block does not have any free pages.  */
        if (status)
        {

            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, search_block, 0);

            /* Determine if the error is fatal.  */
            if (status != LX_NAND_ERROR_CORRECTED)
            {
                            
                /* Return the error.  */
                return(status);
            }
        }
        
        /* Setup pointer to the page 0 data.  */
        word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

        /* Pickup the block erase count (first word in the page 0 data area).  */
        if (*word_ptr != LX_ALL_ONES)
            *erase_count =  *word_ptr;
        
        /* Now determine if the remaining words are all ones, i.e. haven't been written to.  */
        j = 1;
        while (j < (nand_flash -> lx_nand_flash_pages_per_block + 1))
        {
        
            /* Has this word been written to?  */
            if (word_ptr[j] != LX_ALL_ONES)
                break;

            /* Move to next word.  */
            j++;                
        }

        /* Does this block have any free pages?  */
        if (j < (nand_flash -> lx_nand_flash_pages_per_block + 1))
        {
        
            /* Move to next search block.  */
            search_block++;
        
            /* Have we wrapped around yet?  */
            if (search_block >= nand_flash -> lx_nand_flash_total_blocks)
            {
         
                /* Reset the search block to the beginning.   */
                search_block =  0;
            }
  
            /* No, the data area has been written to.  Simply move to the next block.  */
            continue;
        }
        
        /* Now search for an available page within the block.  */
        search_page =  1;
        while (search_page < nand_flash -> lx_nand_flash_pages_per_block)
        {
        
            /* Read the extra information for this page to see if there is a sector mapping.  */
            status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, search_block, search_page, (UCHAR *) &extra_info, sizeof(extra_info));    
        
            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, search_block, search_page);
                
                /* Return the error.  */
                return(status);
            }

            /* Now determine if this page is mapped or free.  */
            if (extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
            {
            
                /* Now make sure the data area of this page is free.  */
                status =  _lx_nand_flash_driver_page_erased_verify(nand_flash, search_block, search_page);

                /* Is the page free?  */
                if (status != LX_SUCCESS)
                {
                
                    /* No, the page is not actually free.  For now, simply mark the extra info area to indicate
                       the page is not available and continue our search.  */
                    extra_info.lx_nand_page_extra_info_logical_sector =  0;
                
                    /* Decrease the number of free pages.  */
                    nand_flash -> lx_nand_flash_free_pages--;
                    
                    /* Increase the number of obsolete pages.  */
                    nand_flash -> lx_nand_flash_obsolete_pages++;
                
                    /* Write out the extra info for this page.  */
                    status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, search_block, search_page, (UCHAR *) &extra_info, sizeof(extra_info));    
        
                    /* Check for an error from flash driver.   */
                    if (status)
                    {
        
                        /* Call system error handler.  */
                        _lx_nand_flash_system_error(nand_flash, status, search_block, search_page);

                        /* Return the error.  */
                        return(status);
                    }    
                    
                    /* Determine if this is the last entry of the block.  */
                    if (search_page == (nand_flash -> lx_nand_flash_pages_per_block - 1))
                    {
                    
                        /* Yes, we need to update page 0 of the block with the list of mapped 
                           pages for this block.  */
                        _lx_nand_flash_block_full_update(nand_flash, search_block, *erase_count);
                    }
                }
                else
                {
            
                    /* Yes, we have found a free page in this block. Get out of the page search loop.  */
                    break;
                }
            }

            /* Move to next search page.  */
            search_page++;
        }
        
        /* Determine if we have found the free block.  */
        if (search_page < nand_flash -> lx_nand_flash_pages_per_block)
        {
        
            /* Remember the block to search.  */
            nand_flash -> lx_nand_flash_free_block_search =  search_block;
            
            /* Determine if the allocated page is the last one in this block.  */
            if (search_page == (nand_flash -> lx_nand_flash_pages_per_block - 1))
            {
            
                /* Yes, move to the next block.  */
                nand_flash -> lx_nand_flash_free_block_search++;
                
                /* Have we wrapped around yet?  */
                if (nand_flash -> lx_nand_flash_free_block_search >= nand_flash -> lx_nand_flash_total_blocks)
                {
         
                    /* Wrap condition - reset the search block to the beginning.   */
                    nand_flash -> lx_nand_flash_free_block_search =  0;
                }
            }
                                                
            /* Prepare the return information.  */
            *block =  search_block;
            *page =   search_page;

            /* Determine if this is the first data page of the block.  */
            if (search_page == 1)
            {
            
                /* Yes, we need to update the extra bytes of page 0 to indicate the block is no longer empty.  */
                extra_info.lx_nand_page_extra_info_logical_sector =  ((ULONG) LX_NAND_PAGE_FREE) & ~((ULONG) LX_NAND_BLOCK_EMPTY);
                status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, search_block, 0, (UCHAR *) &extra_info, sizeof(extra_info));    
                
                /* Check for an error from flash driver.   */
                if (status)
                {
        
                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, search_block, 0);
                }           
            }

            /* Return success!  */
            return(LX_SUCCESS);                     
        }
        
        /* Move to next search block.  */
        search_block++;
        
        /* Have we wrapped around yet?  */
        if (search_block >= nand_flash -> lx_nand_flash_total_blocks)
        {
         
            /* Reset the search block to the beginning.   */
            search_block =  0;
        }
    }

    /* Increment the number of failed allocations.  */
    nand_flash -> lx_nand_flash_diagnostic_page_allocate_errors++;

    /* Return error.  */
    return(LX_NO_PAGES);
}

