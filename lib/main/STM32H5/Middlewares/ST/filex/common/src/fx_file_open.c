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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_open                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified file.  If found, */
/*    the open request is validated and the file is opened.  During the   */
/*    opening process, all of the FAT entries for this file are examined  */
/*    for their integrity.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_ptr                              File control block pointer    */
/*    file_name                             Name pointer                  */
/*    open_type                             Type of open requested        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable fast open and       */
/*                                            consecutive detect,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name, UINT open_type)
{

UINT     status;
#ifndef FX_DISABLE_CONSECUTIVE_DETECT
UINT     leading_consecutive;
#endif /* FX_DISABLE_CONSECUTIVE_DETECT */
ULONG    cluster;
ULONG    contents = 0;
ULONG    open_count;
FX_FILE *tail_ptr;
FX_FILE *search_ptr;
ULONG    bytes_per_cluster;
UINT     last_cluster;
ULONG    cluster_count;
ULONG64  bytes_available;
ULONG64  bytes_remaining;
ULONG    fat_last;
#ifndef FX_DISABLE_FAST_OPEN
UINT     fast_open;
#endif /* FX_DISABLE_FAST_OPEN */
UCHAR    not_a_file_attr;


    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_opens++;
#endif

    /* Clear the notify function. */
    file_ptr -> fx_file_write_notify = FX_NULL;

    /* Determine the type of FAT and setup variables accordingly.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        fat_last        = FX_LAST_CLUSTER_exFAT;
        not_a_file_attr = FX_DIRECTORY;
    }
    else if (media_ptr -> fx_media_FAT_type == FX_FAT32)
#else
    if (media_ptr -> fx_media_32_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
    {
        fat_last        = FX_LAST_CLUSTER_1_32;
        not_a_file_attr = FX_DIRECTORY | FX_VOLUME;
    }
    else
    {
        fat_last        = FX_LAST_CLUSTER_1;
        not_a_file_attr = FX_DIRECTORY | FX_VOLUME;
    }

#ifndef FX_DISABLE_FAST_OPEN
    /* Determine if a fast open is selected.  */
    if (open_type == FX_OPEN_FOR_READ_FAST)
    {

        /* Yes, convert the open type to a standard read.  */
        open_type =  FX_OPEN_FOR_READ;

        /* Set the open fast flag.  */
        fast_open =  FX_TRUE;
    }
    else
    {

        /* A fast open is not selected, set the flag to false.  */
        fast_open =  FX_FALSE;
    }
#endif /* FX_DISABLE_FAST_OPEN */

    /* If trace is enabled, register this object.  */
    FX_TRACE_OBJECT_REGISTER(FX_TRACE_OBJECT_TYPE_FILE, file_ptr, file_name, 0, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_OPEN, media_ptr, file_ptr, file_name, open_type, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Setup file name pointer.  */
    file_ptr -> fx_file_dir_entry.fx_dir_entry_name =  file_ptr -> fx_file_name_buffer;
    file_ptr -> fx_file_dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, file_name, &(file_ptr -> fx_file_dir_entry), FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check to make sure the found entry is a file.  */
    if (file_ptr -> fx_file_dir_entry.fx_dir_entry_attributes & not_a_file_attr)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a file error code.  */
        return(FX_NOT_A_FILE);
    }

#ifdef FX_SINGLE_OPEN_LEGACY
    /* Check to make sure the access is okay.  */
    if (open_type == FX_OPEN_FOR_READ)
    {

        /* Check the list of open files for others open for writing.  */
        open_count =  media_ptr -> fx_media_opened_file_count;
        search_ptr =  media_ptr -> fx_media_opened_file_list;
        while (open_count)
        {

            /* Look at each opened file to see if the same file is opened
               for writing.  */
            if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector) &&
                (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset) &&
                (search_ptr -> fx_file_open_mode))
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* The file has been opened for writing by a previous call.  */
                return(FX_ACCESS_ERROR);
            }

            /* Adjust the pointer and decrement the search count.  */
            search_ptr =  search_ptr -> fx_file_opened_next;
            open_count--;
        }
    }
    else
