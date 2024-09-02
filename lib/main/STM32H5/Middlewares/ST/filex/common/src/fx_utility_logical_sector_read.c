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
/*    _fx_utility_logical_sector_read                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles logical sector read requests for all FileX    */
/*    components.  If the logical sector is currently in the logical      */
/*    sector cache, the function simply sets the appropriate pointer and  */
/*    returns a successful status to the caller.  Otherwise, physical I/O */
/*    is requested through the corresponding I/O driver.                  */
/*                                                                        */
/*    Note: Conversion of the logical sector is done inside the driver.   */
/*          This results in a performance boost for FLASH or RAM media    */
/*          devices.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Logical sector number         */
/*    buffer_ptr                            Pointer of receiving buffer   */
/*    sectors                               Number of sectors to read     */
/*    sector_type                           Type of sector(s) to read     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_cache_entry_read                         */
/*                                          Read logical sector cache     */
/*    _fx_utility_logical_sector_flush      Flush and invalidate sectors  */
/*                                          that overlap with non-cache   */
/*                                          sector I/O.                   */
/*    _fx_utility_memory_copy               Copy cache sector             */
/*    _fx_fault_tolerant_read_directory_sector                            */
/*                                          Read directory sector         */
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
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            verified memcpy usage, and  */
/*                                            added conditional to        */
/*                                            disable cache,              */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            updated check for logical   */
/*                                            sector value,               */
/*                                            resulting in version 6.1.6  */
/*  10-31-2022     Tiejun Zhou              Modified comment(s),          */
/*                                            fixed memory buffer when    */
/*                                            cache is disabled,          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_logical_sector_read(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                      VOID *buffer_ptr, ULONG sectors, UCHAR sector_type)
{
#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry;
FX_CACHED_SECTOR *previous_cache_entry;
ULONG64           end_sector;
#endif /* FX_DISABLE_CACHE */

#ifdef FX_ENABLE_FAULT_TOLERANT
UINT              status;
#endif /* FX_ENABLE_FAULT_TOLERANT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Determine if the request is for FAT sector.  */
    if (sector_type == FX_FAT_SECTOR)
    {

        /* Increment the number of FAT sector reads.  */
        media_ptr -> fx_media_fat_sector_reads++;
    }

    /* Increment the number of logical sectors read.  */
    media_ptr -> fx_media_logical_sector_reads++;
#endif

    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION

#ifndef FX_DISABLE_CACHE
    /* Determine if the request is for the internal media buffer area.  */
    if ((((UCHAR *)buffer_ptr) >= media_ptr -> fx_media_memory_buffer) &&
        (((UCHAR *)buffer_ptr) <= media_ptr -> fx_media_sector_cache_end))
    {

        /* Internal cache buffer is requested.  */

        /* Examine the logical sector cache.  */
        cache_entry = _fx_utility_logical_sector_cache_entry_read(media_ptr, logical_sector, &previous_cache_entry);

        /* Was the sector found?  */
        if (cache_entry == FX_NULL)
        {

            /* Yes, the sector was found. Return success!  */
            return(FX_SUCCESS);
        }

        /* At this point, we need to read in a sector from the media.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of logical sectors cache read misses.  */
        media_ptr -> fx_media_logical_sector_cache_read_misses++;
#endif

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_LOG_SECTOR_CACHE_MISS, media_ptr, logical_sector, media_ptr -> fx_media_logical_sector_cache_read_misses, media_ptr -> fx_media_sector_cache_size, FX_TRACE_INTERNAL_EVENTS, 0, 0)
#else

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_LOG_SECTOR_CACHE_MISS, media_ptr, logical_sector, 0, media_ptr -> fx_media_sector_cache_size, FX_TRACE_INTERNAL_EVENTS, 0, 0)
#endif

        /* First, check and see if the last used entry has been
           modified.  */
        if ((cache_entry -> fx_cached_sector_valid) &&
            (cache_entry -> fx_cached_sector_buffer_dirty))
        {

            /* Yes, we need to flush this buffer out to the physical media
               before we read in the new buffer.  */

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

            /* Determine if the sector is a data sector or a system sector.  */
            if (cache_entry -> fx_cached_sector_type != FX_DATA_SECTOR)
            {

                /* System sector is present.  */
                media_ptr -> fx_media_driver_system_write =  FX_TRUE;
            }

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

            /* Decrement the number of outstanding dirty cache entries.  */
            media_ptr -> fx_media_sector_cache_dirty_count--;
        }

        /* At this point, we can go out and setup this cached sector
           entry.  */

        /* Compare against logical sector to make sure it is valid.  */
        if (logical_sector >= media_ptr -> fx_media_total_sectors)
        {
            return(FX_SECTOR_INVALID);
        }

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver read sector(s) requests.  */
        media_ptr -> fx_media_driver_read_requests++;
