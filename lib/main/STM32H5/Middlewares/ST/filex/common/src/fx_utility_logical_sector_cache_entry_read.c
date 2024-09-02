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
/** FileX Component                                                       */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_logical_sector_cache_entry_read         PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles logical sector cache read requests for the    */
/*    logical sector read function. If the function finds the requested   */
/*    sector in the cache, it setup the appropriate pointers and          */
/*    returns a FX_NULL.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Logical sector number         */
/*    previous_cache_entry                  Pointer to previous entry in  */
/*                                            non-hashed cache            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_CACHED_SECTOR *                    Cache entry to setup          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_utility_logical_sector_read       Logical sector read function  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable cache,              */
/*                                            resulting in version 6.1    */
/*  01-31-2022     William E. Lamie         Modified comment(s), fixed    */
/*                                            errors without cache,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
FX_CACHED_SECTOR  *_fx_utility_logical_sector_cache_entry_read(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                                               FX_CACHED_SECTOR **previous_cache_entry)
{

#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry;
FX_CACHED_SECTOR  temp_storage;
ULONG             cache_size;
ULONG             index;


    /* Determine if the logical sector cache access should use the hash function.  */
    if (media_ptr -> fx_media_sector_cache_hashed)
    {

        /* Calculate the area of the cache for this logical sector.  */

        /* First compute the hashed value of this index by simply using the lower bits of
           the sector number.  */
        index =  (ULONG)(logical_sector & media_ptr -> fx_media_sector_cache_hash_mask);

        /* Set the bit indicating there is one or more valid sectors at this cache index.  */
        media_ptr -> fx_media_sector_cache_hashed_sector_valid |=  ((ULONG)1) << (index % 32);

        /* Compute the actual array index by multiplying by the cache depth.  */
        index =  index * FX_SECTOR_CACHE_DEPTH;

        /* Build a pointer to the cache entry.  */
        cache_entry =  &(media_ptr -> fx_media_sector_cache[index]);

        /* Determine if the logical sector is in the cache - assuming the depth of the
           sector cache is 4 entries.  */
        if ((cache_entry -> fx_cached_sector_valid) && (cache_entry -> fx_cached_sector == logical_sector))
        {

            /* Yes, we found a match.  Simply setup the pointer to this
               buffer and return.  */
            media_ptr -> fx_media_memory_buffer =  cache_entry -> fx_cached_sector_memory_buffer;

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of logical sectors cache read hits.  */
            media_ptr -> fx_media_logical_sector_cache_read_hits++;
#endif
            /* Success, return to caller immediately!  */
            return(FX_NULL);
        }
        else if (((cache_entry + 1) -> fx_cached_sector_valid) && ((cache_entry + 1) -> fx_cached_sector == logical_sector))
        {

            /* Yes, we found a match.  Simply setup the pointer to this
               buffer and return.  */
            media_ptr -> fx_media_memory_buffer =  (cache_entry + 1) -> fx_cached_sector_memory_buffer;

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of logical sectors cache read hits.  */
            media_ptr -> fx_media_logical_sector_cache_read_hits++;
#endif

            /* Swap the first and second cache entries to keep the most recently used
               at the top.  */
            temp_storage.fx_cached_sector_memory_buffer =           (cache_entry) -> fx_cached_sector_memory_buffer;
            temp_storage.fx_cached_sector =                         (cache_entry) -> fx_cached_sector;
            temp_storage.fx_cached_sector_buffer_dirty =            (cache_entry) -> fx_cached_sector_buffer_dirty;
            temp_storage.fx_cached_sector_valid =                   (cache_entry) -> fx_cached_sector_valid;
            temp_storage.fx_cached_sector_type =                    (cache_entry) -> fx_cached_sector_type;

            (cache_entry) -> fx_cached_sector_memory_buffer =       (cache_entry + 1) -> fx_cached_sector_memory_buffer;
            (cache_entry) -> fx_cached_sector =                     (cache_entry + 1) -> fx_cached_sector;
            (cache_entry) -> fx_cached_sector_buffer_dirty =        (cache_entry + 1) -> fx_cached_sector_buffer_dirty;
            (cache_entry) -> fx_cached_sector_valid =               (cache_entry + 1) -> fx_cached_sector_valid;
            (cache_entry) -> fx_cached_sector_type =                (cache_entry + 1) -> fx_cached_sector_type;

            (cache_entry + 1) -> fx_cached_sector_memory_buffer =   temp_storage.fx_cached_sector_memory_buffer;
            (cache_entry + 1) -> fx_cached_sector =                 temp_storage.fx_cached_sector;
            (cache_entry + 1) -> fx_cached_sector_buffer_dirty =    temp_storage.fx_cached_sector_buffer_dirty;
            (cache_entry + 1) -> fx_cached_sector_valid =           temp_storage.fx_cached_sector_valid;
            (cache_entry + 1) -> fx_cached_sector_type =            temp_storage.fx_cached_sector_type;

            /* Success, return to caller immediately!  */
            return(FX_NULL);
        }
        else if (((cache_entry + 2) -> fx_cached_sector_valid) && ((cache_entry + 2) -> fx_cached_sector == logical_sector))
        {

            /* Yes, we found a match.  Simply setup the pointer to this
               buffer and return.  */
            media_ptr -> fx_media_memory_buffer =  (cache_entry + 2) -> fx_cached_sector_memory_buffer;

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of logical sectors cache read hits.  */
            media_ptr -> fx_media_logical_sector_cache_read_hits++;
#endif

            /* Move the third entry to the top and the first two entries down.  */
            temp_storage.fx_cached_sector_memory_buffer =           (cache_entry) -> fx_cached_sector_memory_buffer;
            temp_storage.fx_cached_sector =                         (cache_entry) -> fx_cached_sector;
            temp_storage.fx_cached_sector_buffer_dirty =            (cache_entry) -> fx_cached_sector_buffer_dirty;
            temp_storage.fx_cached_sector_valid =                   (cache_entry) -> fx_cached_sector_valid;
            temp_storage.fx_cached_sector_type =                    (cache_entry) -> fx_cached_sector_type;

            (cache_entry) -> fx_cached_sector_memory_buffer =       (cache_entry + 2) -> fx_cached_sector_memory_buffer;
            (cache_entry) -> fx_cached_sector =                     (cache_entry + 2) -> fx_cached_sector;
            (cache_entry) -> fx_cached_sector_buffer_dirty =        (cache_entry + 2) -> fx_cached_sector_buffer_dirty;
            (cache_entry) -> fx_cached_sector_valid =               (cache_entry + 2) -> fx_cached_sector_valid;
            (cache_entry) -> fx_cached_sector_type =                (cache_entry + 2) -> fx_cached_sector_type;

            (cache_entry + 2) -> fx_cached_sector_memory_buffer =   (cache_entry + 1) -> fx_cached_sector_memory_buffer;
            (cache_entry + 2) -> fx_cached_sector =                 (cache_entry + 1) -> fx_cached_sector;
            (cache_entry + 2) -> fx_cached_sector_buffer_dirty =    (cache_entry + 1) -> fx_cached_sector_buffer_dirty;
            (cache_entry + 2) -> fx_cached_sector_valid =           (cache_entry + 1) -> fx_cached_sector_valid;
            (cache_entry + 2) -> fx_cached_sector_type =            (cache_entry + 1) -> fx_cached_sector_type;

            (cache_entry + 1) -> fx_cached_sector_memory_buffer =   temp_storage.fx_cached_sector_memory_buffer;
            (cache_entry + 1) -> fx_cached_sector =                 temp_storage.fx_cached_sector;
            (cache_entry + 1) -> fx_cached_sector_buffer_dirty =    temp_storage.fx_cached_sector_buffer_dirty;
            (cache_entry + 1) -> fx_cached_sector_valid =           temp_storage.fx_cached_sector_valid;
            (cache_entry + 1) -> fx_cached_sector_type =            temp_storage.fx_cached_sector_type;

            /* Success, return to caller immediately!  */
            return(FX_NULL);
        }
        else if (((cache_entry + 3) -> fx_cached_sector_valid) && ((cache_entry + 3) -> fx_cached_sector == logical_sector))
        {

            /* Yes, we found a match.  Simply setup the pointer to this
               buffer and return.  */
            media_ptr -> fx_media_memory_buffer =  (cache_entry + 3) -> fx_cached_sector_memory_buffer;

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of logical sectors cache read hits.  */
            media_ptr -> fx_media_logical_sector_cache_read_hits++;
#endif

            /* Move the last entry to the top and the first three entries down.  */
            temp_storage.fx_cached_sector_memory_buffer =           (cache_entry) -> fx_cached_sector_memory_buffer;
            temp_storage.fx_cached_sector =                         (cache_entry) -> fx_cached_sector;
            temp_storage.fx_cached_sector_buffer_dirty =            (cache_entry) -> fx_cached_sector_buffer_dirty;
            temp_storage.fx_cached_sector_valid =                   (cache_entry) -> fx_cached_sector_valid;
            temp_storage.fx_cached_sector_type =                    (cache_entry) -> fx_cached_sector_type;

            (cache_entry) -> fx_cached_sector_memory_buffer =       (cache_entry + 3) -> fx_cached_sector_memory_buffer;
            (cache_entry) -> fx_cached_sector =                     (cache_entry + 3) -> fx_cached_sector;
            (cache_entry) -> fx_cached_sector_buffer_dirty =        (cache_entry + 3) -> fx_cached_sector_buffer_dirty;
            (cache_entry) -> fx_cached_sector_valid =               (cache_entry + 3) -> fx_cached_sector_valid;
            (cache_entry) -> fx_cached_sector_type =                (cache_entry + 3) -> fx_cached_sector_type;

            (cache_entry + 3) -> fx_cached_sector_memory_buffer =   (cache_entry + 2) -> fx_cached_sector_memory_buffer;
            (cache_entry + 3) -> fx_cached_sector =                 (cache_entry + 2) -> fx_cached_sector;
            (cache_entry + 3) -> fx_cached_sector_buffer_dirty =    (cache_entry + 2) -> fx_cached_sector_buffer_dirty;
            (cache_entry + 3) -> fx_cached_sector_valid =           (cache_entry + 2) -> fx_cached_sector_valid;
            (cache_entry + 3) -> fx_cached_sector_type =            (cache_entry + 2) -> fx_cached_sector_type;

            (cache_entry + 2) -> fx_cached_sector_memory_buffer =   (cache_entry + 1) -> fx_cached_sector_memory_buffer;
            (cache_entry + 2) -> fx_cached_sector =                 (cache_entry + 1) -> fx_cached_sector;
            (cache_entry + 2) -> fx_cached_sector_buffer_dirty =    (cache_entry + 1) -> fx_cached_sector_buffer_dirty;
            (cache_entry + 2) -> fx_cached_sector_valid =           (cache_entry + 1) -> fx_cached_sector_valid;
            (cache_entry + 2) -> fx_cached_sector_type =            (cache_entry + 1) -> fx_cached_sector_type;

            (cache_entry + 1) -> fx_cached_sector_memory_buffer =   temp_storage.fx_cached_sector_memory_buffer;
            (cache_entry + 1) -> fx_cached_sector =                 temp_storage.fx_cached_sector;
            (cache_entry + 1) -> fx_cached_sector_buffer_dirty =    temp_storage.fx_cached_sector_buffer_dirty;
            (cache_entry + 1) -> fx_cached_sector_valid =           temp_storage.fx_cached_sector_valid;
            (cache_entry + 1) -> fx_cached_sector_type =            temp_storage.fx_cached_sector_type;

            /* Success, return to caller immediately!  */
            return(FX_NULL);
        }

        /* At this point we have a cache miss.  We need to move all of the sectors down one slot, swapping
           the 4th entry with the first.  */
        temp_storage.fx_cached_sector_memory_buffer =           (cache_entry + 3) -> fx_cached_sector_memory_buffer;
        temp_storage.fx_cached_sector =                         (cache_entry + 3) -> fx_cached_sector;
        temp_storage.fx_cached_sector_buffer_dirty =            (cache_entry + 3) -> fx_cached_sector_buffer_dirty;
        temp_storage.fx_cached_sector_valid =                   (cache_entry + 3) -> fx_cached_sector_valid;
        temp_storage.fx_cached_sector_type =                    (cache_entry + 3) -> fx_cached_sector_type;

        (cache_entry + 3) -> fx_cached_sector_memory_buffer =   (cache_entry + 2) -> fx_cached_sector_memory_buffer;
        (cache_entry + 3) -> fx_cached_sector =                 (cache_entry + 2) -> fx_cached_sector;
        (cache_entry + 3) -> fx_cached_sector_buffer_dirty =    (cache_entry + 2) -> fx_cached_sector_buffer_dirty;
        (cache_entry + 3) -> fx_cached_sector_valid =           (cache_entry + 2) -> fx_cached_sector_valid;
        (cache_entry + 3) -> fx_cached_sector_type =            (cache_entry + 2) -> fx_cached_sector_type;

        (cache_entry + 2) -> fx_cached_sector_memory_buffer =   (cache_entry + 1) -> fx_cached_sector_memory_buffer;
        (cache_entry + 2) -> fx_cached_sector =                 (cache_entry + 1) -> fx_cached_sector;
        (cache_entry + 2) -> fx_cached_sector_buffer_dirty =    (cache_entry + 1) -> fx_cached_sector_buffer_dirty;
        (cache_entry + 2) -> fx_cached_sector_valid =           (cache_entry + 1) -> fx_cached_sector_valid;
        (cache_entry + 2) -> fx_cached_sector_type =            (cache_entry + 1) -> fx_cached_sector_type;

        (cache_entry + 1) -> fx_cached_sector_memory_buffer =   (cache_entry) -> fx_cached_sector_memory_buffer;
        (cache_entry + 1) -> fx_cached_sector =                 (cache_entry) -> fx_cached_sector;
        (cache_entry + 1) -> fx_cached_sector_buffer_dirty =    (cache_entry) -> fx_cached_sector_buffer_dirty;
        (cache_entry + 1) -> fx_cached_sector_valid =           (cache_entry) -> fx_cached_sector_valid;
        (cache_entry + 1) -> fx_cached_sector_type =            (cache_entry) -> fx_cached_sector_type;

        (cache_entry) -> fx_cached_sector_memory_buffer =       temp_storage.fx_cached_sector_memory_buffer;
        (cache_entry) -> fx_cached_sector =                     temp_storage.fx_cached_sector;
        (cache_entry) -> fx_cached_sector_buffer_dirty =        temp_storage.fx_cached_sector_buffer_dirty;
        (cache_entry) -> fx_cached_sector_valid =               temp_storage.fx_cached_sector_valid;
        (cache_entry) -> fx_cached_sector_type =                temp_storage.fx_cached_sector_type;

        /* Set the previous pointer to NULL to avoid the linked list update below.  */
        *previous_cache_entry =  FX_NULL;
    }
    else
    {

        /* Search for an entry in the cache that matches this request.  */
        cache_size =            media_ptr -> fx_media_sector_cache_size;
        cache_entry =           media_ptr -> fx_media_sector_cache_list_ptr;
        *previous_cache_entry =  FX_NULL;

        /* Look at the cache entries until a match is found or the end of
           the cache is reached.  */
        while (cache_size--)
        {

            /* Determine if the requested sector has been found.  */
            if ((cache_entry -> fx_cached_sector_valid) && (cache_entry -> fx_cached_sector == logical_sector))
            {

                /* Yes, we found a match.  Simply setup the pointer to this
                   buffer and return.  */
                media_ptr -> fx_media_memory_buffer =  cache_entry -> fx_cached_sector_memory_buffer;

                /* Determine if we need to update the last used list.  */
                if (*previous_cache_entry)
                {

                    /* Yes, the current entry is not at the front of the list
                       so we need to change the order.  */

                    /* Link the previous entry to this entry's next pointer.  */
                    (*previous_cache_entry) -> fx_cached_sector_next_used =
                        cache_entry -> fx_cached_sector_next_used;

                    /* Place this entry at the head of the list.  */
                    cache_entry -> fx_cached_sector_next_used =
                        media_ptr -> fx_media_sector_cache_list_ptr;
                    media_ptr -> fx_media_sector_cache_list_ptr =  cache_entry;
                }

#ifndef FX_MEDIA_STATISTICS_DISABLE

                /* Increment the number of logical sectors cache read hits.  */
                media_ptr -> fx_media_logical_sector_cache_read_hits++;
#endif

                /* Success, return to caller immediately!  */
                return(FX_NULL);
            }

            /* Otherwise, we have not found the cached entry yet.  */

            /* If there are more entries, move to the next one.  */
            if (cache_entry -> fx_cached_sector_next_used)
            {

                *previous_cache_entry =  cache_entry;
                cache_entry =           cache_entry -> fx_cached_sector_next_used;
            }
        }
    }

    /* The requested sector is not in cache, return the last cache entry.  */
    return(cache_entry);
#else
    FX_PARAMETER_NOT_USED(media_ptr);
    FX_PARAMETER_NOT_USED(logical_sector);
    FX_PARAMETER_NOT_USED(previous_cache_entry);
    return(FX_NULL);
#endif /* FX_DISABLE_CACHE */
}

