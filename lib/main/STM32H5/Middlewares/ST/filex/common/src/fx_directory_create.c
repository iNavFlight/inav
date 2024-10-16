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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_create                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified directory.       */
/*    If found, the create request is invalid and an error is returned    */
/*    to the caller.  After the file name verification is made, a search  */
/*    for a free directory entry will be made.  If nothing is available,  */
/*    an error will be returned to the caller.  Otherwise, if all is      */
/*    okay, an empty directory will be created.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Directory name                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_name_extract            Pickup next part of name      */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_directory_free_search             Search for a free directory   */
/*                                            entry                       */
/*    _fx_utility_exFAT_bitmap_free_cluster_find                          */
/*                                            Find exFAT free cluster     */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_logical_sector_flush      Flush the written log sector  */
/*    _fx_utility_logical_sector_read       Read logical sector           */
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
UINT  _fx_directory_create(FX_MEDIA *media_ptr, CHAR *directory_name)
{

UINT         status;
ULONG        FAT_index;
ULONG        FAT_value;
UINT         sectors;
ULONG        total_clusters;
CHAR        *work_ptr;
ULONG64      logical_sector;
ULONG        i;
FX_DIR_ENTRY dir_entry;
FX_DIR_ENTRY sub_dir_entry;
FX_DIR_ENTRY search_directory;

#ifdef FX_ENABLE_EXFAT
ULONG64      dir_size;
#endif /* FX_ENABLE_EXFAT */
FX_INT_SAVE_AREA


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_creates++;
#endif

    /* Determine if the supplied name is less than the maximum supported name size. The
       maximum name (FX_MAX_LONG_NAME_LEN) is defined in fx_api.h.  */
    i =  0;
    work_ptr =  (CHAR *)directory_name;
    while (*work_ptr && (i < FX_MAX_LONG_NAME_LEN))
    {

        /* Determine if the character designates a new path.  */
        if ((*work_ptr == '\\') || (*work_ptr == '/'))
        {
            /* Yes, reset the name size.  */
            i =  0;
        }
        /* Check for leading spaces.  */
        else if ((*work_ptr != ' ') || (i != 0))
        {

            /* No leading spaces, increment the name size.  */
            i++;
        }

        /* Move to the next character.  */
        work_ptr++;
    }

    /* Determine if the supplied name is valid.  */
    if ((i == 0) || (i >= FX_MAX_LONG_NAME_LEN))
    {

        /* Return an invalid name value.  */
        return(FX_INVALID_NAME);
    }

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name = media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Setup search directory pointer to another media name buffer.  */
    search_directory.fx_dir_entry_name = media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Clear the search short name string.  */
    search_directory.fx_dir_entry_short_name[0] =  0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_CREATE, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

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

    /* Make sure there is at least one cluster remaining for the new file.  */
    if (!media_ptr -> fx_media_available_clusters)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return a no-space error.  */
        return(FX_NO_MORE_SPACE);
    }

    /* Search the system for the supplied directory name.  */
    status =  _fx_directory_search(media_ptr, directory_name, &dir_entry, &search_directory, &work_ptr);

    /* Determine if the search was successful.  */
    if (status == FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Directory found - Return the error code.  */
        return(FX_ALREADY_CREATED);
    }

    /* Determine if there is anything left after the name.  */
    if (_fx_directory_name_extract(work_ptr, &dir_entry.fx_dir_entry_name[0]))
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Extra information after the file name, return an invalid name
           error.  */
        return(FX_INVALID_PATH);
    }

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        if (((dir_entry.fx_dir_entry_name[0] == '.') && (dir_entry.fx_dir_entry_name[1] == 0)) ||
            ((dir_entry.fx_dir_entry_name[0] == '.') && (dir_entry.fx_dir_entry_name[1] == '.') && (dir_entry.fx_dir_entry_name[2] == 0)))
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* We don't need '.' or '..' dirs for exFAT.  */
            return(FX_ALREADY_CREATED);
        }
    }

    /* Save the directory entry size.  */
    dir_size = search_directory.fx_dir_entry_file_size;
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Find a free slot for the new directory.  */
    status =  _fx_directory_free_search(media_ptr, &search_directory, &dir_entry);

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

    /* Now allocate a cluster for our new sub-directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Find free cluster from exFAT media.  */
        status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                            media_ptr -> fx_media_cluster_search_start,
                                                            &FAT_index);

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
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        FAT_index    =      media_ptr -> fx_media_cluster_search_start;
        total_clusters =    media_ptr -> fx_media_total_clusters;

        /* Loop to find the first available cluster.  */
        do
        {

            /* Make sure we don't go past the FAT table.  */
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

    /* Decrease the number of available clusters for the media.  */
    media_ptr -> fx_media_available_clusters--;

    /* Defer the writing of the FAT entry until the directory's first sector
       has been properly written. If a power loss occurs prior to writing
       the FAT entry, it will not result in a lost cluster.  */

    /* Populate the directory entry.  */

    /* Isolate the file name.  */
    _fx_directory_name_extract(work_ptr, &dir_entry.fx_dir_entry_name[0]);

    /* Lockout interrupts for time/date access.  */
    FX_DISABLE_INTS

    /* Set time and date stamps.  */
    dir_entry.fx_dir_entry_time =  _fx_system_time;
    dir_entry.fx_dir_entry_date =  _fx_system_date;

    /* Restore interrupts.  */
    FX_RESTORE_INTS

    /* Set the attributes for the file.  */
    dir_entry.fx_dir_entry_attributes =  FX_DIRECTORY;

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        dir_entry.fx_dir_entry_file_size =
            media_ptr -> fx_media_sectors_per_cluster * media_ptr -> fx_media_bytes_per_sector;
        dir_entry.fx_dir_entry_available_file_size = dir_entry.fx_dir_entry_file_size;

        /* Don't use FAT by default.  */
        dir_entry.fx_dir_entry_dont_use_fat = (CHAR)((INT)((search_directory.fx_dir_entry_dont_use_fat & 1) << 1) | 1);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        /* Set file size to 0. */
        dir_entry.fx_dir_entry_file_size =  0;

#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Set the cluster to EOF.  */
    dir_entry.fx_dir_entry_cluster =    FAT_index;

    /* Is there a leading dot?  */
    if (dir_entry.fx_dir_entry_name[0] == '.')
    {

        /* Yes, toggle the hidden attribute bit.  */
        dir_entry.fx_dir_entry_attributes |=  FX_HIDDEN;
    }

    /* In previous versions, the new directory was written here.  It
       is now at the bottom of the file - after the FAT and the initial
       sub-directory is written out.  This makes the directory create
       more fault tolerant.  */

    /* Calculate the first sector of the sub-directory file.  */
    logical_sector =    ((ULONG) media_ptr -> fx_media_data_sector_start) +
                         (((ULONG64) FAT_index - FX_FAT_ENTRY_START) *
                         ((ULONG) media_ptr -> fx_media_sectors_per_cluster));

    /* Pickup the number of sectors for the initial sub-directory cluster.  */
    sectors =  media_ptr -> fx_media_sectors_per_cluster;

    /* Read the first logical sector associated with the allocated
       cluster.  */
    status =  _fx_utility_logical_sector_read(media_ptr, logical_sector,
                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return error code.  */
        return(status);
    }

    /* Clear the entire first sector of the new sub-directory cluster.  */
    work_ptr =  (CHAR *)media_ptr -> fx_media_memory_buffer;
    i =  0;
    while (i < media_ptr -> fx_media_bytes_per_sector)
    {

        /* Clear 4 bytes.  */
        *((ULONG *)work_ptr) =  (ULONG)0;

        /* Increment pointer.  */
        work_ptr =  work_ptr + sizeof(ULONG);

        /* Increment counter.  */
        i =  i + (ULONG)sizeof(ULONG);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
    {

        /* Write back. */
        status =  _fx_utility_logical_sector_write(media_ptr, logical_sector,
                                                   media_ptr -> fx_media_memory_buffer, (ULONG)1, FX_DIRECTORY_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error code.  */
            return(status);
        }

        /* Force flush the internal logical sector cache.  */
        status =  _fx_utility_logical_sector_flush(media_ptr, logical_sector, sectors, FX_TRUE);

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

    /* Determine if there are more sectors to clear in the first cluster of the new
       sub-directory.  */
    if (sectors > 1)
    {

        /* Yes, invalidate all cached sectors that are contained in the newly allocated first
           cluster of the directory.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled == FX_FALSE)
#endif /* FX_ENABLE_FAULT_TOLERANT */
        {

            /* Flush the internal logical sector cache.  */
            status =  _fx_utility_logical_sector_flush(media_ptr, logical_sector + 1, ((ULONG64)(sectors - 1)), FX_TRUE);

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
        }

        /* Clear all additional sectors of new sub-directory.  */
        sectors--;
        while (sectors)
        {

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of driver write sector(s) requests.  */
            media_ptr -> fx_media_driver_write_requests++;
#endif

            /* Build Write request to the driver.  */
            media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
            media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
            media_ptr -> fx_media_driver_buffer =           media_ptr -> fx_media_memory_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
            media_ptr -> fx_media_driver_logical_sector =   logical_sector + ((ULONG)sectors);
#else
            media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector + ((ULONG)sectors);
#endif
            media_ptr -> fx_media_driver_sectors =          1;
            media_ptr -> fx_media_driver_sector_type =      FX_DIRECTORY_SECTOR;

            /* Set the system write flag since we are writing a directory sector.  */
            media_ptr -> fx_media_driver_system_write =  FX_TRUE;

            /* If trace is enabled, insert this event into the trace buffer.  */
            FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, ((ULONG)logical_sector) + ((ULONG)sectors), 1, media_ptr -> fx_media_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

            /* Invoke the driver to write the sector.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

            /* Clear the system write flag.  */
            media_ptr -> fx_media_driver_system_write =  FX_FALSE;

            /* Determine if an error occurred.  */
            if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
            {

#ifdef FX_ENABLE_FAULT_TOLERANT
                FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return error code.  */
                return(media_ptr -> fx_media_driver_status);
            }

            /* Decrease the number of sectors to clear.  */
            sectors--;
        }
    }

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Build Write request to the driver.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           media_ptr -> fx_media_memory_buffer;
        media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
        media_ptr -> fx_media_driver_sectors =          1;
        media_ptr -> fx_media_driver_sector_type =      FX_DIRECTORY_SECTOR;

        /* Set the system write flag since we are writing a directory sector.  */
        media_ptr -> fx_media_driver_system_write =  FX_TRUE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, ((ULONG)logical_sector) + ((ULONG)sectors), 1, media_ptr -> fx_media_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to write the sector.  */
            (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear the system write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Determine if an error occurred.  */
        if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error code.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Mark the cluster as used.  */
        status = _fx_utility_exFAT_cluster_state_set(media_ptr, FAT_index, FX_EXFAT_BITMAP_CLUSTER_OCCUPIED);

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

        /* Move cluster search pointer forward. */
        media_ptr -> fx_media_cluster_search_start = FAT_index + 1;

        /* Determine if this needs to be wrapped. */
        if (media_ptr -> fx_media_cluster_search_start >= media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
        {

            /* Wrap the search to the beginning FAT entry. */
            media_ptr -> fx_media_cluster_search_start = FX_FAT_ENTRY_START;
        }

        if (search_directory.fx_dir_entry_name[0])
        {

            /* Not root directory.  */
            /* Copy the date and time from the actual sub-directory.  */
            search_directory.fx_dir_entry_time = dir_entry.fx_dir_entry_time;
            search_directory.fx_dir_entry_date = dir_entry.fx_dir_entry_date;

            /* Check if the directory size has changed.  */
            if (search_directory.fx_dir_entry_file_size == dir_size)
            {

                /* Not changed, we need only update time stamps.  */
                status = _fx_directory_exFAT_entry_write(media_ptr, &search_directory, UPDATE_FILE);
            }
            else
            {

                /* Directory size changed, update time stamps and the stream size.  */
                status = _fx_directory_exFAT_entry_write(media_ptr, &search_directory, UPDATE_STREAM);
            }

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
        }
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        /* Now setup the first sector with the initial sub-directory information.  */

        /* Copy the base directory entry to the sub-directory entry.  */
        sub_dir_entry =  dir_entry;

        /* Setup pointer to media name buffer.  */
        sub_dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + (FX_MAX_LONG_NAME_LEN * 3);

        /* Set the directory entry name to all blanks.  */
        work_ptr =  &sub_dir_entry.fx_dir_entry_name[0];
        for (i = 0; i < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); i++)
        {
            *work_ptr++ = ' ';
        }

        sub_dir_entry.fx_dir_entry_long_name_present = 0;

        /* Now build the "." directory entry.  */
        sub_dir_entry.fx_dir_entry_name[0] =        '.';
        sub_dir_entry.fx_dir_entry_name[1] =        0;
        sub_dir_entry.fx_dir_entry_log_sector =     logical_sector;
        sub_dir_entry.fx_dir_entry_byte_offset =    0;

        /* Write the directory's first entry.  */
        status =  _fx_directory_entry_write(media_ptr, &sub_dir_entry);

        /* Check for error condition.  */
        if (status != FX_SUCCESS)
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error status.  */
            return(status);
        }

        /* Now build the ".." directory entry.  */

        /* Determine if the search directory is the root.  */
        if (search_directory.fx_dir_entry_name[0])
        {

            /* Previous directory is not the root directory.  */

            /* Copy into the working directory entry.  */
            sub_dir_entry =  search_directory;

            /* Copy the date and time from the actual sub-directory.  */
            sub_dir_entry.fx_dir_entry_time =  dir_entry.fx_dir_entry_time;
            sub_dir_entry.fx_dir_entry_date =  dir_entry.fx_dir_entry_date;

            /* Adjust pointer to media name buffer.  */
            sub_dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + (FX_MAX_LONG_NAME_LEN * 3);

            /* Change the name to ".."  */
            work_ptr =  &sub_dir_entry.fx_dir_entry_name[0];
            for (i = 0; i < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); i++)
            {
                *work_ptr++ = ' ';
            }

            sub_dir_entry.fx_dir_entry_name[0] =  '.';
            sub_dir_entry.fx_dir_entry_name[1] =  '.';
            sub_dir_entry.fx_dir_entry_name[2] =   0;

            sub_dir_entry.fx_dir_entry_long_name_present = 0;

            /* Set file size to 0. */
            sub_dir_entry.fx_dir_entry_file_size =  0;

            /* Change the logical sector for this entry.  */
            sub_dir_entry.fx_dir_entry_log_sector =  logical_sector;
        }
        else
        {

            /* Just modify the current directory since the parent
               directory is the root.  */
            sub_dir_entry.fx_dir_entry_name[1] =  '.';
            sub_dir_entry.fx_dir_entry_name[2] =   0;

            /* Clear the cluster to indicate the root directory.  */
            sub_dir_entry.fx_dir_entry_cluster =  0;
        }

        /* Setup the byte offset.  */
        sub_dir_entry.fx_dir_entry_byte_offset =  FX_DIR_ENTRY_SIZE;

        /* Write the directory's second entry.  */
        status =  _fx_directory_entry_write(media_ptr, &sub_dir_entry);

        /* Check for error condition.  */
        if (status != FX_SUCCESS)
        {

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error status.  */
            return(status);
        }

        /* Write an EOF in the found FAT entry.  */
        status =  _fx_utility_FAT_entry_write(media_ptr, FAT_index, media_ptr -> fx_media_fat_last);

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

#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_FAULT_TOLERANT

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);
#endif

    /* Now write out the new directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        status = _fx_directory_exFAT_entry_write(media_ptr, &dir_entry, UPDATE_FULL);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        status = _fx_directory_entry_write(media_ptr, &dir_entry);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

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

    /* Directory create is complete, return status.  */
    return(status);
}

