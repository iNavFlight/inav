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
#include "fx_file.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"
#endif /* FX_ENABLE_EXFAT */
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_delete                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified directory.       */
/*    If found, the directory is examined to make sure it is empty.  If   */
/*    the directory is not empty, an error code is returned to the        */
/*    caller.  Otherwise, the directory will be deleted and its clusters  */
/*    will be made available.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Directory name to delete      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_read              Read a directory entry        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_utility_logical_sector_flush      Flush the written log sector  */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*    _fx_utility_FAT_entry_read            Read FAT entries to calculate */
/*                                            the sub-directory size      */
/*    _fx_utility_FAT_entry_write           Write FAT entry               */
/*    _fx_utility_FAT_flush                 Flush FAT cache               */
/*    _fx_fault_tolerant_transaction_start  Start fault tolerant          */
/*                                            transaction                 */
/*    _fx_fault_tolerant_transaction_end    End fault tolerant transaction*/
/*    _fx_fault_tolerant_recover            Recover FAT chain             */
/*    _fx_fault_tolerant_reset_log_file     Reset the log file            */
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
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_delete(FX_MEDIA *media_ptr, CHAR *directory_name)
{

UINT         status;
ULONG        cluster, next_cluster;
ULONG        i, directory_size;
FX_DIR_ENTRY dir_entry;
FX_DIR_ENTRY search_directory;
FX_DIR_ENTRY search_entry;

#ifdef FX_ENABLE_EXFAT
ULONG        clusters_count;
ULONG        bytes_per_cluster;
#endif /* FX_ENABLE_EXFAT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_deletes++;
#endif

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Setup search pointer to media name buffer.  */
    search_directory.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;

    /* Setup search entry pointer to media name buffer.  */
    search_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 3;

    /* Clear the short name string of the three file names that will be worked with.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;
    search_directory.fx_dir_entry_short_name[0] =  0;
    search_entry.fx_dir_entry_short_name[0] =  0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_DELETE, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Search the system for the supplied directory name.  */
    status =  _fx_directory_search(media_ptr, directory_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check to make sure the found entry is a directory.  */
    if (!(dir_entry.fx_dir_entry_attributes & (UCHAR)(FX_DIRECTORY)))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a directory error code.  */
        return(FX_NOT_DIRECTORY);
    }

    /* Check if the entry is read only */
    if (dir_entry.fx_dir_entry_attributes & (UCHAR)(FX_READ_ONLY))
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a directory error code.  */
        return(FX_WRITE_PROTECT);
    }

    /* Copy the directory entry to the search directory structure for
       looking at the specified sub-directory contents.  */
    search_directory =  dir_entry;

    /* Ensure that the search directory's last search cluster is cleared.  */
    search_directory.fx_dir_entry_last_search_cluster =  0;

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        directory_size = (ULONG)search_directory.fx_dir_entry_file_size / FX_DIR_ENTRY_SIZE;
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        /* Calculate the directory size by counting the allocated
           clusters for it.  */
        i =        0;
        cluster =  search_directory.fx_dir_entry_cluster;
        while (cluster < media_ptr -> fx_media_fat_reserved)
        {

            /* Increment the cluster count.  */
            i++;

            /* Read the next FAT entry.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

            /* Check the return status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(status);
            }

            if ((cluster < FX_FAT_ENTRY_START) || (cluster == next_cluster) || (i > media_ptr -> fx_media_total_clusters))
            {
#ifdef FX_ENABLE_FAULT_TOLERANT
               FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(FX_FAT_READ_ERROR);
            }

            cluster = next_cluster;
        }

        /* Now we can calculate the directory size.  */
        directory_size =  (((ULONG)media_ptr -> fx_media_bytes_per_sector) *
                           ((ULONG)media_ptr -> fx_media_sectors_per_cluster) * i) /
                           (ULONG)FX_DIR_ENTRY_SIZE;

        /* Also save this in the directory entry so we don't have to
           calculate it later.  */
        search_directory.fx_dir_entry_file_size =  directory_size;
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Make sure the new name is not in the current directory.  */
#ifdef FX_ENABLE_EXFAT
    if (directory_size > 0)
    {

        /* exFAT directories do not record '.' and '..' directories.  */
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            i = 0;
        }
        else
        {
            i = 2;
        }
#else
        /* The first two entries are skipped because they are just part of the sub-directory.  */
        i = 2;
#endif /* FX_ENABLE_EXFAT */

        do
        {

            /* Read an entry from the directory.  */
            status = _fx_directory_entry_read(media_ptr, &search_directory, &i, &search_entry);

            /* Check for error status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return error condition.  */
                return(status);
            }

            /* Determine if this is the last directory entry.  */
#ifdef FX_ENABLE_EXFAT
            if (search_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER)
#else
            if (search_entry.fx_dir_entry_name[0] == FX_DIR_ENTRY_DONE)
#endif /* FX_ENABLE_EXFAT */
            {
                break;
            }

#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {

                /* Skip '.' and '..' folders if exists.  */
                if ((i == 1) && (search_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY) &&
                    (search_entry.fx_dir_entry_name[0] == '.') &&
                    (search_entry.fx_dir_entry_name[1] == 0))
                {

                    continue;
                }
                if ((i == 2) && (search_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY) &&
                    (search_entry.fx_dir_entry_name[0] == '.') && (search_entry.fx_dir_entry_name[1] == '.') &&
                    (search_entry.fx_dir_entry_name[2] == 0))
                {

                    continue;
                }
            }
