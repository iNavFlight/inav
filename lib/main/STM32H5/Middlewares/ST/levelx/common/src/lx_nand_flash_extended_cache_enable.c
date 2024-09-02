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
/*    _lx_nand_flash_extended_cache_enable                PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the extended NAND cache for block status,     */ 
/*    page extra bytes, and page 0 contents. The routine will enable as   */ 
/*    many cache capabilities as possible until the supplied memory is    */ 
/*    exhausted.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    memory                                Pointer to memory for caches  */ 
/*    size                                  Size of memory in bytes       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT  _lx_nand_flash_extended_cache_enable(LX_NAND_FLASH  *nand_flash, VOID *memory, ULONG size)
{

UCHAR   *working_ptr;
ULONG   adjust_size;
ULONG   i;


    /* Setup the working pointer.  */
    working_ptr =  (UCHAR *) memory;
    
    /* Check for a NULL pointer. If NULL, disable all the caches.  */
    if (working_ptr == LX_NULL)
    {
    
        /* Disable all extended caches.  */
        nand_flash -> lx_nand_flash_block_status_cache =      LX_NULL;
        nand_flash -> lx_nand_flash_page_extra_bytes_cache =  LX_NULL;
        nand_flash -> lx_nand_flash_page_0_cache =            LX_NULL;
        
        /* Return success!  */
        return(LX_SUCCESS);   
    }

    /* Determine if there is not enough memory for the block status cache.  */
    if (size < nand_flash -> lx_nand_flash_total_blocks)
    {
    
        /* Return an error since there is not enough memory for even the smallest
           cache.  */
        return(LX_ERROR);
    }
    
    /* There is enough memory, setup the block status cache.  */
    nand_flash -> lx_nand_flash_block_status_cache =  working_ptr;
        
    /* Clear the block status cache.  */
    for (i = 0; i < nand_flash -> lx_nand_flash_total_blocks; i++)
    {
        
        /* Clear the block status cache.  */
        nand_flash -> lx_nand_flash_block_status_cache[i] =  ((UCHAR) 0);
    }

    /* Calculate the size to adjust to achieve ULONG alignment.  */
    adjust_size =  nand_flash -> lx_nand_flash_total_blocks + ((ULONG) (sizeof(ULONG) - 1));
    adjust_size =  (adjust_size/sizeof(ULONG)) * sizeof(ULONG);

    /* Adjust the size.  */
    if (size >= adjust_size)
    {
     
        /* Subtract the aligned size.  */
        size = size - adjust_size;
    }
    else
    {
    
        /* Just set size to 0.  */
        size =  0;
    }
    
    /* Determine if there is enough memory for the page extra bytes cache for each page.  */
    if (size >= (nand_flash -> lx_nand_flash_total_pages * sizeof(LX_NAND_PAGE_EXTRA_INFO)))
    {
    
        /* Yes, there is enough memory for the extra bytes cache.  */
        
        /* First adjust the working pointer to the start of this memory.  */
        working_ptr =  working_ptr + adjust_size;
        
        /* Setup the pointer to page extra bytes cache.  */
        nand_flash -> lx_nand_flash_page_extra_bytes_cache =  (LX_NAND_PAGE_EXTRA_INFO *) working_ptr;
        
        /* Clear the page extra bytes cache.  */
        for (i = 0; i < nand_flash -> lx_nand_flash_total_pages; i++)
        {
        
            /* Clear the block status cache.  */
            nand_flash -> lx_nand_flash_page_extra_bytes_cache[i].lx_nand_page_extra_info_logical_sector =  ((ULONG) 0);
        }

        /* Calculate the size to adjust to achieve ULONG alignment.  */
        adjust_size =  (nand_flash -> lx_nand_flash_total_pages * sizeof(LX_NAND_PAGE_EXTRA_INFO) + ((ULONG) (sizeof(ULONG) - 1)));
        adjust_size =  (adjust_size/sizeof(ULONG)) * sizeof(ULONG);

        /* Adjust the size.  */
        if (size >= adjust_size)
        {
     
            /* Subtract the aligned size.  */
            size = size - adjust_size;
        }
        else
        {
    
            /* Just set size to 0.  */
            size =  0;
        }
    }

    /* Determine if there is enough memory for the page 0 cache for each block.  */
    if (size >= ((nand_flash -> lx_nand_flash_total_blocks * (nand_flash -> lx_nand_flash_pages_per_block + 1)) * sizeof(ULONG)))
    {
    
        /* Yes, there is enough memory for the page 0 cache.  */
        
        /* First adjust the working pointer to the start of this memory.  */
        working_ptr =  working_ptr + adjust_size;
    
        /* Setup the pointer to page 0 cache.  */
        nand_flash -> lx_nand_flash_page_0_cache =  (ULONG *) working_ptr;
        
        /* Clear the page 0 cache.  */
        for (i = 0; i < (nand_flash -> lx_nand_flash_total_blocks * (nand_flash -> lx_nand_flash_pages_per_block + 1)); i++)
        {
        
            /* Clear the block status cache.  */
            nand_flash -> lx_nand_flash_page_0_cache[i] =  ((ULONG) 0);
        }      
    }
    
    /* Return success.  */
    return(LX_SUCCESS);
}

