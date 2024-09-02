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
#include "fx_file.h"
#include "fx_utility.h"
#include "fx_directory.h"
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
/*    _fx_directory_default_set                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the default directory of the media to the path   */
/*    specified by the caller.  If this path is not found, an error code  */
/*    is returned.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    new_path_name                         New path to set current       */
/*                                            working directory to        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search for the directory name */
/*                                          in the directory structure    */
/*    _fx_utility_absolute_path_get         Get absolute path             */
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
UINT  _fx_directory_default_set(FX_MEDIA *media_ptr, CHAR *new_path_name)
{

UINT         status;
FX_DIR_ENTRY dir_entry;
CHAR        *path_string_ptr;
UINT         path_string_capacity;
FX_PATH     *path_ptr;
UINT         i;

#ifdef FX_ENABLE_EXFAT
CHAR         abs_str[FX_MAXIMUM_PATH]; /* TODO: possible will be better to use dynamic memory for exFAT only.  */
#endif /* FX_ENABLE_EXFAT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_default_sets++;
#endif

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Clear the long name string.  */
    dir_entry.fx_dir_entry_name[0] =  (CHAR)0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_DEFAULT_SET, media_ptr, new_path_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Setup the path string pointer to the start of the current path.  */
    path_string_ptr =  &(media_ptr -> fx_media_default_path.fx_path_string[0]);

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Get the absolute path.  */
        if (_fx_utility_absolute_path_get(path_string_ptr, new_path_name, abs_str) == FX_SUCCESS)
        {

            new_path_name = &(abs_str[0]);
        }
        else
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Invalid Path - Return the error code.  */
            return(FX_INVALID_PATH);
        }
    }
