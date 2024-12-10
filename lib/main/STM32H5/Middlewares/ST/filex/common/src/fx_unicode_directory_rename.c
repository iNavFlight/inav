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
#include "fx_utility.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_directory_rename                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the specified unicode directory.              */
/*    This function first attempts to find the specified directory.       */
/*    If found, the unicode rename request is valid and the directory     */
/*    will be changed to the new unicode name. Otherwise, if the directory*/
/*    is not found, the appropriate error code is returned to the caller. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    old_unicode_name                      Pointer to old unicode name   */
/*    old_unicode_length                    Old unicode name length       */
/*    new_unicode_name                      Pointer to new unicode name   */
/*    new_unicode_length                    Old unicode name length       */
/*    new_short_name                        Pointer to new short name     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_short_name_get            Get the directory short name  */
/*    _fx_unicode_directory_search          Search for unicode name       */
/*    _fx_unicode_directory_entry_change    Change unicode file name      */
/*    _fx_directory_rename                  Rename the directory          */
/*    _fx_directory_search                  Search directory              */
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
UINT  _fx_unicode_directory_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                                   UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name)
{

FX_DIR_ENTRY dir_entry;
UINT         i, status;
ULONG        temp_length;
CHAR         old_dir_short_name[13];
CHAR         new_dir_short_name[13];
UCHAR        alpha, beta;


    /* Clear the return short name.  */
    new_short_name[0] =  FX_NULL;

    /* Set shortname to null.  */
    old_dir_short_name[0] =  FX_NULL;
    new_dir_short_name[0] =  FX_NULL;

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

    /* Get the old short name.  */
    status = _fx_unicode_short_name_get(media_ptr, old_unicode_name, old_unicode_length, &old_dir_short_name[0]);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error code.  */
        return(status);
    }

    /* Protect media.  */
    FX_PROTECT

    /* Setup temporary length.  */
    temp_length =  new_unicode_length;

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  FX_NULL;

    /* Determine if the new directory name is already present.  */
    status =  _fx_unicode_directory_search(media_ptr, &dir_entry, (UCHAR *)new_dir_short_name, sizeof(new_dir_short_name), new_unicode_name, &temp_length, 0);

    /* Determine if the search was successful.  */
    if (status == FX_SUCCESS)
    {

        /* Determine if the name length are equal.  */
        if (old_unicode_length == new_unicode_length)
        {

            /* Determine if the new name simply has a UNICODE case change. If so, simply let the processing
               continue.  */
            i =  0;
            do
            {

                /* Pickup an old name and new name character and convert to upper case if necessary.  */
                alpha =  old_unicode_name[i * 2];
                if ((alpha >= 'a') && (alpha <= 'z') && (old_unicode_name[i * 2 + 1] == 0))
                {

                    /* Lower case, convert to upper case!  */
                    alpha =  (UCHAR)((INT)alpha - 0x20);
                }
                beta =   new_unicode_name[i * 2];
                if ((beta >= 'a') && (beta <= 'z') && (new_unicode_name[i * 2 + 1] == 0))
                {

                    /* Lower case, convert to upper case!  */
                    beta =  (UCHAR)((INT)beta - 0x20);
                }

                /* Now compare the characters.  */
                if (alpha != beta)
                {

                    /* Get out of this loop!  */
                    break;
                }

                /* Pickup the high bytes. */
                alpha = old_unicode_name[i * 2 + 1];
                beta =   new_unicode_name[i * 2 + 1];

                /* Now compare the byte.  */
                if ((alpha != beta))
                {

                    /* Get out of this loop!  */
                    break;
                }

                /* Move to next character.  */
                i++;
            } while (i < old_unicode_length);
        }
        else
        {
            alpha = 0;
            beta = 1;
        }

        /* Now determine if the names match.  */
        if (alpha != beta)
        {

            /* Yes, the directory name already exists in the target location.  */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error code.  */
            return(FX_ALREADY_CREATED);
         }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Okay, at this point we need to create a new long directory name that has enough space for the
       eventual unicode directory name.  */

    /* Copy the characters from the unicode directory name and make sure they are
       within the ASCII range.  */
    _fx_unicode_temp_long_file_name[0] =  'z';
    for (i = 1; i < new_unicode_length; i++)
    {

        /* Build temporary long file name.  */
        _fx_unicode_temp_long_file_name[i] =  (UCHAR)((UINT)'0' + (i % 9));
    }
    _fx_unicode_temp_long_file_name[i] =  FX_NULL;

    /* Loop to try different temp long file names... if necessary.  */
    do
    {

        /* Rename the directory name.  */
        status =  _fx_directory_rename(media_ptr, old_dir_short_name, (CHAR *)_fx_unicode_temp_long_file_name);

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
    dir_entry.fx_dir_entry_short_name[0] =  FX_NULL;

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
    status = _fx_unicode_directory_entry_change(media_ptr, &dir_entry, new_unicode_name, new_unicode_length);

    /* Determine if the rename was successful.  */
    if (status == FX_SUCCESS)
    {

        /* Yes, copy the short file name to the destination.  */
        /* The new short name only have 8 characters, since we didn't include a dot in temp_long_file_name. */
        for (i = 0; i < FX_DIR_NAME_SIZE; i++)
        {

            /* Copy a character.  */
            new_short_name[i] =  dir_entry.fx_dir_entry_short_name[i];

            /* Are we done?  */
            if (new_short_name[i] == FX_NULL)
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