#endif

        /* Build Read request to the driver.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           cache_entry -> fx_cached_sector_memory_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
        media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
        media_ptr -> fx_media_driver_sectors =          1;
        media_ptr -> fx_media_driver_sector_type =      sector_type;

        /* Determine if the sector is a data sector or a system sector.  */
        if (sector_type == FX_DATA_SECTOR)
        {

            /* Data sector is present.  */
            media_ptr -> fx_media_driver_data_sector_read =  FX_TRUE;
        }

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, logical_sector, 1, cache_entry -> fx_cached_sector_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to read the sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear data sector is present flag.  */
        media_ptr -> fx_media_driver_data_sector_read =  FX_FALSE;

        /* Determine if the read was successful.  */
        if (media_ptr -> fx_media_driver_status == FX_SUCCESS)
        {

            /* Remember the sector number.  */
            cache_entry -> fx_cached_sector =  logical_sector;

            /* Make the cache entry valid.  */
            cache_entry -> fx_cached_sector_valid =  FX_TRUE;

            /* Remember the sector type.  */
            cache_entry -> fx_cached_sector_type =  sector_type;

            /* Place this entry that the head of the cached sector
               list.  */

            /* Determine if we need to update the last used list.  */
            if (previous_cache_entry)
            {

                /* Yes, the current entry is not at the front of the list
                   so we need to change the order.  */

                /* Link the previous entry to this entry's next pointer.  */
                previous_cache_entry -> fx_cached_sector_next_used =
                    cache_entry -> fx_cached_sector_next_used;

                /* Place this entry at the head of the list.  */
                cache_entry -> fx_cached_sector_next_used =
                    media_ptr -> fx_media_sector_cache_list_ptr;
                media_ptr -> fx_media_sector_cache_list_ptr =  cache_entry;
            }

#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled &&
                (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED) &&
                (sector_type == FX_DIRECTORY_SECTOR))
            {

                /* Read sector from log file. */
                status = _fx_fault_tolerant_read_directory_sector(media_ptr, logical_sector, cache_entry -> fx_cached_sector_memory_buffer, 1);

                /* Check for successful completion.  */
                if (status)
                {

                    /* Return the error status. */
                    return(status);
                }
            }
#endif /* FX_ENABLE_FAULT_TOLERANT */
        }
        else
        {

            /* Invalidate the cache entry on read errors.  */
            cache_entry -> fx_cached_sector_valid =  FX_FALSE;

            /* Put all ones in the sector value.  */
            cache_entry -> fx_cached_sector =  (~(ULONG64)0);
        }

        /* Simply setup the pointer to this buffer and return.  */
        media_ptr -> fx_media_memory_buffer =  cache_entry -> fx_cached_sector_memory_buffer;

        /* Return the driver status.  */
        return(media_ptr -> fx_media_driver_status);
    }
#else
    if ((logical_sector == media_ptr -> fx_media_memory_buffer_sector) && (sectors == 1) && (buffer_ptr == media_ptr -> fx_media_memory_buffer))
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled &&
            (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED) &&
            (sector_type == FX_DIRECTORY_SECTOR))
        {

            /* Read sector from log file. */
            status = _fx_fault_tolerant_read_directory_sector(media_ptr, logical_sector, buffer_ptr, 1);

            /* Check for successful completion.  */
            if (status)
            {

                /* Return the error status. */
                return(status);
            }
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */
        return(FX_SUCCESS);
    }
