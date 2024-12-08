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
/*    _lx_nand_flash_sector_release                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function releases a logical sector from being managed in the   */ 
/*    NAND flash.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
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
/*    _lx_nand_flash_block_obsoleted_check  Check for block obsoleted     */ 
/*    _lx_nand_flash_block_reclaim          Reclaim one flash block       */ 
/*    _lx_nand_flash_sector_mapping_cache_invalidate                      */ 
/*                                          Invalidate cache entry        */ 
/*    _lx_nand_flash_logical_sector_find    Find logical sector           */ 
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
UINT  _lx_nand_flash_sector_release(LX_NAND_FLASH *nand_flash, ULONG logical_sector)
{

LX_NAND_PAGE_EXTRA_INFO extra_info;
UINT                    status;
ULONG                   found_block;
ULONG                   found_page;
ULONG                  *block_word_ptr;
ULONG                   i;


#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of release requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_release_requests++;

    /* See if we can find the logical sector in the current mapping.  */
    _lx_nand_flash_logical_sector_find(nand_flash, logical_sector, LX_FALSE, &found_block, &found_page);
    
    /* Determine if the logical sector to page mapping was found.  */
    if (found_page)
    {
    
        /* Yes, we were able to find the logical sector.  */

        /* Determine if this sector is the current maximum sector.  */
        if (nand_flash -> lx_nand_flash_max_mapped_sector == logical_sector)
        {
        
            /* Yes, this is the maximum sector. Reduce the maximum, if possible.  */
            if (logical_sector)
            {
            
                /* Decrement the logical sector.  */
                nand_flash -> lx_nand_flash_max_mapped_sector--;
            }
        }
        
        /* Read the extra information from the page.  */
        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, found_block, found_page, (UCHAR *) &extra_info, sizeof(extra_info));

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, found_block, found_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Mark this sector as invalid.  */
        
        /* Now clear bits 31 and 30, which indicates this sector is now obsoleted.  */
        extra_info.lx_nand_page_extra_info_logical_sector =  extra_info.lx_nand_page_extra_info_logical_sector & 
                                                            ~(((ULONG) LX_NAND_PAGE_VALID) | ((ULONG) LX_NAND_PAGE_SUPERCEDED));
            
        /* Write the value back to the flash to clear bits 31 & 30.  */
        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, found_block, found_page, (UCHAR *) &extra_info, sizeof(extra_info));
            
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, found_block, found_page);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Increment the number of obsolete pages.  */
        nand_flash -> lx_nand_flash_obsolete_pages++;

        /* Decrement the number of mapped pages.  */
        nand_flash -> lx_nand_flash_mapped_pages--;

        /* Setup pointer to internal buffer.  */
        block_word_ptr =  nand_flash -> lx_nand_flash_page_buffer;

        /* Now read page 0 of the block, which has the erase count in the first 4 bytes. */
        status =  _lx_nand_flash_driver_read(nand_flash, found_block, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, found_block, 0);

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

        /* Determine if the mapping list in page 0 is valid.  */        
        if ((block_word_ptr[1] != LX_NAND_PAGE_FREE) &&
            (block_word_ptr[nand_flash -> lx_nand_flash_pages_per_block] == LX_NAND_PAGE_LIST_VALID))
        {
        
            /* Mark the entry as invalid.  */
            block_word_ptr[found_page] &= ~LX_NAND_PAGE_VALID;

            /* Write the invalid entry.  */
            status =  _lx_nand_flash_driver_write(nand_flash, found_block, 0, block_word_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        }
#endif

        /* Ensure the sector mapping cache no longer has this sector.  */
        _lx_nand_flash_sector_mapping_cache_invalidate(nand_flash, logical_sector);

        /* Call routine to see if this block is completely obsoleted.  If so, 
           we can reclaim it immediately.  */
        _lx_nand_flash_block_obsoleted_check(nand_flash, found_block);
        
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
    }
    else
    {

        /* Could not find the logical sector.  */
        status =  LX_SECTOR_NOT_FOUND;
    }
    
#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return status.  */
    return(status);
}

