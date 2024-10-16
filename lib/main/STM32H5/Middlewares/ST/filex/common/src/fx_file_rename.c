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
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"
#endif /* FX_ENABLE_EXFAT */
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_rename                                     PORTABLE C      */
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
/*    media_ptr                             Media control block pointer   */
/*    old_file_name                         Old file name pointer         */
/*    new_file_name                         New file name pointer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_free_search             Search for a free directory   */
/*                                            entry in target directory   */
/*    _fx_directory_name_extract            Extract the new filename      */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_fault_tolerant_transaction_start  Start fault tolerant          */
/*                                            transaction                 */
/*    _fx_fault_tolerant_transaction_end    End fault tolerant transaction*/
/*    _fx_fault_tolerant_recover            Recover FAT chain             */
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
UINT  _fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name)
{

ULONG        i;
CHAR        *work_ptr;
CHAR         alpha, beta;
UINT         status;

#ifndef FX_DONT_UPDATE_OPEN_FILES
ULONG        open_count;
FX_FILE     *search_ptr;
#endif
CHAR        *new_name_ptr;
FX_DIR_ENTRY old_dir_entry, new_dir_entry;
FX_DIR_ENTRY search_directory;
#ifdef FX_RENAME_PATH_INHERIT
UINT         j;
#endif
UCHAR        not_a_file_attr;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_renames++;
#endif

    /* Setup pointers to media name buffers.  */
    old_dir_entry.fx_dir_entry_name =     media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    new_dir_entry.fx_dir_entry_name =     media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;
    search_directory.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 3;

    /* Clear the short name strings.  */
    old_dir_entry.fx_dir_entry_short_name[0] =     0;
    new_dir_entry.fx_dir_entry_short_name[0] =     0;
    search_directory.fx_dir_entry_short_name[0] =  0;

    /* Determine if the supplied name is less than the maximum supported name size. The
       maximum name (FX_MAX_LONG_NAME_LEN) is defined in fx_api.h.  */
    i =  0;
    work_ptr =  (CHAR *)new_file_name;
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

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_RENAME, media_ptr, old_file_name, new_file_name, 0, FX_TRACE_FILE_EVENTS, 0, 0)

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
    status =  _fx_directory_search(media_ptr, old_file_name, &old_dir_entry, &search_directory, FX_NULL);

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

    /* Check to make sure the found entry is a file.  */
    if (old_dir_entry.fx_dir_entry_attributes & not_a_file_attr)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a file error code.  */
        return(FX_NOT_A_FILE);
    }

#ifdef FX_RENAME_PATH_INHERIT

    /* Determine if the source file name has a path and the target file name does not.  */
    if (((old_file_name[0] == '/') || (old_file_name[0] == '\\')) && (new_file_name[0] != '/') && (new_file_name[0] != '\\'))
    {

        /* In this case, we need to prepend the path of the old file name to that of the new file name.  */

        /* Setup pointer to the rename buffer.  */
        work_ptr =  (CHAR *)media_ptr -> fx_media_rename_buffer;

        /* First, copy the path of the old file name.  */
        i =  0;
        j =  0;
        while ((old_file_name[i]) && (i < FX_MAXIMUM_PATH))
        {

            /* Copy a character into the rename buffer.  */
            *work_ptr++ =  old_file_name[i];

            /* Determine if this character is directory separator.  */
            if ((old_file_name[i] == '/') || (old_file_name[i] == '\\'))
            {

                /* Yes, directory separator has been found - remember the index.  */
                j =  i;
            }

            /* Move to next position in the old file name.  */
            i++;
        }

        /* At this point, we have the path stored in the rename buffer.  */

        /* Position past the last slash or backslash.  */
        j++;

        /* Reset the working pointer to the position after the last directory separator.  */
        work_ptr =  (CHAR *)&(media_ptr -> fx_media_rename_buffer[j]);

        /* Now copy the new file name into the rename buffer.  */
        i =  0;
        while ((new_file_name[i]) && (j < FX_MAXIMUM_PATH))
        {

            /* Copy a character into the rename buffer.  */
            *work_ptr++ =  new_file_name[i];

            /* Move to next character.  */
            i++;
            j++;
        }

        /* Determine if the path was successfully prepended.  */
        if (new_file_name[i])
        {

            /* No, there was not enough room in the destination buffer.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the invalid path error code.  */
            return(FX_INVALID_PATH);
        }

        /* Place a NULL at the end of the string.  */
        *work_ptr =  (CHAR)FX_NULL;

        /* At this point, we have successfully prepended the path in the new file name, override
           the new file name so it is used from now on.  */
        new_file_name =  (CHAR *)media_ptr -> fx_media_rename_buffer;
    }
