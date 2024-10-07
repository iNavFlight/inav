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
/*    _fx_unicode_file_rename                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified file.  If found, */
/*    the rename request is valid and the directory entry will be changed */
/*    to the new file name.  Otherwise, if the file is not found, the     */
/*    appropriate error code is returned to the caller.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    old_unicode_name                      Pointer to old unicode name   */
/*    old_unicode_length                    Old unicode name length       */
/*    new_unicode_name                      Pointer to new unicode name   */
/*    new_unicode_length                    New unicode name length       */
/*    new_short_name                        Designated new short name     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_short_name_get            Get short name of unicode name*/
/*    _fx_directory_search                  Search directory              */
/*    _fx_unicode_directory_entry_change    Change unicode file name      */
/*    _fx_unicode_directory_search          Search for unicode name       */
/*    _fx_file_rename                       Rename for ASCII file name    */
/*    _fx_fault_tolerant_transaction_start  Start fault tolerant          */
/*                                            transaction                 */
/*    _fx_fault_tolerant_transaction_end    End fault tolerant transaction*/
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
UINT _fx_unicode_file_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                             UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name)
{
ULONG        i;
UINT         status;
CHAR         alpha, beta;
CHAR         old_shortname[13];
CHAR         new_shortname[13];
FX_DIR_ENTRY dir_entry;


    /* Clear the return short name.  */
    new_short_name[0] =  FX_NULL;

    /* Set shortname to null.  */
    old_shortname[0] =  FX_NULL;
    new_shortname[0] =  FX_NULL;

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

    /* Get shortname of old unicode name. */
    status = _fx_unicode_short_name_get(media_ptr, old_unicode_name, old_unicode_length, old_shortname);

    /* Determine if the result was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error code.  */
        return(status);
    }

    /* Protect media.  */
    FX_PROTECT

    /* Setup pointer to media name buffer.  */
    dir_entry.fx_dir_entry_name = media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;

    /* Clear the short name string.  */
    dir_entry.fx_dir_entry_short_name[0] =  FX_NULL;

    /* Search the target directory for the same file name.  */
    status = _fx_unicode_directory_search(media_ptr, &dir_entry, (UCHAR *)new_shortname, sizeof(new_shortname),
                                          new_unicode_name, &new_unicode_length, 0);

    /* Determine if the name already exists.  */
    if (status == FX_SUCCESS)
    {

        if (old_unicode_length == new_unicode_length)
        {

            /* Determine if the new name simply has an ASCII case change. If so, simply let the processing
               continue.  */
            i =  0;
            do
            {

                /* Pickup an old name and new name character and convert to upper case if necessary.  */
                alpha = (CHAR)old_unicode_name[i << 1];
                if ((alpha >= 'a') && (alpha <= 'z') && (old_unicode_name[(i << 1) + 1] == 0))
                {

                    /* Lower case, convert to upper case!  */
                    alpha =  (CHAR)((INT)alpha - 0x20);
                }
                beta =   (CHAR)new_unicode_name[i << 1];
                if ((beta >= 'a') && (beta <= 'z') && (new_unicode_name[(i << 1) + 1] == 0))
                {

                    /* Lower case, convert to upper case!  */
                    beta =  (CHAR)((INT)beta - 0x20);
                }

                /* Now compare the characters.  */
                if ((alpha != beta))
                {

                    /* Get out of this loop!  */
                    break;
                }

                /* Pickup the high bytes. */
                alpha = (CHAR)old_unicode_name[(i << 1) + 1];
                beta =  (CHAR)new_unicode_name[(i << 1) + 1];

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

            /* Names not match. */
            alpha = 0;
            beta = 1;
        }

        /* Now determine if the names match.  */
        if (alpha != beta)
        {

            /* No, the names do not match so simply return an error
               to the caller.  */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the not a file error code.  */
            return(FX_ALREADY_CREATED);
        }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Okay, at this point we need to rename to a new long file name that has enough space for the
       eventual unicode file name.  */

    /* Copy the characters from the unicode file name and make sure they are
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

        /* Create a new file with the temp long file name.  */
        status = _fx_file_rename(media_ptr, old_shortname, (CHAR *)_fx_unicode_temp_long_file_name);

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
    status =  _fx_unicode_directory_entry_change(media_ptr, &dir_entry,  new_unicode_name, new_unicode_length);

    /* Was this successful?  */
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

