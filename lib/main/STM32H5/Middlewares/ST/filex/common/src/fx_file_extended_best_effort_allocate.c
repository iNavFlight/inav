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
#include "fx_utility.h"
#include "fx_directory.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_extended_best_effort_allocate              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to allocate the number of consecutive        */
/*    clusters required to satisfy the user's request.  If there are not  */
/*    enough clusters, the largest set of clusters are allocated and      */
/*    linked to the file.  If there are no free clusters, an error        */
/*    code is returned to the caller.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    size                                  Number of bytes to allocate   */
/*    actual_size_allocated                 Number of bytes allocated     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Update directory entry        */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_exFAT_bitmap_free_cluster_find                          */
/*                                            Find exFAT free cluster     */
/*    _fx_utility_exFAT_cluster_state_get   Get cluster state             */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_utility_logical_sector_flush      Flush the written log sector  */
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
UINT  _fx_file_extended_best_effort_allocate(FX_FILE *file_ptr, ULONG64 size, ULONG64 *actual_size_allocated)
{

UINT                   status;
ULONG                  i;
UINT                   found;
ULONG                  bytes_per_cluster;
ULONG                  FAT_index, start_FAT_index;
ULONG                  FAT_value;
ULONG                  clusters, maximum_clusters;
FX_MEDIA              *media_ptr;

#ifdef FX_ENABLE_EXFAT
UCHAR                  cluster_state;
#endif /* FX_ENABLE_EXFAT */

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
    media_ptr -> fx_media_file_best_effort_allocates++;
#endif

    /* Make sure this file is open for writing.  */
    if (file_ptr -> fx_file_open_mode != FX_OPEN_FOR_WRITE)
    {

        /* Return the access error exception - a write was attempted from
           a file opened for reading!  */
        return(FX_ACCESS_ERROR);
    }

    /* Determine if the requested allocation is for zero bytes.  */
    if (size == 0)
    {

        /* Return a size allocated of zero.  */
        *actual_size_allocated =  0;

        /* Return a successful completion - nothing needs to be done.  */
        return(FX_SUCCESS);
    }

    /* Setup pointer to associated media control block.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_BEST_EFFORT_ALLOCATE, file_ptr, size, 0, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

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

    /* Calculate the number of consecutive clusters needed to satisfy this
       request.  */
    clusters =  (ULONG)(((size + bytes_per_cluster - 1) / bytes_per_cluster));
    
    /* Determine if cluster count is 0.  */
    if (clusters == 0)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Size overflow when rounding to the next cluster, return an error status.  */
        return(FX_NO_MORE_SPACE);
    }

    /* Determine if there are no available clusters on the media.  */
    if (!media_ptr -> fx_media_available_clusters)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return a size allocated of zero, since no clusters were available.  */
        *actual_size_allocated =  0;

        /* Not enough clusters, return an error status.  */
        return(FX_NO_MORE_SPACE);
    }

    /* Determine if the requested file allocation would exceed the physical limit of the file.  */
    if (((file_ptr -> fx_file_current_available_size + (((ULONG64) clusters) * ((ULONG64) bytes_per_cluster))) < file_ptr -> fx_file_current_available_size) ||
        ((file_ptr -> fx_file_current_available_size + (((ULONG64) clusters) * ((ULONG64) bytes_per_cluster))) > 0xFFFFFFFFULL))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the no more space error, since the new file size would be larger than
           the 32-bit field to represent it in the file's directory entry.  */
        return(FX_NO_MORE_SPACE);
    }

    /* Now we need to find the consecutive clusters.  */
    found =             FX_FALSE;