#endif

    /* Search the target directory for the same file name.  */
    status = _fx_directory_search(media_ptr, new_file_name, &new_dir_entry, &search_directory, &new_name_ptr);

    /* Determine if the name already exists.  */
    if (status == FX_SUCCESS)
    {

        /* Determine if the new name simply has an ASCII case change. If so, simply let the processing
           continue.  */
        i =  0;
        do
        {

            /* Pickup an old name and new name character and convert to upper case if necessary.  */
            alpha =  old_file_name[i];
            if ((alpha >= 'a') && (alpha <= 'z'))
            {

                /* Lower case, convert to upper case!  */
                alpha =  (CHAR)((INT)alpha - 0x20);
            }
            beta =   new_file_name[i];
            if ((beta >= 'a') && (beta <= 'z'))
            {

                /* Lower case, convert to upper case!  */
                beta = (CHAR)((INT)beta - 0x20);
            }

            /* Now compare the characters.  */
            if ((alpha != beta) || (alpha == 0))
            {

                /* Get out of this loop!  */
                break;
            }

            /* Move to next character.  */
            i++;
        } while (i < (FX_MAXIMUM_PATH-1));

        /* Now determine if the names match.  */
        if (alpha != beta)
        {

            /* No, the names do not match so simply return an error
               to the caller.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the not a file error code.  */
            return(FX_ALREADY_CREATED);
        }
    }

    /* Change the file name and look for extra stuff at the end.  */
    if (_fx_directory_name_extract(new_name_ptr, &new_dir_entry.fx_dir_entry_name[0]))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Invalid name, return error status.  */
        return(FX_INVALID_NAME);
    }

    /* Search for a free spot in the target directory.  */
    status = _fx_directory_free_search(media_ptr, &search_directory, &new_dir_entry);

    /* Determine if a free spot was found.  */
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

    /* Extract the new file name.  */
    _fx_directory_name_extract(new_name_ptr, &new_dir_entry.fx_dir_entry_name[0]);

    /* Determine if a long name is present.  */
    if (new_dir_entry.fx_dir_entry_long_name_present)
    {

        /* Yes, clear the short file name.  */
        new_dir_entry.fx_dir_entry_short_name[0] =  0;
    }

    /* Save the updated directory parameters.  */
    new_dir_entry.fx_dir_entry_attributes =          old_dir_entry.fx_dir_entry_attributes;
    new_dir_entry.fx_dir_entry_cluster    =          old_dir_entry.fx_dir_entry_cluster;
    new_dir_entry.fx_dir_entry_file_size  =          old_dir_entry.fx_dir_entry_file_size;

    /* Save the reserved field.  */
    new_dir_entry.fx_dir_entry_reserved =            old_dir_entry.fx_dir_entry_reserved;

    /* Set time and date stamps.  */
    new_dir_entry.fx_dir_entry_created_time_ms =     old_dir_entry.fx_dir_entry_created_time_ms;
    new_dir_entry.fx_dir_entry_created_time =        old_dir_entry.fx_dir_entry_created_time;
    new_dir_entry.fx_dir_entry_created_date =        old_dir_entry.fx_dir_entry_created_date;
    new_dir_entry.fx_dir_entry_last_accessed_date =  old_dir_entry.fx_dir_entry_last_accessed_date;
    new_dir_entry.fx_dir_entry_time =                old_dir_entry.fx_dir_entry_time;
    new_dir_entry.fx_dir_entry_date =                old_dir_entry.fx_dir_entry_date;

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        new_dir_entry.fx_dir_entry_dont_use_fat =              old_dir_entry.fx_dir_entry_dont_use_fat;
        new_dir_entry.fx_dir_entry_type =                  old_dir_entry.fx_dir_entry_type;
        new_dir_entry.fx_dir_entry_available_file_size =   old_dir_entry.fx_dir_entry_available_file_size;
        new_dir_entry.fx_dir_entry_secondary_count =       old_dir_entry.fx_dir_entry_secondary_count;
    }
