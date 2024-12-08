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
/*    _lx_nand_flash_open                                 PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a NAND flash instance and ensures the           */ 
/*    NAND flash is in a coherent state.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    name                                  Name of NAND flash instance   */ 
/*    nand_driver_initialize                Driver initialize             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (nand_driver_initialize)              Driver initialize             */ 
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Driver block status get       */ 
/*    _lx_nand_flash_driver_block_status_set                              */ 
/*                                          Driver block status set       */ 
/*    _lx_nand_flash_driver_extra_bytes_get Driver get extra bytes        */
/*    _lx_nand_flash_driver_extra_bytes_set Driver set extra bytes        */ 
/*    _lx_nand_flash_driver_read            Driver page read              */ 
/*    _lx_nand_flash_driver_write           Driver page write             */ 
/*    _lx_nand_flash_driver_block_erased_verify                           */ 
/*                                          Driver verify block erased    */ 
/*    _lx_nand_flash_driver_block_erase     Driver block erase            */ 
/*    _lx_nand_flash_driver_page_erased_verify                            */ 
/*                                          Driver verify page erased     */ 
/*    _lx_nand_flash_block_full_update      Write mapping to page 0       */ 
/*    _lx_nand_flash_logical_sector_find    Find logical sector           */ 
/*    _lx_nand_flash_system_error           System error handler          */ 
/*    tx_mutex_create                       Create thread-safe mutex      */ 
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
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            removed multiple write      */
/*                                            to page 0,                  */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_open(LX_NAND_FLASH  *nand_flash, CHAR *name, UINT (*nand_driver_initialize)(LX_NAND_FLASH *))
{

LX_NAND_PAGE_EXTRA_INFO     extra_info;
ULONG                       block;
ULONG                       page;
ULONG                       found_block, found_page;
UCHAR                       block_status;
ULONG                       *page_word_ptr;
ULONG                       temp;
ULONG                       free_pages;
ULONG                       obsolete_pages;
ULONG                       mapped_pages;
ULONG                       max_mapped_sector;
ULONG                       erased_count, min_erased_count, max_erased_count;
UINT                        status;
LX_NAND_FLASH               *tail_ptr;
#ifdef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
ULONG                       logical_sector;
#endif

LX_INTERRUPT_SAVE_AREA

    LX_PARAMETER_NOT_USED(name);

    /* Clear the NAND flash control block.  */
    LX_MEMSET(nand_flash, 0, sizeof(LX_NAND_FLASH));
    
    /* Call the flash driver's initialization function.  */
    (nand_driver_initialize)(nand_flash);

    /* Determine if we can support this NAND flash size.  */
    if (((nand_flash -> lx_nand_flash_pages_per_block * sizeof(ULONG)) + 8) > nand_flash -> lx_nand_flash_bytes_per_page)
    {

        /* Not enough room in page 0 to contain the erase count and logical sector mapping list.  */
        
        /* Return error.  */
        return(LX_ERROR);    
    }
    
    /* Determine if the driver supplied a RAM buffer for reading the page.  */
    if (nand_flash -> lx_nand_flash_page_buffer == LX_NULL)
    {

        /* Return an error.  */
        return(LX_NO_MEMORY);
    }


    /* Calculate the number of words per block and per page.  */
    nand_flash -> lx_nand_flash_words_per_page =   (nand_flash -> lx_nand_flash_bytes_per_page / sizeof(ULONG));
    nand_flash -> lx_nand_flash_words_per_block =  (nand_flash -> lx_nand_flash_words_per_page * nand_flash -> lx_nand_flash_pages_per_block);

    /* Calculate the total pages.  */
    nand_flash -> lx_nand_flash_total_pages =   nand_flash -> lx_nand_flash_total_blocks * nand_flash -> lx_nand_flash_pages_per_block;
   
    /* Setup default values for the max/min erased counts.  */
    min_erased_count =  LX_ALL_ONES;
    max_erased_count =  0;

    /* Default the max mapped sector to 0.  */
    max_mapped_sector =  0;

    /* Setup the block word pointer to the internal buffer area.  */
    page_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

    /* Initialize the last found block and page search markers.  */
    nand_flash -> lx_nand_flash_found_block_search =  0;
    nand_flash -> lx_nand_flash_found_page_search =   1;
    
    /* Loop through the blocks to check for bad blocks and determine the minimum and maximum erase count for each good block.  */
    for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
    {
    
        /* First, check to make sure this block is good.  */
        status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);
            
            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Is this block bad?  */
        if (block_status != LX_NAND_GOOD_BLOCK)
        {
        
            /* Yes, this block is bad.  */
                
            /* Increment the number of bad blocks.  */
            nand_flash -> lx_nand_flash_bad_blocks++;
                
            /* Continue to the next block.  */
            continue;
        }

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
                            
                /* Return an error.  */
                return(LX_ERROR);
            }
        }

        /* Is the block erased?  */
        if (((page_word_ptr[0] & LX_BLOCK_ERASED) != LX_BLOCK_ERASED) && (page_word_ptr[0] != LX_BLOCK_ERASE_STARTED))
        {
        
            /* No, valid block.  Isolate the erased count.  */
            erased_count =  (page_word_ptr[0] & LX_BLOCK_ERASE_COUNT_MASK);
            
            /* Is this the new minimum?  */
            if (erased_count < min_erased_count)
            {
                
                /* Yes, remember the new minimum.  */
                min_erased_count =  erased_count;
            }
            
            /* Is this the new maximum?  */
            if (erased_count > max_erased_count)
            {
            
                /* Yes, remember the new maximum.  */
                max_erased_count =  erased_count;
            }
        }
    }    

    /* If we haven't found any erased counts, we can assume the flash is completely erased and needs to 
       be setup for the first time.  */
    if (min_erased_count == LX_ALL_ONES)
    {
    
        /* Indicate that this is the initial format.  */
        nand_flash -> lx_nand_flash_diagnostic_initial_format =  LX_TRUE;
    
        /* Loop through the blocks to setup the flash the first time.  */
        for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
        {

            /* First, check to make sure this block is good.  */
            status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);

                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Is this block bad?  */
            if (block_status != LX_NAND_GOOD_BLOCK)
            {
        
                /* Yes, this block is bad.  */

                /* Increment the number of bad blocks.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
                
                /* Continue to the next block.  */
                continue;
            }
  
            /* Setup the initial erase count to 1.  */
            page_word_ptr[0] =  (((ULONG) 1));

            /* Write the initial erase count for the block.  */            
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
                    
                    /* Return an error.  */
                    return(LX_ERROR);
                }
            
                /* Increment the bad block count.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
            }
            else
            {

                /* Update the number of free pages. Subtract 1, since page 0 is used for the erase count and list of mapped pages.  */
                nand_flash -> lx_nand_flash_free_pages =   nand_flash -> lx_nand_flash_free_pages + 
                                                                        nand_flash -> lx_nand_flash_pages_per_block - 1;
            }
        }    

        /* Update the overall minimum and maximum erase count.  */
        nand_flash -> lx_nand_flash_minimum_erase_count =  1;
        nand_flash -> lx_nand_flash_maximum_erase_count =  1;
    }
    else
    {

        /* At this point, we have a previously managed flash structure. This needs to be traversed to prepare for the 
           current flash operation.  */

        /* Setup the maximum mapped sector value so all searches are valid.  */
        nand_flash -> lx_nand_flash_max_mapped_sector =  0xFFFFFFFF;

        /* Default the flash block search to an invalid value.  */
        nand_flash -> lx_nand_flash_free_block_search =  nand_flash -> lx_nand_flash_total_blocks;

        /* Loop through the blocks.  */
        for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
        {
         
            /* First, check to make sure this block is good.  */
            status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

            /* Check for an error from flash driver.   */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);
                
                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Is this block bad?  */
            if (block_status != LX_NAND_GOOD_BLOCK)
            {
        
                /* Yes, this block is bad.  */
                
                /* Increment the number of bad blocks.  */
                nand_flash -> lx_nand_flash_bad_blocks++;
                
                /* Continue to the next block.  */
                continue;
            }

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
                            
                    /* Return an error.  */
                    return(LX_ERROR);
                }
            }

            /* Is the block erased?  */
            if (((page_word_ptr[0] & LX_BLOCK_ERASED) == LX_BLOCK_ERASED) || (page_word_ptr[0] == LX_BLOCK_ERASE_STARTED))
            {

                /* This can happen if we were previously in the process of erasing the flash block and a 
                   power interruption occurs.  It should only occur once though. */

                /* Is this the first time?  */
                if (nand_flash -> lx_nand_flash_diagnostic_erased_block)
                {
                            
                    /* No, this is a potential format error, since this should only happen once in a given
                       NAND flash format.  */
                    _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_INVALID_BLOCK, block, 0);
                }

                /* Increment the erased block count.  */
                nand_flash -> lx_nand_flash_diagnostic_erased_block++;

                /* Check to see if the block is erased. */
                status =  _lx_nand_flash_driver_block_erased_verify(nand_flash, block);

                /* Is the block completely erased?  */
                if (status != LX_SUCCESS)
                {
                
                    /* Is this the first time?  */
                    if (nand_flash -> lx_nand_flash_diagnostic_re_erase_block)
                    {
                            
                        /* No, this is a potential format error, since this should only happen once in a given
                           NAND flash format.  */
                        _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_INVALID_BLOCK, block, 0);
                    }

                    /* Increment the erased block diagnostic.  */
                    nand_flash -> lx_nand_flash_diagnostic_re_erase_block++;
        
                    /* No, the block is not fully erased, erase it again.  */
                    status =  _lx_nand_flash_driver_block_erase(nand_flash, block, max_erased_count);
                    
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

                            /* Return an error.  */
                            return(LX_ERROR);
                        }
            
                        /* Increment the bad block count.  */
                        nand_flash -> lx_nand_flash_bad_blocks++;
                        
                        /* Continue with next block.  */
                        continue;
                    }
                }

                /* Now read page 0 of the block, which is all ones at this point. */
                status =  _lx_nand_flash_driver_read(nand_flash, block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));

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

                        /* Return an error.  */
                        return(LX_ERROR);
                    }
            
                    /* Increment the bad block count.  */
                    nand_flash -> lx_nand_flash_bad_blocks++;
                        
                    /* Continue with next block.  */
                    continue;
                }
              
                /* Write the final erase count for the block.  */            
                page_word_ptr[0] =  max_erased_count;
                status =  _lx_nand_flash_driver_write(nand_flash, block, 0, page_word_ptr, 1);

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

                        /* Return an error.  */
                        return(LX_ERROR);
                    }
            
                    /* Increment the bad block count.  */
                    nand_flash -> lx_nand_flash_bad_blocks++;
                        
                    /* Continue with next block.  */
                    continue;
                }

                /* Update the number of free pages.  */
                nand_flash -> lx_nand_flash_free_pages =   nand_flash -> lx_nand_flash_free_pages + 
                                                                                        nand_flash -> lx_nand_flash_pages_per_block - 1;
            }
            else 
            {

                /* Check for a format error, an interruption while writing the mapped sector list.  */
                if ((page_word_ptr[1] != LX_NAND_PAGE_FREE) &&
                    (page_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] != LX_NAND_PAGE_LIST_VALID))
                {
                
                     /* Increment the invalid mapping counter.  */
                     nand_flash -> lx_nand_flash_diagnostic_mapping_invalid++;
                }
            
                /* Calculate the number of free pages by traversing the pages in the block.  */
                free_pages =      0;
                obsolete_pages =  0;
                mapped_pages =    0;
                for (page = 1; page < nand_flash -> lx_nand_flash_pages_per_block; page++)
                {
                
                    /* Read the logical sector mapping for this page.  */
                    status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, block, page, (UCHAR *) &extra_info, sizeof(extra_info));

                    /* Check for an error from flash driver.   */
                    if (status)
                    {
        
                        /* Call system error handler.  */
                        _lx_nand_flash_system_error(nand_flash, status, block, 0);

                        /* Return an error.  */
                        return(LX_ERROR);
                    }
                    
                    /* Determine if the page is free.   */
                    if (extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
                    {
                    
                        /* The page is free and we know that the pages are allocated sequentially
                           so there is no need to go further.  */
                    
                        /* Increment the free pages counter.  */
                        free_pages =  free_pages + (nand_flash -> lx_nand_flash_pages_per_block - page);
                        
                        /* Verify that the same position in the mapped list from page 0 is also free.  */
                        if (page_word_ptr[page] != LX_NAND_PAGE_FREE)
                        {
                            
                            /* Increment the invalid mapping counter.  */
                            nand_flash -> lx_nand_flash_diagnostic_mapping_invalid++;
                            
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_INVALID_FORMAT, block, page);
                        }
                        
                        /* Determine if the associated page is really free... it could be the case that the 
                           page data area was in the process of being written or just completed when a power
                           interruption occurs before the extra bytes are setup with the logical sector.  */

                        /* Read verify the page is erased.  */
                        status =  _lx_nand_flash_driver_page_erased_verify(nand_flash, block, page);

                        /* Check for an error from flash driver.   */
                        if (status == LX_ERROR)
                        {

                            /* Increment the page data not free diagnostic counter.  */
                            nand_flash -> lx_nand_flash_diagnostic_page_data_not_free++;
                            
                            /* Decrement the free page counter and increment the obsolete page counter.  */
                            free_pages--;
                            obsolete_pages++;
                            
                            /* Now setup the extra info to show this page is obsolete.  */
                            extra_info.lx_nand_page_extra_info_logical_sector =  0;
                            status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, block, page, (UCHAR *) &extra_info, sizeof(extra_info));

                            /* Check for an error from flash driver.   */
                            if (status)
                            {
        
                                /* Call system error handler.  */
                                _lx_nand_flash_system_error(nand_flash, status, block, page);

                                /* Return an error.  */
                                return(LX_ERROR);
                            }
                        }
                           
                        /* At this point we can break out of the page traversal loop, since nothing else can exist after this page.  */
                        break;
                    }
                    else 
                    {
                    
                        /* Determine if the mapping list is not present and the value doesn't match the 
                           logical sector value.  */
                        if ((page_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] == LX_NAND_PAGE_LIST_VALID) &&
                            ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) !=
                                                                            (page_word_ptr[page] & LX_NAND_LOGICAL_SECTOR_MASK)))
                        {
                        
                            /* Increment the invalid mapping counter.  */
                            nand_flash -> lx_nand_flash_diagnostic_mapping_invalid++;

                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_INVALID_FORMAT, block, page);
                        }

                        /* Determine if this mapped page is obsolete.  */
                        if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) ==
                                                extra_info.lx_nand_page_extra_info_logical_sector)
                        {
                        
                            /* Yes, page mapping is obsolete. Increment the obsolete count.  */
                            obsolete_pages++;
                            
                            /* Continue looking at next page.  */
                            continue;
                        }

                        /* Determine if the mapping for this page isn't yet valid.  */
                        if (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_MAPPING_NOT_VALID)
                        {
                        
                            /* Yes, a power interruption or reset occurred while the sector mapping entry was being written.  */
                            
                            /* Increment the obsolete count.  */
                            obsolete_pages++;

                            /* Increment the interrupted mapping counter.  */                           
                            nand_flash -> lx_nand_flash_diagnostic_mapping_write_interrupted++;

                            /* Invalidate this entry - clearing valid bit, superceded bit and logical sector.  */
                            extra_info.lx_nand_page_extra_info_logical_sector =  0;
                            
                            /* Write it out the page extra information.  */        
                            status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, block, page, (UCHAR *) &extra_info, sizeof(extra_info));

                            /* Check for an error from flash driver.   */
                            if (status)
                            {
        
                                /* Call system error handler.  */
                                _lx_nand_flash_system_error(nand_flash, status, block, page);

                                /* Return an error.  */
                                return(LX_ERROR);
                            }
                            
                            /* Continue looking at next page.  */
                            continue;                       
                        }

                        /* Determine if this page is still mapped.  */
                        if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_VALID) &&
                            (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_PAGE_SUPERCEDED))
                        {
                        
                            /* Yes, sector is mapped. Increment the mapped count.  */
                            mapped_pages++;

                            /* Determine if this is a new maximum mapped sector.  */
                            if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) > max_mapped_sector)
                            {
                            
                                /* Remember the maximum mapped logical sector.  */
                                max_mapped_sector =  (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK);
                            }
                            
