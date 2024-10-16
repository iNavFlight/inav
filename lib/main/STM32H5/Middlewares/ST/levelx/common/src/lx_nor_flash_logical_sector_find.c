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
/**   NOR Flash                                                           */
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
/*    _lx_nor_flash_logical_sector_find                   PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find the specified logical sector in      */ 
/*    the NOR flash.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    logical_sector                        Logical sector number         */ 
/*    superceded_check                      Check for sector being        */ 
/*                                            superceded (can happen if   */ 
/*                                            on interruptions of sector  */ 
/*                                            write)                      */ 
/*    physical_sector_map_entry             Destination for physical      */ 
/*                                            sector map entry address    */ 
/*    physical_sector_address               Destination for physical      */ 
/*                                            sector data                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nor_flash_driver_read             Driver flash sector read      */ 
/*    _lx_nor_flash_system_error            Internal system error handler */ 
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
UINT  _lx_nor_flash_logical_sector_find(LX_NOR_FLASH *nor_flash, ULONG logical_sector, ULONG superceded_check, ULONG **physical_sector_map_entry, ULONG **physical_sector_address)
{

ULONG                               *block_word_ptr;
ULONG                               *list_word_ptr;
ULONG                               list_word;
ULONG                               min_logical_sector;
ULONG                               max_logical_sector;
ULONG                               mapped_sectors;
ULONG                               total_blocks;
ULONG                               total_sectors;
ULONG                               i, j;
ULONG                               search_start;
LX_NOR_SECTOR_MAPPING_CACHE_ENTRY   *sector_mapping_cache_entry_ptr = LX_NULL;
LX_NOR_SECTOR_MAPPING_CACHE_ENTRY   temp_sector_mapping_cache_entry;
#ifndef LX_DIRECT_READ
UINT                                status;
#endif


    /* Initialize the return parameters.  */
    *physical_sector_map_entry =  (ULONG *) 0;
    *physical_sector_address =    (ULONG *) 0;
    
    /* Determine if there are any mapped physical sectors.  */
    if (nor_flash -> lx_nor_flash_mapped_physical_sectors == 0)
    {

        /* No mapped sector so nothing can be found!.  */
        return(LX_SECTOR_NOT_FOUND);
    }

    /* Determine if the sector mapping cache is enabled.  */
    if (nor_flash -> lx_nor_flash_sector_mapping_cache_enabled)
    {
    
        /* Calculate the starting index of the sector cache for this sector entry.  */
        i =  (logical_sector & LX_NOR_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NOR_SECTOR_MAPPING_CACHE_DEPTH;

        /* Build a pointer to the cache entry.  */
        sector_mapping_cache_entry_ptr =  &nor_flash -> lx_nor_flash_sector_mapping_cache[i];

        /* Determine if the sector is in the sector mapping cache - assuming the depth of the sector 
           mapping cache is LX_NOR_SECTOR_MAPPING_CACHE_DEPTH entries.  */
        if ((sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_logical_sector) == (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nor_flash -> lx_nor_flash_sector_mapping_cache_hits++;

            /* Yes, return the cached values associated with the sector.  */
            *physical_sector_map_entry =  sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_map_entry;
            *physical_sector_address =    sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_address;

            /* Don't move anything since we found the entry at the top.  */

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 1) -> lx_nor_sector_mapping_cache_logical_sector) == (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nor_flash -> lx_nor_flash_sector_mapping_cache_hits++;

            /* Yes, return the cached values associated with the sector.  */
            *physical_sector_map_entry =  (sector_mapping_cache_entry_ptr + 1) -> lx_nor_sector_mapping_cache_physical_sector_map_entry;
            *physical_sector_address =    (sector_mapping_cache_entry_ptr + 1) -> lx_nor_sector_mapping_cache_physical_sector_address;

            /* Just swap the first and second entry.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 2) -> lx_nor_sector_mapping_cache_logical_sector) == (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nor_flash -> lx_nor_flash_sector_mapping_cache_hits++;

            /* Yes, return the cached value.  */
            *physical_sector_map_entry =  (sector_mapping_cache_entry_ptr + 2) -> lx_nor_sector_mapping_cache_physical_sector_map_entry;
            *physical_sector_address =    (sector_mapping_cache_entry_ptr + 2) -> lx_nor_sector_mapping_cache_physical_sector_address;

            /* Move the third entry to the top and the first two entries down.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
        else if (((sector_mapping_cache_entry_ptr + 3) -> lx_nor_sector_mapping_cache_logical_sector) == (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID))
        {

            /* Increment the sector mapping cache hit counter.  */
            nor_flash -> lx_nor_flash_sector_mapping_cache_hits++;

            /* Yes, return the cached value.  */
            *physical_sector_map_entry =  (sector_mapping_cache_entry_ptr + 3) -> lx_nor_sector_mapping_cache_physical_sector_map_entry;
            *physical_sector_address =    (sector_mapping_cache_entry_ptr + 3) -> lx_nor_sector_mapping_cache_physical_sector_address;

            /* Move the last entry to the top and the first three entries down.  */
            temp_sector_mapping_cache_entry =        *(sector_mapping_cache_entry_ptr);
            *(sector_mapping_cache_entry_ptr) =      *(sector_mapping_cache_entry_ptr + 3);
            *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  temp_sector_mapping_cache_entry;

            /* Return a successful status.  */
            return(LX_SUCCESS);
        }
    
        /* If we get here, we have a cache miss so increment the counter before we fall through the loop.  */
        nor_flash -> lx_nor_flash_sector_mapping_cache_misses++;
    }

    /* Setup the total number of mapped sectors.  */
    mapped_sectors =  nor_flash -> lx_nor_flash_mapped_physical_sectors;

    /* Start searching from the last found block.  */
    i =  nor_flash -> lx_nor_flash_found_block_search;

    /* Setup the starting sector to look at.  */
    j =  nor_flash -> lx_nor_flash_found_sector_search;

    /* Pickup the total number of blocks.  */
    total_blocks =  nor_flash -> lx_nor_flash_total_blocks;

    /* Loop through the blocks to attempt to find the mapped logical sector.  */
    while (total_blocks--) 
    {

        /* Setup the block word pointer to the first word of the search block.  */
        block_word_ptr =  (nor_flash -> lx_nor_flash_base_address + (i * nor_flash -> lx_nor_flash_words_per_block));

        /* Determine if the minimum and maximum logical sector values are present in the block header.  If these are 
           present, we can quickly skip blocks that don't have our sector.  */

        /* Read the minimum and maximum logical sector values in this block.  */
#ifdef LX_DIRECT_READ
        
        /* Read the word directly.  */
        min_logical_sector =  *(block_word_ptr + LX_NOR_FLASH_MIN_LOGICAL_SECTOR_OFFSET);
#else
        status =  _lx_nor_flash_driver_read(nor_flash, block_word_ptr + LX_NOR_FLASH_MIN_LOGICAL_SECTOR_OFFSET, &min_logical_sector, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
#endif
#ifdef LX_DIRECT_READ
        
        /* Read the word directly.  */
        max_logical_sector =  *(block_word_ptr + LX_NOR_FLASH_MAX_LOGICAL_SECTOR_OFFSET);
#else
        status =  _lx_nor_flash_driver_read(nor_flash, block_word_ptr + LX_NOR_FLASH_MAX_LOGICAL_SECTOR_OFFSET, &max_logical_sector, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
#endif

        /* Are the values valid?  */
        if ((min_logical_sector != LX_ALL_ONES) && (max_logical_sector != LX_ALL_ONES))
        {

            /* Now let's check to see if the search sector is within this range.  */
            if ((logical_sector < min_logical_sector) || (logical_sector > max_logical_sector))
            {

                /* Move to the next block.  */
                i++;
      
                /* Determine if we have wrapped.  */
                if (i >= nor_flash -> lx_nor_flash_total_blocks)
                {
        
                    /* Yes, we have wrapped, set to block 0.  */
                    i =  0;
                }

                /* Start at the first sector in the next block.  */
                j =  0;
              
                /* No point in looking further into this block, just continue the loop.  */
                continue;            
            }
        }
       
        /* Setup the total number of sectors.  */
        total_sectors =  nor_flash -> lx_nor_flash_physical_sectors_per_block;
        
        /* Remember the start of the search.  */
        search_start =  j;
        
        /* Now search through the sector list to find a match.  */
        while (total_sectors--)
        {

            /* Setup a pointer to the mapped list.  */
            list_word_ptr =  block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset + j;

            
            /* Read in the mapped list for this block.  */
#ifdef LX_DIRECT_READ
        
            /* Read the word directly.  */
            list_word =  *(list_word_ptr);
#else
            status =  _lx_nor_flash_driver_read(nor_flash, list_word_ptr, &list_word, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }
#endif
            
            /* Determine if the entry hasn't been used.  */
            if (list_word == LX_NOR_PHYSICAL_SECTOR_FREE)
            {
                
                /* Since the mapping is done sequentially in the block, we know nothing
                   else exists after this point.  */
              
                /* Determine if the search started at the beginning of the block.  */
                if (search_start == 0)
                {
                 
                    /* Yes, we started at the beginning of the block.  We are now done with this block. */
                    break;
                }
                else
                {
              
                    /* Setup the new total to the search start.  */
                    total_sectors =  search_start;
                    
                    /* Clear search start.  */
                    search_start =  0;
                    
                    /* Start search over.  */
                    j =  0;
                    continue;
                }
            }
            
            /* Is this entry valid?  */
            if ((list_word & (LX_NOR_PHYSICAL_SECTOR_VALID | LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID)) == LX_NOR_PHYSICAL_SECTOR_VALID)
            {
                
                /* Decrement the number of mapped sectors.  */
                mapped_sectors--;    

                /* Do we have a valid sector match?  */
                if ((list_word & LX_NOR_LOGICAL_SECTOR_MASK) == logical_sector)
                {

                    /* Determine if we care about the superceded bit.  */
                    if (superceded_check == LX_FALSE)
                    {
                                    
                        /* Prepare the return information.  */
                        *physical_sector_map_entry =  list_word_ptr;
                        *physical_sector_address =    block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_offset + (j * LX_NOR_SECTOR_SIZE);

                        /* Determine if the sector mapping cache is enabled.  */
                        if (nor_flash -> lx_nor_flash_sector_mapping_cache_enabled)
                        {

                            /* Yes, update the cache with the sector mapping.  */
                            
                            /* Move all the cache entries down so the oldest is at the bottom.  */
                            *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
                            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
                            *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr);

                            /* Setup the new sector information in the cache.  */
                            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_logical_sector =             (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID);
                            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_map_entry =  *physical_sector_map_entry;
                            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_address =    *physical_sector_address;
                        }

                        /* Remember the last found block for next search.  */
                        nor_flash -> lx_nor_flash_found_block_search =  i;
                        
                        /* Remember the last found sector.  */
                        nor_flash -> lx_nor_flash_found_sector_search =  j+1;
                        
                        /* Has this wrapped around?  */
                        if (nor_flash -> lx_nor_flash_found_sector_search >= nor_flash -> lx_nor_flash_physical_sectors_per_block)
                        {
                        
                            /* Reset to the beginning sector.  */
                            nor_flash -> lx_nor_flash_found_sector_search =  0;
                        }

                        /* Return success!  */
                        return(LX_SUCCESS);                     
                    }

                    /* Check for the superceded bit being clear, which means the sector was superceded.  */
                    else if (list_word & LX_NOR_PHYSICAL_SECTOR_SUPERCEDED)
                    {
                        
                        /* Prepare the return information.  */
                        *physical_sector_map_entry =  list_word_ptr;
                        *physical_sector_address =    block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_offset + (j * LX_NOR_SECTOR_SIZE);

                        /* No need to update the cache here, since this condition only happens during initialization.  */

                        /* Remember the last found block for next search.  */
                        nor_flash -> lx_nor_flash_found_block_search =  i;

                        /* Remember the last found sector.  */
                        nor_flash -> lx_nor_flash_found_sector_search =  j+1;
                        
                        /* Has this wrapped around?  */
                        if (nor_flash -> lx_nor_flash_found_sector_search >= nor_flash -> lx_nor_flash_physical_sectors_per_block)
                        {
                        
                            /* Reset to the beginning sector.  */
                            nor_flash -> lx_nor_flash_found_sector_search =  0;
                        }

                        /* Return success!  */
                        return(LX_SUCCESS);                     
                    }
                }
            }

            /* Move to the next list entry.  */
            j++;
            
            /* Check for wrap around.  */
            if (j >= nor_flash -> lx_nor_flash_physical_sectors_per_block)
            {
            
                /* Yes, wrap around, go back to the beginning.  */
                j =  0;
            }
        }

        /* Determine if there are any more mapped sectors.  */
        if (mapped_sectors == 0)
            break;
            
        /* Move to the next block.  */
        i++;
       
        /* Determine if we have wrapped.  */
        if (i >= nor_flash -> lx_nor_flash_total_blocks)
        {
        
            /* Yes, we have wrapped, set to block 0.  */
            i =  0;
        }

        /* Start at the first sector in the next block.  */
        j =  0;
    }

    /* Return sector not found status.  */
    return(LX_SECTOR_NOT_FOUND);  
}