#endif /* FX_ENABLE_EXFAT */

    /* Is there a leading dot?  */
    if (new_dir_entry.fx_dir_entry_name[0] == '.')
    {

        /* Yes, toggle the hidden attribute bit.  */
        new_dir_entry.fx_dir_entry_attributes |=  FX_HIDDEN;
    }

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Invalidate the directory cache.  */
    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;
#endif

    /* Now write out the directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        status = _fx_directory_exFAT_entry_write(media_ptr, &new_dir_entry, UPDATE_FULL);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        status =  _fx_directory_entry_write(media_ptr, &new_dir_entry);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Determine if the write was successful.  */
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

#ifndef FX_DONT_UPDATE_OPEN_FILES

    /* Search the opened files to update any currently opened files.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    search_ptr =  media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Look at each opened file to see if it matches the file being renamed.  */
        if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector ==
             old_dir_entry.fx_dir_entry_log_sector) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset ==
             old_dir_entry.fx_dir_entry_byte_offset))
        {

            /* Yes, the file being renamed is already open.  Update the file's
               information so that it is kept current.  */
            search_ptr -> fx_file_dir_entry.fx_dir_entry_cluster =      new_dir_entry.fx_dir_entry_cluster;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =    new_dir_entry.fx_dir_entry_file_size;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector =   new_dir_entry.fx_dir_entry_log_sector;
            search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset =  new_dir_entry.fx_dir_entry_byte_offset;

            /* Copy the new name into the file's name buffer.  */
            i =  0;
            while (i < (FX_MAX_LONG_NAME_LEN - 1))
            {

                /* Copy byte of the new name.  */
                search_ptr -> fx_file_dir_entry.fx_dir_entry_name[i] =  new_dir_entry.fx_dir_entry_name[i];

                /* Move to the next character.  */
                i++;

                /* Determine if we are at the end of the name.  */
                if (new_dir_entry.fx_dir_entry_name[i] == FX_NULL)
                {
                
                    /* Determine if we are not at the maximum name size.  */
                    if (i < (FX_MAX_LONG_NAME_LEN - 1))
                    {
                    
                        /* Get out of the loop.   */
                        break;
                    }
                }
            }

            /* Set the NULL termination in the copy of the new name.  */
            search_ptr -> fx_file_dir_entry.fx_dir_entry_name[i] =  FX_NULL;
        }

        /* Adjust the pointer and decrement the search count.  */
        search_ptr =  search_ptr -> fx_file_opened_next;
        open_count--;
    }
#endif

    /* Now we are ready to remove the old directory entry.  */
    old_dir_entry.fx_dir_entry_name[0] =        (CHAR)FX_DIR_ENTRY_FREE;
    old_dir_entry.fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;

    /* Now wipe out the old directory entry.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        status = _fx_directory_exFAT_entry_write(media_ptr, &old_dir_entry, UPDATE_DELETE);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */
        status =  _fx_directory_entry_write(media_ptr, &old_dir_entry);
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

    /* File rename is complete, return status.  */
    return(status);
}