#else
    if (open_type == FX_OPEN_FOR_WRITE)
#endif
    {

        /* A open for write request is present, check the file attributes
           and the list of open files for any other open instance of
           this file.  */
        if (media_ptr -> fx_media_driver_write_protect)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return write protect error.  */
            return(FX_WRITE_PROTECT);
        }

        if (file_ptr -> fx_file_dir_entry.fx_dir_entry_attributes & (UCHAR)(FX_READ_ONLY))
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the not a file error code.  */
            return(FX_ACCESS_ERROR);
        }

        /* Also search the opened files to see if this file is currently
           opened.  */
        open_count =  media_ptr -> fx_media_opened_file_count;
        search_ptr =  media_ptr -> fx_media_opened_file_list;
        while (open_count)
        {

            /* Look at each opened file to see if the same file is already opened.  */
#ifdef FX_SINGLE_OPEN_LEGACY
            if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector) &&
                (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset))
#else
            /* Look at each opened file to see if the same file is already opened
               for writing.  */
            if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector) &&
                (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset ==
                 file_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset) &&
                (search_ptr -> fx_file_open_mode == FX_OPEN_FOR_WRITE))
#endif
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* The file is currently open.  */
                return(FX_ACCESS_ERROR);
            }

            /* Adjust the pointer and decrement the search count.  */
            search_ptr =  search_ptr -> fx_file_opened_next;
            open_count--;
        }
    }

    /* At this point, we are ready to walk list of clusters to setup the
       initial condition of this file as well as to verify its integrity.  */
    cluster =           file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster;
    bytes_remaining =   file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size;
    bytes_per_cluster = ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);
    file_ptr -> fx_file_current_physical_cluster =  0;

    /* Check for invalid value.  */
    if (bytes_per_cluster == 0)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Invalid media, return error.  */
        return(FX_MEDIA_INVALID);
    }

    last_cluster =      0;
    cluster_count =     0;

#ifndef FX_DISABLE_CONSECUTIVE_DETECT
    leading_consecutive = 1;
#endif /* FX_DISABLE_CONSECUTIVE_DETECT */
    file_ptr -> fx_file_consecutive_cluster = 1;
