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
/*    _lx_nand_flash_block_full_update                    PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes the list of page to logical sector mapping     */ 
/*    when the block is full.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block that is full            */ 
/*    erase_count                           Block erase count             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_write           Driver flash page write       */ 
/*    _lx_nand_flash_driver_extra_bytes_get Get extra bytes from spare    */ 
/*    _lx_nand_flash_driver_extra_bytes_set Set extra bytes from spare    */ 
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
UINT  _lx_nand_flash_block_full_update(LX_NAND_FLASH *nand_flash, ULONG block, ULONG erase_count)
{

LX_NAND_PAGE_EXTRA_INFO extra_info;
UINT                    status;
ULONG                   i;
ULONG                   *list_ptr;


    /* Setup pointer to internal buffer.  */
    list_ptr =  nand_flash -> lx_nand_flash_page_buffer;

    /* Set the internal buffer of all ones.  */
    for (i = 0; i < nand_flash -> lx_nand_flash_words_per_page; i++)
    {
    
        /* Set the buffer to all ones.  */
        list_ptr[i] =  LX_ALL_ONES;
    }

    /* Set the initial word to the erase count (this is what is already there!).  */
    list_ptr[0] =  erase_count;

    /* Now walk through the pages in the block to build the list of mapped logical 
       sectors to pages.  */
    i =  1;
    while (i < nand_flash -> lx_nand_flash_pages_per_block)
    {
    
        /* Read the extra bytes of each page to pickup the associated mapping.  */
        status =  _lx_nand_flash_driver_extra_bytes_get(nand_flash, block, i, (UCHAR *) &extra_info, sizeof(extra_info));    
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, i);
            
            /* Return the error... no point in continuing.  */
            return(status);
        }

        /* Determine if the extra info indicates the page has not yet been allocated.  */
        if (extra_info.lx_nand_page_extra_info_logical_sector == LX_NAND_PAGE_FREE)
        {
        
            /* Block not full, get out of the loop.  */
            break;
        }

        /* Save the mapping.  */
        list_ptr[i] =  extra_info.lx_nand_page_extra_info_logical_sector;

        /* Move to next entry.  */
        i++;
    }
    
    /* Check for a full block.  */
    if (i == nand_flash -> lx_nand_flash_pages_per_block)
    {
    
        /* Yes, the block is full.  */
    
        /* Place the list complete marker so we know the list is intact.  */
        list_ptr[i] =  LX_NAND_PAGE_LIST_VALID;
    
        /* At this point we can write the buffer to page 0 of the block.  */
    
        /* Write the mapping list to page 0.  */
        status =  _lx_nand_flash_driver_write(nand_flash, block, 0, list_ptr, (nand_flash -> lx_nand_flash_pages_per_block + 1));
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);

            /* Return the error... no point in continuing.  */
            return(status);
        }
    
        /* Write the extra bytes for page 0.  */
        extra_info.lx_nand_page_extra_info_logical_sector =  ((ULONG) LX_NAND_PAGE_FREE) & ~(((ULONG) LX_NAND_BLOCK_EMPTY) | ((ULONG) LX_NAND_BLOCK_FULL));
        status =  _lx_nand_flash_driver_extra_bytes_set(nand_flash, block, 0, (UCHAR *) &extra_info, sizeof(extra_info));    
        
        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);
        }
    }
        
    /* Return status.  */
    return(status);
}