#ifdef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                            /* Logical sector is mapped, setup the direct cache.  */
                            logical_sector =  extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK;

                            /* Determine if this logical sector fits in the logical sector direct cache mapping.  */
                            if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                            {

                                /* Remember the mapping for this logical sector.  */
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) block;
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) page;
                            }
#endif
                            /* Continue looking at next page.  */
                            continue;
                        }

                        /* At this point, the page was in the process of being superceded.  */

                        /* Determine if this is a new maximum mapped sector.  */
                        if ((extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK) > max_mapped_sector)
                        {
                            
                            /* Remember the maximum mapped logical sector.  */
                            max_mapped_sector =  (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK);
                        }

                        /* Increment the being obsoleted count.  */
                        nand_flash -> lx_nand_flash_diagnostic_page_being_obsoleted++;

                        /* Save the currently mapped pages.  */
                        temp =  nand_flash -> lx_nand_flash_mapped_pages;
                                
                        /* Indicate all the pages are mapped for the purpose of this search.  */
                        nand_flash -> lx_nand_flash_mapped_pages =  nand_flash -> lx_nand_flash_total_pages;

                        /* Yes, this block was about to become obsolete. Perform a search for a logical sector entry that
                           has both of these bits set.  */
                        _lx_nand_flash_logical_sector_find(nand_flash, (extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK), 
                                                                                LX_TRUE, &found_block, &found_page);

                        /* Read page 0 data again.  */
                        status =  _lx_nand_flash_driver_read(nand_flash, block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
                        /* Check for an error from flash driver.   */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, block, 0);

                            /* Determine if the error is fatal.  */
                            if (status != LX_NAND_ERROR_CORRECTED)
                            {
                            
                                /* Return an error.  */
                                return(LX_ERROR);
                            }
                        }

                        /* Restore the number of mapped pages.  */
                        nand_flash -> lx_nand_flash_mapped_pages =  temp;
               
                        /* Determine if the new logical sector entry is present.  */
                        if (found_page)
                        {
                                
                            /* Yes, make the current entry obsolete in favor of the new entry.  */

                            /* Increment the obsolete count.  */
                            obsolete_pages++;

                            /* Clear the valid bit.  */
                            extra_info.lx_nand_page_extra_info_logical_sector =  extra_info.lx_nand_page_extra_info_logical_sector & ~((ULONG) LX_NAND_PAGE_VALID);
                            
                            /* Write it out the page extra information.  */        
                            status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, block, page, (UCHAR *) &extra_info, sizeof(extra_info));

                            /* Check for an error from flash driver.   */
                            if (status)
                            {
        
                                /* Call system error handler.  */
                                _lx_nand_flash_system_error(nand_flash, status, block, page);

                                /* Return an error.  */
                                return(LX_ERROR);
                            }

                            /* Is this the first time?  */
                            if (nand_flash -> lx_nand_flash_diagnostic_page_obsoleted)
                            {
                            
                                /* No, this is a potential format error, since this should only happen once in a given
                                   NAND flash format.  */
                                _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_INVALID_SECTOR_MAP, block, page);
                            }

                            /* Increment the obsoleted count.  */
                            nand_flash -> lx_nand_flash_diagnostic_page_obsoleted++;

