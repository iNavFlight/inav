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
#include "fx_directory.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_long_name_get_extended                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the long file name via the supplied short file   */
/*    name. If there is no long file name, the short file name will be    */
/*    returned.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    short_file_name                       Pointer to short (8.3) name   */
/*    long_file_name                        Pointer to long (max 255) name*/
/*    long_file_name_buffer_length          Buffer length for long name   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search for the file name in   */
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
UINT  _fx_directory_long_name_get_extended(FX_MEDIA *media_ptr, CHAR *short_file_name, CHAR *long_file_name, UINT long_file_name_buffer_length)
{

UINT         status;
UINT         i;
FX_DIR_ENTRY dir_entry;


    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_LONG_NAME_GET, media_ptr, short_file_name, long_file_name, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied short directory name.  */
    status =  _fx_directory_search(media_ptr, short_file_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Copy the long name portion into the destination string.  */
    i =  0;
    while ((i < (FX_MAX_LONG_NAME_LEN - 1)) && (dir_entry.fx_dir_entry_name[i]) && (i < (long_file_name_buffer_length - 1)))
    {

        /* Copy a character of the long name into the destination name.  */
        long_file_name[i] =  dir_entry.fx_dir_entry_name[i];

        /* Move to next character.  */
        i++;
    }

    /* Ensure the long file name is NULL terminated.  */
    long_file_name[i] =  FX_NULL;

    /* Check if the buffer is too short for the name.  */
    if ((i == (long_file_name_buffer_length - 1)) && (dir_entry.fx_dir_entry_name[i]))
    {

        /* Buffer too short, return error.  */
        status = FX_BUFFER_ERROR;
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return the completion status.  */
    return(status);
}