#ifdef FX_ENABLE_EXFAT
    if ((file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1) &&
        (file_ptr -> fx_file_last_physical_cluster > FX_FAT_ENTRY_START) &&
        (file_ptr -> fx_file_last_physical_cluster < media_ptr -> fx_media_total_clusters - clusters + FX_FAT_ENTRY_START))
    {
        found = FX_TRUE;

        /* Try to keep clusters consecutive.  */
        for (FAT_index = file_ptr -> fx_file_last_physical_cluster + 1;
             FAT_index < clusters + file_ptr -> fx_file_last_physical_cluster + 1;
             FAT_index++)
        {

            /* Get cluster state.  */
            status = _fx_utility_exFAT_cluster_state_get(media_ptr, FAT_index, &cluster_state);

            /* Check for a successful status.  */
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

            /* Determine if the entry is free.  */
            if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_OCCUPIED)
            {
                found = FX_FALSE;
                break;
            }
        }
        FAT_index = file_ptr -> fx_file_last_physical_cluster + 1;
    }

    if (!found)
    {
#endif /* FX_ENABLE_EXFAT */
        FAT_index =         FX_FAT_ENTRY_START;
        maximum_clusters =  0;
        start_FAT_index =   FAT_index;

        while (FAT_index < (media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START))
        {

            /* Determine if enough consecutive FAT entries are available.  */
            i =  0;

#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {
                do
                {

                    /* Get cluster state.  */
                    status = _fx_utility_exFAT_cluster_state_get(media_ptr, (FAT_index + i), &cluster_state);

                    /* Check for a successful status.  */
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

                    /* Determine if the entry is free.  */
                    if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_OCCUPIED)
                    {
                        break;
                    }

                    /* Otherwise, increment the consecutive FAT indices.  */
                    i++;
                } while ((i < clusters) && ((FAT_index + i) < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START));
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */

                do
                {

                    /* Read a FAT entry.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, (FAT_index + i), &FAT_value);

                    /* Check for a successful status.  */
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

                    /* Determine if the entry is free.  */
                    if (FAT_value != FX_FREE_CLUSTER)
                    {
                        break;
                    }

                    /* Otherwise, increment the consecutive FAT indices.  */
                    i++;
                } while ((i < clusters) && ((FAT_index + i) < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START));
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Determine if a new maximum number of clusters has been found.  */
            if (i > maximum_clusters)
            {

                /* Yes, remember the maximum number of clusters and the starting
                   cluster.  */
                maximum_clusters =      i;
                start_FAT_index =       FAT_index;
            }

            /* Determine if we found enough FAT entries.  */
            if (i >= clusters)
            {

                /* Yes, we have found enough FAT entries - set the found
                   flag and get out of this loop.  */
                found =  FX_TRUE;
                break;
            }
            else
            {
#ifdef FX_ENABLE_EXFAT
                if (media_ptr -> fx_media_FAT_type == FX_exFAT)
                {

                    /* Find free cluster from exFAT media.  */
                    status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                                        FAT_index + i + 1,
                                                                        &FAT_value);
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

                    if (FAT_value < FAT_index + i + 1)
                    {

                        /* If we wrapped.  */
                        FAT_index = media_ptr -> fx_media_total_clusters  + FX_FAT_ENTRY_START;
                    }
                    else
                    {
                        FAT_index = FAT_value;
                    }
                }
                else
                {
#endif /* FX_ENABLE_EXFAT */

                    /* Position to the next possibly free FAT entry.  */
                    FAT_index =  FAT_index + i + 1;
#ifdef FX_ENABLE_EXFAT
                }
#endif /* FX_ENABLE_EXFAT */
            }
        }

        /* Determine if the total request could not be satisfied, but a partial allocation
           could be satisfied.  */
        if (maximum_clusters)
        {

            /* Yes, there was at least one cluster.  Prepare to return this
               to the caller.  */
            FAT_index =  start_FAT_index;
            clusters =   maximum_clusters;
            found =      FX_TRUE;
        }
#ifdef FX_ENABLE_EXFAT
    }
