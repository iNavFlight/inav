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
/*    _fx_utility_FAT_entry_write                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes to the supplied FAT entry to all FATs in       */
/*    the media.  12-bit, 16-bit, and 32-bit FAT writing is supported.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*    next_cluster                          Next cluster integer pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_flush                 FLUSH dirty entries in the    */
/*                                            FAT cache                   */
/*    _fx_fault_tolerant_add_fat_log        Add FAT redo log              */
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_FAT_entry_write(FX_MEDIA *media_ptr, ULONG cluster, ULONG next_cluster)
{

UINT                status, index, i;
FX_FAT_CACHE_ENTRY *cache_entry_ptr;
#ifdef FX_ENABLE_FAULT_TOLERANT
ULONG               FAT_sector;

    /* While fault_tolerant is enabled, only FAT entries in the same sector are allowed to be cached. */
    /* We must flush FAT sectors in the order of FAT chains. */
    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {

        if (!(media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN))
        {

            /* Redirect this request to log file. */
            return(_fx_fault_tolerant_add_FAT_log(media_ptr, cluster, next_cluster));
        }

        /* Get sector number of the incoming FAT entry. */
        FAT_sector = _fx_utility_FAT_sector_get(media_ptr, cluster);

        /* Is FAT sector changed? */
        if (FAT_sector != media_ptr -> fx_media_fault_tolerant_cached_FAT_sector)
        {

            /* Are there any cached FAT entries? */
            if (media_ptr -> fx_media_fault_tolerant_cached_FAT_sector)
            {

                /* Current FAT entry is located in different sector. Force flush. */
                /* Flush the cached individual FAT entries */
                _fx_utility_FAT_flush(media_ptr);

#ifdef FX_ENABLE_EXFAT
                if (media_ptr -> fx_media_FAT_type == FX_exFAT)
                {

                    /* Flush exFAT bitmap.  */
                    _fx_utility_exFAT_bitmap_flush(media_ptr);
                }
#endif /* FX_ENABLE_EXFAT */
            }

            /* Record sector number of current FAT entry. */
            media_ptr -> fx_media_fault_tolerant_cached_FAT_sector = FAT_sector;
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Increment the number of FAT entry writes and cache hits.  */
    media_ptr -> fx_media_fat_entry_writes++;
    media_ptr -> fx_media_fat_entry_cache_write_hits++;
#endif

    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION

    /* Calculate the area of the cache for this FAT entry.  */
    index =  (cluster & FX_FAT_CACHE_HASH_MASK) * FX_FAT_CACHE_DEPTH;

    /* Build a pointer to the cache entry.  */
    cache_entry_ptr =  &media_ptr -> fx_media_fat_cache[index];

    /* First search for the entry in the FAT entry cache.  */
    for (i = 0; i < FX_FAT_CACHE_DEPTH; i++)
    {

        /* See if the entry matches the write request.  */
        if (((cache_entry_ptr + i) -> fx_fat_cache_entry_cluster) == cluster)
        {

            /* Yes, we have a matching entry.  Save the new information in the FAT
               cache and mark this entry as dirty.  */
            (cache_entry_ptr + i) -> fx_fat_cache_entry_value =     next_cluster;
            (cache_entry_ptr + i) -> fx_fat_cache_entry_dirty =     1;

            /* Determine if the driver has requested notification when data sectors in the media
               become free.  This can be useful to FLASH manager software.  */
            if ((media_ptr -> fx_media_driver_free_sector_update) && (next_cluster == FX_FREE_CLUSTER))
            {

                /* Yes, the driver does wish to know that these specific sector(s) are
                   not in use.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

                /* Increment the number of driver release sectors requests.  */
                media_ptr -> fx_media_driver_release_sectors_requests++;
#endif

                /* This cluster is being released so inform the driver that the
                   corresponding sectors are now available.  */
                media_ptr -> fx_media_driver_request =          FX_DRIVER_RELEASE_SECTORS;
                media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
                media_ptr -> fx_media_driver_logical_sector =   (media_ptr -> fx_media_data_sector_start +
                                                                 ((cluster - FX_FAT_ENTRY_START) * media_ptr -> fx_media_sectors_per_cluster));
                media_ptr -> fx_media_driver_sectors =          media_ptr -> fx_media_sectors_per_cluster;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_RELEASE_SECTORS, media_ptr, media_ptr -> fx_media_driver_logical_sector, media_ptr -> fx_media_driver_sectors, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Call the driver.  */
                (media_ptr -> fx_media_driver_entry)(media_ptr);
            }

            /* Done, return successful status.  */
            return(FX_SUCCESS);
        }
    }

    /* If we reach this point, we know that the FAT write request is not in
       the cache.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Decrement the number of cache hits.  */
    media_ptr -> fx_media_fat_entry_cache_write_hits--;

    /* Increment the number of cache misses.  */
    media_ptr -> fx_media_fat_entry_cache_write_misses++;
#endif

    /* Determine if the oldest entry is dirty and needs to be flushed.  */
    if (media_ptr -> fx_media_fat_cache[index + 3].fx_fat_cache_entry_dirty == 1)
    {

        /* Flush the dirty entry so it can be used to hold the current
           FAT entry write request.  */
        status = _fx_utility_FAT_flush(media_ptr);

        /* Determine if the write was successful.  */
        if (status != FX_SUCCESS)
        {

            /* No, return error status to caller.  */
            return(status);
        }

    }

    /* Move all the cache entries down so the oldest is at the bottom.  */
    *(cache_entry_ptr + 3) =  *(cache_entry_ptr + 2);
    *(cache_entry_ptr + 2) =  *(cache_entry_ptr + 1);
    *(cache_entry_ptr + 1) =  *(cache_entry_ptr);

    /* Save the current FAT entry write request and mark as dirty.  */
    cache_entry_ptr -> fx_fat_cache_entry_dirty =    1;
    cache_entry_ptr -> fx_fat_cache_entry_cluster =  cluster;
    cache_entry_ptr -> fx_fat_cache_entry_value =    next_cluster;

    /* Determine if the driver has requested notification when data sectors in the media
       become free.  This can be useful to FLASH manager software.  */
    if ((media_ptr -> fx_media_driver_free_sector_update) && (next_cluster == FX_FREE_CLUSTER))
    {

        /* Yes, the driver does wish to know that these specific sector(s) are
           not in use.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver release sectors requests.  */
        media_ptr -> fx_media_driver_release_sectors_requests++;
#endif

        /* This cluster is being released so inform the driver that the
              corresponding sectors are now available.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_RELEASE_SECTORS;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_logical_sector =   (media_ptr -> fx_media_data_sector_start +
                                                         ((cluster - FX_FAT_ENTRY_START) * media_ptr -> fx_media_sectors_per_cluster));
        media_ptr -> fx_media_driver_sectors =          media_ptr -> fx_media_sectors_per_cluster;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_RELEASE_SECTORS, media_ptr, media_ptr -> fx_media_driver_logical_sector, media_ptr -> fx_media_driver_sectors, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Call the driver.  */
        (media_ptr -> fx_media_driver_entry)(media_ptr);
    }

    /* Return success to caller.  */
    return(FX_SUCCESS);
}

