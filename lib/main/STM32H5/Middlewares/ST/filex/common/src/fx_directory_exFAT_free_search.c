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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_free_search                           PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches the media for a free directory entry.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_ptr                         Pointer to directory to       */
/*                                            search in                   */
/*    entry_ptr                             Pointer to directory entry    */
/*                                            record                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_exFAT_entry_read        Read entries from directory   */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_utility_logical_sector_flush      Flush logical sector cache    */
/*    _fx_utility_logical_sector_read       Read logical sector           */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state in bitmap   */
/*    _fx_utility_exFAT_bitmap_flush        Flush written exFAT bitmap    */
/*    _fx_utility_exFAT_allocate_new_cluster                              */
/*                                          Allocate new cluster          */
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
/*  01-31-2022     William E. Lamie         Modified comment(s), fixed    */
/*                                            errors without cache,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_exFAT_free_search(FX_MEDIA *media_ptr, FX_DIR_ENTRY *directory_ptr, FX_DIR_ENTRY *entry_ptr)
{

ULONG             i;
UCHAR            *work_ptr;
UINT              status, total_entries;
ULONG64           entry_sector = 0;
ULONG64           entry_next_sector = 0;
ULONG             entry_offset = 0;
ULONG             cluster;
ULONG             last_cluster;
ULONG             directory_entries;
ULONG64           logical_sector;
FX_DIR_ENTRY     *search_dir_ptr;
ULONG             free_entry_start;
UINT              sectors;
#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry_ptr;
#endif
ULONG             bytes_per_cluster;


    /* Get name length.  */
    i = _fx_utility_string_length_get(entry_ptr -> fx_dir_entry_name, FX_MAX_EX_FAT_NAME_LEN);

    /* Check if name is valid.  */
    if (entry_ptr -> fx_dir_entry_name[i] != 0)
    {

        /* Invalid name, return error.  */
        return(FX_INVALID_NAME);
    }
    /* Determine the total entries.  */
    total_entries = 2 + (i + 14) / 15;

    /* Determine if the search is in the root directory or in a
       sub-directory.  Note: the directory search function clears the
       first character of the name for the root directory.  */
    if (directory_ptr -> fx_dir_entry_name[0])
    {

        /* Search for a free entry in a sub-directory.  */
        directory_entries =  (ULONG)directory_ptr -> fx_dir_entry_file_size / FX_DIR_ENTRY_SIZE;

        /* Point the search directory pointer to this entry.  */
        search_dir_ptr =  directory_ptr;
    }
    else
    {

        /* Find a free entry in the root directory.  */

        /* Setup the number of directory entries.  */
        directory_entries =  (ULONG)media_ptr -> fx_media_root_directory_entries;

        /* Set the search pointer to NULL since we are working off of the
           root directory.  */
        search_dir_ptr =  FX_NULL;

        /* Clear dont_use_fat for root directory.  */
        directory_ptr -> fx_dir_entry_dont_use_fat = 0;
    }

    /* Loop through entries in the search directory.  Yes, this is a
       linear search!  */
    i =  0;
    free_entry_start =  directory_entries;

    if (directory_entries > 0)
    {
        do
        {
            /* Read an entry from the directory.  */
            status =
                _fx_directory_exFAT_entry_read(media_ptr,
                                               search_dir_ptr, &i, entry_ptr, 0, FX_TRUE, NULL, NULL);

            /* Check for error status.  */
            if (status != FX_SUCCESS)
            {
                return(status);
            }

            /* Determine if this is an empty entry.  */
            if ((entry_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_FREE) ||
                (entry_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER))
            {
                /* Determine if we are at the first free entry.  */
                if (free_entry_start == directory_entries)
                {

                    /* Remember the start of the free entry.  */
                    free_entry_start =  i;
                    entry_sector =      entry_ptr -> fx_dir_entry_log_sector;
                    entry_next_sector = entry_ptr -> fx_dir_entry_next_log_sector;
                    entry_offset  =     entry_ptr -> fx_dir_entry_byte_offset;
                }

                /* Determine if there are enough free entries to satisfy the request.  */
                if ((i - free_entry_start + 1) >= total_entries)
                {
                    FX_INT_SAVE_AREA

                    /* Found an empty slot.  All the pertinent information is already
                       in the entry structure.  */
                    entry_ptr -> fx_dir_entry_log_sector =      entry_sector;
                    entry_ptr -> fx_dir_entry_next_log_sector = entry_next_sector;
                    entry_ptr -> fx_dir_entry_byte_offset =     entry_offset;

                    /* Initialize the additional directory entries.  */
                    entry_ptr -> fx_dir_entry_created_time_ms =     0;

                    /* Lockout interrupts for time/date access.  */
                    FX_DISABLE_INTS

                    entry_ptr -> fx_dir_entry_created_time =        _fx_system_time;
                    entry_ptr -> fx_dir_entry_created_date =        _fx_system_date;
                    entry_ptr -> fx_dir_entry_last_accessed_date =  _fx_system_date;

                    /* Restore interrupts.  */
                    FX_RESTORE_INTS

                    /* Return a successful completion.  */
                    return(FX_SUCCESS);
                }
            }
            else
            {

                /* Reset the free entry start.  */
                free_entry_start =  directory_entries;
            }

            /* Move to the next entry.  */
            i++;
        } while (i < directory_entries);
    }

    /* No empty entries were found.  If the specified directory is a sub-directory,
       attempt to allocate another cluster to it.  */
    if (media_ptr -> fx_media_available_clusters)
    {
    ULONG64 size;
        bytes_per_cluster = ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
            ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

        /* Check if it is a sub directory.  */
        if (search_dir_ptr)
        {

            /* Set size to the available file size.  */
            size = search_dir_ptr -> fx_dir_entry_available_file_size;
        }
        else
        {

            /* Root directory, set size to root directory size.  */
            size = media_ptr -> fx_media_root_directory_entries * FX_DIR_ENTRY_SIZE;
        }

        /* Check for directory restriction */
        if (size + bytes_per_cluster > FX_EXFAT_MAX_DIRECTORY_SIZE)
        {
            return(FX_NO_MORE_SPACE);
        }

        /* Search the FAT for the next available cluster.  */
        status = _fx_utility_exFAT_allocate_new_cluster(media_ptr, search_dir_ptr, &last_cluster, &cluster);

        if (status != FX_SUCCESS)
        {
            return(status);
        }

        /* Decrease the available clusters in the media.  */
        media_ptr -> fx_media_available_clusters--;

        /* Defer the update of the FAT entry and the last cluster of the current
           directory entry until after the new cluster is initialized and written out.  */

        /* Update entry size. */
        if (search_dir_ptr)
        {
            search_dir_ptr -> fx_dir_entry_file_size += bytes_per_cluster;
            search_dir_ptr -> fx_dir_entry_available_file_size = search_dir_ptr -> fx_dir_entry_file_size;
        }
        else
        {

            /* Change root directory entry count - FAT32 has a variable sized root directory.  */
            media_ptr -> fx_media_root_directory_entries += bytes_per_cluster / FX_DIR_ENTRY_SIZE;
        }

        /* Calculate the logical sector of this cluster.  */
        logical_sector =  ((ULONG)media_ptr -> fx_media_data_sector_start) +
            ((((ULONG64)cluster) - FX_FAT_ENTRY_START) *
             ((ULONG)media_ptr -> fx_media_sectors_per_cluster));

        /* Pickup the number of sectors for the next directory cluster.  */
        sectors =  media_ptr -> fx_media_sectors_per_cluster;

        /* Read the logical sector just for cache reasons.  */
        status =  _fx_utility_logical_sector_read(media_ptr, logical_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

        /* Check the return value.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error status.  */
            return(status);
        }

        /* Clear the entire first sector of the new sub-directory cluster.  */
        work_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer;
        i =  0;
        while (i < media_ptr -> fx_media_bytes_per_sector)
        {

            /* Clear 4 bytes.  */
            *((ULONG *)work_ptr) =  (ULONG)0;

            /* Increment pointer.  */
            work_ptr =  work_ptr + sizeof(ULONG);

            /* Increment counter.  */
            i =  i + sizeof(ULONG);
        }

#ifndef FX_DISABLE_CACHE
        /* Invalidate all cached sectors that are contained in the newly allocated first
           cluster of the directory.  */

        /* Pickup the total number of cache entries.  */
        i =  (UINT)media_ptr -> fx_media_sector_cache_size;

        /* Setup pointer to first cache entry (not in list order).  */
        cache_entry_ptr =  media_ptr -> fx_media_sector_cache;

        /* Examine all the logical sector cache entries.  */
        while (i--)
        {

            /* Determine if the cached entry is a sector in the first cluster of the
               new directory.  We don't need to worry about the first sector since it
               was read using the logical sector read utility earlier.  */
            if ((cache_entry_ptr -> fx_cached_sector >= (logical_sector + 1)) &&
                (cache_entry_ptr -> fx_cached_sector <  (logical_sector + sectors)))
            {

                /* Yes, we have found a logical sector in the cache that is one of the directory
                   sectors that will be written with zeros.  Because of this, simply make this
                   cache entry invalid.  */
                cache_entry_ptr -> fx_cached_sector =                0;
                cache_entry_ptr -> fx_cached_sector_buffer_dirty =   FX_FALSE;
            }

            /* Move to next entry in the cached sector list.  */
            if (cache_entry_ptr -> fx_cached_sector_next_used)
            {
                cache_entry_ptr =  cache_entry_ptr -> fx_cached_sector_next_used;
            }
        }
#endif

        /* Clear all sectors of new sub-directory cluster.  */
        do
        {

            /* Decrease the number of sectors to clear.  */
            sectors--;

            /* Build Write request to the driver.  */
            media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
            media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
            media_ptr -> fx_media_driver_buffer =           media_ptr -> fx_media_memory_buffer;
            media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector + ((ULONG)sectors);
            media_ptr -> fx_media_driver_sectors =          1;

            /* Set the system write flag since we are writing a directory sector.  */
            media_ptr -> fx_media_driver_system_write =  FX_TRUE;

            /* Invoke the driver to write the sector.  */
            (media_ptr -> fx_media_driver_entry)(media_ptr);

            /* Clear the system write flag.  */
            media_ptr -> fx_media_driver_system_write =  FX_FALSE;

            /* Determine if an error occurred.  */
            if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
            {

                /* Return error code.  */
                return(status);
            }
        } while (sectors);

#ifdef FX_FAULT_TOLERANT

        /* Flush the internal logical sector.  */
        _fx_utility_logical_sector_flush(media_ptr, logical_sector, media_ptr -> fx_media_sectors_per_cluster, FX_FALSE);
#endif

        if ((!search_dir_ptr) || (!(search_dir_ptr -> fx_dir_entry_dont_use_fat & 1)))
        {

            /* At this point, link up the last cluster with the new cluster.  */

            /* Setup the last cluster to indicate the end of the chain.  */
            status =  _fx_utility_FAT_entry_write(media_ptr, cluster, FX_LAST_CLUSTER_exFAT);

            /* Check the return value.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

            /* Link the last cluster of the directory to the new cluster.  */
            if (last_cluster)
            {

                status =  _fx_utility_FAT_entry_write(media_ptr, last_cluster, cluster);

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }
            }
        }

        if (!last_cluster && search_dir_ptr)
        {

            search_dir_ptr -> fx_dir_entry_cluster = cluster;
        }

        /* Mark the cluster as used. */
        status = _fx_utility_exFAT_cluster_state_set(media_ptr, cluster, FX_EXFAT_BITMAP_CLUSTER_OCCUPIED);

        /* Check the return value.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error status.  */
            return(status);
        }

        /* Move cluster search pointer forward. */
        media_ptr -> fx_media_cluster_search_start = cluster + 1;

        /* Determine if this needs to be wrapped. */
        if (media_ptr -> fx_media_cluster_search_start >= media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
        {

            /* Wrap the search to the beginning FAT entry. */
            media_ptr -> fx_media_cluster_search_start = FX_FAT_ENTRY_START;
        }

#ifdef FX_FAULT_TOLERANT

        /* Flush the cached individual FAT entries */
        _fx_utility_FAT_flush(media_ptr);

        /* Flush the bitmap and its logical sector.  */
        _fx_utility_exFAT_bitmap_flush(media_ptr);
        _fx_utility_logical_sector_flush(media_ptr, media_ptr -> fx_media_exfat_bitmap_start_sector +
                                         (cluster >> media_ptr -> fx_media_exfat_bitmap_clusters_per_sector_shift), ((ULONG64) 1), FX_FALSE);
#endif

        /* Finally load up the directory entry with information for the
           beginning of the new cluster.  */
        {
            FX_INT_SAVE_AREA

            entry_ptr -> fx_dir_entry_name[0] = 0;
            if (free_entry_start == directory_entries)
            {

                /* start from begining of new cluster */
                entry_ptr -> fx_dir_entry_log_sector = logical_sector;

                /* There are more sectors in this cluster.  */
                entry_ptr -> fx_dir_entry_next_log_sector = logical_sector + 1;

                /* Clear the byte offset since this is a new entry.  */
                entry_ptr -> fx_dir_entry_byte_offset = 0;
            }
            else
            {

                /* start from end of last cluster */
                entry_ptr -> fx_dir_entry_log_sector = entry_sector;

                /* Next sector in new cluster */
                entry_ptr -> fx_dir_entry_next_log_sector = logical_sector;

                /* Set byte offset */
                entry_ptr -> fx_dir_entry_byte_offset = entry_offset;
            }

            /* Lockout interrupts for time/date access.  */
            FX_DISABLE_INTS

            entry_ptr -> fx_dir_entry_created_time =        _fx_system_time;
            entry_ptr -> fx_dir_entry_created_date =        _fx_system_date;
            entry_ptr -> fx_dir_entry_last_accessed_date =  _fx_system_date;

            /* Restore interrupts.  */
            FX_RESTORE_INTS
        }

        entry_ptr -> fx_dir_entry_type = FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER;

        /* Return a successful status.  */
        return(FX_SUCCESS);
    }

    /* Return "not found" status to the caller.  */
    return(FX_NOT_FOUND);
}
#endif /* FX_ENABLE_EXFAT */