#ifdef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                            /* Logical sector is mapped, setup the direct cache.  */
                            logical_sector =  extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK;

                            /* Determine if this logical sector fits in the logical sector direct cache mapping.  */
                            if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                            {

                                /* Remember the mapping for this logical sector.  */
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) found_block;
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) found_page;
                            }
#endif
                        }
                        else
                        {
                        
                            /* A new page was not found. Increment the number of mapped pages.  */
                            mapped_pages++;

#ifdef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

                            /* Logical sector is mapped, setup the direct cache.  */
                            logical_sector =  extra_info.lx_nand_page_extra_info_logical_sector & LX_NAND_LOGICAL_SECTOR_MASK;

                            /* Determine if this logical sector fits in the logical sector direct cache mapping.  */
                            if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
                            {

                                /* Remember the mapping for this logical sector.  */
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  (USHORT) block;
                                nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   (USHORT) page;
                            }
#endif
                        }    
                    }
                }

                /* Update the number of free, obsolete, and mapped pages.  */
                nand_flash -> lx_nand_flash_free_pages =      nand_flash -> lx_nand_flash_free_pages + free_pages;
                nand_flash -> lx_nand_flash_mapped_pages =    nand_flash -> lx_nand_flash_mapped_pages + mapped_pages;
                nand_flash -> lx_nand_flash_obsolete_pages =  nand_flash -> lx_nand_flash_obsolete_pages + obsolete_pages;
                
                /* Determine if the mapping list needs to be written to the flash.  */
                if ((free_pages == 0) && (page_word_ptr[1] == LX_ALL_ONES))
                {

                    /* Yes, the mapping list should be written to page 0.  */
                    _lx_nand_flash_block_full_update(nand_flash, block, page_word_ptr[0]);

                    /* Read page 0 data again.  */
                    status =  _lx_nand_flash_driver_read(nand_flash, block, 0, page_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
                    /* Check for an error from flash driver.   */
                    if (status)
                    {
        
                        /* Call system error handler.  */
                        _lx_nand_flash_system_error(nand_flash, status, block, 0);

                        /* Determine if the error is fatal.  */
                        if (status != LX_NAND_ERROR_CORRECTED)
                        {
                            
                            /* Return an error.  */
                            return(LX_ERROR);
                        }
                    }
                }
            }       
        }

        /* Update the overall minimum and maximum erase count.  */
        nand_flash -> lx_nand_flash_minimum_erase_count =  min_erased_count;
        nand_flash -> lx_nand_flash_maximum_erase_count =  max_erased_count;

        /* Determine if we need to update the free sector search pointer.  */
        if (nand_flash -> lx_nand_flash_free_block_search == nand_flash -> lx_nand_flash_total_blocks)
        {
                            
            /* Just start at the beginning.  */
            nand_flash -> lx_nand_flash_free_block_search =  0;
        }
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* If the thread safe option is enabled, create a ThreadX mutex that will be used in all external APIs 
       in order to provide thread-safe operation.  */
    status =  tx_mutex_create(&nand_flash -> lx_nand_flash_mutex, "NAND Flash Mutex", TX_NO_INHERIT);

    /* Determine if the mutex creation encountered an error.  */
    if (status != LX_SUCCESS)
    {
    
        /* Call system error handler, since this should not happen.  */
        _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_MUTEX_CREATE_FAILED, 0, 0);
    
        /* Return error to caller.  */
        return(LX_ERROR);
    }
