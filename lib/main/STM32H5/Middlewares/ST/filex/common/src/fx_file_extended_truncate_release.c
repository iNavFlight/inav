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
/*    _fx_file_extended_truncate_release                  PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the file to the specified size, if smaller than  */
/*    the current file size.  If the new file size is less than the       */
/*    current file read/write position, the internal file pointers will   */
/*    also be modified.  Any unused clusters are released back to the     */
/*    media.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    size                                  New size of the file in bytes */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write directory entry         */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
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
/*  12-31-2020     William E. Lamie         Modified comment(s), fixed    */
/*                                            available cluster issue,    */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_file_extended_truncate_release(FX_FILE *file_ptr, ULONG64 size)
{

UINT                   status;
ULONG                  cluster;
ULONG                  contents;
ULONG                  bytes_per_cluster;
ULONG                  last_cluster;
ULONG                  cluster_count;
ULONG64                bytes_remaining;
FX_MEDIA              *media_ptr;

#ifdef FX_ENABLE_EXFAT
ULONG                  original_last_physical_cluster;
#endif /* FX_ENABLE_EXFAT */

#ifndef FX_DONT_UPDATE_OPEN_FILES
ULONG                  open_count;
FX_FILE               *search_ptr;
#endif

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Setup pointer to media structure.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_truncate_releases++;
#endif

    /* Setup pointer to associated media control block.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_TRUNCATE_RELEASE, file_ptr, size, file_ptr -> fx_file_current_file_size, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Make sure this file is open for writing.  */
    if (file_ptr -> fx_file_open_mode != FX_OPEN_FOR_WRITE)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the access error exception - a write was attempted from
           a file opened for reading!  */
        return(FX_ACCESS_ERROR);
    }

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

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    /* Check for invalid value.  */
    if (bytes_per_cluster == 0)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Invalid media, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Setup the new file available size - if less than the current available size.  */
    if (size < file_ptr -> fx_file_current_available_size)
    {

        /* Yes, the file needs to be truncated.  */

        /* Update the available file size.  */
        file_ptr -> fx_file_current_available_size =  ((size + bytes_per_cluster - 1) / bytes_per_cluster) * bytes_per_cluster;

        /* Is the new available size less than the actual file size?  */
        if (size < file_ptr -> fx_file_current_file_size)
        {

            /* Setup the new file size.  */
            file_ptr -> fx_file_current_file_size =  size;

            /* Set the modified flag.  */
            file_ptr -> fx_file_modified =  FX_TRUE;

            /* Copy the new file size into the directory entry.  */
            file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =  size;

            /* Set the first cluster to NULL.  */
            if (size == 0)
            {

                /* Yes, the first cluster needs to be cleared since the entire
                   file is going to be truncated.  */
                file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =  FX_NULL;
            }

            /* Write the directory entry to the media.  */
#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {
                status = _fx_directory_exFAT_entry_write(
                        media_ptr, &(file_ptr -> fx_file_dir_entry), UPDATE_STREAM);
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */
                status = _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Check for a good status.  */
            if (status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Error writing the directory.  */
                return(status);
            }
        }

        /* Update the trace event with the truncated size.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_TRUNCATE_RELEASE, 0, 0, 0, file_ptr -> fx_file_current_file_size)
    }
    else
    {

        /* Update the trace event with the truncated size.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_TRUNCATE_RELEASE, 0, 0, 0, file_ptr -> fx_file_current_file_size)

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Just return, the truncate size is larger than the available size.  */
        return(FX_SUCCESS);
    }

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    /* Now check to see if the read/write internal file pointers need
       to be adjusted.  */
    if (file_ptr -> fx_file_current_file_offset > file_ptr -> fx_file_current_file_size)
    {


        /* At this point, we are ready to walk list of clusters to setup the
           seek position of this file.  */
        cluster =           file_ptr -> fx_file_first_physical_cluster;
        bytes_remaining =   size;
        last_cluster =      0;
        cluster_count =     0;

        /* Follow the link of FAT entries.  */
        while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
        {

            /* Increment the number of clusters.  */
            cluster_count++;

#ifdef FX_ENABLE_EXFAT
            if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
            {
                if (cluster >= file_ptr -> fx_file_last_physical_cluster)
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

            /* Save the last valid cluster.  */
            last_cluster =  cluster;

            /* Setup for the next cluster.  */
            cluster =  contents;

            /* Determine if this is the last written cluster.  */
            if (bytes_remaining >= bytes_per_cluster)
            {

                /* Still more seeking, just decrement the working byte offset.  */
                bytes_remaining =  bytes_remaining - bytes_per_cluster;
            }
            else
            {

                /* This is the cluster that contains the seek position.  */
                break;
            }
        }
        
        /* Check for errors in traversal of the FAT chain.  */
        if (size > (((ULONG64) bytes_per_cluster) * ((ULONG64) cluster_count)))
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* This is an error that suggests a corrupt file.  */
            return(FX_FILE_CORRUPT);
        }

        /* Position the pointers to the new offset.  */
        file_ptr -> fx_file_current_physical_cluster =  last_cluster;
        file_ptr -> fx_file_current_relative_cluster =  cluster_count - 1;
        file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
            (((ULONG64)file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
             ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
            (bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector);
        file_ptr -> fx_file_current_relative_sector =   (UINT)((bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector));
        file_ptr -> fx_file_current_file_offset =       size;
        file_ptr -> fx_file_current_logical_offset =    (ULONG)bytes_remaining % ((ULONG)media_ptr -> fx_media_bytes_per_sector);
    }

    /* Determine how many clusters are actually in-use now.  */
    cluster_count =  (ULONG) (file_ptr -> fx_file_current_available_size + (((ULONG64) bytes_per_cluster) - ((ULONG64) 1)))/bytes_per_cluster;

    /* Save the number of clusters in-use.  */
    file_ptr -> fx_file_total_clusters =  cluster_count;

    /* At this point, we are ready to walk list of clusters to find the clusters
       that can be released.  */
    cluster =           file_ptr -> fx_file_first_physical_cluster;
#ifdef FX_ENABLE_EXFAT
    original_last_physical_cluster = file_ptr -> fx_file_last_physical_cluster;
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Is this the last used cluster?  */
    if ((cluster_count == 0) && (media_ptr -> fx_media_fault_tolerant_enabled))
    {
#ifdef FX_ENABLE_EXFAT
        if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
        {
#endif /* FX_ENABLE_EXFAT */

            /* Set undo log. */
            status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_FALSE, FX_FREE_CLUSTER,
                                                      media_ptr -> fx_media_fat_last, cluster, media_ptr -> fx_media_fat_last);

            /* Determine if the write was successful.  */
            if (status != FX_SUCCESS)
            {

                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error code.  */
                return(status);
            }
#ifdef FX_ENABLE_EXFAT
        }
        else
        {

            /* Set undo log. */
            status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_TRUE, FX_FREE_CLUSTER,
                                                      media_ptr -> fx_media_fat_last, cluster, file_ptr -> fx_file_last_physical_cluster + 1);

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
#endif /* FX_ENABLE_EXFAT && FX_ENABLE_FAULT_TOLERANT */

        file_ptr -> fx_file_last_physical_cluster =  cluster;
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Follow the link of FAT entries.  */
    while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
    {
#ifdef FX_ENABLE_EXFAT
        if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
        {
            if (cluster >= original_last_physical_cluster)
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

        /* Determine if are more clusters to release.  */
        if (cluster_count > 0)
        {

            /* Decrement the number of clusters.  */
            cluster_count--;

            /* Is this the last used cluster?  */
            if (cluster_count == 0)
            {
#ifdef FX_ENABLE_EXFAT
                if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
#endif /* FX_ENABLE_EXFAT */
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    if (media_ptr -> fx_media_fault_tolerant_enabled)
                    {

                        /* Set undo phase. */
                        media_ptr -> fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN;

                        /* Read the current cluster entry from the FAT.  */
                        status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &contents);

                        /* Check the return value.  */
                        if (status != FX_SUCCESS)
                        {

                            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                            /* Release media protection.  */
                            FX_UNPROTECT

                            /* Return the error status.  */
                            return(status);
                        }

                        /* Set undo log. */
                        status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_FALSE, cluster,
                                                                  media_ptr -> fx_media_fat_last, contents, media_ptr -> fx_media_fat_last);

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
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Yes, it should be designated as last cluster.  */
                    status = _fx_utility_FAT_entry_write(media_ptr, cluster, media_ptr -> fx_media_fat_last);

                    /* Check the return value.  */
                    if (status)
                    {

#ifdef FX_ENABLE_FAULT_TOLERANT
                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the error status.  */
                        return(status);
                    }

#ifdef FX_ENABLE_FAULT_TOLERANT
                    if (media_ptr -> fx_media_fault_tolerant_enabled)
                    {

                        /* Clear undo phase. */
                        media_ptr -> fx_media_fault_tolerant_state &= (UCHAR)(~FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN & 0xff);
                    }
#endif /* FX_ENABLE_FAULT_TOLERANT */
                }
#if defined(FX_ENABLE_EXFAT) && defined(FX_ENABLE_FAULT_TOLERANT)
                else if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Set undo log. */
                    status = _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_TRUE, cluster,
                                                              media_ptr -> fx_media_fat_last, contents, file_ptr -> fx_file_last_physical_cluster + 1);

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
#endif /* FX_ENABLE_EXFAT && FX_ENABLE_FAULT_TOLERANT */

                file_ptr -> fx_file_last_physical_cluster =  cluster;

#ifdef FX_FAULT_TOLERANT

                /* Flush the cached individual FAT entries.  */
                status = _fx_utility_FAT_flush(media_ptr);

                /* Check the return value.  */
                if (status)
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }
#endif
            }
        }
        else
        {

            /* This is a cluster after the clusters used by the file, release
               it back to the media.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled == FX_FALSE)
            {
#endif /* FX_ENABLE_FAULT_TOLERANT */

#ifdef FX_ENABLE_EXFAT
                if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
                {
#endif /* FX_ENABLE_EXFAT */
                    status = _fx_utility_FAT_entry_write(media_ptr, cluster, FX_FREE_CLUSTER);

                    /* Check the return value.  */
                    if (status)
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
                    _fx_utility_exFAT_cluster_state_set(media_ptr, cluster, FX_EXFAT_BITMAP_CLUSTER_FREE);
                }
#endif /* FX_ENABLE_EXFAT */

                /* Increment the number of available clusters.  */
                media_ptr -> fx_media_available_clusters++;

#ifdef FX_ENABLE_FAULT_TOLERANT
            }
