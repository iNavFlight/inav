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

#ifndef FX_NO_LOCAL_PATH
FX_LOCAL_PATH_SETUP
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_local_path_set                        PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the local default directory of the media to the  */
/*    path specified by the caller.  If this path is not found, an error  */
/*    code is returned.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    local_path                            Local path control block ptr  */
/*    new_path_name                         New path to set current local */
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
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name)
{

#ifndef FX_NO_LOCAL_PATH
UINT         status;
CHAR        *path_string_ptr;
UINT         path_string_capacity;
FX_PATH     *path_ptr;
UINT         i, j;
#endif
FX_DIR_ENTRY dir_entry;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_local_path_sets++;
#endif

    /* Setup pointer to a name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Setup the local path name buffer pointer.  */
    local_path_ptr -> fx_path_directory.fx_dir_entry_name =  local_path_ptr -> fx_path_name_buffer;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  (CHAR)0;

    /* Clear the long name string.  */
    dir_entry.fx_dir_entry_name[0] =  (CHAR)0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

#ifdef FX_NO_LOCAL_PATH

    FX_PARAMETER_NOT_USED(new_path_name);

    /* Error, return to caller.  */
    return(FX_NOT_IMPLEMENTED);
#else

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_LOCAL_PATH_SET, media_ptr, local_path_ptr, new_path_name, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Look for a root directory selection.  */
    if ((!new_path_name) || (((new_path_name[0] == '\\') || (new_path_name[0] == '/')) && (new_path_name[1] == (CHAR)0)))
    {

        /* Set the default local directory to the root.  */
        local_path_ptr -> fx_path_directory.fx_dir_entry_name[0] =  (CHAR)0;
        local_path_ptr -> fx_path_string[0] =                       (CHAR)0;
        local_path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] =       (CHAR)0;

        /* Setup thread control block to use this local path pointer.  */
        _tx_thread_current_ptr -> tx_thread_filex_ptr =  (VOID *)local_path_ptr;
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

        /* Setup the path string pointer to the start of the current path.  */
        path_string_ptr =  &(local_path_ptr -> fx_path_string[0]);

        /* Setup the path string's capacity.  */
        path_string_capacity =  FX_MAXIMUM_PATH - 1;

        /* Determine if the new path is relative from the current path.  */
        if ((new_path_name[0] != '\\') && (new_path_name[0] != '/'))
        {

            /* Yes, a relative path was found.  */

            /* Setup the default path pointer to the local path.  */
            path_ptr =  local_path_ptr;

            /* Determine if the local path is different than the current local path.  */
            if (local_path_ptr !=  (FX_LOCAL_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr)
            {

                /* Yes, there is a difference.  */

                /* Should we copy from the default media path or from the previous default
                   path.  */
                if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
                {

                    /* There is a local path so copy the relative path info from it.  */
                    i =  0;
                    do
                    {

                        /* Copy from previous local to new local path.  */
                        local_path_ptr -> fx_path_string[i] =
                            ((FX_LOCAL_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_string[i];
                            
                        /* Determine if we are done.  */
                        if (local_path_ptr -> fx_path_string[i] == 0)
                        {
                        
                            /* Are we not at the end of the string?  */
                            if (i < (FX_MAXIMUM_PATH - 1))
                            {
                            
                                /* Yes, break the loop.  */
                                break;
                            }
                        }
                        
                        /* Move to the next character.  */
                        i++;
                        
                    } while (i < FX_MAXIMUM_PATH);
                }
                else
                {

                    /* No local path, so copy the relative path information from the media
                       default.  */
                    i =  0;
                    do
                    {

                        /* Copy from the media default to new local path.  */
                        local_path_ptr -> fx_path_string[i] =
                            media_ptr -> fx_media_default_path.fx_path_string[i];
                            
                        /* Determine if we are done.  */
                        if (local_path_ptr -> fx_path_string[i] == 0)
                        {
                        
                            /* Are we not at the end of the string?  */
                            if (i < (FX_MAXIMUM_PATH - 1))
                            {
                            
                                /* Yes, break the loop.  */
                                break;
                            }
                        }

                        /* Move to the next character.  */
                        i++;

                    } while (i < FX_MAXIMUM_PATH);
                }
            }

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
                    dir_entry.fx_dir_entry_name[0] =  (CHAR)0;

                    /* Clear the current path string.  */
                    path_ptr -> fx_path_string[0] =  (CHAR)0;

                    /* Clear the overflow flag in the current path string... just in
                       case! */
                    path_ptr -> fx_path_string[FX_MAXIMUM_PATH - 2] =  (CHAR)0;
                }

                /* Copy the new directory entry into the media control block.  */
                path_ptr -> fx_path_directory =  dir_entry;

                /* Reset the local path name buffer pointer, since it was clobbered earlier.  */
                local_path_ptr -> fx_path_directory.fx_dir_entry_name =  local_path_ptr -> fx_path_name_buffer;

                /* Copy the directory name entry to the local path name area.  */
                for (j = 0; j < FX_MAX_LONG_NAME_LEN; j++)
                {

                    /* Copy the name buffer.  */
                    local_path_ptr -> fx_path_directory.fx_dir_entry_name[j] =  dir_entry.fx_dir_entry_name[j];
                }

                /* Setup thread control block to use this local path pointer.  */
                _tx_thread_current_ptr -> tx_thread_filex_ptr =  (VOID *)local_path_ptr;

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

            /* Setup the default path pointer to the local path.  */
            path_ptr =  local_path_ptr;

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
        if (!dir_entry.fx_dir_entry_cluster)
        {
            /* Set the current directory back to the root directory.  */
            dir_entry.fx_dir_entry_name[0] =            (CHAR)0;
            local_path_ptr -> fx_path_name_buffer[0] =  (CHAR)0;
        }

        /* Copy the new directory entry into the media control block.  */
        path_ptr -> fx_path_directory =  dir_entry;

        /* Reset the local path name buffer pointer, since it was clobbered earlier.  */
        local_path_ptr -> fx_path_directory.fx_dir_entry_name =  local_path_ptr -> fx_path_name_buffer;

        /* Copy the directory name entry to the local path name area.  */
        for (j = 0; j < FX_MAX_LONG_NAME_LEN; j++)
        {

            /* Copy the name buffer.  */
            local_path_ptr -> fx_path_directory.fx_dir_entry_name[j] =  dir_entry.fx_dir_entry_name[j];
        }

        /* Setup thread control block to use this local path pointer.  */
        _tx_thread_current_ptr -> tx_thread_filex_ptr =  (VOID *)local_path_ptr;
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Default directory set is complete, return status.  */
    return(FX_SUCCESS);
#endif
}

