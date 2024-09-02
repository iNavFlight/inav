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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"
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
/*    _fx_unicode_file_create                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the specified unicode name.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_unicode_name                   Pointer to source unicode name*/
/*    source_unicode_length                 Unicode name length           */
/*    short_name                            Designated short name         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search directory              */
/*    _fx_unicode_directory_entry_change    Change unicode file name      */
/*    _fx_unicode_directory_search          Search for unicode name       */
/*    _fx_file_create                       Create a file                 */
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
UINT  _fx_unicode_file_create(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length, CHAR *short_name)
{

FX_DIR_ENTRY dir_entry;
UINT         i, status;
ULONG        temp_length;
UCHAR        destination_shortname[13];


    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  FX_NULL;

    /* Set destination shortname to null.  */
    destination_shortname[0] =  FX_NULL;

    /* Clear the return short name.  */
    short_name[0] =  FX_NULL;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

#ifdef FX_ENABLE_EXFAT
    /* Check if media format is exFAT.  */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Return the not implemented error.  */
        return(FX_NOT_IMPLEMENTED);
    }
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_UNICODE_FILE_CREATE, media_ptr, source_unicode_name, source_unicode_length, short_name, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect media.  */
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

    /* Setup temporary length.  */
    temp_length =  source_unicode_length;

    /* Determine if the destination file is already present.  */
    status =  _fx_unicode_directory_search(media_ptr, &dir_entry, destination_shortname, sizeof(destination_shortname), source_unicode_name, &temp_length, 0);

    /* Determine if the search was successful.  */
    if (status == FX_SUCCESS)
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(FX_ALREADY_CREATED);
    }

    /* Okay, at this point we need to create a new long file name that has enough space for the
       eventual unicode file name.  */

    /* Copy the characters from the unicode file name and make sure they are
       within the ASCII range.  */
    _fx_unicode_temp_long_file_name[0] =  'z';
    for (i = 1; i < source_unicode_length; i++)
    {

        /* Build temporary long file name.  */
        _fx_unicode_temp_long_file_name[i] =  (UCHAR)((UINT)'0' + (i % 9));
    }
    _fx_unicode_temp_long_file_name[i] =  FX_NULL;

    /* Loop to try different temp long file names... if necessary.  */
    do
    {

        /* Create a new file with the temp long file name.  */
        status =  _fx_file_create(media_ptr, (CHAR *)_fx_unicode_temp_long_file_name);

        /* Determine if there was an error.  */
        if (status == FX_ALREADY_CREATED)
        {

            /* Adjust the name slightly and try again!  */
            _fx_unicode_temp_long_file_name[0]--;

            /* Determine if it is outside the lower case boundary.  */
            if (_fx_unicode_temp_long_file_name[0] < 0x61)
            {
                break;
            }
        }
    } while (status == FX_ALREADY_CREATED);

    /* Determine if there was an error.  */
    if (status)
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return error.  */
        return(status);
    }

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  0;

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, (CHAR *)_fx_unicode_temp_long_file_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error status.  */
        return(status);
    }

    /* We can now change the temporary long file name with the destination unicode name.  */
    status =  _fx_unicode_directory_entry_change(media_ptr, &dir_entry,  source_unicode_name, source_unicode_length);

    /* Was this successful?  */
    if (status == FX_SUCCESS)
    {

        /* Yes, copy the short file name to the destination.  */
        /* The new short name only have 8 characters, since we didn't include a dot in temp_long_file_name. */
        for (i = 0; i < FX_DIR_NAME_SIZE; i++)
        {

            /* Copy a character.  */
            short_name[i] =  dir_entry.fx_dir_entry_short_name[i];

            /* Are we done?  */
            if (short_name[i] == FX_NULL)
            {
                break;
            }
        }
    }

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

    /* Release the protection.  */
    FX_UNPROTECT

    /* Return completion status.  */
    return(status);
}