#endif /* FX_ENABLE_FAULT_TOLERANT */
        }

        /* Setup for the next cluster.  */
        cluster =  contents;
    }

    /* Determine if we need to adjust the number of leading consecutive clusters.  */
    if (file_ptr -> fx_file_consecutive_cluster > file_ptr -> fx_file_total_clusters)
    {

        /* Adjust the leading consecutive cluster count. */
        file_ptr -> fx_file_consecutive_cluster =  file_ptr -> fx_file_total_clusters;
    }

    /* Determine if the file available size has been truncated to zero.  */
    if (file_ptr -> fx_file_current_available_size == 0)
    {

        /* Yes, the first cluster has already been released.  Update the file info
           to indicate the file has no clusters.  */
        file_ptr -> fx_file_last_physical_cluster =     0;
        file_ptr -> fx_file_first_physical_cluster =    0;
        file_ptr -> fx_file_current_physical_cluster =  0;
        file_ptr -> fx_file_current_logical_sector =    0;
        file_ptr -> fx_file_current_relative_cluster =  0;
        file_ptr -> fx_file_current_relative_sector =   0;
        file_ptr -> fx_file_current_available_size =    0;
        file_ptr -> fx_file_consecutive_cluster =       1;
    }

#ifndef FX_DONT_UPDATE_OPEN_FILES

    /* Search the opened files list to see if the same file is opened for reading.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    search_ptr =  media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Is this file the same file opened for reading?  */
        if ((search_ptr != file_ptr) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector ==
             file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset ==
             file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset))
        {

            /* Yes, the same file is opened for reading.  */

            /* Setup the new file size.  */
            search_ptr -> fx_file_current_file_size =  size;

            /* Setup the new total clusters.  */
            search_ptr -> fx_file_total_clusters =  file_ptr -> fx_file_total_clusters;

            /* Copy the directory entry.  */
            search_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =      file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =    file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector =   file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset =  file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset;

            /* Setup the other file parameters.  */
            search_ptr -> fx_file_last_physical_cluster =     file_ptr -> fx_file_last_physical_cluster;
            search_ptr -> fx_file_first_physical_cluster =    file_ptr -> fx_file_first_physical_cluster;
            search_ptr -> fx_file_current_available_size =    file_ptr -> fx_file_current_available_size;
            search_ptr -> fx_file_consecutive_cluster =       file_ptr -> fx_file_consecutive_cluster;

            /* Determine if the truncated file is smaller than the current file offset.  */
            if (search_ptr -> fx_file_current_file_offset > size)
            {

                /* Yes, the current file parameters need to be changed since the file was
                   truncated to a position prior to the current file position.  */

                /* Calculate the number of bytes per cluster.  */
                bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
                    ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

                /* At this point, we are ready to walk list of clusters to setup the
                   seek position of this file.  */
                cluster =           search_ptr -> fx_file_first_physical_cluster;
                bytes_remaining =   size;
                last_cluster =      0;
                cluster_count =     0;

                /* Follow the link of FAT entries.  */
                while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                {

                    /* Increment the number of clusters.  */
                    cluster_count++;

#ifdef FX_ENABLE_EXFAT
                    if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
                    {
                        if (cluster >= file_ptr -> fx_file_last_physical_cluster)
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

                    /* Save the last valid cluster.  */
                    last_cluster =  cluster;

                    /* Setup for the next cluster.  */
                    cluster =  contents;

                    /* Determine if this is the last written cluster.  */
                    if (bytes_remaining >= bytes_per_cluster)
                    {

                        /* Still more seeking, just decrement the working byte offset.  */
                        bytes_remaining =  bytes_remaining - bytes_per_cluster;
                    }
                    else
                    {

                        /* This is the cluster that contains the seek position.  */
                        break;
                    }
                }

                /* Check for errors in traversal of the FAT chain.  */
                if (size > (((ULONG64) bytes_per_cluster) * ((ULONG64) cluster_count)))
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* This is an error that suggests a corrupt file.  */
                    return(FX_FILE_CORRUPT);
                }

                /* Position the pointers to the new offset.  */

                /* Determine if there is at least one cluster.  */
                if (cluster_count)
                {

                    /* Calculate real file parameters.  */
                    search_ptr -> fx_file_current_physical_cluster =  last_cluster;
                    search_ptr -> fx_file_current_relative_cluster =  cluster_count - 1;
                    search_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                        (((ULONG64)search_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
                         ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                        (bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector);
                    search_ptr -> fx_file_current_relative_sector =   (UINT)((bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector));
                    search_ptr -> fx_file_current_file_offset =       size;
                    search_ptr -> fx_file_current_logical_offset =    (ULONG)bytes_remaining % ((ULONG)media_ptr -> fx_media_bytes_per_sector);
                }
                else
                {

                    /* Calculate zero-length file parameters.  */
                    search_ptr -> fx_file_current_physical_cluster =  0;
                    search_ptr -> fx_file_current_relative_cluster =  0;
                    search_ptr -> fx_file_current_logical_sector =    0;
                    search_ptr -> fx_file_current_relative_sector =   0;
                    search_ptr -> fx_file_current_file_offset =       0;
                    search_ptr -> fx_file_current_logical_offset =    0;
                }
            }
        }

        /* Adjust the pointer and decrement the search count.  */
        search_ptr =  search_ptr -> fx_file_opened_next;
        open_count--;
    }
#endif

#ifdef FX_FAULT_TOLERANT

    /* Flush the cached individual FAT entries */
    status = _fx_utility_FAT_flush(media_ptr);

    /* Check the return value.  */
    if (status)
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
    /* Flush Bit Map.  */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        _fx_utility_exFAT_bitmap_flush(media_ptr);
    }
#endif /* FX_ENABLE_EXFAT */

#endif

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size = file_ptr -> fx_file_current_file_size;
    }

    /* Write the directory entry to the media.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        status = _fx_directory_exFAT_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry), UPDATE_STREAM);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        status =  _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Check for a good status.  */
    if (status != FX_SUCCESS)
    {

        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

        /* Release media protection.  */
        FX_UNPROTECT

        /* Error writing the directory.  */
        return(status);
    }

    /* End transaction. */
    status = _fx_fault_tolerant_transaction_end(media_ptr);

    /* Check for a bad status.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the bad status.  */
        return(status);
    }

    /* Update maximum size used if necessary. */
    if (size < file_ptr -> fx_file_maximum_size_used)
    {
        file_ptr -> fx_file_maximum_size_used = size;
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* Truncate is complete, return successful status.  */
    return(FX_SUCCESS);
}

