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
#endif /* FX_ENABLE_EXFAT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_free_search                           PORTABLE C      */
/*                                                           6.1.12       */
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
/*    _fx_directory_entry_read              Read entries from directory   */
/*    _fx_directory_entry_write             Write entries to directory    */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_utility_logical_sector_flush      Flush logical sector cache    */
/*    _fx_utility_logical_sector_read       Read logical sector           */
/*    _fx_utility_logical_sector_write      Write logical sector          */
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
/*  07-29-2022     Bhupendra Naphade        Modified comment(s),          */
/*                                            updated available cluster   */
/*                                            check for sub directory,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_free_search(FX_MEDIA *media_ptr, FX_DIR_ENTRY *directory_ptr, FX_DIR_ENTRY *entry_ptr)
{

ULONG         i, j;
UCHAR        *work_ptr;
UINT          status, total_entries;
ULONG         entry_sector, entry_offset;
ULONG         FAT_index, FAT_value;
ULONG         cluster, total_clusters, clusters_needed;
ULONG         first_new_cluster, last_cluster, clusters;
ULONG         directory_index;
ULONG         directory_entries;
ULONG         logical_sector;
FX_DIR_ENTRY *search_dir_ptr;
ULONG         free_entry_start;
UINT          sectors;

FX_INT_SAVE_AREA


#ifdef FX_ENABLE_EXFAT
    /* Check if media format is exFAT.  */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Call exFAT specific function.  */
        return(_fx_directory_exFAT_free_search(media_ptr, directory_ptr, entry_ptr));
    }
#endif /* FX_ENABLE_EXFAT */

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory free entry search requests.  */
    media_ptr -> fx_media_directory_free_searches++;
