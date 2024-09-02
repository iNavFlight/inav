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
#include "fx_file.h"
#include "fx_directory.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_write                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the specified number of bytes into the file's  */
/*    data area.  The status of the write operation is returned to the    */
/*    caller.  In addition, various internal file pointers in the file    */
/*    control block are also updated.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    buffer_ptr                            Buffer pointer                */
/*    size                                  Number of bytes to write      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Update the file's size        */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_exFAT_bitmap_free_cluster_find                          */
/*                                          Find exFAT free cluster       */
/*    _fx_utility_exFAT_cluster_state_get   Get cluster state             */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_utility_logical_sector_flush      Flush written logical sectors */
/*    _fx_utility_logical_sector_read       Read a logical sector         */
/*    _fx_utility_logical_sector_write      Write a logical sector        */
/*    _fx_utility_memory_copy               Fast memory copy routine      */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), verified */
/*                                            memcpy usage,               */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size)
{

UINT                   status;
ULONG64                bytes_remaining;
ULONG                  i;
ULONG                  copy_bytes;
ULONG                  bytes_per_cluster;
UCHAR                 *source_ptr;
ULONG                  first_new_cluster;
ULONG                  last_cluster;
ULONG                  cluster, next_cluster;
ULONG                  FAT_index;
ULONG                  FAT_value;
ULONG                  clusters;
ULONG                  total_clusters;
UINT                   sectors;
FX_MEDIA              *media_ptr;

#ifdef FX_ENABLE_EXFAT
UCHAR                  cluster_state;
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_FAULT_TOLERANT_DATA
FX_INT_SAVE_AREA
#endif

#ifndef FX_DONT_UPDATE_OPEN_FILES
ULONG                  open_count;
FX_FILE               *search_ptr;
#endif

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif

#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR                  data_append = FX_FALSE;      /* Whether or not new data would extend beyond the size of the original file.
                                                      Operations such as append or overwrite could cause the new file to exceed its
                                                      original size, resulting in data-appending operation. */
ULONG                  copy_head_cluster = 0;       /* The first cluster of the original chain that needs to be replaced. */
ULONG                  copy_tail_cluster = 0;       /* The last cluster of the original chain that needs to be replaced. */
ULONG                  insertion_back;              /* The insertion point (back) */
ULONG                  insertion_front = 0;         /* The insertion point (front) */
ULONG                  replace_clusters = 0;        /* The number of clusters to be replaced. */
UCHAR                  dont_use_fat_old = FX_FALSE; /* Used by exFAT logic to indicate whether or not the FAT table should be used. */
#endif /* FX_ENABLE_FAULT_TOLERANT */


    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

    /* Setup pointer to media structure.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_writes++;
#endif


#ifdef FX_ENABLE_EXFAT
    if ((media_ptr -> fx_media_FAT_type != FX_exFAT) &&
        (file_ptr -> fx_file_current_file_offset + size > 0xFFFFFFFFULL))
#else
    if (file_ptr -> fx_file_current_file_offset + size > 0xFFFFFFFFULL)
#endif /* FX_ENABLE_EXFAT */
    {

        /* Return the no more space error, since the new file size would be larger than
           the 32-bit field to represent it in the file's directory entry.  */
        return(FX_NO_MORE_SPACE);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Initialize next cluster of tail. */
    insertion_back = media_ptr -> fx_media_fat_last;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_WRITE, file_ptr, buffer_ptr, size, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Make sure this file is open for writing.  */
    if (file_ptr -> fx_file_open_mode != FX_OPEN_FOR_WRITE)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the access error exception - a write was attempted from
           a file opened for reading!  */
        return(FX_ACCESS_ERROR);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

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

    /* Initialized first new cluster. */
    first_new_cluster =  0;

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Calculate clusters need to be replaced when fault tolerant is enabled. */
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        if (file_ptr -> fx_file_current_file_offset == file_ptr -> fx_file_current_file_size)
        {

            /* Appending data. No need to replace existing clusters. */
            replace_clusters = 0;
        }
        else if (file_ptr -> fx_file_current_file_offset == file_ptr -> fx_file_maximum_size_used)
        {

            /* Similar to append data. No actual data after fx_file_maximum_size_used.
             * No need to replace existing clusters. */
            replace_clusters = 0;
        }
        else if (((file_ptr -> fx_file_current_file_offset / media_ptr -> fx_media_bytes_per_sector) ==
                 ((file_ptr -> fx_file_current_file_offset + size - 1) / media_ptr -> fx_media_bytes_per_sector)) &&
                 ((file_ptr -> fx_file_current_file_offset + size) <= file_ptr -> fx_file_current_file_size))
        {

            /* Overwriting data in one sector. No need to replace existing clusters. */
            replace_clusters = 0;
        }
        else if ((file_ptr -> fx_file_current_available_size - file_ptr -> fx_file_current_file_offset) < size)
        {

            /* Replace all clusters from current_cluster. */
            replace_clusters = (UINT)(file_ptr -> fx_file_total_clusters - file_ptr -> fx_file_current_relative_cluster);
            copy_head_cluster = file_ptr -> fx_file_current_physical_cluster;

            data_append = FX_TRUE;
        }
        else
        {

            /* Replace clusters from current_cluster to the end of written cluster. */
            replace_clusters = (UINT)((file_ptr -> fx_file_current_file_offset + size + bytes_per_cluster - 1) / bytes_per_cluster -
                                      file_ptr -> fx_file_current_relative_cluster);
            copy_head_cluster = file_ptr -> fx_file_current_physical_cluster;
            data_append = FX_FALSE;
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Next, determine if there is enough room to write the specified number of
       bytes to the clusters already allocated to this file.  */
    if (((file_ptr -> fx_file_current_available_size - file_ptr -> fx_file_current_file_offset) < size)
#ifdef FX_ENABLE_FAULT_TOLERANT
        || (replace_clusters)
#endif /* FX_ENABLE_FAULT_TOLERANT */
       )
    {

        /* The new request will not fit within the currently allocated clusters.  */

        /* Calculate the number of additional clusters that must be allocated for this
           write request.  */
#ifdef FX_ENABLE_FAULT_TOLERANT
        /* Find the number of clusters that need to be replaced. */
        if (media_ptr -> fx_media_fault_tolerant_enabled)
        {

            /* Mark the recovery phase. */
            media_ptr -> fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN;

            bytes_remaining = file_ptr -> fx_file_current_file_offset + size;

            if (replace_clusters > 0)
            {

#ifdef FX_ENABLE_EXFAT
                if ((media_ptr -> fx_media_FAT_type == FX_exFAT) && (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
                {
                    /* Find the insertion points in the exFAT with bitmap case.. this is a simple case.. */
                    /* The previous cluster is not initialized. Find it. */
                    if (copy_head_cluster != file_ptr -> fx_file_first_physical_cluster)
                    {
                        insertion_front = copy_head_cluster - 1;
                    }

                    /* Find copy tail cluster. */
                    if (data_append == FX_FALSE)
                    {
                        copy_tail_cluster = file_ptr -> fx_file_first_physical_cluster + (ULONG)((bytes_remaining - 1) / bytes_per_cluster);
                        if (copy_tail_cluster != file_ptr -> fx_file_last_physical_cluster)
                        {
                            insertion_back = copy_tail_cluster + 1;
                        }
                    }
                }
                else
                {
#endif /* FX_ENABLE_EXFAT */

                    /* Find previous cluster of copy head cluster. */
                    /* The previous cluster is not initialized. Find it. */
                    if (copy_head_cluster != file_ptr -> fx_file_first_physical_cluster)
                    {

                        /* The copy head cluster is not the first cluster of file. */
                        /* Copy head is not the first cluster of file. */
                        if (file_ptr -> fx_file_current_relative_cluster < file_ptr -> fx_file_consecutive_cluster)
                        {

                            /* Clusters before current cluster are consecutive. */
                            insertion_front = copy_head_cluster - 1;
                            bytes_remaining -= file_ptr -> fx_file_current_relative_cluster * bytes_per_cluster;
                        }
                        else
                        {

                            /* Skip consecutive clusters first. */
                            cluster = file_ptr -> fx_file_first_physical_cluster;

                            /* Loop the link of FAT to find the previous cluster. */
                            while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                            {

                                /* Reduce remaining bytes. */
                                bytes_remaining -= bytes_per_cluster;

                                /* Read the current cluster entry from the FAT.  */
                                status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &FAT_value);

                                /* Check the return value.  */
                                if (status != FX_SUCCESS)
                                {

                                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                                    /* Release media protection.  */
                                    FX_UNPROTECT

                                    /* Return the error status.  */
                                    return(status);
                                }

                                if (FAT_value == copy_head_cluster)
                                {
                                    break;
                                }

                                /* Move to next cluster. */
                                cluster = FAT_value;
                            }

                            if ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                            {

                                /* Find the previous cluster. */
                                insertion_front = cluster;
                            }
                            else
                            {

                                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                                /* Release media protection.  */
                                FX_UNPROTECT

                                /* Return the error status.  */
                                return(FX_NOT_FOUND);
                            }
                        }
                    }

                    /* Find copy tail cluster. */
                    if (bytes_remaining <= bytes_per_cluster)
                    {

                        /* Only one cluster is modified. */
                        copy_tail_cluster = copy_head_cluster;
                    }
                    else if (data_append == FX_FALSE)
                    {

                        /* Search from copy head cluster. */
                        cluster = copy_head_cluster;
                        FAT_value = FX_FAT_ENTRY_START;

                        /* Loop the link of FAT to find the previous cluster. */
                        while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                        {

                            if (bytes_remaining <= bytes_per_cluster)
                            {
                                break;
                            }

                            /* Reduce remaining bytes. */
                            bytes_remaining -= bytes_per_cluster;

                            /* Read the current cluster entry from the FAT.  */
                            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &FAT_value);

                            /* Check the return value.  */
                            if (status != FX_SUCCESS)
                            {

                                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                                /* Release media protection.  */
                                FX_UNPROTECT

                                /* Return the error status.  */
                                return(status);
                            }

                            /* Move to next cluster. */
                            cluster = FAT_value;
                        }

                        /* Find the previous cluster. */
                        copy_tail_cluster = FAT_value;
                    }

                    /* Get the cluster next to copy tail. */
                    if (data_append == FX_FALSE)
                    {

                        /* Read FAT entry.  */
                        status =  _fx_utility_FAT_entry_read(media_ptr, copy_tail_cluster, &insertion_back);

                        /* Check for a bad status.  */
                        if (status != FX_SUCCESS)
                        {

                            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                            /* Release media protection.  */
                            FX_UNPROTECT

                            /* Return the bad status.  */
                            return(status);
                        }
                    }
                    else
                    {
                        insertion_back = media_ptr -> fx_media_fat_last;
                    }
#ifdef FX_ENABLE_EXFAT
                }
#endif /* FX_ENABLE_EXFAT */
            }
            else
            {
                insertion_back = media_ptr -> fx_media_fat_last;
            }
        }

        if (file_ptr -> fx_file_current_available_size - file_ptr -> fx_file_current_file_offset < size)
        {
#endif /* FX_ENABLE_FAULT_TOLERANT */
            /* Calculate clusters that are needed for data append except ones overwritten. */
            clusters =  (UINT)((size + (bytes_per_cluster - 1) -
                                (file_ptr -> fx_file_current_available_size - file_ptr -> fx_file_current_file_offset)) /
                               bytes_per_cluster);
#ifdef FX_ENABLE_FAULT_TOLERANT
        }
        else
        {
            clusters = 0;
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Determine if we have enough space left.  */
#ifdef FX_ENABLE_FAULT_TOLERANT
        if (clusters + replace_clusters > media_ptr -> fx_media_available_clusters)
#else
        if (clusters > media_ptr -> fx_media_available_clusters)
#endif /* FX_ENABLE_FAULT_TOLERANT */
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Out of disk space.  */
            return(FX_NO_MORE_SPACE);
        }

        /* Update the file total cluster count.  */
        file_ptr -> fx_file_total_clusters =  file_ptr -> fx_file_total_clusters + clusters;

        /* Check for wrap-around when updating the available size.  */
#ifdef FX_ENABLE_EXFAT
        if ((media_ptr -> fx_media_FAT_type != FX_exFAT) &&
            (file_ptr -> fx_file_current_available_size + (ULONG64)bytes_per_cluster * (ULONG64)clusters > 0xFFFFFFFFULL))
#else
        if (file_ptr -> fx_file_current_available_size + (ULONG64)bytes_per_cluster * (ULONG64)clusters > 0xFFFFFFFFULL)
#endif /* FX_ENABLE_EXFAT */
        {

            /* 32-bit wrap around condition is present.  Just set the available file size to all ones, which is
               the maximum file size.  */
            file_ptr -> fx_file_current_available_size =  0xFFFFFFFFULL;
        }
        else
        {

            /* Normal condition, update the available size.  */
            file_ptr -> fx_file_current_available_size =
                file_ptr -> fx_file_current_available_size + (ULONG64)bytes_per_cluster * (ULONG64)clusters;
        }

#ifdef FX_ENABLE_FAULT_TOLERANT
        /* Account for newly allocated clusters. */
        clusters += replace_clusters;
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Decrease the available clusters in the media control block. */
        media_ptr -> fx_media_available_clusters =  media_ptr -> fx_media_available_clusters - clusters;

#if defined(FX_ENABLE_EXFAT) && defined(FX_ENABLE_FAULT_TOLERANT)
        /* Get dont_use_fat value. */
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            dont_use_fat_old = (UCHAR)file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat;
        }
#endif /* FX_ENABLE_EXFAT && FX_ENABLE_FAULT_TOLERANT */

        /* Search for the additional clusters we need.  */
        total_clusters =     media_ptr -> fx_media_total_clusters;

#ifdef FX_ENABLE_FAULT_TOLERANT
        if (replace_clusters > 0)
        {

            /* Reset consecutive cluster. */
            file_ptr -> fx_file_consecutive_cluster = 1;

            last_cluster =   insertion_front;

#ifdef FX_ENABLE_EXFAT
            if ((file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1) &&
                (file_ptr -> fx_file_total_clusters > replace_clusters))
            {

                /* Now we should use FAT.  */
                file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat &= (CHAR)0xfe; /* Clear bit 0.  */

                /* Build FAT chain.  */
                for (i = file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster; i < file_ptr -> fx_file_last_physical_cluster; ++i)
                {

                    status = _fx_utility_FAT_entry_write(media_ptr, i, i + 1);

                    if (status != FX_SUCCESS)
                    {

                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the bad status.  */
                        return(status);
                    }
                }

                /* Write the last cluster FAT entry.  */
                status = _fx_utility_FAT_entry_write(media_ptr, i, FX_LAST_CLUSTER_exFAT);
                if (status != FX_SUCCESS)
                {

                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the bad status.  */
                    return(status);
                }
            }
#endif /* FX_ENABLE_EXFAT */
        }
        else
#endif /* FX_ENABLE_FAULT_TOLERANT */
        {
            last_cluster =   file_ptr -> fx_file_last_physical_cluster;
        }

        FAT_index    =       media_ptr -> fx_media_cluster_search_start;

        /* Loop to find the needed clusters.  */
        while (clusters)
        {

            /* Decrease the cluster count.  */
            clusters--;
#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {

                /* Find a free cluster.  */
                if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
                {

                    if (last_cluster)
                    {

                        cluster_state = FX_EXFAT_BITMAP_CLUSTER_OCCUPIED;
                        FAT_index = last_cluster + 1;

                        if (FAT_index < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
                        {

                            /* Get the state of the cluster.  */
                            status = _fx_utility_exFAT_cluster_state_get(media_ptr, FAT_index, &cluster_state);

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

                        /* Check if we still can do not use FAT.  */
                        if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_FREE)
                        {

                            /* Clusters are still consecutive.  */
                            file_ptr -> fx_file_consecutive_cluster++;
                        }
                        else
                        {

                            /* Now we should use FAT.  */
                            file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat &= (CHAR)0xfe; /* Clear bit 0.  */

                            /* Build FAT chain.  */
                            for (i = file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster; i < last_cluster; ++i)
                            {

                                status = _fx_utility_FAT_entry_write(media_ptr, i, i + 1);

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

                            /* Write the last cluster FAT entry.  */
                            status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, FX_LAST_CLUSTER_exFAT);
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

                            /* Find free cluster from exFAT media.  */
                            status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                                                media_ptr -> fx_media_cluster_search_start,
                                                                                &FAT_index);
                        }
                    }
                    else
                    {

                        /* Find the first cluster for file.  */
                        status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                                            media_ptr -> fx_media_cluster_search_start,
                                                                            &FAT_index);
                    }
                }
                else
                {

                    /* Find free cluster from exFAT media.  */
                    status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                                        media_ptr -> fx_media_cluster_search_start,
                                                                        &FAT_index);
                }

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
            else
            {
#endif /* FX_ENABLE_EXFAT */

                /* Loop to find the first available cluster.  */
                do
                {

                    /* Make sure we stop looking after one pass through the FAT table.  */
                    if (!total_clusters)
                    {

#ifdef FX_ENABLE_FAULT_TOLERANT
                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Something is wrong with the media - the desired clusters were
                           not found in the FAT table.  */
                        return(FX_NO_MORE_SPACE);
                    }

                    /* Read FAT entry.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, FAT_index, &FAT_value);

                    /* Check for a bad status.  */
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

                    /* Decrement the total cluster count.  */
                    total_clusters--;

                    /* Determine if the FAT entry is free.  */
                    if (FAT_value == FX_FREE_CLUSTER)
                    {

                        /* Move cluster search pointer forward.  */
                        media_ptr -> fx_media_cluster_search_start =  FAT_index + 1;

                        /* Determine if this needs to be wrapped.  */
                        if (media_ptr -> fx_media_cluster_search_start >= (media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START))
                        {

                            /* Wrap the search to the beginning FAT entry.  */
                            media_ptr -> fx_media_cluster_search_start =  FX_FAT_ENTRY_START;
                        }

                        /* Break this loop.  */
                        break;
                    }
                    else
                    {

                        /* FAT entry is not free... Advance the FAT index.  */
                        FAT_index++;

                        /* Determine if we need to wrap the FAT index around.  */
                        if (FAT_index >= (media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START))
                        {

                            /* Wrap the search to the beginning FAT entry.  */
                            FAT_index =  FX_FAT_ENTRY_START;
                        }
                    }
                } while (FX_TRUE);
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Determine if we have found the first new cluster yet.  */
            if (first_new_cluster == 0)
            {

                /* Remember the first new cluster. */
                first_new_cluster =  FAT_index;

#ifdef FX_ENABLE_FAULT_TOLERANT
                if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Set the undo log now. */
                    if (copy_head_cluster == 0)
                    {
                        status = _fx_fault_tolerant_set_FAT_chain(media_ptr, dont_use_fat_old, file_ptr -> fx_file_current_physical_cluster,
                                                                  first_new_cluster, media_ptr -> fx_media_fat_last, media_ptr -> fx_media_fat_last);
                    }
#ifdef FX_ENABLE_EXFAT
                    else if (dont_use_fat_old && (insertion_back == media_ptr -> fx_media_fat_last))
                    {
                        status = _fx_fault_tolerant_set_FAT_chain(media_ptr, dont_use_fat_old, insertion_front, first_new_cluster,
                                                                  copy_head_cluster, file_ptr -> fx_file_last_physical_cluster + 1);
                    }
#endif /* FX_ENABLE_EXFAT */
                    else
                    {

                        status = _fx_fault_tolerant_set_FAT_chain(media_ptr, dont_use_fat_old, insertion_front, first_new_cluster,
                                                                  copy_head_cluster, insertion_back);
                    }

                    /* Check for good completion status.  */
                    if (status !=  FX_SUCCESS)
                    {

                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the error status.  */
                        return(status);
                    }
                }
#endif /* FX_ENABLE_FAULT_TOLERANT */
            }

            /* Make a quick check to see if an empty, cluster-less file
               is being written to for the first time.  */
            if (last_cluster)
            {

                /* Check for the file's cluster.  We won't perform this link until the
                   entire FAT chain is built.  */
                if (last_cluster != file_ptr -> fx_file_last_physical_cluster)
                {
#ifdef FX_ENABLE_EXFAT
                    if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
#ifdef FX_ENABLE_FAULT_TOLERANT
                        || (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
#endif /* FX_ENABLE_FAULT_TOLERANT */
                       )
                    {
#endif /* FX_ENABLE_EXFAT */

                        /* Normal condition - link the last cluster with the new
                           found cluster.  */
                        status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, FAT_index);
#ifdef FX_ENABLE_EXFAT
                    }
#endif /* FX_ENABLE_EXFAT */
                }

                /* Check for a bad FAT write status.  */
                if (status !=  FX_SUCCESS)
                {

#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the bad status.  */
                    return(status);
                }

                /* Determine if we are adding a sector after a write filled the previously
                   allocated cluster exactly.  */
                if ((file_ptr -> fx_file_current_relative_sector >=
                     (media_ptr -> fx_media_sectors_per_cluster - 1)) &&
                    (file_ptr -> fx_file_current_logical_offset >=
                     media_ptr -> fx_media_bytes_per_sector))
                {

                    /* Yes, we need to adjust all of the pertinent file parameters for
                       writing into this newly allocated cluster.  */
                    file_ptr -> fx_file_current_physical_cluster =  FAT_index;
                    file_ptr -> fx_file_current_relative_cluster++;
                    file_ptr -> fx_file_current_relative_sector =   0;
                    file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                        (((ULONG64)(FAT_index - FX_FAT_ENTRY_START)) *
                         ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
                    file_ptr -> fx_file_current_logical_offset =    0;
                }
            }
            else
            {

                /* This is the first cluster allocated for the file.  Just
                   remember it as being the first and setup the other file
                   pointers accordingly.  */
                file_ptr -> fx_file_first_physical_cluster =    FAT_index;
                file_ptr -> fx_file_current_physical_cluster =  FAT_index;
                file_ptr -> fx_file_current_relative_cluster =  0;
                file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                    (((ULONG64)(FAT_index - FX_FAT_ENTRY_START)) *
                     ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
#ifdef FX_ENABLE_FAULT_TOLERANT
                if (file_ptr -> fx_file_last_physical_cluster == 0)
#endif /* FX_ENABLE_FAULT_TOLERANT */
                {
                    file_ptr -> fx_file_current_logical_offset =    0;
                    file_ptr -> fx_file_current_file_offset =       0;
                }

                /* Also remember this as the first cluster in the directory
                   entry.  */
                file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =  FAT_index;
            }
#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {

                /* Update Bitmap */
                status = _fx_utility_exFAT_cluster_state_set(media_ptr, FAT_index, FX_EXFAT_BITMAP_CLUSTER_OCCUPIED);

                if (status !=  FX_SUCCESS)
                {
#ifdef FX_ENABLE_FAULT_TOLERANT
                    FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the bad status.  */
                    return(status);
                }

                /* Move cluster search pointer forward. */
                media_ptr -> fx_media_cluster_search_start = FAT_index + 1;

                /* Determine if this needs to be wrapped. */
                if (media_ptr -> fx_media_cluster_search_start >= media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
                {

                    /* Wrap the search to the beginning FAT entry. */
                    media_ptr -> fx_media_cluster_search_start = FX_FAT_ENTRY_START;
                }
            }
#endif /* FX_ENABLE_EXFAT */

            /* Otherwise, remember the new FAT index as the last.  */
            last_cluster =  FAT_index;

            /* Move to the next FAT entry.  */
            FAT_index =  media_ptr -> fx_media_cluster_search_start;
        }
#ifdef FX_ENABLE_EXFAT
        if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
#ifdef FX_ENABLE_FAULT_TOLERANT
            || (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
#endif /* FX_ENABLE_FAULT_TOLERANT */
           )
        {
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled)
            {

                /* Link the last cluster back to original FAT.  */
                status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, insertion_back);
            }
            else
#endif /* FX_ENABLE_FAULT_TOLERANT */
            {

                /* Place an end-of-file marker on the last cluster.  */
                status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, media_ptr -> fx_media_fat_last);
            }

            /* Check for a bad FAT write status.  */
            if (status !=  FX_SUCCESS)
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

        /* Determine if the file already had clusters.  */
        if (file_ptr -> fx_file_last_physical_cluster)
        {
#ifdef FX_ENABLE_EXFAT
            if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
            {
#endif /* FX_ENABLE_EXFAT */

                /* Now, link the file's old last cluster to the first cluster of the new chain.  */
#ifdef FX_ENABLE_FAULT_TOLERANT
                if (insertion_front)
                {
                    status = _fx_utility_FAT_entry_write(media_ptr, insertion_front, first_new_cluster);
                }
                else if ((media_ptr -> fx_media_fault_tolerant_enabled == FX_FALSE) ||
                         ((replace_clusters == 0) && (first_new_cluster)))
                {
                    status = _fx_utility_FAT_entry_write(media_ptr, file_ptr -> fx_file_last_physical_cluster, first_new_cluster);
                }
#else
                status = _fx_utility_FAT_entry_write(media_ptr, file_ptr -> fx_file_last_physical_cluster, first_new_cluster);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Check for a bad FAT write status.  */
                if (status !=  FX_SUCCESS)
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
        }

#ifdef FX_FAULT_TOLERANT

#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {

            /* Flush exFAT bitmap.  */
            _fx_utility_exFAT_bitmap_flush(media_ptr);
        }