#endif

    /* Determine if we found enough consecutive clusters to satisfy the
       request.  */
    if (found)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled)
        {

            /* Record the FAT chain being applied to the file system. This information aids
               recovery effort if fault happens. */
            media_ptr -> fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN;
            _fx_fault_tolerant_set_FAT_chain(media_ptr, FX_FALSE, file_ptr -> fx_file_last_physical_cluster,
                                             FAT_index, media_ptr -> fx_media_fat_last, media_ptr -> fx_media_fat_last);
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

#ifdef FX_ENABLE_EXFAT
        if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
        {
            if ((file_ptr -> fx_file_total_clusters) &&
                (FAT_index != file_ptr -> fx_file_last_physical_cluster + 1))
            {

                /* Clusters are not consecutive.  */
                file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat &= (CHAR)0xfe; /* Set 0bit to 0.  */

                /* Rebuild FAT.  */
                FAT_value = file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster +
                            file_ptr -> fx_file_total_clusters - 1; /* Last cluster */

                for (i = file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster; i < FAT_value; ++i)
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

                /* Close chain.  */
                status = _fx_utility_FAT_entry_write(media_ptr, FAT_value, media_ptr -> fx_media_fat_last);
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

#ifdef FX_ENABLE_FAULT_TOLERANT
                if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Clear the Fault Tolerant Set FAT Chain flag. */
                    media_ptr -> fx_media_fault_tolerant_state &= (UCHAR)(~FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN & 0xff);
                }
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Update stream.  */
                status = _fx_directory_exFAT_entry_write(
                        media_ptr, &(file_ptr -> fx_file_dir_entry), UPDATE_STREAM);

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

#ifdef FX_ENABLE_FAULT_TOLERANT
                if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Set undo phase. */
                    media_ptr -> fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN;
                }
#endif /* FX_ENABLE_FAULT_TOLERANT */
            }
        }
#endif /* FX_ENABLE_EXFAT */

        /* Update the link pointers in the new clusters.  */
        for (i = 0; i < (clusters - 1); i++)
        {
#ifdef FX_ENABLE_EXFAT
            if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
#ifdef FX_ENABLE_FAULT_TOLERANT
                || (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
#endif /* FX_ENABLE_FAULT_TOLERANT */
               )
            {
#endif /* FX_ENABLE_EXFAT */

                /* Update the cluster links.  Since the allocation is
                   sequential, we just have to link each FAT entry to the
                   next one.  */
                status =  _fx_utility_FAT_entry_write(media_ptr, FAT_index + i, FAT_index + i + 1);

                /* Check for a bad status.  */
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

                /* Mark the cluster as used.  */
                status = _fx_utility_exFAT_cluster_state_set(media_ptr, FAT_index + i, FX_EXFAT_BITMAP_CLUSTER_OCCUPIED);

                /* Check for a bad status.  */
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
            }
#endif /* FX_ENABLE_EXFAT */
        }

#ifdef FX_ENABLE_EXFAT
        if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
        {
#endif /* FX_ENABLE_EXFAT */

            /* Now place an EOF in the last cluster entry.  */
            status =  _fx_utility_FAT_entry_write(media_ptr, FAT_index + clusters - 1, media_ptr -> fx_media_fat_last);

            /* Check for a bad status.  */
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

            /* Mark the cluster as used.  */
            status = _fx_utility_exFAT_cluster_state_set(media_ptr, FAT_index + clusters - 1, FX_EXFAT_BITMAP_CLUSTER_OCCUPIED);

            /* Check for a bad status.  */
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
        }
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_FAULT_TOLERANT

        /* Flush the cached individual FAT entries */
        _fx_utility_FAT_flush(media_ptr);
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            _fx_utility_exFAT_bitmap_flush(media_ptr);
        }