#endif /* FX_ENABLE_EXFAT */

    /* Look for a root directory selection.  */
    if ((!new_path_name) || (((new_path_name[0] == '\\') || (new_path_name[0] == '/')) && (new_path_name[1] == (CHAR)0)))
    {

        /* Set the media current working directory to the root.  */
        media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0] =  (CHAR) 0;
        media_ptr -> fx_media_default_path.fx_path_string[0] =                       (CHAR) 0;
        media_ptr -> fx_media_default_path.fx_path_string[FX_MAXIMUM_PATH - 2] =     (CHAR) 0;
    }
    else
    {

        /* Search the system for the supplied path and directory name.  */
        status =  _fx_directory_search(media_ptr, new_path_name, &dir_entry, FX_NULL, FX_NULL);

        /* Determine if the search failed or if the entry found is not a
           directory.  */
        if ((status != FX_SUCCESS) || (!(dir_entry.fx_dir_entry_attributes & FX_DIRECTORY)))
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Invalid Path - Return the error code.  */
            return(FX_INVALID_PATH);
        }

        /* Now update the current path string.  */

        /* Setup the path string's capacity.  */
        path_string_capacity =  FX_MAXIMUM_PATH - 1;

        /* Determine if the new path is relative from the current path.  */
        if ((new_path_name[0] != '\\') && (new_path_name[0] != '/'))
        {

            /* Yes, a relative path was found.  */

            /* First check for a local path pointer stored in the thread control block.
               This is only available in ThreadX Version 4 and above.  */
#ifndef FX_NO_LOCAL_PATH
            if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
            {

            CHAR *saved_name_ptr;

                /* First, save the name pointer of the default path.  */
                saved_name_ptr =  media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name;

                /* Setup the default path pointer to the local path.  */
                path_ptr =  (FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr;

                /* Copy the local path to the default path.  */
                media_ptr -> fx_media_default_path =  *path_ptr;

                /* Restore the name pointer of the default path.  */
                media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name =  saved_name_ptr;

                /* Set the path pointer to the default pointer.  */
                path_ptr =  &media_ptr -> fx_media_default_path;
            }
            else
            {

                /* Setup the default path to the global media path.  */
                path_ptr =  &media_ptr -> fx_media_default_path;
            }
#else

            /* Setup the default path to the global media path.  */
            path_ptr =  &media_ptr -> fx_media_default_path;
#endif

            /* First, check the current path for string overflow.  If this is set,
               don't attempt to update the current path with relative information.
               The path won't be valid again until a complete path is given.  */
            if (path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] == '*')
            {

                /* Yes, don't update the string, just finish the path set processing.  */

                /* Determine if we are at the root directory.  */
                if (!dir_entry.fx_dir_entry_cluster)
                {
                    /* Set the current directory back to the root directory.  */
                    path_ptr -> fx_path_directory.fx_dir_entry_name[0] =  (CHAR) 0;

                    /* Set name to NULL.  */
                    dir_entry.fx_dir_entry_name[0] =  (CHAR) 0;

                    /* Clear the current path string.  */
                    path_ptr -> fx_path_string[0] =  (CHAR)0;

                    /* Clear the overflow flag in the current path string... just in
                       case! */
                    path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] =  (CHAR)0;
                }

                /* Copy the new directory entry into the media control block.  */
                path_ptr -> fx_path_directory =  dir_entry;

                /* Copy the name into the path name buffer.  */
                for (i = 0; dir_entry.fx_dir_entry_name[i]; i++)
                {
                    path_ptr -> fx_path_name_buffer[i] =  dir_entry.fx_dir_entry_name[i];
                }

                /* Reassign the pointer.  */
                path_ptr -> fx_path_directory.fx_dir_entry_name =  path_ptr -> fx_path_name_buffer;

                /* Release media protection.  */
                FX_UNPROTECT

                /* Default directory set is complete, return status.  */
                return(FX_SUCCESS);
            }

            /* Move the current path starting pointer to the end of the current
               path string.  */
            while ((path_string_capacity) && (*path_string_ptr != FX_NULL))
            {
                path_string_ptr++;
                path_string_capacity--;
            }

            /* If room, place the \ character in the path string.  */
            if (path_string_capacity)
            {

                /* There is room, place the directory marker in the string.  */
                *path_string_ptr++ =  '\\';
                path_string_capacity--;
            }
        }
        else
        {

            /* Setup the default path pointer.  */

            /* First check for a local path pointer stored in the thread control block.
               This is only available in ThreadX Version 4 and above.  */
#ifndef FX_NO_LOCAL_PATH
            if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
            {

            CHAR *saved_name_ptr;

                /* First, save the name pointer of the default path.  */
                saved_name_ptr =  media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name;

                /* Setup the default path pointer to the local path.  */
                path_ptr =  (FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr;

                /* Copy the local path to the default path.  */
                media_ptr -> fx_media_default_path =  *path_ptr;

                /* Restore the name pointer of the default path.  */
                media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name =  saved_name_ptr;

                /* Set the path pointer to the default pointer.  */
                path_ptr =  &media_ptr -> fx_media_default_path;
            }
            else
            {

                /* Setup the default path to the global media path.  */
                path_ptr =  &media_ptr -> fx_media_default_path;
            }
#else

            /* Setup the default path to the global media path.  */
            path_ptr =  &media_ptr -> fx_media_default_path;
#endif

            /* Complete path name given.  Check to see if we need to clear an
               overflow character from a previous current path string update.  */
            if (path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] == '*')
            {
                path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] = (CHAR)0;
            }
        }

        /* Copy what we can into the current path.  */
        while (*new_path_name)
        {

            /* Determine if there is a ".." character sequence that specifies the
               previous path.  */
            if ((*new_path_name == '.') && (*(new_path_name + 1) == '.'))
            {

                /* Yes, a backward path is found.  The current path pointer
                   must be moved back to just after the previous \ character.  */

                /* Skip the current \0 that is at the end of the current path.  */
                path_string_capacity =  path_string_capacity + 2;
                path_string_ptr =       path_string_ptr - 2;

                while (path_string_capacity <= (FX_MAXIMUM_PATH - 1))
                {

                    /* Move the current path pointer backwards until
                       a \ character is found.  */

                    if ((*path_string_ptr == '\\') || (*path_string_ptr == '/'))
                    {

                        /* Yes, we have successfully backed up one directory.  */
                        break;
                    }

                    /* Backup another character.  */
                    path_string_capacity++;
                    path_string_ptr--;
                }

                /* Adjust the new directory pointer past the .. characters  */
                new_path_name =  new_path_name + 2;
            }
            else
            {

                /* Normal characters that need to be copied into the current path.  */

                if (path_string_capacity)
                {

                    /* Copy character from the new path into the current path string.  */
                    *path_string_ptr++ =  *new_path_name++;

                    path_string_capacity--;
                }
                else
                {

                    /* No more room in the current path string!  */
                    break;
                }
            }
        }

        /* Determine if there is still room in the current path string.  */
        if (path_string_capacity)
        {

            /* Yes, there is still room, place a NULL character at the
               end of the path.  */
            *path_string_ptr =  (CHAR)FX_NULL;
        }
        else
        {

            /* No more room.  Determine if the entire path was successfully
               copied into the current path.  */
            if (*new_path_name)
            {

                /* No, we couldn't fit the entire path.  Place a "*" character
                   at the end to indicate that we had overflow.  Note that
                   the path is kept just the the directory default get call, so
                   the new default path is valid.  */
                path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] =  '*';
            }
        }

        /* Determine if we are at the root directory.  */
#ifdef FX_ENABLE_EXFAT
        if ((!dir_entry.fx_dir_entry_cluster) && (media_ptr -> fx_media_FAT_type != FX_exFAT))
#else
        if (!dir_entry.fx_dir_entry_cluster)
#endif
        {

            /* Set the current directory back to the root directory.  */
            dir_entry.fx_dir_entry_name[0] =      (CHAR)0;
            path_ptr -> fx_path_name_buffer[0] =  (CHAR)0;
        }

        /* Copy the new directory entry into the media control block.  */
        path_ptr -> fx_path_directory =  dir_entry;

        /* Copy the name.  */
        for (i = 0; dir_entry.fx_dir_entry_name[i]; i++)
        {
            path_ptr -> fx_path_name_buffer[i] =  dir_entry.fx_dir_entry_name[i];
        }

        /* Reassign the pointer.  */
        path_ptr -> fx_path_directory.fx_dir_entry_name =  path_ptr -> fx_path_name_buffer;
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Default directory set is complete, return status.  */
    return(FX_SUCCESS);
}