#endif /* FX_ENABLE_EXFAT */

        /* Ensure the new FAT chain is properly written to the media.  */

        /* Flush the cached individual FAT entries */
        _fx_utility_FAT_flush(media_ptr);
#endif

#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled)
        {

            /* Clear undo phase. */
            media_ptr -> fx_media_fault_tolerant_state &= (UCHAR)(~FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN & 0xff);

            /* Update the last cluster? */
            if (insertion_back == media_ptr -> fx_media_fat_last)
            {

                /* Yes. Update the file control block with the last physical cluster.  */
                file_ptr -> fx_file_last_physical_cluster =  last_cluster;
            }
        }
        else
#endif /* FX_ENABLE_FAULT_TOLERANT */
        {

            /* Update the file control block with the last physical cluster.  */
            file_ptr -> fx_file_last_physical_cluster =  last_cluster;
        }
    }

    /* Check for a need to increment to the next sector within a previously
       allocated cluster.  */
    if (file_ptr -> fx_file_current_logical_offset >=
        media_ptr -> fx_media_bytes_per_sector)
    {

        /* Update the sector specific file parameters to start at the
           next logical sector.  */
        file_ptr -> fx_file_current_logical_sector++;
        file_ptr -> fx_file_current_relative_sector++;
        file_ptr -> fx_file_current_logical_offset =  0;
    }

    /* At this point there is enough room to perform the file write operation.  */

    /* Setup local buffer pointer.  */
    source_ptr =  (UCHAR *)buffer_ptr;

    /* Setup the remaining number of bytes to write.  */
    bytes_remaining =  size;

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (replace_clusters > 0)
    {

        /* Force update current cluster and sector. */
        file_ptr -> fx_file_current_physical_cluster = first_new_cluster;
        file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
            (((ULONG64)(first_new_cluster - FX_FAT_ENTRY_START)) *
             ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
            file_ptr -> fx_file_current_relative_sector;

        /* Copy sectors in replaced cluster but not overwritten at the front. */
        for (i = 0; i < file_ptr -> fx_file_current_relative_sector; i++)
        {
            status =  _fx_utility_logical_sector_read(media_ptr,
                                                      ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                      (((ULONG)(copy_head_cluster - FX_FAT_ENTRY_START)) *
                                                       ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) + i,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {

                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }

            /* Write back the current logical sector.  */
            status =  _fx_utility_logical_sector_write(media_ptr,
                                                       ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                       (((ULONG)(file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START)) *
                                                        ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) + i,
                                                       media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {

                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Loop to write all of the bytes.  */
    while (bytes_remaining)
    {

        /* Determine if a beginning or ending partial write is required.  */
        if ((file_ptr -> fx_file_current_logical_offset) ||
            (bytes_remaining < media_ptr -> fx_media_bytes_per_sector))
        {

            /* A partial sector write is required.  */

            /* Read the current logical sector.  */
#ifdef FX_ENABLE_FAULT_TOLERANT
            if (replace_clusters > 0)
            {
                if (file_ptr -> fx_file_current_physical_cluster == first_new_cluster)
                {

                    /* It's at beginning. */
                    status =  _fx_utility_logical_sector_read(media_ptr,
                                                              ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                              (((ULONG)(copy_head_cluster - FX_FAT_ENTRY_START)) *
                                                               ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                                                              file_ptr -> fx_file_current_relative_sector,
                                                              media_ptr -> fx_media_memory_buffer, (ULONG)1, FX_DATA_SECTOR);
                }
                else if (data_append == FX_FALSE)
                {

                    /* It's at ending. */
                    status =  _fx_utility_logical_sector_read(media_ptr,
                                                              ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                              (((ULONG)(copy_tail_cluster - FX_FAT_ENTRY_START)) *
                                                               ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                                                              file_ptr -> fx_file_current_relative_sector,
                                                              media_ptr -> fx_media_memory_buffer, (ULONG)1, FX_DATA_SECTOR);
                }
                else
                {

                    /* It's at ending. */
                    status =  _fx_utility_logical_sector_read(media_ptr,
                                                              file_ptr -> fx_file_current_logical_sector,
                                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);
                }
            }
            else
#endif /* FX_ENABLE_FAULT_TOLERANT */
            {
                status =  _fx_utility_logical_sector_read(media_ptr,
                                                          file_ptr -> fx_file_current_logical_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);
            }

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {
#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }

            /* Copy the appropriate number of bytes into the destination buffer.  */
            copy_bytes =  media_ptr -> fx_media_bytes_per_sector -
                file_ptr -> fx_file_current_logical_offset;

            /* Check to see if only a portion of the sector needs to be
               copied.  */
            if (copy_bytes > bytes_remaining)
            {

                /* Adjust the number of bytes to copy.  */
                copy_bytes =  (ULONG)bytes_remaining;
            }

            /* Actually perform the memory copy.  */
            _fx_utility_memory_copy(source_ptr, ((UCHAR *)media_ptr -> fx_media_memory_buffer) +  /* Use case of memcpy is verified. */
                                    file_ptr -> fx_file_current_logical_offset,
                                    copy_bytes);

            /* Write back the current logical sector.  */
            status =  _fx_utility_logical_sector_write(media_ptr, file_ptr -> fx_file_current_logical_sector,
                                                       media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {
#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }


            /* Increment the logical sector byte offset.  */
            file_ptr -> fx_file_current_logical_offset =
                file_ptr -> fx_file_current_logical_offset + copy_bytes;

            /* Adjust the remaining bytes to read.  */
            bytes_remaining =  bytes_remaining - copy_bytes;

            /* Adjust the pointer to the source buffer.  */
            source_ptr =  source_ptr + copy_bytes;
        }
        else
        {

            /* Attempt to write multiple sectors directly to the media.  */

            /* Calculate the number of whole sectors to write.  */
            sectors =  (UINT)(bytes_remaining / media_ptr -> fx_media_bytes_per_sector);

            next_cluster = cluster = file_ptr -> fx_file_current_physical_cluster;

            for (i = (media_ptr -> fx_media_sectors_per_cluster -
                      file_ptr -> fx_file_current_relative_sector); i < sectors; i += media_ptr -> fx_media_sectors_per_cluster)
            {
#ifdef FX_ENABLE_EXFAT
                if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
                {
                    cluster++;
                }
                else
                {
#endif /* FX_ENABLE_EXFAT */
                    status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

                    /* Determine if an error is present.  */
                    if ((status != FX_SUCCESS) || (next_cluster < FX_FAT_ENTRY_START) ||
                        (next_cluster > media_ptr -> fx_media_fat_reserved))
                    {
#ifdef FX_ENABLE_FAULT_TOLERANT
                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Send error message back to caller.  */
                        if (status != FX_SUCCESS)
                        {
                            return(status);
                        }
                        else
                        {
                            return(FX_FILE_CORRUPT);
                        }
                    }

                    if (next_cluster != cluster + 1)
                    {
                        break;
                    }
                    else
                    {
                        cluster = next_cluster;
                    }
#ifdef FX_ENABLE_EXFAT
                }
#endif /* FX_ENABLE_EXFAT */
            }

            if (i < sectors)
            {
                sectors = i;
            }

            /* Perform the data write directly from the user's buffer of
               the appropriate number of sectors.  */
            status =  _fx_utility_logical_sector_write(media_ptr, file_ptr -> fx_file_current_logical_sector,
                                                       source_ptr, (ULONG) sectors, FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {
#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }

            /* Now adjust the various file pointers.  */

            /* Increment the current logical sector.  Subtract one from
               the sector count because we are going to use the logical
               offset to do additional sector/cluster arithmetic below.  */
            file_ptr -> fx_file_current_logical_sector =
                file_ptr -> fx_file_current_logical_sector +
                (sectors - 1);

            /* Move the relative cluster and sector as well.  */
            file_ptr -> fx_file_current_relative_cluster = file_ptr -> fx_file_current_relative_cluster +
                (file_ptr -> fx_file_current_relative_sector + (sectors - 1)) /
                media_ptr -> fx_media_sectors_per_cluster;

            file_ptr -> fx_file_current_relative_sector =
                (file_ptr -> fx_file_current_relative_sector + (sectors - 1)) %
                media_ptr -> fx_media_sectors_per_cluster;

            /* Increment the logical sector byte offset.  */
            file_ptr -> fx_file_current_logical_offset =
                media_ptr -> fx_media_bytes_per_sector;

            file_ptr -> fx_file_current_physical_cluster = cluster;

            /* Adjust the remaining bytes.  */
            bytes_remaining =  bytes_remaining -
                (((ULONG)media_ptr -> fx_media_bytes_per_sector) * sectors);

            /* Adjust the pointer to the source buffer.  */
            source_ptr =  source_ptr +
                (((ULONG)media_ptr -> fx_media_bytes_per_sector) * sectors);
        }

        /* At this point, we have either written a partial sector or have successfully
           written one or more whole sectors.  Determine if we are at the end of
           the current logical sector.  */
        if (file_ptr -> fx_file_current_logical_offset >=
            media_ptr -> fx_media_bytes_per_sector)
        {

            /* Determine if we are at the exact physical end of the file.  */
            if ((bytes_remaining == 0) &&
                ((file_ptr -> fx_file_current_file_offset + size) >=
                 file_ptr -> fx_file_current_available_size))
            {

                /* Skip the following file parameter adjustments.  The next write will
                   detect the logical offset out of the range of the sector and reset
                   all of the pertinent information.  */
                break;
            }

            /* We need to move to the next logical sector, but first
               determine if the next logical sector is within the same
               cluster.  */

            /* Increment the current relative sector in the cluster.  */
            file_ptr -> fx_file_current_relative_sector++;

            /* Determine if this is in a new cluster.  */
            if (file_ptr -> fx_file_current_relative_sector >=
                media_ptr -> fx_media_sectors_per_cluster)
            {

                /* Yes, we need to move to the next cluster.  */
#ifdef FX_ENABLE_EXFAT
                if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
                {

                    next_cluster = file_ptr -> fx_file_current_physical_cluster + 1;
                }
                else
                {
#endif /* FX_ENABLE_EXFAT */

                    /* Read the FAT entry of the current cluster to find
                       the next cluster.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr,
                                                         file_ptr -> fx_file_current_physical_cluster, &next_cluster);

                    /* Determine if an error is present.  */
                    if ((status != FX_SUCCESS) || (next_cluster < FX_FAT_ENTRY_START) ||
                        (next_cluster > media_ptr -> fx_media_fat_reserved))
                    {
#ifdef FX_ENABLE_FAULT_TOLERANT
                        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Send error message back to caller.  */
                        if (status != FX_SUCCESS)
                        {
                            return(status);
                        }
                        else
                        {
                            return(FX_FILE_CORRUPT);
                        }
                    }
#ifdef FX_ENABLE_EXFAT
                }
#endif /* FX_ENABLE_EXFAT */

                /* Otherwise, we have a new cluster.  Save it in the file
                   control block and calculate a new logical sector value.  */
                file_ptr -> fx_file_current_physical_cluster =  next_cluster;
                file_ptr -> fx_file_current_relative_cluster++;
                file_ptr -> fx_file_current_logical_sector = ((ULONG)media_ptr -> fx_media_data_sector_start) +
                    ((((ULONG64)next_cluster) - FX_FAT_ENTRY_START) *
                     ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
                file_ptr -> fx_file_current_relative_sector =  0;
            }
            else
            {

                /* Still within the same cluster so just increment the
                   logical sector.  */
                file_ptr -> fx_file_current_logical_sector++;
            }

            /* In either case, we are now positioned at a new sector so
               clear the logical sector offset.  */
            file_ptr -> fx_file_current_logical_offset =  0;
        }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (((replace_clusters > 0) && (data_append == FX_FALSE)) &&
        (file_ptr -> fx_file_current_logical_offset || file_ptr -> fx_file_current_relative_sector))
    {

        /* Copy sectors in replaced cluster but not overwritten at the end. */
        if (file_ptr -> fx_file_current_logical_offset == 0)
        {
            i = file_ptr -> fx_file_current_relative_sector;
        }
        else
        {
            i = file_ptr -> fx_file_current_relative_sector + 1;
        }
        for (; i < media_ptr -> fx_media_sectors_per_cluster; i++)
        {
            status =  _fx_utility_logical_sector_read(media_ptr,
                                                      ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                      (((ULONG)(copy_tail_cluster - FX_FAT_ENTRY_START)) *
                                                       ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) + i,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {

                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }

            /* Write back the current logical sector.  */
            status =  _fx_utility_logical_sector_write(media_ptr,
                                                       ((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                       (((ULONG)(file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START)) *
                                                        ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) + i,
                                                       media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {

                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Adjust the current file offset accordingly.  */
    file_ptr -> fx_file_current_file_offset =
        file_ptr -> fx_file_current_file_offset + size;

    /* Copy the new file size into the directory entry.  */
    file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =
        file_ptr -> fx_file_current_file_size;

    /* Determine if this write was done past the previous file size.  */
    if (file_ptr -> fx_file_current_file_offset >
        file_ptr -> fx_file_current_file_size)
    {

        /* Yes, we have written past the previous end of the file.  Update
           the file size.  */
        file_ptr -> fx_file_current_file_size =  file_ptr -> fx_file_current_file_offset;

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

                /* Setup the new size.  */
                search_ptr -> fx_file_current_file_size =  file_ptr -> fx_file_current_file_offset;

                /* Setup the new directory entry.  */
                search_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =      file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster;
                search_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =    file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size;
                search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector =   file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector;
                search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset =  file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset;

                /* Setup the last cluster. This really isn't used during reading, but it is nice to keep things
                   consistent.  */
                search_ptr -> fx_file_last_physical_cluster =  file_ptr -> fx_file_last_physical_cluster;

                /* Update the available clusters as well.  */
                search_ptr -> fx_file_current_available_size =  file_ptr -> fx_file_current_available_size;

                /* Determine if an empty file was previously opened.  */
                if (search_ptr -> fx_file_total_clusters == 0)
                {

                    /* Setup initial parameters.  */
                    search_ptr -> fx_file_total_clusters =            file_ptr -> fx_file_total_clusters;
                    search_ptr -> fx_file_current_physical_cluster =  file_ptr -> fx_file_first_physical_cluster;
                    search_ptr -> fx_file_current_relative_cluster =  0;
                    search_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                        (((ULONG64)(file_ptr -> fx_file_first_physical_cluster - FX_FAT_ENTRY_START)) *
                         ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
                    search_ptr -> fx_file_current_relative_sector =   0;
                    search_ptr -> fx_file_current_logical_offset =    0;
                    search_ptr -> fx_file_current_file_offset =       0;
                }
            }

            /* Adjust the pointer and decrement the search count.  */
            search_ptr =  search_ptr -> fx_file_opened_next;
            open_count--;
        }
#endif
    }

    /* Finally, mark this file as modified.  */
    file_ptr -> fx_file_modified =  FX_TRUE;

#ifdef FX_FAULT_TOLERANT_DATA

    /* Ensure that all file data is flushed out.  */

    /* Flush the internal logical sector cache.  */
    _fx_utility_logical_sector_flush(media_ptr, 1, media_ptr -> fx_media_total_sectors, FX_FALSE);

    /* Lockout interrupts for time/date access.  */
    FX_DISABLE_INTS

    /* Set the new time and date.  */
    file_ptr -> fx_file_dir_entry.fx_dir_entry_time =  _fx_system_time;
    file_ptr -> fx_file_dir_entry.fx_dir_entry_date =  _fx_system_date;

    /* Restore interrupts.  */
    FX_RESTORE_INTS

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size = file_ptr -> fx_file_current_file_size;
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

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
        status =  _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));
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
#endif

    /* Update the trace event with the bytes written.  */
    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_WRITE, 0, 0, 0, size)

#ifdef FX_ENABLE_FAULT_TOLERANT
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
    if (file_ptr -> fx_file_current_file_offset > file_ptr -> fx_file_maximum_size_used)
    {
        file_ptr -> fx_file_maximum_size_used = file_ptr -> fx_file_current_file_offset;
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Invoke file write callback. */
    if (file_ptr -> fx_file_write_notify)
    {
        file_ptr -> fx_file_write_notify(file_ptr);
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return a successful status to the caller.  */
    return(FX_SUCCESS);
}