#endif

    /* Enable the sector mapping cache.  */
    nand_flash -> lx_nand_flash_sector_mapping_cache_enabled =  LX_TRUE;
    
    /* Initialize the last found block and page search markers.  */
    nand_flash -> lx_nand_flash_found_block_search =  0;
    nand_flash -> lx_nand_flash_found_page_search =   1;

    /* Remember the maximum mapped sector.  */
    nand_flash -> lx_nand_flash_max_mapped_sector =  max_mapped_sector;

    /* Lockout interrupts.  */
    LX_DISABLE

    /* At this point, the NAND flash has been opened successfully.  Place the 
       NAND flash control block on the linked list of currently opened NAND flashes.  */

    /* Set the NAND flash state to open.  */
    nand_flash -> lx_nand_flash_state =  LX_NAND_FLASH_OPENED;

    /* Place the NAND flash control block on the list of opened NAND flashes.  First,
       check for an empty list.  */
    if (_lx_nand_flash_opened_count)
    {

        /* List is not empty - other NAND flashes are open.  */

        /* Pickup tail pointer.  */
        tail_ptr =  _lx_nand_flash_opened_ptr -> lx_nand_flash_open_previous;

        /* Place the new NAND flash control block in the list.  */
        _lx_nand_flash_opened_ptr -> lx_nand_flash_open_previous =  nand_flash;
        tail_ptr -> lx_nand_flash_open_next =                       nand_flash;

        /* Setup this NAND flash's opened links.  */
        nand_flash -> lx_nand_flash_open_previous =  tail_ptr;
        nand_flash -> lx_nand_flash_open_next =      _lx_nand_flash_opened_ptr;   
    }
    else
    {

        /* The opened NAND flash list is empty.  Add the NAND flash to empty list.  */
        _lx_nand_flash_opened_ptr =                 nand_flash;
        nand_flash -> lx_nand_flash_open_next =      nand_flash;
        nand_flash -> lx_nand_flash_open_previous =  nand_flash;
    }

    /* Increment the opened NAND flash counter.  */
    _lx_nand_flash_opened_count++;

    /* Restore interrupts.  */
    LX_RESTORE

    /* Return a successful completion.  */
    return(LX_SUCCESS);
}