#endif /* FX_ENABLE_EXFAT */
#endif

        /* Actually link up the new clusters to the file.  */

        /* Determine if there are already clusters allocated for this file.  */
        if (file_ptr -> fx_file_total_clusters)
        {

#ifdef FX_ENABLE_EXFAT
            if (!(file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
            {
#endif /* FX_ENABLE_EXFAT */

                /* Linkup the last cluster.  */
                status =  _fx_utility_FAT_entry_write(media_ptr,
                                                      file_ptr -> fx_file_last_physical_cluster, FAT_index);

                /* Check for a bad status.  */
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

            /* Determine if we are adding a sector after a write filled the previously
               allocated cluster exactly.  */
            if ((file_ptr -> fx_file_current_relative_sector >=
                 (media_ptr -> fx_media_sectors_per_cluster - 1)) &&            
                (file_ptr -> fx_file_current_logical_offset >=
                    media_ptr -> fx_media_bytes_per_sector))
            {

                /* Yes, we need to adjust all of the pertinent file parameters for
                   access into this newly allocated cluster.  */
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

            /* These new clusters are also the first!  Setup the initial
               file parameters.  */
            file_ptr -> fx_file_first_physical_cluster =    FAT_index;
            file_ptr -> fx_file_current_physical_cluster =  file_ptr -> fx_file_first_physical_cluster;
            file_ptr -> fx_file_current_relative_cluster =  0;
            file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                (((ULONG64)(file_ptr -> fx_file_first_physical_cluster - FX_FAT_ENTRY_START)) *
                 ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
            file_ptr -> fx_file_current_logical_offset =    0;
            file_ptr -> fx_file_current_file_offset =       0;

            /* Setup the consecutive clusters at the beginning of the file.  */
            file_ptr -> fx_file_consecutive_cluster =       clusters;

            /* Update the first cluster in the directory entry.  */
            file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =  FAT_index;

#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled)
            {

                /* Clear undo phase. */
                media_ptr -> fx_media_fault_tolerant_state &= (UCHAR)(~FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN & 0xff);
            }
#endif /* FX_ENABLE_FAULT_TOLERANT */
        }

        /* Remember the last physical cluster.  */
        file_ptr -> fx_file_last_physical_cluster =     FAT_index + clusters - 1;

        /* Update the available size.  */
#ifdef FX_ENABLE_EXFAT
        if (file_ptr -> fx_file_current_available_size > (file_ptr -> fx_file_current_available_size + (bytes_per_cluster * clusters)))
        {
            /* 64-bit wrap around condition is present.  Just set the available file size to all ones, which is
               the maximum file size.  */
            file_ptr -> fx_file_current_available_size =  0xFFFFFFFFFFFFFFFFULL;
        }

        /* Check for wrap-around when updating the available size.  */
        if ((media_ptr -> fx_media_FAT_type != FX_exFAT) &&
            (file_ptr -> fx_file_current_available_size + (bytes_per_cluster * clusters) > 0xFFFFFFFFUL))
        {

            /* 32-bit wrap around condition is present.  Just set the available file size to all ones, which is
               the maximum file size.  */
            file_ptr -> fx_file_current_available_size =  0xFFFFFFFFUL;
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */

            /* Normal condition, update the available size.  */
            file_ptr -> fx_file_current_available_size =
                file_ptr -> fx_file_current_available_size + bytes_per_cluster * clusters;
#ifdef FX_ENABLE_EXFAT
        }

        if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
        {
            file_ptr -> fx_file_consecutive_cluster += clusters;
        }
#endif /* FX_ENABLE_EXFAT */

        /* Increment the total clusters for this file.  */
        file_ptr -> fx_file_total_clusters =
            file_ptr -> fx_file_total_clusters + clusters;

        /* Decrease the available clusters on the media.  */
        media_ptr -> fx_media_available_clusters =
            media_ptr -> fx_media_available_clusters - clusters;

        /* Return the actual size allocated.  */
        *actual_size_allocated =  ((ULONG64)clusters) * bytes_per_cluster;

#if defined(FX_UPDATE_FILE_SIZE_ON_ALLOCATE) || defined(FX_ENABLE_FAULT_TOLERANT)

        /* Set the file size the current size plus what what was added.  */
        if (size < *actual_size_allocated)
        {
            file_ptr -> fx_file_current_file_size +=  size;
        }
        else
        {
            file_ptr -> fx_file_current_file_size +=  *actual_size_allocated;
        }

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =  file_ptr -> fx_file_current_file_size;
#endif

        /* Update the trace event with the bytes allocated.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_BEST_EFFORT_ALLOCATE, 0, 0, *actual_size_allocated, 0);

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

            /* Return the error status.  */
            return(status);
        }
    }
    else
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return a size allocated of zero, since no clusters were available.  */
        *actual_size_allocated =  0;

        /* Not enough contiguous space on the media.  Return error status.  */
        return(FX_NO_MORE_SPACE);
    }

#ifdef FX_FAULT_TOLERANT

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);
#endif

    /* Flush the internal logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_sectors_per_FAT), FX_FALSE);

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

    /* Return status to the caller.  */
    return(status);
}

