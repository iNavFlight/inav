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
/*    _lx_nand_flash_driver_extra_bytes_set               PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver extra bytes set operation and        */ 
/*    updates the internal cache.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    page                                  Page number                   */  
/*    source                                Pointer to source extra bytes */ 
/*    size                                  Number of extra bytes         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_extra_bytes_set)                              */ 
/*                                          NAND flash set extra bytes    */ 
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
UINT  _lx_nand_flash_driver_extra_bytes_set(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, UCHAR *source, UINT size)
{

ULONG   cache_index;
ULONG   *destination_ptr;
UINT    status;


    /* Increment the page extra bytes set count.  */
    nand_flash -> lx_nand_flash_diagnostic_page_extra_bytes_sets++;

    /* Call driver extra bytes set function.  */
    status =  (nand_flash -> lx_nand_flash_driver_extra_bytes_set)(block, page, source, size);

    /* Determine if the page extra bytes set cache is enabled.  */
    if (nand_flash ->  lx_nand_flash_page_extra_bytes_cache != LX_NULL)
    {
    
        /* Calculate the cache index.  */
        cache_index =  (block * nand_flash -> lx_nand_flash_pages_per_block) + page;

        /* Build destination address.  */
        destination_ptr =    &nand_flash -> lx_nand_flash_page_extra_bytes_cache[cache_index].lx_nand_page_extra_info_logical_sector;
    
        /* Now save this in the cache.  */
        *destination_ptr =  *((ULONG *) source);  
    }

    /* Return status.  */
    return(status);
}


