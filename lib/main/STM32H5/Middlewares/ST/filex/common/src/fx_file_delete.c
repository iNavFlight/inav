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
/**   File                                                                */
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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_delete                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified file.  If found, */
/*    the delete request is valid and all of its clusters will be         */
/*    released and its directory entry will be marked as available.       */
/*    Otherwise, if the file is not found, the appropriate error code is  */
/*    returned to the caller.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_name                             File name pointer             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_fault_tolerant_transaction_start  Start fault tolerant          */
/*                                            transaction                 */
/*    _fx_fault_tolerant_transaction_end    End fault tolerant transaction*/
/*    _fx_fault_tolerant_recover            Recover FAT chain             */
/*    _fx_fault_tolerant_reset_log_file     Reset the log file            */
/*    _fx_fault_tolerant_set_FAT_chain      Set data of FAT chain         */
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
UINT  _fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name)
{

UINT         status;
ULONG        cluster;
ULONG        contents;
ULONG        open_count;
FX_FILE     *search_ptr;
ULONG        cluster_count;
FX_DIR_ENTRY dir_entry;
UCHAR        not_a_file_attr;
#ifdef FX_ENABLE_EXFAT
ULONG        bytes_per_cluster;
ULONG        clusters_count;
#endif /* FX_ENABLE_EXFAT */

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_deletes++;
#endif

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_DELETE, media_ptr, file_name, 0, 0, FX_TRACE_FILE_EVENTS, 0, 0)

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

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, file_name, &dir_entry, FX_NULL, FX_NULL);

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

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        not_a_file_attr = FX_DIRECTORY;
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        not_a_file_attr = FX_DIRECTORY | FX_VOLUME;
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Check to make sure the found entry is a file.  */
    if (dir_entry.fx_dir_entry_attributes & not_a_file_attr)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a file error code.  */
        return(FX_NOT_A_FILE);
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

    /* Search the opened files to see if this file is currently
       opened.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    search_ptr =  media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Look at each opened file to see if the same file is opened.  */
        if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector == dir_entry.fx_dir_entry_log_sector) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset == dir_entry.fx_dir_entry_byte_offset))
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* The file is currently open.  */
            return(FX_ACCESS_ERROR);
        }

        /* Adjust the pointer and decrement the search count.  */
        search_ptr =  search_ptr -> fx_file_opened_next;
        open_count--;
    }

    /* Pickup the starting cluster of the file.  */
    cluster =           dir_entry.fx_dir_entry_cluster;

    /* At this point, make the directory entry invalid in order to delete the file.  */

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Invalidate the directory search saved information.  */
    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;
#endif

    /* Mark the directory entry as available, while leaving the other
       information for the sake of posterity.  */
    dir_entry.fx_dir_entry_name[0] =        (CHAR)FX_DIR_ENTRY_FREE;
    dir_entry.fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;

    /* Now write out the directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        status = _fx_directory_exFAT_entry_write(media_ptr, &dir_entry, UPDATE_DELETE);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        status = _fx_directory_entry_write(media_ptr, &dir_entry);
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

    /* Now that the directory entry is no longer valid and pointing at the chain of clusters,
       walk the chain of allocated FAT entries and mark each of them as free.  */
    cluster_count =     0;

#ifdef FX_ENABLE_EXFAT
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    clusters_count =
        (ULONG)((dir_entry.fx_dir_entry_file_size + bytes_per_cluster - 1) / bytes_per_cluster - 1);
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        /* Note:  Directory entries are already written to log files. FAT chain is updated as the last step.
           Since there are no others FAT entries written to the log.  Therefore there is no need to set
           the flag FX_FRAULT_TOLERANT_STATE_SET_FAT_CHAIN here. */
#ifdef FX_ENABLE_EXFAT
        if (dir_entry.fx_dir_entry_dont_use_fat & 1)
        {
            status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_TRUE, 0,
                                                      media_ptr -> fx_media_fat_last, cluster, cluster + clusters_count);
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */
            status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_FALSE, 0,
                                                      media_ptr -> fx_media_fat_last, cluster, media_ptr -> fx_media_fat_last);
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        /* Determine if the write was successful.  */
        if (status != FX_SUCCESS)
        {

            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error code.  */
            return(status);
        }
    }
    else
    {
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Follow the link of FAT entries.  */
        while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
        {

            /* Increment the number of clusters.  */
            cluster_count++;

#ifdef FX_ENABLE_EXFAT
            if (dir_entry.fx_dir_entry_dont_use_fat & 1)
            {

                /* Check for file size range.  */
                if (cluster_count - 1 >= clusters_count)
                {
                    contents = FX_LAST_CLUSTER_exFAT;
                }
                else
                {
                    contents = cluster + 1;
                }
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */

                /* Read the current cluster entry from the FAT.  */
                status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &contents);

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            if ((cluster == contents) || (cluster_count > media_ptr -> fx_media_total_clusters))
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the bad status.  */
                return(FX_FAT_READ_ERROR);
            }

#ifdef FX_ENABLE_EXFAT
            if (!(dir_entry.fx_dir_entry_dont_use_fat & 1))
            {
#endif /* FX_ENABLE_EXFAT */

                /* Make the current cluster available.  */
                status =  _fx_utility_FAT_entry_write(media_ptr, cluster, FX_FREE_CLUSTER);

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
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

            /* Setup for the next cluster.  */
            cluster =  contents;
        }
#ifdef FX_ENABLE_FAULT_TOLERANT
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Update the free clusters in the media control block.  */
    media_ptr -> fx_media_available_clusters =
        media_ptr -> fx_media_available_clusters + cluster_count;

#ifdef FX_FAULT_TOLERANT

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);
#endif

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* End transaction. */
    status = _fx_fault_tolerant_transaction_end(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* File delete is complete, return status.  */
    return(status);
}

