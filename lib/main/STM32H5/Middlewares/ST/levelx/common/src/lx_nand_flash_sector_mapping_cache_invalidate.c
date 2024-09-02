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
/*    _lx_nand_flash_sector_mapping_cache_invalidate      PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function invalidates the sector's entry in the NAND flash      */ 
/*    cache.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
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
VOID  _lx_nand_flash_sector_mapping_cache_invalidate(LX_NAND_FLASH *nand_flash, ULONG logical_sector)
{

#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
ULONG                               i;
LX_NAND_SECTOR_MAPPING_CACHE_ENTRY  *sector_mapping_cache_entry_ptr;
#endif

    /* Determine if the sector mapping cache is enabled.  */
    if (nand_flash -> lx_nand_flash_sector_mapping_cache_enabled)
    {
    
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE

        /* Calculate the starting index of the sector mapping cache for this sector entry.  */
        i =  (logical_sector & LX_NAND_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NAND_SECTOR_MAPPING_CACHE_DEPTH;

        /* Build a pointer to the cache entry.  */
        sector_mapping_cache_entry_ptr =  &nand_flash -> lx_nand_flash_sector_mapping_cache[i];

        /* Determine if the sector is in the sector mapping cache - assuming the depth of the sector 
           mapping cache is LX_NAND_SECTOR_MAPPING_CACHE_DEPTH entries.  */
        if ((sector_mapping_cache_entry_ptr -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Move all cache entries up and invalidate the last entry.  */
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 3);
            
            /* Invalidate the last entry.  */
            (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector =   0;
        }
        else if (((sector_mapping_cache_entry_ptr + 1) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Move all subsequent cache entries up and invalidate the last entry.  */
            *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 3);
            
            /* Invalidate the last entry.  */
            (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector =   0;
        }
        else if (((sector_mapping_cache_entry_ptr + 2) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Move all subsequent cache entries up and invalidate the last entry.  */
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 3);
            
            /* Invalidate the last entry.  */
            (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector =   0;
        }
        else if (((sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector) == (logical_sector | LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Simply invalidate the last entry.  */
            (sector_mapping_cache_entry_ptr + 3) -> lx_nand_sector_mapping_cache_logical_sector =   0;
        }
#else

        /* Direct mapping cache is defined.  */
    
        /* Determine if this logical sector fits in the logical sector cache mapping.  */
        if (logical_sector < LX_NAND_SECTOR_MAPPING_CACHE_SIZE)
        {

            /* Yes, invalidate the logical sector cache.  */
            nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_block =  0;    
            nand_flash -> lx_nand_flash_sector_mapping_cache[logical_sector].lx_nand_sector_mapping_cache_page =   0;
        }
#endif
    }
}

