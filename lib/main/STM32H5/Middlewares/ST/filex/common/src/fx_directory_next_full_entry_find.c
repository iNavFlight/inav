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

#ifndef FX_NO_LOCAL_PATH
FX_LOCAL_PATH_SETUP
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_next_full_entry_find                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the name of the next entry in the current     */
/*    working directory and various information about the name.  The      */
/*    function that returns the first name in the current directory must  */
/*    be called prior to this function.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Destination for directory     */
/*                                            name                        */
/*    attributes                            Destination for attributes    */
/*    size                                  Destination for size          */
/*    year                                  Destination for year          */
/*    month                                 Destination for month         */
/*    day                                   Destination for day           */
/*    hour                                  Destination for hour          */
/*    minute                                Destination for minute        */
/*    second                                Destination for second        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_read              Read entries from root dir    */
/*    _fx_utility_FAT_entry_read            Read FAT entries              */
/*    _fx_directory_exFAT_entry_read        Read exFAT entries            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code and                                                */
/*    FileX System Functions                                              */
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
UINT  _fx_directory_next_full_entry_find(FX_MEDIA *media_ptr,
                                         CHAR *directory_name, UINT *attributes, ULONG *size,
                                         UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second)
{

ULONG         i;
UINT          status;
UINT          temp_status;
ULONG         cluster, next_cluster = 0;
ULONG64       directory_size;
FX_DIR_ENTRY  entry;
FX_DIR_ENTRY *search_dir_ptr;
FX_PATH      *path_ptr;
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
UINT          index;
CHAR         *path_string_ptr =  FX_NULL;
#endif
#ifdef FX_ENABLE_EXFAT
/* TODO: maybe will be better to use dynamic memory, just because unicode_name is not needed in case of not exFAT.  */
UCHAR         unicode_name[FX_MAX_LONG_NAME_LEN * 2];
UINT          unicode_len = 0;
#endif /* FX_ENABLE_EXFAT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_next_full_entry_finds++;
#endif

    /* Setup pointer to media name buffer.  */
    entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    entry.fx_dir_entry_short_name[0] =  0;

#ifdef FX_ENABLE_EXFAT
    /* Will be set by exFAT.  */
    entry.fx_dir_entry_secondary_count = 0;
#endif /* FX_ENABLE_EXFAT */

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_NEXT_FULL_ENTRY_FIND, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* First check for a local path pointer stored in the thread control block.  This
       is only available in ThreadX Version 4 and above.  */
#ifndef FX_NO_LOCAL_PATH
    if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
    {

        /* Setup the default path pointer.  */
        path_ptr =  (FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr;

        /* Determine if we are at the root directory.  */
        if (path_ptr -> fx_path_directory.fx_dir_entry_name[0])
        {

            /* No, we are not at the root directory.  */

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

            /* Setup pointer to the path.  */
            path_string_ptr =  ((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_string;
#endif

            /* Set the internal pointer to the search directory as well.  */
            search_dir_ptr =  &path_ptr -> fx_path_directory;
        }
        else
        {

            /* The current default directory is the root so just set the
               search directory pointer to NULL.  */
            search_dir_ptr =  FX_NULL;
        }
    }
    else
#endif

    /* Set the initial search directory to the current working
       directory - if there is one.  */
    if (media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0])
    {

        /* Setup the path pointer to the global media path.  */
        path_ptr =  &media_ptr -> fx_media_default_path;

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

        /* Setup pointer to the path.  */
        path_string_ptr =  media_ptr -> fx_media_default_path.fx_path_string;
#endif

        /* Set the internal pointer to the search directory as well.  */
        search_dir_ptr =  &path_ptr -> fx_path_directory;
    }
    else
    {

        /* Setup the path pointer to the global media path.  */
        path_ptr =  &media_ptr -> fx_media_default_path;

        /* The current default directory is the root so just set the
           search directory pointer to NULL.  */
        search_dir_ptr =  FX_NULL;
    }

    /* Calculate the directory size.  */
    if (search_dir_ptr)
    {

#ifdef FX_ENABLE_EXFAT

        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            directory_size = search_dir_ptr -> fx_dir_entry_file_size / FX_DIR_ENTRY_SIZE;
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */

            /* Determine the directory size.  */
            if (path_ptr -> fx_path_current_entry !=  0)
            {

                /* Pickup the previously saved directory size.  */
                directory_size =  search_dir_ptr -> fx_dir_entry_file_size;
            }
            else
            {

                /* This should only be done on the first time into next directory find.  */

                /* Ensure that the search directory's last search cluster is cleared.  */
                search_dir_ptr -> fx_dir_entry_last_search_cluster =  0;

                /* Calculate the directory size by counting the allocated
                clusters for it.  */
                i =        0;
                cluster =  search_dir_ptr -> fx_dir_entry_cluster;
                while (cluster < media_ptr -> fx_media_fat_reserved)
                {

                    /* Increment the cluster count.  */
                    i++;

                    /* Read the next FAT entry.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);


                    /* Check the return status.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the bad status.  */
                        return(status);
                    }

                    if ((cluster < FX_FAT_ENTRY_START) || (cluster == next_cluster) || (i > media_ptr -> fx_media_total_clusters))
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the bad status.  */
                        return(FX_FAT_READ_ERROR);
                    }

                    cluster = next_cluster;
                }

                /* Now we can calculate the directory size.  */
                directory_size =  (((ULONG64) media_ptr -> fx_media_bytes_per_sector) *
                                   ((ULONG64) media_ptr -> fx_media_sectors_per_cluster) * i)
                                    / (ULONG64) FX_DIR_ENTRY_SIZE;

                /* Save how many entries there are in the directory.  */
                search_dir_ptr -> fx_dir_entry_file_size =  directory_size;
            }
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */
    }
    else
    {

        /* Directory size is the number of entries in the root directory.  */
        directory_size =  (ULONG)media_ptr -> fx_media_root_directory_entries;
    }

    /* Preset status with an error return.  */
    status =  FX_NO_MORE_ENTRIES;

    /* Determine if the current entry is inside of the directory's range.  */
    while (path_ptr -> fx_path_current_entry < directory_size)
    {

        /* Read an entry from the directory.  */
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            temp_status = _fx_directory_exFAT_entry_read(media_ptr, search_dir_ptr,
                                                         &(path_ptr -> fx_path_current_entry), &entry,
                                                         0, FX_FALSE, unicode_name, &unicode_len);
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */
            temp_status = _fx_directory_entry_read(media_ptr, search_dir_ptr,
                                                   &(path_ptr -> fx_path_current_entry), &entry);
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        /* Check for error status.  */
        if (temp_status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error status.  */
            return(temp_status);
        }

#ifdef FX_ENABLE_EXFAT
        if (entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER)
        {
            /* Set the error code.  */
            status =  FX_NO_MORE_ENTRIES;

            /* Get out of the loop.  */
            break;
        }
#endif /* FX_ENABLE_EXFAT */

        /* Check to see if the entry has something in it.  */
#ifdef FX_ENABLE_EXFAT
        else if (entry.fx_dir_entry_type != FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY)
#else
        if (((UCHAR)entry.fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE) && (entry.fx_dir_entry_short_name[0] == 0))
#endif /* FX_ENABLE_EXFAT */
        {

            /* Current entry is free, skip to next entry and continue the loop.  */
            path_ptr -> fx_path_current_entry++;
            continue;
        }

        /* Determine if a valid directory entry is present.  */
#ifdef FX_ENABLE_EXFAT
        else /* FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY */
#else
        else if ((UCHAR)entry.fx_dir_entry_name[0] != (UCHAR)FX_DIR_ENTRY_DONE)
#endif /* FX_ENABLE_EXFAT */
        {

            /* A valid directory entry is present.  */

            /* Copy the name into the destination.  */
            for (i = 0; entry.fx_dir_entry_name[i]; i++)
            {

                *directory_name =  entry.fx_dir_entry_name[i];
                directory_name++;
            }

            /* Place a NULL at the end of the directory name.  */
            *directory_name =  (CHAR)0;

            /* Determine if the entry's attributes are required.  */
            if (attributes)
            {

                /* Pickup the entry's attributes.  */
                *attributes =  entry.fx_dir_entry_attributes;
            }

            /* Determine if the entry's size is required.  */
            if (size)
            {

                /* Pickup the entry's size.  */
                *size =  (ULONG)entry.fx_dir_entry_file_size;
            }

            /* Determine if the entry's year is required.  */
            if (year)
            {

                /* Pickup the entry's year.  */
                *year =  ((entry.fx_dir_entry_date >> FX_YEAR_SHIFT) & FX_YEAR_MASK) + FX_BASE_YEAR;
            }

            /* Determine if the entry's month is required.  */
            if (month)
            {

                /* Pickup the entry's month.  */
                *month =  (entry.fx_dir_entry_date >> FX_MONTH_SHIFT) & FX_MONTH_MASK;
            }

            /* Determine if the entry's day is required.  */
            if (day)
            {

                /* Pickup the entry's day.  */
                *day =  entry.fx_dir_entry_date & FX_DAY_MASK;
            }

            /* Determine if the entry's hour is required.  */
            if (hour)
            {

                /* Pickup the entry's hour.  */
                *hour =  (entry.fx_dir_entry_time >> FX_HOUR_SHIFT) & FX_HOUR_MASK;
            }

            /* Determine if the entry's minute is required.  */
            if (minute)
            {

                /* Pickup the entry's minute.  */
                *minute =  (entry.fx_dir_entry_time >> FX_MINUTE_SHIFT) & FX_MINUTE_MASK;
            }

            /* Determine if the entry's second is required.  */
            if (second)
            {

                /* Pickup the entry's second.  */
                *second =  (entry.fx_dir_entry_time & FX_SECOND_MASK) * 2;
            }

            /* Increment the current entry for the media.  */
            path_ptr -> fx_path_current_entry++;

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
            {
            UINT v, j;


                /* If a subsequent search for the same name is done, it will find it immediately.  */

                /* Set the index of the saved name string.  */
                v =  0;

                /* First, build the full path and name.  */
                if (path_string_ptr)
                {

                    /* Copy the path into the destination.  */
                    while ((v < (FX_MAX_LAST_NAME_LEN - 1)) && (path_string_ptr[v]))
                    {

                        /* Copy one character.   */
                        media_ptr -> fx_media_last_found_name[v] =  path_string_ptr[v];

                        /* Move to next character.  */
                        v++;
                    }
                }

                /* We know there is room at this point, place a directory separator character.  */
                media_ptr -> fx_media_last_found_name[v++] =  '/';

                /* Now append the name to the path.  */
                j =  0;
                while ((v < FX_MAX_LAST_NAME_LEN) && (entry.fx_dir_entry_name[j]))
                {

                    /* Copy one character.   */
                    media_ptr -> fx_media_last_found_name[v] =  entry.fx_dir_entry_name[j];

                    /* Move to next character.  */
                    v++;
                    j++;
                }

                /* Null terminate the last name string.   */
                if (v < FX_MAX_LAST_NAME_LEN)
                {

                    /* Null terminate.  */
                    media_ptr -> fx_media_last_found_name[v] =  FX_NULL;
                }
                else
                {

                    /* The string is too big, NULL the string so it won't be used in searching.  */
                    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;
                }

                /* Determine if there is a search pointer.  */
                if (search_dir_ptr)
                {

                    /* Yes, there is a search directory pointer so save it!   */
                    media_ptr -> fx_media_last_found_directory =  *search_dir_ptr;

                    /* Indicate the search directory is valid.  */
                    media_ptr -> fx_media_last_found_directory_valid =  FX_TRUE;
                }
                else
                {

                    /* Indicate the search directory is not valid.  */
                    media_ptr -> fx_media_last_found_directory_valid =  FX_FALSE;
                }

                /* Copy the directory entry.  */
                media_ptr -> fx_media_last_found_entry =  entry;

                /* Setup the last found directory entry to point at the last found internal file name.  */
                media_ptr -> fx_media_last_found_entry.fx_dir_entry_name =  media_ptr -> fx_media_last_found_file_name;

                /* Copy the actual directory name into the cached directory name.  */
                for (index = 0; index < FX_MAX_LONG_NAME_LEN; index++)
                {

                    /* Copy character into the cached directory name.  */
                    media_ptr -> fx_media_last_found_file_name[index] =  entry.fx_dir_entry_name[index];

                    /* See if we have copied the NULL termination character.  */
                    if (entry.fx_dir_entry_name[index] == (CHAR)FX_NULL)
                    {
                    
                        /* Check to see if we use the break to get out of the loop.  */
                        if (v < (FX_MAX_LONG_NAME_LEN - 1))
                        {
                        
                            /* Yes, not at the end of the string, break.  */
                            break;
                        }
                    }
                }
            }
#endif

            /* Set return status to success.  */
            status =  FX_SUCCESS;

            /* Get out of the loop.  */
            break;
        }
#ifndef FX_ENABLE_EXFAT
        else
        {
            /* Set the error code.  */
            status =  FX_NO_MORE_ENTRIES;

            /* Get out of the loop.  */
            break;
        }
#endif /* FX_ENABLE_EXFAT */
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return status to the caller.  */
    return(status);
}

