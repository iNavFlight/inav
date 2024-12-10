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
#include "fx_directory.h"
#include "fx_file.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_information_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified directory name.  */
/*    If found, the complete file parameters are placed in all non-NULL   */
/*    return parameters.  If the file name is not found, the appropriate  */
/*    error code is returned to the caller.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Directory name pointer        */
/*    attributes                            Pointer to attributes         */
/*    size                                  Pointer to size               */
/*    year                                  Pointer to year               */
/*    month                                 Pointer to month              */
/*    day                                   Pointer to day                */
/*    hour                                  Pointer to hour               */
/*    minute                                Pointer to minute             */
/*    second                                Pointer to second             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), verified */
/*                                            memcpy usage,               */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name,
                                    UINT *attributes, ULONG *size,
                                    UINT *year, UINT *month, UINT *day,
                                    UINT *hour, UINT *minute, UINT *second)
{

UINT         status;
FX_DIR_ENTRY dir_entry;
ULONG        open_count;
FX_FILE     *search_ptr;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_information_gets++;
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

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_INFORMATION_GET, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied directory name.  */
    status =  _fx_directory_search(media_ptr, directory_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check the list of open files for others open for writing.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    search_ptr =  media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Look at each opened file to see if the same file is opened
           for writing.  */
        if ((search_ptr -> fx_file_dir_entry.fx_dir_entry_log_sector == dir_entry.fx_dir_entry_log_sector) &&
            (search_ptr -> fx_file_dir_entry.fx_dir_entry_byte_offset == dir_entry.fx_dir_entry_byte_offset) &&
            (search_ptr -> fx_file_open_mode))
        {

            /* The file has been opened for writing by a previous call. Use the information used by
               the writer instead of what is currently on the media.  */
            _fx_utility_memory_copy((UCHAR *)&search_ptr -> fx_file_dir_entry, (UCHAR *)&dir_entry, sizeof(FX_DIR_ENTRY)); /* Use case of memcpy is verified. */
            break;
        }

        /* Adjust the pointer and decrement the search count.  */
        search_ptr =  search_ptr -> fx_file_opened_next;
        open_count--;
    }

    /* Check to see if attributes are required.  */
    if (attributes)
    {

        /* Pickup the attributes.  */
        *attributes =  dir_entry.fx_dir_entry_attributes;
    }

    /* Check to see if the size is required.  */
    if (size)
    {

        /* Pickup the size.  */
        *size =  (ULONG)dir_entry.fx_dir_entry_file_size;
    }

    /* Check to see if the year is required.  */
    if (year)
    {

        /* Pickup the year.  */
        *year =  ((dir_entry.fx_dir_entry_date >> FX_YEAR_SHIFT) & FX_YEAR_MASK) +
                                                                        FX_BASE_YEAR;
    }

    /* Check to see if the month is required.  */
    if (month)
    {

        /* Pickup the month.  */
        *month =  (dir_entry.fx_dir_entry_date >> FX_MONTH_SHIFT) & FX_MONTH_MASK;
    }

    /* Check to see if the day is required.  */
    if (day)
    {

        /* Pickup the day.  */
        *day =  dir_entry.fx_dir_entry_date & FX_DAY_MASK;
    }

    /* Check to see if the hour is required.  */
    if (hour)
    {

        /* Pickup the hour.  */
        *hour =  (dir_entry.fx_dir_entry_time >> FX_HOUR_SHIFT) & FX_HOUR_MASK;
    }

    /* Check to see if the minute is required.  */
    if (minute)
    {

        /* Pickup the minute.  */
        *minute =  (dir_entry.fx_dir_entry_time >> FX_MINUTE_SHIFT) & FX_MINUTE_MASK;
    }

    /* Check to see if the second is required.  */
    if (second)
    {

        /* Pickup the second.  */
        *second =  (dir_entry.fx_dir_entry_time & FX_SECOND_MASK) * 2;
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Directory information get is complete, return successful status.  */
    return(FX_SUCCESS);
}