#endif

    /* Initialize the entry sector values.  */
    entry_sector = entry_offset = 0;

    /* Set the long file name flag to false.  */
    entry_ptr -> fx_dir_entry_long_name_present =  0;

    /* Are there leading dots?  */
    if (entry_ptr -> fx_dir_entry_name[0] == '.')
    {

        /* Is there more than 1 dot?  */
        if (entry_ptr -> fx_dir_entry_name[1] == '.')
        {
            /* Yes, consider the name invalid.  */
            return(FX_INVALID_NAME);
        }
    }

    /* Determine if a long file name is present.  */
    for (i = 0, j = 0; entry_ptr -> fx_dir_entry_name[i]; i++)
    {

        /* Check for upper-case characters.  */
        if ((entry_ptr -> fx_dir_entry_name[i] >= 'A') && (entry_ptr -> fx_dir_entry_name[i] <= 'Z'))
        {
            continue;
        }
        /* Check for numeric characters.  */
        else if ((entry_ptr -> fx_dir_entry_name[i] >= '0') && (entry_ptr -> fx_dir_entry_name[i] <= '9'))
        {
            continue;
        }
        /* Check for any lower-case characters.  */
        else if ((entry_ptr -> fx_dir_entry_name[i] >= 'a') && (entry_ptr -> fx_dir_entry_name[i] <= 'z'))
        {
            entry_ptr -> fx_dir_entry_long_name_present =  1;
        }
        /* Check for a space in the middle of the name.  */
        else if (entry_ptr -> fx_dir_entry_name[i] == ' ')
        {
            entry_ptr -> fx_dir_entry_long_name_present = 1;
        }
        /* Check for a dot in the name.  */
        else if (entry_ptr -> fx_dir_entry_name[i] == '.')
        {
            /* Determine if this is the first dot detected.  */
            if (j == 0)
            {
                /* First dot, remember where it was.  */
                j = i;

                /* Determine if this is a leading dot.  */
                if (i == 0)
                {

                    /* Leading dot detected, treat as a long filename.  */
                    entry_ptr -> fx_dir_entry_long_name_present =  1;
                }
            }
            else
            {
                /* Second dot detected, must have a long file name.  */
                entry_ptr -> fx_dir_entry_long_name_present = 1;
            }
        }
        /* Check for a special 0xE5 character.  */
        else if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] == (UCHAR)0xE5)
        {
            entry_ptr -> fx_dir_entry_long_name_present = 1;
        }
        /* Check for code point value greater than 127.  */
        else if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] > (UCHAR)127)
        {
            continue;
        }
        /* Check for any special characters.  */
        else if ((entry_ptr -> fx_dir_entry_name[i] == '~') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '-') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '_') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '}') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '{') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '(') ||
                 (entry_ptr -> fx_dir_entry_name[i] == ')') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '`') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '\'') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '!') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '#') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '$') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '&') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '@') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '^') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '%'))
        {
            continue;
        }
        /* Check for long filename special characters.  */
        else if ((entry_ptr -> fx_dir_entry_name[i] == '+') ||
                 (entry_ptr -> fx_dir_entry_name[i] == ',') ||
                 (entry_ptr -> fx_dir_entry_name[i] == ';') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '=') ||
                 (entry_ptr -> fx_dir_entry_name[i] == '[') ||
                 (entry_ptr -> fx_dir_entry_name[i] == ']'))
        {
            entry_ptr -> fx_dir_entry_long_name_present = 1;
        }
        /* Something is wrong with the supplied name.  */
        else
        {
            return(FX_INVALID_NAME);
        }
    }

    /* Determine if a dot was found.  */
    if (j != 0)
    {

        /* Yes, Determine if the extension exceeds a 3 character extension.  */
        if ((i - j) > 4)
        {

            /* Yes, long file name is present.  */
            entry_ptr -> fx_dir_entry_long_name_present = 1;
        }
    }

    /* Calculate the total entries needed.  */
    if ((i <= 12) && (entry_ptr -> fx_dir_entry_long_name_present == 0))
    {

        /* Initialize the total entries to 1.  */
        total_entries = 1;

        /* Check for special instance of long file name.  */
        if ((j >= 9) || ((i - j) >= 9))
        {

            /* The dot is after 8 character or there is no dot and the name
               is greater than 8 character. */
            entry_ptr -> fx_dir_entry_long_name_present = 1;
            total_entries = 2;
        }
    }
    else
    {

        /* Long file name is present, calculate how many entries are needed
           to represent it.  */
        if (i % 13 == 0)
        {
            /* Exact fit, just add one for the 8.3 short name.  */
            total_entries = i / 13 + 1;
        }
        else
        {
            /* Non-exact fit, add two for 8.3 short name and overlap.  */
            total_entries = i / 13 + 2;
        }
    }

    /* Determine if the search is in the root directory or in a
       sub-directory.  Note: the directory search function clears the
       first character of the name for the root directory.  */
    if (directory_ptr -> fx_dir_entry_name[0])
    {

        /* Search for a free entry in a sub-directory.  */

        /* Pickup the number of entries in this directory.  This was placed
           into the unused file size field.  */
        directory_entries =  (ULONG)directory_ptr -> fx_dir_entry_file_size;

        /* Point the search directory pointer to this entry.  */
        search_dir_ptr =  directory_ptr;

        /* Ensure that the search directory's last search cluster is cleared.  */
        search_dir_ptr -> fx_dir_entry_last_search_cluster =  0;

        /* Set the initial index to 2, since the first two directory entries are
           always allocated.  */
        directory_index =  2;
    }
    else
    {

        /* Find a free entry in the root directory.  */

        /* Setup the number of directory entries.  */
        directory_entries =  (ULONG)media_ptr -> fx_media_root_directory_entries;

        /* Set the search pointer to NULL since we are working off of the
           root directory.  */
        search_dir_ptr =  FX_NULL;

        /* Set the initial index to 0, since the first entry of the root directory is valid.  */
        directory_index =  0;
    }

    /* Loop through entries in the search directory.  Yes, this is a
       linear search!  */
    free_entry_start = directory_entries;
    do
    {

        /* Read an entry from the directory.  */
        status =  _fx_directory_entry_read(media_ptr, search_dir_ptr, &directory_index, entry_ptr);

        /* Check for error status.  */
        if (status != FX_SUCCESS)
        {
            return(status);
        }

        /* Determine if this is an empty entry.  */
        if ((((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE) && (entry_ptr -> fx_dir_entry_short_name[0] == 0)) ||
            ((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_DONE))
        {

            /* Determine how many entries are needed.  */
            if (total_entries > 1)
            {

                /* Multiple entries are needed for long file names.  Mark this
                   entry as free. */
                if (entry_ptr -> fx_dir_entry_name[0] == FX_DIR_ENTRY_DONE)
                {

                    entry_ptr -> fx_dir_entry_long_name_present =  0;
                    entry_ptr -> fx_dir_entry_name[0] =      (CHAR)FX_DIR_ENTRY_FREE;
                    entry_ptr -> fx_dir_entry_name[1] =      (CHAR)0;

                    /* Write out the directory entry.  */
                    status = _fx_directory_entry_write(media_ptr, entry_ptr);
                    if(status != FX_SUCCESS)
                    {  
                        return(status);
                    }
  
                    /* Note that for long names we need to avoid holes in the middle,
                       i.e. entries must be logically contiguous.  */
                }
            }

            /* Determine if we are at the first free entry.  */
            if (free_entry_start == directory_entries)
            {

                /* Remember the start of the free entry.  */
                free_entry_start =  directory_index;
                entry_sector =      (ULONG)entry_ptr -> fx_dir_entry_log_sector;
                entry_offset  =     entry_ptr -> fx_dir_entry_byte_offset;
            }

            /* Determine if there are enough free entries to satisfy the request.  */
            if ((directory_index - free_entry_start + 1) >= total_entries)
            {

                /* Found an empty slot.  Most pertinent information is already
                   in the entry structure.  */

                /* Setup the the sector and the offset.  */
                entry_ptr -> fx_dir_entry_log_sector =      entry_sector;
                entry_ptr -> fx_dir_entry_byte_offset =     entry_offset;

                /* Initialize the additional directory entries.  */
                entry_ptr -> fx_dir_entry_reserved =            0;
                entry_ptr -> fx_dir_entry_created_time_ms =     0;

                /* Lockout interrupts for time/date access.  */
                FX_DISABLE_INTS

                entry_ptr -> fx_dir_entry_created_time =        _fx_system_time;
                entry_ptr -> fx_dir_entry_created_date =        _fx_system_date;
                entry_ptr -> fx_dir_entry_last_accessed_date =  _fx_system_date;

                /* Restore interrupts.  */
                FX_RESTORE_INTS

                /* Determine if a long file name is present.  */
                if (total_entries == 1)
                {
                    entry_ptr -> fx_dir_entry_long_name_present =  0;
                }
                else
                {
                    entry_ptr -> fx_dir_entry_long_name_present =  1;
                }

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
        directory_index++;

        /* Determine if we have exceeded the number of entries in the current directory.  */
        if (directory_index >= directory_entries)
        {

            /* Calculate how many sectors we need for the new directory entry.  */
            sectors =  ((total_entries * FX_DIR_ENTRY_SIZE) + (media_ptr -> fx_media_bytes_per_sector - 1))/
                                                                            media_ptr -> fx_media_bytes_per_sector;

            /* Now calculate how many clusters we need for the new directory entry.  */
            clusters_needed = (sectors + (media_ptr -> fx_media_sectors_per_cluster - 1)) / media_ptr -> fx_media_sectors_per_cluster;

            /* Not enough empty entries were found.  If the specified directory is a sub-directory,
               attempt to allocate another cluster to it.  */
            if (((search_dir_ptr) || (media_ptr -> fx_media_32_bit_FAT)) && (media_ptr -> fx_media_available_clusters >= clusters_needed))
            {

                /* Search for the additional clusters we need.  */
                first_new_cluster =  0;
                total_clusters =     media_ptr -> fx_media_total_clusters;
                last_cluster =       0;
                FAT_index    =       media_ptr -> fx_media_cluster_search_start;
                clusters =           clusters_needed;

                /* Loop to find the needed clusters.  */
                while (clusters)
                {

                    /* Decrease the cluster count.  */
                    clusters--;

                    /* Loop to find the first available cluster.  */
                    do
                    {

                        /* Make sure we stop looking after one pass through the FAT table.  */
                        if (!total_clusters)
                        {

                            /* Something is wrong with the media - the desired clusters were
                               not found in the FAT table.  */
                            return(FX_NO_MORE_SPACE);
                        }

                        /* Read FAT entry.  */
                        status =  _fx_utility_FAT_entry_read(media_ptr, FAT_index, &FAT_value);

                        /* Check for a bad status.  */
                        if (status != FX_SUCCESS)
                        {

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

                    /* We found an available cluster.  We now need to clear all of entries in
                       each of the cluster's sectors.  */

                    /* Calculate the logical sector of this cluster.  */
                    logical_sector =  ((ULONG) media_ptr -> fx_media_data_sector_start) +
                                       ((((ULONG) FAT_index) - FX_FAT_ENTRY_START) *
                                       ((ULONG) media_ptr -> fx_media_sectors_per_cluster));

                    /* Pickup the number of sectors for the next directory cluster.  */
                    sectors =  media_ptr -> fx_media_sectors_per_cluster;

                    /* Read the logical sector just for cache reasons.  */
                    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
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
                        i =  i + (ULONG)sizeof(ULONG);
                    }

                    /* Write the logical sector to ensure the zeros are written.  */
                    status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                               media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

                    /* Determine if the write was successful.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Return the error code.  */
                        return(status);
                    }

                    /* Determine if there are more sectors to clear in the first cluster of the new
                       sub-directory.  */
                    if (sectors > 1)
                    {

                        /* Yes, invalidate all cached sectors that are contained in the newly allocated first
                           cluster of the directory.  */

                        /* Flush the internal logical sector cache.  */
                        status =  _fx_utility_logical_sector_flush(media_ptr, (ULONG64) (logical_sector + 1), (ULONG64) (sectors - 1), FX_TRUE);

                        /* Determine if the flush was successful.  */
                        if (status != FX_SUCCESS)
                        {

                            /* Return the error code.  */
                            return(status);
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
                            media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector + ((ULONG)sectors);
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

                                /* Return error code.  */
                                return(media_ptr -> fx_media_driver_status);
                            }

                            /* Decrease the number of sectors to clear.  */
                            sectors--;
                        }
                    }

                    /* Determine if we have found the first new cluster yet.  */
                    if (first_new_cluster == 0)
                    {

                        /* Remember the first new cluster. */
                        first_new_cluster =  FAT_index;
                    }

                    /* Check for a valid last cluster to link.  */
                    if (last_cluster)
                    {

                        /* Normal condition - link the last cluster with the new
                           found cluster.  */
                        status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, FAT_index);

                        /* Check for a bad FAT write status.  */
                        if (status !=  FX_SUCCESS)
                        {

                            /* Return the bad status.  */
                            return(status);
                        }
                    }

                    /* Otherwise, remember the new FAT index as the last.  */
                    last_cluster =  FAT_index;

                    /* Move to the next FAT entry.  */
                    FAT_index =  media_ptr -> fx_media_cluster_search_start;
                }

                /* Place an end-of-file marker on the last cluster.  */
                status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, media_ptr -> fx_media_fat_last);

                /* Check for a bad FAT write status.  */
                if (status !=  FX_SUCCESS)
                {

                    /* Return the bad status.  */
                    return(status);
                }

#ifdef FX_FAULT_TOLERANT

                /* Ensure the new FAT chain is properly written to the media.  */

                /* Flush the cached individual FAT entries */
                _fx_utility_FAT_flush(media_ptr);
#endif

                /* Now the new cluster needs to be linked to the sub-directory.  */
                if (search_dir_ptr)
                {
                    cluster = search_dir_ptr -> fx_dir_entry_cluster;
                }
                else
                {
                    cluster = media_ptr -> fx_media_root_cluster_32;
                }

                /* Initialize loop variables.  */
                last_cluster =  0;
                i =  0;

                /* Follow the link of FAT entries.  */
                while (cluster < media_ptr -> fx_media_fat_reserved)
                {

                    /* Read the current cluster entry from the FAT.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &FAT_value);
                    i++;

                    /* Check the return value.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Return the error status.  */
                        return(status);
                    }

                    /* Determine if the FAT read was invalid.  */
                    if ((cluster < FX_FAT_ENTRY_START) || (cluster == FAT_value) || (i > media_ptr -> fx_media_total_clusters))
                    {

                        /* Return the bad status.  */
                        return(FX_FAT_READ_ERROR);
                    }

                    /* Save the last valid cluster.  */
                    last_cluster =  cluster;

                    /* Setup for the next cluster.  */
                    cluster =  FAT_value;
                }

                /* Decrease the available clusters in the media.  */
                media_ptr -> fx_media_available_clusters =  media_ptr -> fx_media_available_clusters - clusters_needed;

                /* Increase the number of directory entries.  */
                directory_entries =  directory_entries + ((clusters_needed * media_ptr -> fx_media_sectors_per_cluster) * media_ptr -> fx_media_bytes_per_sector) / FX_DIR_ENTRY_SIZE;

                /* Determine if we need to reset the free entry start since we changed the
                   number of directory entries.  If the last entry was not free, then we
                   should definitely reset the free entry start.  */
                if (!(((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR) FX_DIR_ENTRY_FREE) && (entry_ptr -> fx_dir_entry_short_name[0] == 0)))
                {

                    /* Reset the free entry start to indicate we haven't found a starting free entry yet.  */
                    free_entry_start =  directory_entries;
                }

                /* Update the directory size field.  */
                directory_ptr -> fx_dir_entry_file_size =  directory_entries;

                /* Defer the update of the FAT entry and the last cluster of the current
                   directory entry until after the new cluster is initialized and written out.  */

                /* Determine if a FAT32 is present.  */
                if ((media_ptr -> fx_media_32_bit_FAT) && (search_dir_ptr == FX_NULL))
                {

                    /* Change root directory entry count - FAT32 has a variable sized root directory.  */
                    media_ptr -> fx_media_root_directory_entries =  directory_entries;
                }

                /* At this point, link up the last cluster with the new cluster.  */
                status =  _fx_utility_FAT_entry_write(media_ptr, last_cluster, first_new_cluster);

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

#ifdef FX_FAULT_TOLERANT

                /* Flush the cached individual FAT entries */
                _fx_utility_FAT_flush(media_ptr);
#endif
            }
        }
    } while (directory_index < directory_entries);

    /* Return FX_NO_MORE_SPACE status to the caller.  */
    return(FX_NO_MORE_SPACE);
}