#endif
    else
    {

        /* Direct I/O to application buffer area.  */

        /* Compare against logical sector to make sure it is valid.  */
        if ((logical_sector + sectors - 1) > (ULONG)media_ptr -> fx_media_total_sectors)
        {
            return(FX_SECTOR_INVALID);
        }

#ifndef FX_DISABLE_CACHE
        /* Attempt to fill the beginning of the buffer from cached sectors.  */
        while (sectors)
        {

            /* Determine if the sector is in the cache.  */
            if (_fx_utility_logical_sector_cache_entry_read(media_ptr, logical_sector, &previous_cache_entry))
            {

                /* Not in the cache - get out of the loop!  */
                break;
            }

            /* Yes, sector is in the cache. Copy the data from the cache to the destination buffer.  */
            _fx_utility_memory_copy(media_ptr -> fx_media_memory_buffer, buffer_ptr, media_ptr -> fx_media_bytes_per_sector); /* Use case of memcpy is verified. */

            /* Advance the destination buffer.  */
            buffer_ptr =  ((UCHAR *)buffer_ptr) + media_ptr -> fx_media_bytes_per_sector;

            /* Advance the sector and decrement the number of sectors left.  */
            logical_sector++;
            sectors--;
        }

        /* Calculate the end sector.  */
        end_sector = logical_sector + sectors - 1;

        /* Attempt to fill the end of the buffer from the opposite direction.  */
        while (sectors)
        {

            /* Determine if the sector is in the cache.  */
            if (_fx_utility_logical_sector_cache_entry_read(media_ptr, end_sector, &previous_cache_entry))
            {

                /* Not in the cache - get out of the loop!  */
                break;
            }

            /* Yes, sector is in the cache. Copy the data from the cache to the destination buffer.  */
            _fx_utility_memory_copy(media_ptr -> fx_media_memory_buffer, /* Use case of memcpy is verified. */
                                    ((UCHAR *)buffer_ptr) + ((sectors - 1) * media_ptr -> fx_media_bytes_per_sector),
                                    media_ptr -> fx_media_bytes_per_sector);

            /* Move sector to previous sector and decrement the number of sectors left.  */
            end_sector--;
            sectors--;
        }

        /* Determine if there are still sectors left to read.  */
        if (sectors == 0)
        {

            /* No more sectors to read - return success!  */
            return(FX_SUCCESS);
        }

        /* Flush and invalidate any entries in the cache that are in this direct I/O read request range.  */
        _fx_utility_logical_sector_flush(media_ptr, logical_sector, (ULONG64) sectors, FX_TRUE);
#endif /* FX_DISABLE_CACHE */

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver read sector(s) requests.  */
        media_ptr -> fx_media_driver_read_requests++;
#endif

        /* Build read request to the driver.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           buffer_ptr;
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
        media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
        media_ptr -> fx_media_driver_sectors =          sectors;
        media_ptr -> fx_media_driver_sector_type =      sector_type;

        /* Determine if the sector is a data sector or a system sector.  */
        if (sector_type == FX_DATA_SECTOR)
        {

            /* Data sector is present.  */
            media_ptr -> fx_media_driver_data_sector_read =  FX_TRUE;
        }

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, logical_sector, sectors, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to read the sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear data sector is present flag.  */
        media_ptr -> fx_media_driver_data_sector_read =  FX_FALSE;

#ifdef FX_DISABLE_CACHE
        if ((media_ptr -> fx_media_driver_status == FX_SUCCESS) && (sectors == 1) && (buffer_ptr == media_ptr -> fx_media_memory_buffer))
        {
            media_ptr -> fx_media_memory_buffer_sector = logical_sector;
#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled &&
                (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED) &&
                (sector_type == FX_DIRECTORY_SECTOR))
            {

                /* Read sector from log file. */
                status = _fx_fault_tolerant_read_directory_sector(media_ptr, logical_sector, buffer_ptr, 1);

                /* Check for successful completion.  */
                if (status)
                {

                    /* Return the error status. */
                    return(status);
                }
            }
#endif /* FX_ENABLE_FAULT_TOLERANT */
            return(FX_SUCCESS);
        }