#endif /* FX_ENABLE_EXFAT */

            /* Determine if this is an empty entry.  */
#ifdef FX_ENABLE_EXFAT
            if (search_entry.fx_dir_entry_type != FX_EXFAT_DIR_ENTRY_TYPE_FREE)
#else
            if ((UCHAR)search_entry.fx_dir_entry_name[0] != (UCHAR)FX_DIR_ENTRY_FREE)
#endif /* FX_ENABLE_EXFAT */
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return error status.  */
                return(FX_DIR_NOT_EMPTY);
            }

            i++;
        } while (i < directory_size);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* At this point, we are going to delete the empty directory.  */

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Invalidate the directory search saved information.  */
    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;
#endif

    /* Mark the sub-directory entry as available.  */
    dir_entry.fx_dir_entry_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;
    dir_entry.fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;

    /* Now write out the directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        status =  _fx_directory_exFAT_entry_write(media_ptr, &dir_entry, UPDATE_DELETE);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        status =  _fx_directory_entry_write(media_ptr, &dir_entry);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

#ifdef FX_ENABLE_EXFAT
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    i = 0;

    clusters_count = (ULONG)((search_directory.fx_dir_entry_file_size + bytes_per_cluster - 1) / bytes_per_cluster - 1);
#endif /* FX_ENABLE_EXFAT */

    /* Walk through the directory's clusters and release them.  */
    cluster =  search_directory.fx_dir_entry_cluster;

    while (cluster < media_ptr -> fx_media_fat_reserved)
    {

        /* Increment the cluster count.  */
        i++;

#ifdef FX_ENABLE_EXFAT

        /* Read the next FAT entry.  */
        if (search_directory.fx_dir_entry_dont_use_fat & 1)
        {

            /* Check for file size range */
            if (i - 1 >= clusters_count)
            {
                next_cluster = FX_LAST_CLUSTER_exFAT;
            }
            else
            {
                next_cluster = cluster + 1;
            }
        }
        else
        {

#endif /* FX_ENABLE_EXFAT */

            /* Read the next FAT entry.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

            /* Check the return status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(status);
            }
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        if ((cluster < FX_FAT_ENTRY_START) || (cluster == next_cluster) || (i > media_ptr -> fx_media_total_clusters))
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the bad status.  */
            return(FX_FAT_READ_ERROR);
        }

        /* Release the current cluster.  */
#ifdef FX_ENABLE_EXFAT
        if (!(search_directory.fx_dir_entry_dont_use_fat & 1))
        {
#endif /* FX_ENABLE_EXFAT */

            status =  _fx_utility_FAT_entry_write(media_ptr, cluster, FX_FREE_CLUSTER);

            /* Check the return status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(status);
            }
#ifdef FX_ENABLE_EXFAT
        }

        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {

            /* Mark the cluster as free.  */
            status = _fx_utility_exFAT_cluster_state_set(media_ptr, cluster, FX_EXFAT_BITMAP_CLUSTER_FREE);

            /* Check the return status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(status);
            }
        }
#endif /* FX_ENABLE_EXFAT */

        /* Increment the number of available clusters for the media.  */
        media_ptr -> fx_media_available_clusters++;

        /* Copy next cluster to current cluster.  */
        cluster =  next_cluster;
    }

#ifdef FX_FAULT_TOLERANT

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);
#endif

    /* Flush the logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64)(media_ptr -> fx_media_sectors_per_FAT), FX_FALSE);

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Check for a bad status.  */
    if (status != FX_SUCCESS)
    {

        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the bad status.  */
        return(status);
    }

    /* End transaction. */
    status = _fx_fault_tolerant_transaction_end(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* Directory delete is complete, return status.  */
    return(status);
}

