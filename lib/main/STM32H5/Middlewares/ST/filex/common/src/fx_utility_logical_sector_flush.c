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
/*    _fx_utility_logical_sector_flush                    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles logical sector flush requests for all FileX   */
/*    components. It will process all dirty logical sectors in the        */
/*    logical sector cache within the range specified. This function      */
/*    optionally invalidates sectors.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    starting_sector                       Starting sector number        */
/*    sectors                               Number of sectors             */
/*    invalidate                            Invalidate flag               */
/*                                            (FX_TRUE -> invalidate)     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    I/O Driver                                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
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
UINT  _fx_utility_logical_sector_flush(FX_MEDIA *media_ptr, ULONG64 starting_sector, ULONG64 sectors, UINT invalidate)
{

#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry;
UINT              cache_size;
UINT              i, bit_set, use_starting_sector;
ULONG             index;
ULONG             remaining_valid;
ULONG             remaining_dirty;
ULONG64           ending_sector;
ULONG             valid_bit_map;


    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_UTILITY_LOGICAL_SECTOR_FLUSH_EXTENSION

    /* Calculate the ending sector.  */
    ending_sector =  starting_sector + sectors - 1;

    /* Pickup the number of dirty sectors currently in the cache.  */
    remaining_dirty =  media_ptr -> fx_media_sector_cache_dirty_count;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_MEDIA_FLUSH, media_ptr, media_ptr -> fx_media_sector_cache_dirty_count, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Determine what type of cache configuration we have.  */
    if (media_ptr -> fx_media_sector_cache_hashed == FX_FALSE)
    {

        /* Linear cache present, simply walk through the search list until
           an unused cache entry is present.  */

        /* Flush and invalidate the internal logical sector cache.  */
        cache_size =            media_ptr -> fx_media_sector_cache_size;
        cache_entry =           media_ptr -> fx_media_sector_cache_list_ptr;

        /* Look at the cache entries that have been written to.  */
        while ((cache_size--) && (cache_entry -> fx_cached_sector))
        {

            /* Determine if invalidation is not required and there are no
               more dirty sectors. */
            if ((remaining_dirty == 0) && (invalidate == FX_FALSE))
            {

                /* Yes, nothing left to do.  */
                break;
            }

            /* Determine if there are any more sectors to process.  */
            if (sectors == 0)
            {

                /* No more sectors required to process.  */
                break;
            }

            /* Determine if this cached sector is within the specified range and is valid.  */
            if ((cache_entry -> fx_cached_sector_valid) &&
                (cache_entry -> fx_cached_sector >= starting_sector) &&
                (cache_entry -> fx_cached_sector <= ending_sector))
            {

                /* Yes, the cache entry is valid and within the specified range. Determine if
                   the requested sector has been written to.  */
                if (cache_entry -> fx_cached_sector_buffer_dirty)
                {

                    /* Yes, write the cached sector out to the media.  */

                    /* Check for write protect at the media level (set by driver).  */
                    if (media_ptr -> fx_media_driver_write_protect == FX_FALSE)
                    {

#ifndef FX_MEDIA_STATISTICS_DISABLE

                        /* Increment the number of driver write sector(s) requests.  */
                        media_ptr -> fx_media_driver_write_requests++;
#endif

                        /* Build write request to the driver.  */
                        media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
                        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
                        media_ptr -> fx_media_driver_buffer =           cache_entry -> fx_cached_sector_memory_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
                        media_ptr -> fx_media_driver_logical_sector =   cache_entry -> fx_cached_sector;
#else
                        media_ptr -> fx_media_driver_logical_sector =   (ULONG)cache_entry -> fx_cached_sector;
#endif
                        media_ptr -> fx_media_driver_sectors =          1;
                        media_ptr -> fx_media_driver_sector_type =      cache_entry -> fx_cached_sector_type;

                        /* Sectors other than FX_DATA_SECTOR will never be dirty when FX_FAULT_TOLERANT is defined. */
#ifndef FX_FAULT_TOLERANT
                        /* Determine if the system write flag needs to be set.  */
                        if (cache_entry -> fx_cached_sector_type != FX_DATA_SECTOR)
                        {

                            /* Yes, a system sector write is present so set the flag.  The driver
                               can use this flag to make extra safeguards in writing the sector
                               out, yielding more fault tolerance.  */
                            media_ptr -> fx_media_driver_system_write =  FX_TRUE;
                        }
#endif /* FX_FAULT_TOLERANT */

                        /* If trace is enabled, insert this event into the trace buffer.  */
                        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, cache_entry -> fx_cached_sector, 1, cache_entry -> fx_cached_sector_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                        /* Invoke the driver to write the sector.  */
                        (media_ptr -> fx_media_driver_entry) (media_ptr);

                        /* Clear the system write flag.  */
                        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

                        /* Check for successful completion.  */
                        if (media_ptr -> fx_media_driver_status)
                        {

                            /* Error writing a cached sector out.  Return the
                               error status.  */
                            return(media_ptr -> fx_media_driver_status);
                        }

                        /* Clear the buffer dirty flag since it has been flushed
                           out.  */
                        cache_entry -> fx_cached_sector_buffer_dirty =  FX_FALSE;

                        /* Decrement the number of dirty sectors currently in the cache.  */
                        media_ptr -> fx_media_sector_cache_dirty_count--;
                        remaining_dirty--;
                    }
                }

                /* Determine if the invalidate option is specified.  */
                if (invalidate)
                {

                    /* Invalidate the cache entry.  */
                    cache_entry -> fx_cached_sector_valid =  FX_FALSE;

                    /* Place all ones in the sector number.  */
                    cache_entry -> fx_cached_sector =  (~(ULONG64)0);

                    /* Determine if this sector is still dirty, this could be the case if
                       write protection was turned on.  */
                    if (cache_entry -> fx_cached_sector_buffer_dirty)
                    {

                        /* Yes, clear the dirty flag.  */
                        cache_entry -> fx_cached_sector_buffer_dirty =  FX_FALSE;

                        /* Decrement the number of dirty sectors currently in the cache.  */
                        media_ptr -> fx_media_sector_cache_dirty_count--;
                        remaining_dirty--;
                    }
                }

                /* Decrement the number of sectors in the range that have been processed.  */
                sectors--;
            }

            /* Move to the next entry in the sector cache.  */
            cache_entry =  cache_entry -> fx_cached_sector_next_used;
        }
    }
    else
    {

        /* Hashed cache is present. Pickup the cache size.  */
        cache_size =            media_ptr -> fx_media_sector_cache_size;

        /* Initialize the loop control parameters.  */
        bit_set =  0;
        valid_bit_map =  media_ptr -> fx_media_sector_cache_hashed_sector_valid;

        /* Determine how to process the hashed cache based on the number of sectors
           to process. If the sequential sector range is less than the bit map size,
           simply use the starting sector to derive the index into the cache.  */
        if (sectors < 32)
        {
            use_starting_sector =  FX_TRUE;
        }
        else
        {
            use_starting_sector =  FX_FALSE;
        }

        /* Determine if there is anything valid in the cache.  */
        while (valid_bit_map)
        {

            /* Determine if invalidation is not required and there are no
               more dirty sectors. */
            if ((remaining_dirty == 0) && (invalidate == FX_FALSE))
            {

                /* Yes, nothing left to do.  */
                break;
            }

            /* Determine if there are any more sectors to process.  */
            if ((sectors == 0) || (starting_sector > ending_sector))
            {

                /* No more sectors required to process.  */
                break;
            }

            /* Determine how to compute the hash index.  */
            if (use_starting_sector)
            {

                /* Calculate the hash value of this sector using the lower bits.  */
                index =  (ULONG)(starting_sector & media_ptr -> fx_media_sector_cache_hash_mask);

                /* Calculate the bit set indicating there is one or more valid sectors at this cache index.  */
                bit_set =  (index % 32);

                /* Compute the actual array index by multiplying by the cache depth.  */
                index =  (bit_set * FX_SECTOR_CACHE_DEPTH);
            }
            else
            {

                /* Walk the bit map to find the next valid entry.  */

                /* Find the next set bit.  */
                while ((valid_bit_map & 1) == 0)
                {

                    /* Otherwise, shift down the bit in the bit map.  */
                    valid_bit_map =  valid_bit_map >> 1;

                    /* Increment the set bit marker.  */
                    bit_set++;
                }

                /* Compute the first actual index into the hashed cache.  */
                index =  (bit_set * FX_SECTOR_CACHE_DEPTH);
            }

            /* At this point, bit_set represents the next group of hashed sectors that could
               have valid cache entries and index represents the index into the sector cache
               of that sector group.  */

            /* Clear the remaining valid sectors for this entry in the bit map.  */
            remaining_valid =  0;

            /* Loop to check the corresponding hash entries.  */
            do
            {

                /* Setup pointer to the cache entry.  */
                cache_entry =  &(media_ptr -> fx_media_sector_cache[index]);

                /* Loop to examine the full depth of the hashed cache.  */
                for (i = 0; i < 4; i++)
                {

                    /* Determine if this cached sector is within the specified range and is valid.  */
                    if ((cache_entry -> fx_cached_sector_valid) &&
                        (cache_entry -> fx_cached_sector >= starting_sector) &&
                        (cache_entry -> fx_cached_sector <= ending_sector))
                    {

                        /* Determine if the requested sector has been written to.  */
                        if (cache_entry -> fx_cached_sector_buffer_dirty)
                        {


                            /* Yes, write the cached sector out to the media.  */

                            /* Check for write protect at the media level (set by driver).  */
                            if (media_ptr -> fx_media_driver_write_protect == FX_FALSE)
                            {

#ifndef FX_MEDIA_STATISTICS_DISABLE

                                /* Increment the number of driver write sector(s) requests.  */
                                media_ptr -> fx_media_driver_write_requests++;
#endif

                                /* Build Write request to the driver.  */
                                media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
                                media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
                                media_ptr -> fx_media_driver_buffer =           cache_entry -> fx_cached_sector_memory_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
                                media_ptr -> fx_media_driver_logical_sector =   cache_entry -> fx_cached_sector;
#else
                                media_ptr -> fx_media_driver_logical_sector =   (ULONG)cache_entry -> fx_cached_sector;
#endif
                                media_ptr -> fx_media_driver_sectors =          1;
                                media_ptr -> fx_media_driver_sector_type =      cache_entry -> fx_cached_sector_type;

                                /* Sectors other than FX_DATA_SECTOR will never be dirty when FX_FAULT_TOLERANT is defined. */
#ifndef FX_FAULT_TOLERANT
                                /* Determine if the system write flag needs to be set.  */
                                if (cache_entry -> fx_cached_sector_type != FX_DATA_SECTOR)
                                {

                                    /* Yes, a system sector write is present so set the flag.  The driver
                                       can use this flag to make extra safeguards in writing the sector
                                       out, yielding more fault tolerance.  */
                                    media_ptr -> fx_media_driver_system_write =  FX_TRUE;
                                }
#endif /* FX_FAULT_TOLERANT */

                                /* If trace is enabled, insert this event into the trace buffer.  */
                                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, cache_entry -> fx_cached_sector, 1, cache_entry -> fx_cached_sector_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                                /* Invoke the driver to write the sector.  */
                                (media_ptr -> fx_media_driver_entry) (media_ptr);

                                /* Clear the system write flag.  */
                                media_ptr -> fx_media_driver_system_write =  FX_FALSE;

                                /* Check for successful completion.  */
                                if (media_ptr -> fx_media_driver_status)
                                {

                                    /* Error writing a cached sector out.  Return the
                                       error status.  */
                                    return(media_ptr -> fx_media_driver_status);
                                }

                                /* Clear the buffer dirty flag since it has been flushed
                                   out.  */
                                cache_entry -> fx_cached_sector_buffer_dirty =  FX_FALSE;

                                /* Decrement the number of dirty sectors currently in the cache.  */
                                media_ptr -> fx_media_sector_cache_dirty_count--;
                                remaining_dirty--;
                            }
                        }

                        /* Determine if the invalidate option is specified.  */
                        if (invalidate)
                        {

                            /* Invalidate the cache entry.  */
                            cache_entry -> fx_cached_sector_valid =  FX_FALSE;

                            /* Place all ones in the sector number.  */
                            cache_entry -> fx_cached_sector =  (~(ULONG64)0);

                            /* Determine if this sector is still dirty, this could be the case if
                               write protection was turned on.  */
                            if (cache_entry -> fx_cached_sector_buffer_dirty)
                            {

                                /* Yes, clear the dirty flag.  */
                                cache_entry -> fx_cached_sector_buffer_dirty =  FX_FALSE;

                                /* Decrement the number of dirty sectors currently in the cache.  */
                                media_ptr -> fx_media_sector_cache_dirty_count--;
                                remaining_dirty--;
                            }
                        }

                        /* Decrement the number of sectors in the range that have been processed.  */
                        sectors--;
                    }
                    else
                    {

                        /* Determine if the sector is valid.  */
                        if (cache_entry -> fx_cached_sector_valid)
                        {

                            /* Increment the number of still remaining but out of range sectors.  */
                            remaining_valid++;
                        }
                    }

                    /* Determine if invalidation is not required and there are no
                       more dirty sectors. */
                    if ((remaining_dirty == 0) && (invalidate == FX_FALSE))
                    {

                        /* Yes, nothing left to do.  */
                        break;
                    }

                    /* Determine if there are any more sectors to process.  */
                    if ((sectors == 0) && (invalidate == FX_FALSE))
                    {

                        /* No more sectors required to process.  */
                        break;
                    }

                    /* Move to the next cache entry.  */
                    cache_entry++;
                }

                /* Move the index to the next position since the bit map can only represent 32
                   cache entries.  */
                index =  index + (32 * FX_SECTOR_CACHE_DEPTH);
            } while (index < cache_size);

            /* Determine if invalidation was required and there are no more valid sectors
               associated with this bit position.  */
            if ((invalidate) && (remaining_valid == 0))
            {

                /* Clear this bit position.  */
                media_ptr -> fx_media_sector_cache_hashed_sector_valid &=  ~(((ULONG)1) << bit_set);
            }

            /* Determine if the starting sector is being used for examination of the hash.  */
            if (use_starting_sector)
            {

                /* Move to the next sector.  */
                starting_sector++;
            }
            else
            {

                /* Move to next bit in the map.  */
                valid_bit_map =  valid_bit_map >> 1;

                /* Increment the set bit marker.  */
                bit_set++;
            }
        }
    }
#else
    FX_PARAMETER_NOT_USED(media_ptr);
    FX_PARAMETER_NOT_USED(starting_sector);
    FX_PARAMETER_NOT_USED(sectors);
    FX_PARAMETER_NOT_USED(invalidate);
#endif /* FX_DISABLE_CACHE */

    /* If we get here, return successful status to the caller.  */
    return(FX_SUCCESS);
}

