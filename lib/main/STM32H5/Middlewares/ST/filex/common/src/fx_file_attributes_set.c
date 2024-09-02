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
/*    _fx_file_attributes_set                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified file.  If found, */
/*    the attribute set request is valid and the directory entry will be  */
/*    modified with the new attributes.  Otherwise, if the file is not    */
/*    found, the appropriate error code is returned to the caller.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_name                             File name pointer             */
/*    attributes                            New file attributes           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
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
UINT  _fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes)
{

UINT         status;
ULONG        open_count;
FX_FILE     *search_ptr;
FX_DIR_ENTRY dir_entry;
UCHAR        not_a_file_attr;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_attributes_sets++;
#endif

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        not_a_file_attr = FX_DIRECTORY;
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        not_a_file_attr = FX_DIRECTORY | FX_VOLUME;
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_ATTRIBUTES_SET, media_ptr, file_name, attributes, 0, FX_TRACE_FILE_EVENTS, 0, 0)

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

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, file_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check to make sure the found entry is a file.  */
    if (dir_entry.fx_dir_entry_attributes & not_a_file_attr)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a file error code.  */
        return(FX_NOT_A_FILE);
    }

    /* Search the opened files to see if this file is currently
       opened.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    search_ptr =  media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Look at each opened file to see if the same file is opened.  */
        if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector == dir_entry.fx_dir_entry_log_sector) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset == dir_entry.fx_dir_entry_byte_offset))
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

    /* Place the new attributes in the directory entry.  */
    dir_entry.fx_dir_entry_attributes =  (UCHAR)attributes;

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Now write out the directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        status = _fx_directory_exFAT_entry_write(media_ptr, &dir_entry, UPDATE_FILE);
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

    /* File attribute set is complete, return status.  */
    return(status);
}

