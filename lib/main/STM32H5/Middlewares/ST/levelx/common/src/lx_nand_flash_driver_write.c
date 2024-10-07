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
/*    _lx_nand_flash_driver_write                         PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver to write data to a NAND page.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    page                                  Page number                   */ 
/*    source                                Pointer to source buffer      */ 
/*    words                                 Number of words to write      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_write)          Driver page write             */ 
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
UINT  _lx_nand_flash_driver_write(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, ULONG *source, ULONG words)
{

ULONG   cache_index;
ULONG   *source_ptr;
ULONG   *destination_ptr;
ULONG   i;
UINT    status;


    /* Determine if this is page 0.  */
    if (page == 0)
    {
    
        /* Determine if the page 0 cache is enabled.  */
        if (nand_flash -> lx_nand_flash_page_0_cache != LX_NULL)
        {
        
            /* Yes, the page 0 cache is enabled.  */

            /* Calculate the cache index.   */
            cache_index =  (block * (nand_flash -> lx_nand_flash_pages_per_block + 1));

            /* Setup destination pointer.  */
            destination_ptr =  &nand_flash -> lx_nand_flash_page_0_cache[cache_index];
    
            /* Setup source pointer.  */
            source_ptr =  (ULONG *) source;
        
            /* Simply copy the page 0 information to the destination.  */
            for (i = 0; i < (nand_flash -> lx_nand_flash_pages_per_block + 1); i++)
            {
          
                /* Copy one word.  */
                *destination_ptr++ =  *source_ptr++;
            }
        }    
    }

    /* Increment the page write count.  */
    nand_flash -> lx_nand_flash_diagnostic_page_writes++;

    /* Call driver write function.  */
    status =  (nand_flash -> lx_nand_flash_driver_write)(block, page, source, words);

    /* Return status.  */
    return(status);
}