#ifndef FX_DISABLE_FAST_OPEN

    /* Determine if the file is being open for reading with the fast option.  */
    if (fast_open)
    {

        /* Calculate the bytes available.  */
        bytes_available =  ((bytes_remaining + bytes_per_cluster - 1) / bytes_per_cluster) * bytes_per_cluster;

#ifdef FX_ENABLE_EXFAT
        if (bytes_remaining && (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
        {
            cluster_count =
                (ULONG)((file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size + bytes_per_cluster - 1) /
                        bytes_per_cluster);
            file_ptr -> fx_file_consecutive_cluster = cluster_count;
        }
#endif /* FX_ENABLE_EXFAT */
    }
    else
#endif /* FX_DISABLE_FAST_OPEN */
    {
#ifdef FX_ENABLE_EXFAT

        /* File is open for writing... walk the FAT chain to position to the end.  */

        if (bytes_remaining && (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1))
        {
            cluster_count =
                (ULONG)((file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size + bytes_per_cluster - 1) /
                        bytes_per_cluster);

            last_cluster = cluster + cluster_count - 1;

            file_ptr -> fx_file_consecutive_cluster = cluster_count;

            file_ptr -> fx_file_current_physical_cluster = last_cluster;

            file_ptr -> fx_file_current_relative_cluster = cluster_count - 1;

            bytes_remaining %= bytes_per_cluster;

            if (!bytes_remaining)
            {
                if (file_ptr -> fx_file_dir_entry.fx_dir_entry_available_file_size >
                    file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size)
                {
                    file_ptr -> fx_file_current_physical_cluster = last_cluster + 1;
                    file_ptr -> fx_file_current_relative_cluster++;
                }
                else
                {
                    bytes_remaining = bytes_per_cluster;
                }
            }

            bytes_available =
                (ULONG)((file_ptr -> fx_file_dir_entry.fx_dir_entry_available_file_size + bytes_per_cluster - 1) /
                        bytes_per_cluster);
            bytes_available *= bytes_per_cluster;
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */

            /* Follow the link of FAT entries.  */
            while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
            {

                /* Increment the number of clusters.  */
                cluster_count++;

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

                /* Determine if the cluster is invalid (points to itself) or the count exceeds the total number of clusters.  */
                if ((cluster == contents) || (cluster_count > media_ptr -> fx_media_total_clusters))
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the bad status.  */
                    return(FX_FAT_READ_ERROR);
                }

#ifndef FX_DISABLE_CONSECUTIVE_DETECT

                /* Check if present and next clusters are consecutive */
                if (cluster + 1 == contents)
                {
            
                    /* Determine if clusters are consecutive so far.  */
                    if (leading_consecutive)
                    {

                        /* Yes, increment the number of leading consecutive clusters.  */
                        file_ptr -> fx_file_consecutive_cluster++;
                    }
                }
                else
                {

                    /* The clusters are no longer consecutive, clear the consecutive flag.  */
                    leading_consecutive = 0;
                }
#endif /* FX_DISABLE_CONSECUTIVE_DETECT */

                /* Save the last valid cluster.  */
                last_cluster =  cluster;

                /* Setup for the next cluster.  */
                cluster =  contents;

                /* Determine if this is the last written cluster.  We need to remember this
                   for open for writing.  */
                if (bytes_remaining > bytes_per_cluster)
                {

                    /* Still more written clusters, just decrement the counter.  */
                    bytes_remaining =  bytes_remaining - bytes_per_cluster;
                }
                else if (!file_ptr -> fx_file_current_physical_cluster)
                {

                    /* Remember this cluster number.  */
                    file_ptr -> fx_file_current_physical_cluster =  last_cluster;

                    /* Remember the relative cluster.  */
                    file_ptr -> fx_file_current_relative_cluster =  cluster_count - 1;

                    /* If the remaining bytes exactly fits the cluster size, check for
                       a possible adjustment to the next cluster.  */
                    if ((bytes_remaining == bytes_per_cluster) &&
                        (cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                    {

                        /* We need to position to next allocated cluster.  */
                        file_ptr -> fx_file_current_physical_cluster =  cluster;
                        file_ptr -> fx_file_current_relative_cluster++;

                        /* Clear the remaining bytes.  */
                        bytes_remaining =  0;
                    }
                }
            }

            /* Determine if the number of clusters is large enough to support the
               specified file size.  */
            bytes_available =  ((ULONG64)media_ptr -> fx_media_bytes_per_sector) *
                ((ULONG64)media_ptr -> fx_media_sectors_per_cluster) *
                ((ULONG64)cluster_count);

            /* Check the bytes available in the cluster chain against the directory entry file size.  */
            if ((bytes_available < file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size) ||
                ((cluster_count) && (contents < fat_last)))
            {
                /* File is corrupt, release media protection.  */
                FX_UNPROTECT

                /* Return a corrupt file error status.  */
                return(FX_FILE_CORRUPT);
            }
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */
    }

    /* The file is okay, populate the file control block and complete the
       file open process.  */
    file_ptr -> fx_file_id =                        FX_FILE_ID;
    file_ptr -> fx_file_name =                      file_ptr -> fx_file_name_buffer;
    file_ptr -> fx_file_media_ptr =                 media_ptr;
    file_ptr -> fx_file_open_mode =                 open_type;
    file_ptr -> fx_file_modified =                  FX_FALSE;
    file_ptr -> fx_file_total_clusters =            cluster_count;
    file_ptr -> fx_file_first_physical_cluster =    file_ptr -> fx_file_dir_entry.fx_dir_entry_cluster;
    file_ptr -> fx_file_last_physical_cluster =     last_cluster;
    file_ptr -> fx_file_current_file_size =         file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size;
    file_ptr -> fx_file_current_available_size =    bytes_available;
    file_ptr -> fx_file_disable_burst_cache =       FX_FALSE;

    /* Set the current settings based on how the file was opened.  */
    if (open_type == FX_OPEN_FOR_READ)
    {

        /* Position the pointers to the beginning of the file.  */
        file_ptr -> fx_file_current_physical_cluster =  file_ptr -> fx_file_first_physical_cluster;
        file_ptr -> fx_file_current_relative_cluster =  0;
        file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
            (((ULONG64)(file_ptr -> fx_file_first_physical_cluster - FX_FAT_ENTRY_START)) *
             ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
        file_ptr -> fx_file_current_relative_sector =   0;
        file_ptr -> fx_file_current_logical_offset =    0;
        file_ptr -> fx_file_current_file_offset =       0;
    }
    else
    {

        /* Open for writing - position the pointers to the end of the file.  */

        /* Determine if the remaining bytes fit exactly into the cluster size.  */
        if (bytes_remaining == bytes_per_cluster)
        {

            /* Position to the end of the cluster.  */
            file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                (((ULONG64)file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
                 ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                ((ULONG)(((bytes_remaining - 1) / (ULONG)media_ptr -> fx_media_bytes_per_sector)));
            file_ptr -> fx_file_current_relative_sector =   (ULONG)(((bytes_remaining - 1) / (ULONG)media_ptr -> fx_media_bytes_per_sector));
            file_ptr -> fx_file_current_file_offset =       file_ptr -> fx_file_current_file_size;
            file_ptr -> fx_file_current_logical_offset =    media_ptr -> fx_media_bytes_per_sector;
        }
        else
        {

            /* Position file parameters at end of last cluster allocation.  */
            file_ptr -> fx_file_current_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                (((ULONG64)file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
                 ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                ((ULONG)((bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector)));
            file_ptr -> fx_file_current_relative_sector =   (ULONG)((bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector));
            file_ptr -> fx_file_current_file_offset =       file_ptr -> fx_file_current_file_size;
            file_ptr -> fx_file_current_logical_offset =    (ULONG)bytes_remaining % ((ULONG)media_ptr -> fx_media_bytes_per_sector);
        }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* By default, the whole file is used. */
    file_ptr -> fx_file_maximum_size_used = file_ptr -> fx_file_current_file_size;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Place newly opened file on the list of open files for
       this media.  First, check for an empty list.  */
    if (media_ptr -> fx_media_opened_file_list)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  (media_ptr -> fx_media_opened_file_list) -> fx_file_opened_previous;

        /* Place the new file in the list.  */
        (media_ptr -> fx_media_opened_file_list) -> fx_file_opened_previous =  file_ptr;
        tail_ptr -> fx_file_opened_next =  file_ptr;

        /* Setup this file's opened links.  */
        file_ptr -> fx_file_opened_previous =  tail_ptr;
        file_ptr -> fx_file_opened_next =      media_ptr -> fx_media_opened_file_list;
    }
    else
    {

        /* The opened media list is empty.  Add the media to empty list.  */
        media_ptr -> fx_media_opened_file_list =   file_ptr;
        file_ptr ->  fx_file_opened_next =         file_ptr;
        file_ptr ->  fx_file_opened_previous =     file_ptr;
    }

    /* Increment the opened file counter.  */
    media_ptr -> fx_media_opened_file_count++;

    /* Release media protection.  */
    FX_UNPROTECT

    /* Open is complete, return successful status.  */
    return(FX_SUCCESS);
}

