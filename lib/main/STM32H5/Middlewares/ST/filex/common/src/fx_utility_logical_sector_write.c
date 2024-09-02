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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_logical_sector_write                    PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles logical sector write requests for all FileX   */
/*    components.  If the logical sector is currently in the media's      */
/*    buffer supplied by the caller, the function simply marks the buffer */
/*    as written to.  Otherwise, physical I/O is requested through the    */
/*    corresponding I/O driver.                                           */
/*                                                                        */
/*    Note: Conversion of the logical sector is done inside the driver.   */
/*          This results in a performance boost for FLASH or RAM media    */
/*          devices.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Logical sector number         */
/*    buffer_ptr                            Pointer of sector buffer      */
/*    sectors                               Number of sectors to write    */
/*    sector_type                           Type of sector(s) to write    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_flush      Flush and invalidate sectors  */
/*                                          that overlap with non-cache   */
/*                                          sector I/O.                   */
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
/*  04-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            updated check for logical   */
/*                                            sector value,               */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_logical_sector_write(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                       VOID *buffer_ptr, ULONG sectors, UCHAR sector_type)
{

#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry;
UINT              cache_size;
UINT              index;
UINT              i;
UCHAR             cache_found = FX_FALSE;
#endif /* FX_DISABLE_CACHE */

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Determine if the request is for FAT sector.  */
    if (sector_type == FX_FAT_SECTOR)
    {

        /* Increment the number of FAT sector writes.  */
        media_ptr -> fx_media_fat_sector_writes++;
    }

    /* Increment the number of logical sectors written.  */
    media_ptr -> fx_media_logical_sector_writes++;
#endif

    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_UTILITY_LOGICAL_SECTOR_WRITE_EXTENSION

#ifndef FX_DISABLE_CACHE
    /* Determine if the request is from the internal media buffer area.  */
    if ((((UCHAR *)buffer_ptr) >= media_ptr -> fx_media_memory_buffer) &&
        (((UCHAR *)buffer_ptr) <= media_ptr -> fx_media_sector_cache_end))
    {

        /* Internal cache buffer is requested.  */

        /* Determine if the logical sector cache access should use the hash function.  */
        if (media_ptr -> fx_media_sector_cache_hashed)
        {

            /* Calculate the area of the cache for this logical sector.  */
            index =  (ULONG)(logical_sector & media_ptr -> fx_media_sector_cache_hash_mask) * FX_SECTOR_CACHE_DEPTH;

            /* Build a pointer to the cache entry.  */
            cache_entry =  &(media_ptr -> fx_media_sector_cache[index]);

            for (i = 0; i < FX_SECTOR_CACHE_DEPTH; i++, cache_entry++)
            {


                /* Determine if the logical sector is in the cache - assuming the depth of the
                   sector cache is 4 entries.  */
                if ((cache_entry -> fx_cached_sector_valid) && (cache_entry -> fx_cached_sector == logical_sector))
                {
                    cache_found = FX_TRUE;
                    break;
                }
            }
        }
        else
        {

            /* Search for an entry in the cache that matches this request.  */
            cache_size =            media_ptr -> fx_media_sector_cache_size;
            cache_entry =           media_ptr -> fx_media_sector_cache_list_ptr;

            /* Look at the cache entries until a match is found or the end of
               the cache is reached.  */
            while (cache_size--)
            {

                /* Determine if the requested sector has been found.  */
                if ((cache_entry -> fx_cached_sector_valid) && (cache_entry -> fx_cached_sector == logical_sector))
                {
                    cache_found = FX_TRUE;
                    break;
                }

                /* Otherwise, we have not found the cached entry yet.  */

                /* If there are more entries, move to the next one.  */
                if (cache_entry -> fx_cached_sector_next_used)
                {

                    /* Move to the next cache entry.  */
                    cache_entry =  cache_entry -> fx_cached_sector_next_used;
                }
            }
        }

#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled &&
            (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED) &&
            (sector_type == FX_DATA_SECTOR) &&
            !(cache_found && (cache_entry -> fx_cached_sector_memory_buffer == buffer_ptr)))
        {

            /* Special use case for file write when fault tolerant is enabled. */
            /* Data are read from one sector but write to another sector. */
            /* Need to invalidate both of original and new caches. */
            if (cache_found)
            {

                /* Invalidate the new cache. */
                cache_entry -> fx_cached_sector_valid = FX_FALSE;
                cache_found = FX_FALSE;
            }

            /* Search for original cache.  */
            cache_size =            media_ptr -> fx_media_sector_cache_size;
            cache_entry =           media_ptr -> fx_media_sector_cache_list_ptr;

            /* Look at the cache entries until a match is found or the end of
               the cache is reached.  */
            while (cache_size--)
            {

                /* Determine if the original sector has been found.  */
                if ((cache_entry -> fx_cached_sector_valid) &&
                    (cache_entry -> fx_cached_sector_memory_buffer == buffer_ptr))
                {

                    /* Invalidate the original cache. */
                    cache_entry -> fx_cached_sector_valid = FX_FALSE;
                    break;
                }

                /* Otherwise, we have not found the cached entry yet.  */

                /* If there are more entries, move to the next one.  */
                if (cache_entry -> fx_cached_sector_next_used)
                {

                    /* Move to the next cache entry.  */
                    cache_entry =  cache_entry -> fx_cached_sector_next_used;
                }
            }
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

        if (cache_found)
        {

            /* Yes, we found a match.  */

#ifdef FX_FAULT_TOLERANT

            /* Check for a system sector. Data sector fault tolerance is selected with
               the FX_FAULT_TOLERANT_DATA option.  */
            if (sector_type != FX_DATA_SECTOR)
            {

                /* With the fault tolerant option enabled, system sectors are written immediately to
                   the media.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

                /* Increment the number of driver write sector(s) requests.  */
                media_ptr -> fx_media_driver_write_requests++;
#endif

                /* Build write request to the driver.  */
                media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
                media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
                media_ptr -> fx_media_driver_buffer =           cache_entry -> fx_cached_sector_memory_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
                media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
                media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
                media_ptr -> fx_media_driver_sectors =          1;
                media_ptr -> fx_media_driver_sector_type =      sector_type;

                /* Yes, a system sector write is present so set the flag.  The driver
                   can use this flag to make extra safeguards in writing the sector
                   out, yielding more fault tolerance.  */
                media_ptr -> fx_media_driver_system_write =  FX_TRUE;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, logical_sector, 1, cache_entry -> fx_cached_sector_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Invoke the driver to write the sector(s).  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                /* Clear the system write flag.  */
                media_ptr -> fx_media_driver_system_write =  FX_FALSE;

                /* Return success.  */
                return(media_ptr -> fx_media_driver_status);
            }
#endif

            /* Determine if this is the first write of this logical sector.  */
            if (cache_entry -> fx_cached_sector_buffer_dirty == FX_FALSE)
            {

                /* Yes, increment the number of outstanding dirty sectors.  */
                media_ptr -> fx_media_sector_cache_dirty_count++;

                /* Simply mark this entry as dirty.  */
                cache_entry -> fx_cached_sector_buffer_dirty =  FX_TRUE;
            }

            /* Don't bother updating the cache linked list since writes are
               preceded by reads anyway.  */

            /* Success, return to caller immediately!  */
            return(FX_SUCCESS);
        }


        /* Okay, so if we are here the request must be for the additional FAT writes, since this is the
           only time a write request is made without a preceding read request.  */

        /* Is the logical sector valid?  */
        if ((logical_sector == 0) || (logical_sector == ((ULONG)0xFFFFFFFF)))
        {
            return(FX_SECTOR_INVALID);
        }

        /* Compare logical sector against total sectors to make sure it is valid.  */
        if ((logical_sector + sectors - 1) >= media_ptr -> fx_media_total_sectors)
        {
            return(FX_SECTOR_INVALID);
        }

        /* Just write the buffer to the media.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver write sector(s) requests.  */
        media_ptr -> fx_media_driver_write_requests++;
#endif

        /* Build write request to the driver.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           buffer_ptr;
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
        media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
        media_ptr -> fx_media_driver_sectors =          sectors;
        media_ptr -> fx_media_driver_sector_type =      sector_type;

        /* Determine if the system write flag needs to be set.  */
        if (sector_type != FX_DATA_SECTOR)
        {

            /* Yes, a system sector write is present so set the flag.  The driver
               can use this flag to make extra safeguards in writing the sector
               out, yielding more fault tolerance.  */
            media_ptr -> fx_media_driver_system_write =  FX_TRUE;
        }

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, logical_sector, sectors, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to write the sector(s).  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear the system write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Check for successful completion.  */
        if (media_ptr -> fx_media_driver_status)
        {

            /* Error writing a internal sector out.  Return the
               error status.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* At this point, we have a successful write.  */
        return(FX_SUCCESS);
    }
    else
#endif /* FX_DISABLE_CACHE */
    {

        /* Otherwise, the write request is being made directly from an application
           buffer. Determine if the logical sector is valid.  */

        /* Is the logical sector valid? */
        if ((logical_sector == 0) || (logical_sector == ((ULONG)0xFFFFFFFF)))
        {
            return(FX_SECTOR_INVALID);
        }

        /* Compare logical sector against total sectors to make sure it is valid.  */
        if ((logical_sector + sectors - 1) >= media_ptr -> fx_media_total_sectors)
        {
            return(FX_SECTOR_INVALID);
        }

        /* Flush and invalidate for any entries in the cache that are in this direct I/O read request range.  */
        _fx_utility_logical_sector_flush(media_ptr, logical_sector, (ULONG64) sectors, FX_TRUE);

#ifdef FX_DISABLE_CACHE
        if ((logical_sector <= media_ptr -> fx_media_memory_buffer_sector) && (logical_sector + sectors >= media_ptr -> fx_media_memory_buffer_sector))
        {
            media_ptr -> fx_media_memory_buffer_sector = (ULONG64)-1;
        }
#endif /* FX_DISABLE_CACHE */

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver write sector(s) requests.  */
        media_ptr -> fx_media_driver_write_requests++;
#endif

        /* Build request to the driver.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           buffer_ptr;
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
        media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
        media_ptr -> fx_media_driver_sectors =          sectors;
        media_ptr -> fx_media_driver_sector_type =      sector_type;

        /* Determine if the system write flag needs to be set.  */
        if (sector_type != FX_DATA_SECTOR)
        {

            /* Yes, a system sector write is present so set the flag.  The driver
               can use this flag to make extra safeguards in writing the sector
               out, yielding more fault tolerance.  */
            media_ptr -> fx_media_driver_system_write =  FX_TRUE;
        }

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, logical_sector, sectors, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to write the sector(s).  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear the system write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Return driver status.  */
        return(media_ptr -> fx_media_driver_status);
    }
}

