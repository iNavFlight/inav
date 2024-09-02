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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_short_name_get_extended                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the short name associated with the supplied     */
/*    unicode name.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_unicode_name                   Pointer to unicode name       */
/*    source_unicode_length                 Length of unicode name        */
/*    destination_short_name                Pointer to destination name   */
/*    short_name_buffer_length              Buffer length of short name   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_directory_search          Search for unicode name       */
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
UINT  _fx_unicode_short_name_get_extended(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name, ULONG short_name_buffer_length)
{

UINT         status;
ULONG        temp_unicode_length;
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

#ifdef FX_ENABLE_EXFAT
    /* Check if media format is exFAT.  */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Return the not implemented error.  */
        return(FX_NOT_IMPLEMENTED);
    }
#endif

    /* Null terminate the short return name.  */
    destination_short_name[0] =  (UCHAR)0;

    /* Setup temporary unicode name length for search call.  */
    temp_unicode_length =  source_unicode_length;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_UNICODE_SHORT_NAME_GET, media_ptr, source_unicode_name, source_unicode_length, destination_short_name, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied short name and return the unicode name if there is a match.  */
    status =  _fx_unicode_directory_search(media_ptr, &dir_entry, (UCHAR *)destination_short_name, short_name_buffer_length, source_unicode_name, &temp_unicode_length, 0);

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return successful completion status.  */
    return(status);
}

