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
#include "fx_directory.h"
#include "fx_file.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_date_time_set                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the specified file's date and time with the      */
/*    values provided.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_name                             File name pointer             */
/*    year                                  Year                          */
/*    month                                 Month                         */
/*    day                                   Day                           */
/*    hour                                  Hour                          */
/*    minute                                Minute                        */
/*    second                                Second                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
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
UINT  _fx_file_date_time_set(FX_MEDIA *media_ptr, CHAR *file_name,
                             UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second)
{

UINT         status;
FX_DIR_ENTRY dir_entry;


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
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_DATE_TIME_SET, media_ptr, file_name, year, month, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied directory name.  */
    status =  _fx_directory_search(media_ptr, file_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Set the new time and date.  */
    dir_entry.fx_dir_entry_time =  (hour << FX_HOUR_SHIFT) | (minute << FX_MINUTE_SHIFT) | (second / 2);
    dir_entry.fx_dir_entry_date =  ((year - FX_BASE_YEAR) << FX_YEAR_SHIFT) | (month << FX_MONTH_SHIFT) | day;

    /* Write the directory entry to the media.  */
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

    /* Release media protection.  */
    FX_UNPROTECT

    /* Directory information write is complete, return status.  */
    return(status);
}

