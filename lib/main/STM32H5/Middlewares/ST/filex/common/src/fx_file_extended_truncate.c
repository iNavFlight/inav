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
/*    _fx_file_extended_truncate                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the file to the specified size, if smaller than  */
/*    the current file size.  If the new file size is less than the       */
/*    current file read/write position, the internal file pointers will   */
/*    also be modified.                                                   */
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
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
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
UINT  _fx_file_extended_truncate(FX_FILE *file_ptr, ULONG64 size)
{

UINT                   status;
ULONG                  cluster;
ULONG                  contents = 0;
ULONG                  bytes_per_cluster;
ULONG                  last_cluster;
ULONG                  cluster_count;
ULONG64                bytes_remaining;
FX_MEDIA              *media_ptr;

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
    media_ptr -> fx_media_file_truncates++;
#endif

    /* Setup pointer to associated media control block.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_TRUNCATE, file_ptr, size, file_ptr -> fx_file_current_file_size, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Make sure this file is open for writing.  */
    if (file_ptr -> fx_file_open_mode != FX_OPEN_FOR_WRITE)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the access error exception - a write was attempted from
           a file opened for reading!  */
        return(FX_ACCESS_ERROR);
    }

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Setup the new file size - if less than the current size.  */
    if (size < file_ptr -> fx_file_current_file_size)
    {

        /* Setup the new size.  */
        file_ptr -> fx_file_current_file_size =  size;

        /* Set the modified flag as well.  */
        file_ptr -> fx_file_modified =  FX_TRUE;

        /* Update the trace event with the truncated size.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_TRUNCATE, 0, 0, 0, size)
    }
    else
    {

        /* Update the trace event with the truncated size.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_TRUNCATE, 0, 0, 0, file_ptr -> fx_file_current_file_size)

        /* Release media protection.  */
        FX_UNPROTECT

        /* Just return, the new size is larger than the current size.  */
        return(FX_SUCCESS);
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
            search_ptr -> fx_file_dir_entry.fx_dir_entry_file_size = size;
        }

        /* Adjust the pointer and decrement the search count.  */
        search_ptr =  search_ptr -> fx_file_opened_next;
        open_count--;
    }
#endif

    /* Now check to see if the read/write internal file pointers need
       to be adjusted.  */
    if (file_ptr -> fx_file_current_file_offset > file_ptr -> fx_file_current_file_size)
    {

        /* Calculate the number of bytes per cluster.  */
        bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
            ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

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

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        /* Start transaction. */
        _fx_fault_tolerant_transaction_start(media_ptr);

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size = file_ptr -> fx_file_current_file_size;

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

        /* Check for a good status.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Error writing the directory.  */
            return(status);
        }

        /* Update maximum size used if necessary. */
        if (size < file_ptr -> fx_file_maximum_size_used)
        {
            file_ptr -> fx_file_maximum_size_used = size;
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* Truncate is complete, return successful status.  */
    return(FX_SUCCESS);
}

