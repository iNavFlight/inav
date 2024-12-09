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
/*    _fx_directory_search                                PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches the media for the supplied name.  The search */
/*    routine will find files as well as directory names.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    name_ptr                              Name pointer                  */
/*    entry_ptr                             Pointer to directory entry    */
/*                                            record                      */
/*    last_dir_ptr                          Pointer to destination for    */
/*                                            the last directory entry    */
/*    last_name_ptr                         Pointer to the last name      */
/*                                            token that was searched for */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_name_extract            Extract directory name from   */
/*                                            input string                */
/*    _fx_directory_entry_read              Read entries from root dir    */
/*    _fx_utility_exFAT_name_hash_get       Get name hash                 */
/*    _fx_utility_FAT_entry_read            Read FAT entries to calculate */
/*                                            the sub-directory size      */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable media search cache, */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            added check for             */
/*                                            volume label,               */
/*                                            resulting in version 6.1.7  */
/*  01-31-2022     William E. Lamie         Modified comment(s), and      */
/*                                            fixed path compare,         */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_search(FX_MEDIA *media_ptr, CHAR *name_ptr, FX_DIR_ENTRY *entry_ptr,
                           FX_DIR_ENTRY *last_dir_ptr, CHAR **last_name_ptr)
{

ULONG         i, n;
UINT          found;
UINT          status;
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
UINT          v, j;
#endif /* FX_MEDIA_DISABLE_SEARCH_CACHE */
ULONG         cluster, next_cluster = 0;
ULONG64       directory_size;
CHAR         *dir_name_ptr;
CHAR         *work_ptr;
CHAR         *source_name_ptr;
CHAR         *destination_name_ptr;
FX_DIR_ENTRY  search_dir;
FX_DIR_ENTRY *search_dir_ptr;
CHAR         *name, alpha, name_alpha;
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
UINT          index;
CHAR         *path_ptr =  FX_NULL;
CHAR         *original_name =  name_ptr;
#endif
#ifdef FX_ENABLE_EXFAT
USHORT        hash = 0;
#endif /* FX_ENABLE_EXFAT */

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory search requests.  */
    media_ptr -> fx_media_directory_searches++;
#endif

    /* Setup pointer to media name buffer.  */
    name =  media_ptr -> fx_media_name_buffer;

    /* Setup the last directory, if required.  */
    if (last_dir_ptr)
    {

        /* Set the first character of the directory entry to NULL to
           indicate root or no directory.  */
        last_dir_ptr -> fx_dir_entry_name[0] =  0;
    }

    /* Determine if the file name has a full directory path.  */
    if ((*name_ptr == '\\') || (*name_ptr == '/'))
    {

        /* Directory name has full path, set the search pointer to NULL.  */
        search_dir_ptr =  FX_NULL;
    }
    else
    {

        /* Set the initial search directory to the current working
           directory - if there is one.  */

        /* First check for a local path pointer stored in the thread control block.  This
           is only available in ThreadX Version 4 and above.  */
#ifndef FX_NO_LOCAL_PATH
        if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
        {

            /* Determine if the local directory is not the root directory.  */
            if (((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_directory.fx_dir_entry_name[0])
            {

                /* Start at the current working directory of the media.  */
                search_dir =   ((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_directory;

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

                /* Setup pointer to the path.  */
                path_ptr =  ((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_string;
#endif

                /* Set the internal pointer to the search directory as well.  */
                search_dir_ptr =  &search_dir;
            }
            else
            {

                /* We are searching in the root directory.  */
                search_dir_ptr =  FX_NULL;
            }
        }
        else
#endif
        if (media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0])
        {

            /* Start at the current working directory of the media.  */
            search_dir =  media_ptr -> fx_media_default_path.fx_path_directory;

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

            /* Setup pointer to the path.  */
            path_ptr =  media_ptr -> fx_media_default_path.fx_path_string;
#endif

            /* Set the internal pointer to the search directory as well.  */
            search_dir_ptr =  &search_dir;
        }
        else
        {

            /* The current default directory is the root so just set the
               search directory pointer to NULL.  */
            search_dir_ptr =  FX_NULL;
        }
    }

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Determine if there is a previously found directory entry.  */
    if (media_ptr -> fx_media_last_found_name[0])
    {

    UINT  match;
    CHAR *temp_ptr, beta;

        /* Yes, there is a previously found directory in our cache.  */

        /* Initialize the index.  */
        v =  0;

        /* Determine if there is a full path.  */
        if ((*name_ptr == '\\') || (*name_ptr == '/'))
        {

            /* Yes, the full path is in the name buffer. Simply compare with what is in
               the last search buffer.  */
            while ((v < (FX_MAX_LAST_NAME_LEN - 1)) && (name_ptr[v]))
            {

                /* Pickup the respective name characters.  */
                alpha =  name_ptr[v];
                beta =   media_ptr -> fx_media_last_found_name[v];

                /* Ensure directory markers are the same.  */
                if (alpha == '\\')
                {
                    alpha =  '/';
                }
                if (beta == '\\')
                {
                    beta =  '/';
                }

                /* Is the name the same?  */
                if (alpha != beta)
                {

                    /* Break out of loop!  */
                    break;
                }

                /* Move to next character.  */
                v++;
            }

            /* Determine if we have a match.  */
            if (name_ptr[v] != media_ptr -> fx_media_last_found_name[v])
            {
                match =  FX_FALSE;
            }
            else
            {
                match =  FX_TRUE;
            }
        }
        else
        {

            /* Default to found.  */
            match =  FX_TRUE;

            /* Determine if there is a default path to compare with.  */
            if (path_ptr)
            {

                /* Yes, compare the current path with what is contained in the last
                   found buffer. Note that the last found name must have at least one
                   path separator as well as room for at least one character for a name. */
                while ((v < (FX_MAX_LAST_NAME_LEN - 1)) && (path_ptr[v]))
                {

                    /* Pickup the respective name characters.  */
                    alpha =  media_ptr -> fx_media_last_found_name[v];
                    beta =   path_ptr[v];

                    /* Ensure directory markers are the same.  */
                    if (alpha == '\\')
                    {
                        alpha =  '/';
                    }
                    if (beta == '\\')
                    {
                        beta =  '/';
                    }

                    /* Is the name the same?  */
                    if (alpha != beta)
                    {

                        /* Break out of loop!  */
                        break;
                    }

                    /* Move to next character.  */
                    v++;
                }

                /* Determine if we don't have a match...  The relative path must be exhausted. */
                if (path_ptr[v])
                {
                    match =  FX_FALSE;
                }
            }

            /* Determine if we still have a match.  */
            if (match)
            {

                /* Now examine the rest of the last name and the newly supplied
                   input name.  */

                /* Determine if a valid directory separator is present.  */
                if ((media_ptr -> fx_media_last_found_name[v] != '\\') &&
                    (media_ptr -> fx_media_last_found_name[v] != '/'))
                {

                    /* Set match to false - invalid directory path separator.  */
                    match =  FX_FALSE;
                }
                else
                {
                    /* Position past the next directory separator in the
                       last name string.  */
                    v++;
                }

                /* Yes, the full path is in the name buffer. Simply compare with what is in
                   the last search buffer.  */
                j =  0;
                while ((v < (FX_MAX_LAST_NAME_LEN - 1)) && (name_ptr[j]) && (match))
                {

                    /* Pickup the respective name characters.  */
                    alpha =  name_ptr[j];
                    beta =   media_ptr -> fx_media_last_found_name[v];

                    /* Ensure directory markers are the same.  */
                    if (alpha == '\\')
                    {
                        alpha =  '/';
                    }
                    if (beta == '\\')
                    {
                        beta =  '/';
                    }

                    /* Is the name the same?  */
                    if (alpha != beta)
                    {

                        /* Break out of loop!  */
                        break;
                    }

                    /* Move to next character.  */
                    v++;
                    j++;
                }

                /* Avoid accessing fx_media_last_found_name out of bounds. */
                if (v >= 256)
                {
                    match = FX_FALSE;
                }
                else if ((match) && (name_ptr[j] != media_ptr -> fx_media_last_found_name[v]))
                {

                    /* We don't have a match.  */
                    match =  FX_FALSE;
                }
            }
        }

        /* Now determine if we actually found a match.  */
        if (match)
        {

            /* Save the directory entry name pointer.  */
            temp_ptr =  entry_ptr -> fx_dir_entry_name;

            /* Copy the saved directory entry.  */
            *entry_ptr =  media_ptr -> fx_media_last_found_entry;

            /* Restore the directory entry name pointer.  */
            entry_ptr -> fx_dir_entry_name =  temp_ptr;

            /* Copy the directory name into the destination directory name.  */
            for (index = 0; index < FX_MAX_LONG_NAME_LEN; index++)
            {

                /* Copy character into the destination.  */
                temp_ptr[index] =  media_ptr -> fx_media_last_found_file_name[index];

                /* See if we have copied the NULL termination character.  */
                if (temp_ptr[index] == (CHAR)FX_NULL)
                {
                
                    /* Determine if we should break here or at the top of the loop.  */
                    if (index < (FX_MAX_LONG_NAME_LEN - 1))
                    {
                    
                        /* Yes, break out of the loop early.  */
                        break;
                    }
                }
            }

            /* Determine if there is a search directory to copy.  */
            if ((last_dir_ptr) && (media_ptr -> fx_media_last_found_directory_valid))
            {

                /* Yes, there was a search directory... and one is requested in this request as well.
                   Simply copy it into the destination.  */

                /* First, save the name pointer from the list directory pointer.  */
                destination_name_ptr =  last_dir_ptr -> fx_dir_entry_name;

                /* Copy the entire directory entry structure.  */
                *last_dir_ptr =  media_ptr -> fx_media_last_found_directory;

                /* Restore the original name buffer pointer.  */
                last_dir_ptr -> fx_dir_entry_name =  destination_name_ptr;

                /* Pickup pointer to name to copy.  */
                source_name_ptr =  media_ptr -> fx_media_last_found_directory.fx_dir_entry_name;

                /* Loop to copy the name into the last directory name buffer.  */
                for (n = 0; n < FX_MAX_LONG_NAME_LEN; n++)
                {
                
                    /* Copy a character.  */
                    destination_name_ptr[n] =  source_name_ptr[n];
                
                    /* See if we have copied the NULL termination character.  */
                    if (source_name_ptr[n] == (CHAR)FX_NULL)
                    {
                
                        /* Determine if we should break here or at the top of the loop.  */
                        if (n < (FX_MAX_LONG_NAME_LEN - 1))
                        {
                    
                            /* Yes, break out of the loop early.  */
                            break;
                        }
                    }
                }
            }

            /* Return the last name pointer, if required.  */
            if (last_name_ptr)
            {

                /* Just set the last name to initial name string.  */
                *last_name_ptr =  temp_ptr;
            }

#ifndef FX_MEDIA_STATISTICS_DISABLE

            /* Increment the number of directory search cache hits.  */
            media_ptr -> fx_media_directory_search_cache_hits++;
#endif

            /* Return success.  */
            return(FX_SUCCESS);
        }
    }

    /* Not a sequential search, invalidate the saved information.  */
    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_DIR_CACHE_MISS, media_ptr, media_ptr -> fx_media_directory_searches - media_ptr -> fx_media_directory_search_cache_hits, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)
#else

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_DIR_CACHE_MISS, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)
#endif
#endif

    /* Loop to traverse the directory paths to find the specified file.  */
    do
    {

        /* Remember the last name pointer, if required.  */
        if (last_name_ptr)
        {

            /* Just set the last name to initial name string.  */
            *last_name_ptr =  name_ptr;
        }

        /* Extract file name.  */
        name_ptr =  _fx_directory_name_extract(name_ptr, name);

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

                        /* Return the bad status.  */
                        return(status);
                    }

                    /* Check for error situation.  */
                    if ((cluster < FX_FAT_ENTRY_START) || (cluster == next_cluster) || (i > media_ptr -> fx_media_total_clusters))
                    {

                        /* Return the bad status.  */
                        return(FX_FAT_READ_ERROR);
                    }

                    cluster = next_cluster;
                }

                /* Now we can calculate the directory size.  */
                directory_size =  (((ULONG64) media_ptr -> fx_media_bytes_per_sector) *
                                   ((ULONG64) media_ptr -> fx_media_sectors_per_cluster) * i)
                                    / (ULONG64) FX_DIR_ENTRY_SIZE;

                /* Also save this in the directory entry so we don't have to
                   calculate it later.  */
                search_dir_ptr -> fx_dir_entry_file_size =  directory_size;
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* If required, copy the last search directory entry into the
               destination.  */
            if (last_dir_ptr)
            {

                /* Copy the last search directory into the destination.  */

                /* First, save the name pointer from the list directory pointer.  */
                destination_name_ptr =  last_dir_ptr -> fx_dir_entry_name;

                /* Copy the entire directory entry structure.  */
                *last_dir_ptr =  *search_dir_ptr;

                /* Restore the original name buffer pointer.  */
                last_dir_ptr -> fx_dir_entry_name =  destination_name_ptr;

                /* Pickup pointer to name to copy.  */
                source_name_ptr =  search_dir_ptr -> fx_dir_entry_name;

                /* Loop to copy the name into the last directory name buffer.  */
                for (n = 0; n < FX_MAX_LONG_NAME_LEN; n++)
                {

                    /* Copy a character.  */
                    destination_name_ptr[n] =  source_name_ptr[n];

                    /* See if we have copied the NULL termination character.  */
                    if (source_name_ptr[n] == (CHAR) FX_NULL)
                    {
                
                        /* Determine if we should break here or at the top of the loop.  */
                        if (n < (FX_MAX_LONG_NAME_LEN - 1))
                        {
                    
                            /* Yes, break out of the loop early.  */
                            break;
                        }
                    }
                }
            }
        }
        else
        {

            /* Directory size is the number of entries in the root directory.  */
            directory_size =  (ULONG)media_ptr -> fx_media_root_directory_entries;
        }

        /* Loop through entries in the directory.  Yes, this is a
           linear search!  */
        i =      0;
        found =  FX_FALSE;

#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {

            /* Get name hash.  */
            hash =  _fx_utility_exFAT_name_hash_get(name);
        }
#endif /* FX_ENABLE_EXFAT */

        do
        {

            /* Read an entry from the directory.  */
#ifdef FX_ENABLE_EXFAT
            status =  _fx_directory_entry_read_ex(media_ptr, search_dir_ptr, &i, entry_ptr, hash);
#else
            status =  _fx_directory_entry_read(media_ptr, search_dir_ptr, &i, entry_ptr);
#endif /* FX_ENABLE_EXFAT */

            i++;

            /* Check for error status.  */
            if (status != FX_SUCCESS)
            {
                return(status);
            }

            /* Determine if this is the last directory entry.  */
#ifdef FX_ENABLE_EXFAT
            if (entry_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER)
#else
            if ((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_DONE)
#endif /* FX_ENABLE_EXFAT */
            {
                break;
            }

            /* Determine if the entry is a volume label entry */
            if ((entry_ptr -> fx_dir_entry_attributes & FX_VOLUME))
            {
                continue;
            }

            /* Determine if this is an empty entry.  */
#ifdef FX_ENABLE_EXFAT
            if (entry_ptr -> fx_dir_entry_type != FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY)
#else
            if (((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE) && (entry_ptr -> fx_dir_entry_short_name[0] == 0))
#endif /* FX_ENABLE_EXFAT */
            {
                continue;
            }

            /* Compare the input name and extension with the directory
               entry.  */
            work_ptr =      &name[0];
            dir_name_ptr =  &(entry_ptr -> fx_dir_entry_name[0]);

            /* Loop to compare names.  */
            do
            {

                /* Pickup character of directory name.  */
                alpha =  *dir_name_ptr;

                /* Pickup character of name.  */
                name_alpha =  *work_ptr;

                /* Determine if its case needs to be changed.  */
                if ((alpha >= 'a') && (alpha <= 'z'))
                {

                    /* Yes, make upper case.  */
                    alpha =  (CHAR)((INT)alpha - 0x20);
                }

                /* Determine if its case needs to be changed.  */
                if ((name_alpha >= 'a') && (name_alpha <= 'z'))
                {

                    /* Yes, make upper case.  */
                    name_alpha =  (CHAR)((INT)name_alpha - 0x20);
                }

                /* Compare name with directory name.  */
                if (alpha != name_alpha)
                {

                    /* The names don't match, get out of the loop. */
                    break;
                }

                /* Otherwise, increment the name pointers.  */
                work_ptr++;
                dir_name_ptr++;
            } while (*dir_name_ptr);

            /* Determine if the requested name has been found.  If so,
               return success to the caller.  */
            if ((*dir_name_ptr == 0) && (*work_ptr == *dir_name_ptr))
            {

                /* Yes, the name was located.  All pertinent directory
                   information is in the directory entry field.  */
                found =  FX_TRUE;
            }
            /* Determine if there is a short name to check.  */
#ifdef FX_ENABLE_EXFAT
            else if ((media_ptr -> fx_media_FAT_type != FX_exFAT) &&
                     (entry_ptr -> fx_dir_entry_short_name[0] != 0))
#else
            else if (entry_ptr -> fx_dir_entry_short_name[0] != 0)
#endif /* FX_ENABLE_EXFAT */
            {

                /* Yes, check for the short part of the name.  */

                /* Compare the input name and extension with the directory entry.  */
                work_ptr =      &name[0];
                dir_name_ptr =  &(entry_ptr -> fx_dir_entry_short_name[0]);

                /* Loop to compare names.  */
                do
                {

                    /* Pickup character of directory name.  */
                    alpha =  *dir_name_ptr;

                    /* Pickup character of name.  */
                    name_alpha =  *work_ptr;

                    /* Determine if its case needs to be changed.  */
                    if ((name_alpha >= 'a') && (name_alpha <= 'z'))
                    {

                        /* Yes, make upper case.  */
                        name_alpha =  (CHAR)((INT)name_alpha - 0x20);
                    }

                    /* Compare name with directory name.  */
                    if (alpha != name_alpha)
                    {

                        /* The names don't match, get out of the loop. */
                        break;
                    }

                    /* Otherwise, move the name pointers and increment the
                       count.  */
                    work_ptr++;
                    dir_name_ptr++;
                } while (*dir_name_ptr);

                /* Determine if the names match.  */
                if ((*dir_name_ptr == 0) && (*work_ptr == *dir_name_ptr))
                {

                    /* Yes, the name was located.  All pertinent directory
                        information is in the directory entry field.  */
                    found =  FX_TRUE;
                }
            }
        } while ((i < directory_size) && (!found));

        /* Now determine if we have a match.  */
        if (!found)
        {

            /* Return a "not found" status to the caller.  */
            return(FX_NOT_FOUND);
        }

        /* Determine if the found entry is indeed a sub-directory.  */
        if (entry_ptr -> fx_dir_entry_attributes & FX_DIRECTORY)
        {

            /* Move the directory search pointer to this entry.  */
            search_dir =      *entry_ptr;
            search_dir_ptr =  &search_dir;

            /* Ensure that the search directory's last search cluster is cleared.  */
            search_dir_ptr -> fx_dir_entry_last_search_cluster =  0;

            /* Now determine if the new search directory is the root
               directory.  */
#ifdef FX_ENABLE_EXFAT
            if ((!search_dir_ptr -> fx_dir_entry_cluster)
                && (media_ptr -> fx_media_FAT_type != FX_exFAT))
#else
            if (!search_dir_ptr -> fx_dir_entry_cluster)
#endif /* FX_ENABLE_EXFAT */
            {

                /* This is a backward link to the root directory.  Make
                   sure this is indicated in the search directory
                   information.  */
                search_dir_ptr -> fx_dir_entry_name[0] =  0;

                /* Determine if we need to remember this in the last
                   directory searched return area.  */
                if (last_dir_ptr)
                {

                    /* Yes, return this value to the caller.  */

                    /* First, save the name pointer from the list directory pointer.  */
                    destination_name_ptr =  last_dir_ptr -> fx_dir_entry_name;

                    /* Copy the entire directory entry structure.  */
                    *last_dir_ptr =  *search_dir_ptr;

                    /* Restore the original name buffer pointer.  */
                    last_dir_ptr -> fx_dir_entry_name =  destination_name_ptr;

                    /* Pickup pointer to name to copy.  */
                    source_name_ptr =  search_dir_ptr -> fx_dir_entry_name;

                    /* Loop to copy the name into the last directory name buffer.  */
                    for (n = 0; n < FX_MAX_LONG_NAME_LEN; n++)
                    {

                        /* Copy a character.  */
                        destination_name_ptr[n] =  source_name_ptr[n];
                    }
                }

                /* Set the search directory pointer to NULL to indicate
                   we are at the root directory.  */
                search_dir_ptr =  FX_NULL;
            }
        }
        else
        {

            /* This is not a directory, we better return not found
               since we can't continue the search.  */
            if (name_ptr)
            {

                /* Return not-found status to caller.  */
                return(FX_NOT_FOUND);
            }
        }
    } while (name_ptr);

    /* If you reach this point, the directory is found absolutely, since !found will return directly in the loop above.   */
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE


    /* At this point, cache the found information.  If a subsequent search for the same name is done,
       it will return immediately.  */

    /* Set the index of the saved name string.  */
    v=  0;

    /* First, build the full path and name.  */
    if ((*original_name != '\\') && (*original_name != '/') && (path_ptr))
    {

        /* Copy the path into the destination.  */
        while ((v< (FX_MAX_LAST_NAME_LEN - 1)) && (path_ptr[v]))
        {

            /* Copy one character.   */
            media_ptr -> fx_media_last_found_name[v] =  path_ptr[v];

            /* Move to next character.  */
            v++;
        }
    }

    /* Now see if there is no directory path symbol in the name itself.  */
    if ((*original_name != '\\') && (*original_name != '/'))
    {

        /* If there is room, place a directory separator character.  */
        if (v < (FX_MAX_LAST_NAME_LEN - 1))
        {
            media_ptr -> fx_media_last_found_name[v++] =  '/';
        }
    }

    /* Now append the name to the path.  */
    j =  0;
    while ((v < FX_MAX_LAST_NAME_LEN) && (original_name[j]))
    {

        /* Copy one character.   */
        media_ptr -> fx_media_last_found_name[v] =  original_name[j];

        /* Move to next character.  */
        v++;
        j++;
    }

    /* Null terminate the last name string.   */
    if (v< FX_MAX_LAST_NAME_LEN)
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
    media_ptr -> fx_media_last_found_entry =  *entry_ptr;

    /* Setup the directory entry for the last found internal file name.  */
    media_ptr -> fx_media_last_found_entry.fx_dir_entry_name =  media_ptr -> fx_media_last_found_file_name;

    /* Copy the actual directory name into the cached directory name.  */
    for (index = 0; index < FX_MAX_LONG_NAME_LEN; index++)
    {

        /* Copy character into the cached directory name.  */
        media_ptr -> fx_media_last_found_file_name[index] =  entry_ptr ->  fx_dir_entry_name[index];

        /* See if we have copied the NULL termination character.  */
        if (entry_ptr -> fx_dir_entry_name[index] == (CHAR)FX_NULL)
        {
                
            /* Check to see if we use the break to get out of the loop.  */
            if (index < (FX_MAX_LONG_NAME_LEN - 1))
            {
                    
                /* Yes, not at the end of the string, break.  */
                break;
            }
        }
    }
#endif

    return(FX_SUCCESS);
}