#endif /* FX_DISABLE_CACHE */

#ifndef FX_DISABLE_DIRECT_DATA_READ_CACHE_FILL

        /* Determine if the read was successful and if number of sectors just read will
           reasonably fit into the cache.  */
        if ((media_ptr -> fx_media_driver_status == FX_SUCCESS) && (sectors < (media_ptr -> fx_media_sector_cache_size / 4)))
        {

            /* Yes, read of direct sectors was successful.  */

            /* Copy the sectors directly read into the cache so they are available on
               subsequent read requests.  */
            while (sectors)
            {

                /* Attempt to read the cache entry.  */
                cache_entry =  _fx_utility_logical_sector_cache_entry_read(media_ptr, logical_sector, &previous_cache_entry);

                /* Extended port-specific processing macro, which is by default defined to white space.  */
                FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1

                /* At this point, a cache entry should always be present since we invalidated
                   the cache over this sector range previously. In any case, check for the error
                   condition.  */
                if (cache_entry == FX_NULL)
                {

                    /* This case should never happen, however, if it does simply give up on updating the
                       cache with the sectors from the direct read.  */
                    return(FX_SUCCESS);
                }

                /* Determine if the cache entry is dirty and needs to be written out before it is used.  */
                if ((cache_entry -> fx_cached_sector_valid) &&
                    (cache_entry -> fx_cached_sector_buffer_dirty))
                {

                    /* Yes, we need to flush this buffer out to the physical media
                       before we read in the new buffer.  */

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

                    /* Only data sectors may be dirty when FX_FAULT_TOLERANT is defined */
#ifndef FX_FAULT_TOLERANT
                    /* Determine if the sector is a data sector or a system sector.  */
                    if (cache_entry -> fx_cached_sector_type != FX_DATA_SECTOR)
                    {

                        /* System sector is present.  */
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

                    /* Decrement the number of outstanding dirty cache entries.  */
                    media_ptr -> fx_media_sector_cache_dirty_count--;
                }

                /* Now setup the cache entry with information from the new sector.  */

                /* Remember the sector number.  */
                cache_entry -> fx_cached_sector =  logical_sector;

                /* Make the cache entry valid.  */
                cache_entry -> fx_cached_sector_valid =  FX_TRUE;

                /* Remember the sector type.  */
                cache_entry -> fx_cached_sector_type =  sector_type;

                /* Place this entry that the head of the cached sector
                   list.  */

                /* Determine if we need to update the last used list.  */
                if (previous_cache_entry)
                {

                    /* Yes, the current entry is not at the front of the list
                       so we need to change the order.  */

                    /* Link the previous entry to this entry's next pointer.  */
                    previous_cache_entry -> fx_cached_sector_next_used =
                        cache_entry -> fx_cached_sector_next_used;

                    /* Place this entry at the head of the list.  */
                    cache_entry -> fx_cached_sector_next_used =
                        media_ptr -> fx_media_sector_cache_list_ptr;
                    media_ptr -> fx_media_sector_cache_list_ptr =  cache_entry;
                }

                /* Copy the data from the destination buffer to the cache entry.  */
                _fx_utility_memory_copy(buffer_ptr, /* Use case of memcpy is verified. */
                                        cache_entry -> fx_cached_sector_memory_buffer,
                                        media_ptr -> fx_media_bytes_per_sector);

                /* Advance the destination buffer.  */
                buffer_ptr =  ((UCHAR *)buffer_ptr) + media_ptr -> fx_media_bytes_per_sector;

                /* Advance the source sector and decrement the sector count.  */
                logical_sector++;
                sectors--;
            }
        }
#endif

        /* Return the driver status.  */
        return(media_ptr -> fx_media_driver_status);
    }
}

