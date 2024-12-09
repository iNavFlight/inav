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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_create                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified file.  If found, */
/*    the create request is invalid and an error is returned to the       */
/*    caller.  After the file name verification is made, a search for a   */
/*    free directory entry will be made.  If nothing is available, an     */
/*    error will be returned to the caller.  Otherwise, if all is okay, a */
/*    file of 0 bytes will be created.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_name                             File name                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_name_extract            Extract directory name        */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_directory_free_search             Search for a free directory   */
/*                                            entry                       */
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
UINT  _fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name)
{

FX_INT_SAVE_AREA

UINT         status;
CHAR        *name_ptr;
UINT         i;
CHAR        *work_ptr;
FX_DIR_ENTRY dir_entry;
FX_DIR_ENTRY search_directory;
#ifdef FX_ENABLE_EXFAT
ULONG64      dir_size;
#endif /* FX_ENABLE_EXFAT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_creates++;
#endif

    /* Determine if the supplied name is less than the maximum supported name size. The
       maximum name (FX_MAX_LONG_NAME_LEN) is defined in fx_api.h.  */
    i =  0;
    work_ptr =  (CHAR *)file_name;
    while (*work_ptr)
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
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Setup another pointer to another media name buffer.  */
    search_directory.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;

    /* Clear the short name strings.  */
    dir_entry.fx_dir_entry_short_name[0] =        0;
    search_directory.fx_dir_entry_short_name[0] = 0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_CREATE, media_ptr, file_name, 0, 0, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, file_name, &dir_entry, &search_directory, &name_ptr);

    /* Determine if the search was successful.  */
    if (status == FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* File found - Return the error code.  */
        return(FX_ALREADY_CREATED);
    }

    /* Determine if there is anything left after the name.  */
    if (_fx_directory_name_extract(name_ptr, &dir_entry.fx_dir_entry_name[0]))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Extra information after the file name, return an invalid path
           error.  */
        return(FX_INVALID_PATH);
    }

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        if (((dir_entry.fx_dir_entry_name[0] == '.') && (dir_entry.fx_dir_entry_name[1] == 0)) ||
            ((dir_entry.fx_dir_entry_name[0] == '.') && (dir_entry.fx_dir_entry_name[1] == '.') && (dir_entry.fx_dir_entry_name[2] == 0)))
        {
#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* We don't need '.' or '..' for exFAT */
            return(FX_ALREADY_CREATED);
        }
    }

    /* Save the directory entry size.  */
    dir_size = search_directory.fx_dir_entry_file_size;
#endif /* FX_ENABLE_EXFAT */

    /* Find a free slot for the new file.  */
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

    /* Populate the directory entry.  */

    /* Isolate the file name.  */
    _fx_directory_name_extract(name_ptr, &dir_entry.fx_dir_entry_name[0]);

    /* Disable interrupts for time/date access.  */
    FX_DISABLE_INTS

    /* Set time and date stamps.  */
    dir_entry.fx_dir_entry_time =  _fx_system_time;
    dir_entry.fx_dir_entry_date =  _fx_system_date;

    /* Restore interrupts.  */
    FX_RESTORE_INTS

    /* Set the attributes for the file.  */
    dir_entry.fx_dir_entry_attributes =  FX_ARCHIVE;

    /* Set file size to 0. */
    dir_entry.fx_dir_entry_file_size =  0;

#ifdef FX_ENABLE_EXFAT
    /* Set available file size to 0. */
    dir_entry.fx_dir_entry_available_file_size = 0;
#endif /* FX_ENABLE_EXFAT */

    /* Set the cluster to NULL.  */
    dir_entry.fx_dir_entry_cluster =    FX_NULL;

    /* Is there a leading dot?  */
    if (dir_entry.fx_dir_entry_name[0] == '.')
    {

        /* Yes, toggle the hidden attribute bit.  */
        dir_entry.fx_dir_entry_attributes |=  FX_HIDDEN;
    }

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Don't use FAT by default.  */
        dir_entry.fx_dir_entry_dont_use_fat = (CHAR)(((search_directory.fx_dir_entry_dont_use_fat & 1) << 1) | 1);

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
        dir_entry.fx_dir_entry_dont_use_fat = 0;
    }
#endif /* FX_ENABLE_EXFAT */

    /* Now write out the directory entry.  */
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

    /* File create is complete, return status.  */
    return(status);
}

