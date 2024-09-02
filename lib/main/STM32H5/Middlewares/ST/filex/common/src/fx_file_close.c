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
#include "fx_directory.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_close                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function closes the specified file.  If the file was written   */
/*    to this function will also write the directory entry (with the new  */
/*    size and time/date stamp) out to disk.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the directory entry     */
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
UINT  _fx_file_close(FX_FILE *file_ptr)
{

UINT      status;
FX_MEDIA *media_ptr;
FX_INT_SAVE_AREA


    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

    /* Setup a pointer to the associated media.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_closes++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_CLOSE, file_ptr, file_ptr -> fx_file_current_file_size, 0, 0, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT
    /* If trace is enabled, unregister this object.  */
    FX_TRACE_OBJECT_UNREGISTER(file_ptr)

    /* Remove this file from the opened list for the media.  */

    /* See if the file is the only one on the open list for this media.  */
    if (file_ptr == file_ptr -> fx_file_opened_next)
    {

        /* Only opened file, just set the opened list to NULL.  */
        media_ptr -> fx_media_opened_file_list =  FX_NULL;
    }
    else
    {

        /* Otherwise, not the only opened file, link-up the neighbors.  */
        (file_ptr -> fx_file_opened_next) -> fx_file_opened_previous =
            file_ptr -> fx_file_opened_previous;
        (file_ptr -> fx_file_opened_previous) -> fx_file_opened_next =
            file_ptr -> fx_file_opened_next;

        /* See if we have to update the opened list head pointer.  */
        if (media_ptr -> fx_media_opened_file_list == file_ptr)
        {

            /* Yes, move the head pointer to the next opened file. */
            media_ptr -> fx_media_opened_file_list =  file_ptr -> fx_file_opened_next;
        }
    }

    /* Decrement the opened file counter.  */
    media_ptr -> fx_media_opened_file_count--;

    /* Finally, Indicate that this file is closed.  */
    file_ptr -> fx_file_id =  FX_FILE_CLOSED_ID;

    /* Check to see if this file needs to have its directory entry written
       back to the media.  */
    if ((file_ptr -> fx_file_open_mode == FX_OPEN_FOR_WRITE) &&
        (file_ptr -> fx_file_modified))
    {

        /* Lockout interrupts for time/date access.  */
        FX_DISABLE_INTS

        /* Set the new time and date.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_time =  _fx_system_time;
        file_ptr -> fx_file_dir_entry.fx_dir_entry_date =  _fx_system_date;

        /* Set the last access date.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_last_accessed_date =  _fx_system_date;

        /* Restore interrupts.  */
        FX_RESTORE_INTS

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =
            file_ptr -> fx_file_current_file_size;

        /* Write the directory entry to the media.  */
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            status = _fx_directory_exFAT_entry_write(
                    media_ptr, &(file_ptr -> fx_file_dir_entry), UPDATE_STREAM);
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */
            status = _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        /* Check for a good status.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Error writing the directory.  */
            return(status);
        }
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return status to the caller.  */
    return(FX_SUCCESS);
}

