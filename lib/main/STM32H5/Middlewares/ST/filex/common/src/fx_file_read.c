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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_read                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the specified number of bytes (or as many as    */
/*    possible into the buffer supplied by the caller.  The actual number */
/*    of bytes and the status of the read operation is returned to the    */
/*    caller.  In addition, various internal file pointers in the file    */
/*    control block are also updated.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    buffer_ptr                            Buffer pointer                */
/*    request_size                          Number of bytes requested     */
/*    actual_size                           Pointer to variable for the   */
/*                                            number of bytes read        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_logical_sector_read       Read a logical sector         */
/*    _fx_utility_memory_copy               Fast memory copy routine      */
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
UINT  _fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size)
{

UINT                   status;
ULONG                  bytes_remaining, i;
ULONG                  copy_bytes;
UCHAR                 *destination_ptr;
ULONG                  cluster, next_cluster;
UINT                   sectors;
FX_MEDIA              *media_ptr;

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
    media_ptr -> fx_media_file_reads++;
#endif

    /* Setup pointer to associated media control block.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_READ, file_ptr, buffer_ptr, request_size, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Next, determine if there is any more bytes to read in the file.  */
    if (file_ptr -> fx_file_current_file_offset >=
        file_ptr -> fx_file_current_file_size)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* The file is at the end, return the proper status and set the
           actual size to 0.  */
        *actual_size =  0;
        return(FX_END_OF_FILE);
    }

    /* At this point there is something to read.  */

    /* Setup local buffer pointer.  */
    destination_ptr =  (UCHAR *)buffer_ptr;

    /* Determine if there are less bytes left in the file than that specified
       by the request.  If so, adjust the requested size.  */
    if ((ULONG64)request_size >
        (file_ptr -> fx_file_current_file_size - file_ptr -> fx_file_current_file_offset))
    {

        /* Adjust the bytes remaining to what's available.  */
        request_size =  (ULONG)(file_ptr -> fx_file_current_file_size - file_ptr -> fx_file_current_file_offset);
    }

    /* Setup the remaining number of bytes to read.  */
    bytes_remaining =  request_size;

    /* Loop to read all of the bytes.  */
    while (bytes_remaining)
    {

        /* Determine if a beginning or ending partial read is required.  */
        if ((file_ptr -> fx_file_current_logical_offset) ||
            (bytes_remaining < media_ptr -> fx_media_bytes_per_sector))
        {

            /* A partial sector read is required.  */

            /* Read the current logical sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr,
                                                      file_ptr -> fx_file_current_logical_sector,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

            /* Check for good completion status.  */
            if (status !=  FX_SUCCESS)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error status.  */
                return(status);
            }

            /* Copy the appropriate number of bytes into the destination buffer.  */
            copy_bytes =  media_ptr -> fx_media_bytes_per_sector -
                file_ptr -> fx_file_current_logical_offset;

            /* Check to see if only a portion of the read sector needs to be
               copied.  */
            if (copy_bytes > bytes_remaining)
            {

                /* Adjust the number of bytes to copy.  */
                copy_bytes =  bytes_remaining;
            }

            /* Actually perform the memory copy.  */
            _fx_utility_memory_copy(((UCHAR *)media_ptr -> fx_media_memory_buffer) + /* Use case of memcpy is verified. */
                                    file_ptr -> fx_file_current_logical_offset,
                                    destination_ptr, copy_bytes);

            /* Increment the logical sector byte offset.  */
            file_ptr -> fx_file_current_logical_offset =
                file_ptr -> fx_file_current_logical_offset + copy_bytes;

            /* Adjust the remaining bytes to read.  */
            bytes_remaining =  bytes_remaining - copy_bytes;

            /* Adjust the pointer to the destination buffer.  */
            destination_ptr =  destination_ptr + copy_bytes;
        }
        else
        {

            /* Attempt to read multiple sectors directly into the destination
               buffer.  */

            /* Calculate the number of whole sectors to read directly into
               the destination buffer.  */
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

            /* Determine if this is a single sector read request.  If so, read the sector so it will
               come from the internal cache.  */
            if (sectors == 1)
            {

                /* Read the current logical sector.  */
                status =  _fx_utility_logical_sector_read(media_ptr,
                                                          file_ptr -> fx_file_current_logical_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

                /* Check for good completion status.  */
                if (status !=  FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }

                /* Actually perform the memory copy.  */
                _fx_utility_memory_copy((UCHAR *)media_ptr -> fx_media_memory_buffer, destination_ptr, media_ptr -> fx_media_bytes_per_sector); /* Use case of memcpy is verified. */
            }
            else
            {

                /* Multiple sector read request.  Read all the sectors at once.  */

                /* Perform the data read directly into the user's buffer of
                   the appropriate number of sectors.  */
                media_ptr -> fx_media_disable_burst_cache = file_ptr -> fx_file_disable_burst_cache;
                status =  _fx_utility_logical_sector_read(media_ptr, file_ptr -> fx_file_current_logical_sector,
                                                          destination_ptr, (ULONG) sectors, FX_DATA_SECTOR);
                media_ptr -> fx_media_disable_burst_cache = FX_FALSE;

                /* Check for good completion status.  */
                if (status !=  FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }
            }

            /* Now adjust the various file pointers.  */

            /* Increment the current logical sector.  Subtract one from
               the sector count because we are going to use the logical
               offset to do additional sector/cluster arithmetic below.  */
            file_ptr -> fx_file_current_logical_sector =
                file_ptr -> fx_file_current_logical_sector +
                (sectors - 1);

            /* Move the relative sector and cluster as well.  */
            file_ptr -> fx_file_current_relative_cluster = file_ptr -> fx_file_current_relative_cluster +
                (file_ptr -> fx_file_current_relative_sector + (sectors - 1)) /
                media_ptr -> fx_media_sectors_per_cluster;

            file_ptr -> fx_file_current_relative_sector =
                (file_ptr -> fx_file_current_relative_sector +
                 (sectors - 1)) % media_ptr -> fx_media_sectors_per_cluster;

            /* Increment the logical sector byte offset.  */
            file_ptr -> fx_file_current_logical_offset =
                media_ptr -> fx_media_bytes_per_sector;

            file_ptr -> fx_file_current_physical_cluster = cluster;

            /* Adjust the remaining bytes.  */
            bytes_remaining =  bytes_remaining -
                (((ULONG)media_ptr -> fx_media_bytes_per_sector) * sectors);

            /* Adjust the pointer to the destination buffer.  */
            destination_ptr =  destination_ptr +
                (((ULONG)media_ptr -> fx_media_bytes_per_sector) * sectors);
        }

        /* At this point, we have either read a partial sector or have successfully
           read one or more whole sectors.  Determine if we are at the end of
           the current logical sector.  */
        if (file_ptr -> fx_file_current_logical_offset >=
            media_ptr -> fx_media_bytes_per_sector)
        {

            /* Determine if we are at the exact physical end of the file at the end of reading.  */
            if ((bytes_remaining == 0) && ((file_ptr -> fx_file_current_file_offset + (ULONG64)request_size) >=
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

    /* Adjust the current file offset accordingly.  */
    file_ptr -> fx_file_current_file_offset =
        file_ptr -> fx_file_current_file_offset + (ULONG64)request_size;

    /* Store the number of bytes actually read.  */
    *actual_size =  request_size;

    /* Update the trace event with the bytes read.  */
    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_FILE_READ, 0, 0, 0, request_size)

    /* Update the last accessed date.  */
    file_ptr -> fx_file_dir_entry.fx_dir_entry_last_accessed_date =  _fx_system_date;

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return a successful status to the caller.  */
    return(FX_SUCCESS);
}

