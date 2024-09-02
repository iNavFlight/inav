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
/*    _lx_nor_flash_extended_cache_read                  PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs a read of the NOR flash memory.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    flash_address                         Address of NOR flash to read  */ 
/*    destination                           Destination for the read      */ 
/*    words                                 Number of words to read       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nor_flash_driver_read)            Actual driver read            */ 
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
UINT  _lx_nor_flash_driver_read(LX_NOR_FLASH *nor_flash, ULONG *flash_address, ULONG *destination, ULONG words)
{
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE

UINT    status;
UINT    i;
ULONG   *cache_entry_start;
ULONG   *cache_entry_end;
ULONG   cache_offset;
UINT    least_used_cache_entry;


    /* Is the request a whole sector or a partial sector.  */
    if ((words == 1) && (nor_flash -> lx_nor_flash_extended_cache_entries))
    {

        /* One word request, which implies that it is a NOR flash metadata read.  */
            
            
        /* Initialize the least used cache entry.  */
        least_used_cache_entry =  0;
            
        do
        {    

            /* Loop through the cache entries to see if there is a sector in cache.  */
            for (i = 0; i < nor_flash -> lx_nor_flash_extended_cache_entries; i++)
            {
        
                /* Search through the cache to find the entry.  */
                
                /* Determine the cache entry addresses.  */
                cache_entry_start =  nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_address;
                cache_entry_end =    cache_entry_start + LX_NOR_SECTOR_SIZE;
                
                /* Determine if the flash address in in the cache entry.  */
                if ((cache_entry_start) && (flash_address >= cache_entry_start) && (flash_address < cache_entry_end))
                {
                
                    /* Yes, we found the entry.  */
                    
                    /* Increment the accessed count.  */
                    nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_access_count++;
                    
                    /* Calculate the offset into the cache entry.  */
                    cache_offset =  (ULONG)(flash_address - cache_entry_start);
                    
                    /* Copy the word from the cache.  */
                    *destination =  *(nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_memory + cache_offset);
                    
                    /* Increment the number of cache hits.  */
                    nor_flash -> lx_nor_flash_extended_cache_hits++;
                    
                    /* Return success.  */
                    return(LX_SUCCESS);                    
                }
                else
                {
                    
                    /* Determine if we have a new least used sector.  */
                    if (i != least_used_cache_entry)
                    {
                    
                        /* Determine if this entry has a smaller accessed count.  */
                        if (nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_access_count <
                            nor_flash -> lx_nor_flash_extended_cache[least_used_cache_entry].lx_nor_flash_extended_cache_entry_access_count)
                        {
                        
                            /* New least used entry.  */
                            least_used_cache_entry =  i;
                        }
                    }
                }
            }

            /* Now read in the sector into the cache.  */
            cache_offset =  (ULONG)(flash_address - nor_flash -> lx_nor_flash_base_address);
            cache_offset =  cache_offset & ~((ULONG) (LX_NOR_SECTOR_SIZE-1));
            cache_entry_start =  nor_flash -> lx_nor_flash_base_address + cache_offset;
            
            /* Call the actual driver read function.  */
            status =  (nor_flash -> lx_nor_flash_driver_read)(cache_entry_start, 
                            nor_flash -> lx_nor_flash_extended_cache[least_used_cache_entry].lx_nor_flash_extended_cache_entry_sector_memory, 
                            LX_NOR_SECTOR_SIZE);
            
            /* Determine if there was an error.  */
            if (status != LX_SUCCESS)
            {
            
                /* Return the error to the caller.  */
                return(status);
            }
            
            /* Setup the cache entry.  */
            nor_flash -> lx_nor_flash_extended_cache[least_used_cache_entry].lx_nor_flash_extended_cache_entry_sector_address =  cache_entry_start;
            nor_flash -> lx_nor_flash_extended_cache[least_used_cache_entry].lx_nor_flash_extended_cache_entry_access_count =    0;
            
            /* Increment the number of cache misses.  */
            nor_flash -> lx_nor_flash_extended_cache_misses++;
            
            /* Decrement the number of cache hits, so that the increment that will happen next will be cancelled out.  */
            nor_flash -> lx_nor_flash_extended_cache_hits--;
            
        } while (status == LX_SUCCESS);
        
        /* Return success.  */
        return(LX_SUCCESS);
    }
    else
    {
    
        /* Call the actual driver read function.  */
        status =  (nor_flash -> lx_nor_flash_driver_read)(flash_address, destination, words);

        /* Return completion status.  */
        return(status);   
    }
#else
UINT    status;


    /* Call the actual driver read function.  */
    status =  (nor_flash -> lx_nor_flash_driver_read)(flash_address, destination, words);

    /* Return completion status.  */
    return(status);   
#endif
}


