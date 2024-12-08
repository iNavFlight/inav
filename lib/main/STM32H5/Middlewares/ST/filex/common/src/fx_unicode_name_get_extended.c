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
/*    _fx_unicode_name_get_extended                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the unicode name associated with the supplied   */
/*    short 8.3 name.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_short_name                     Pointer to short file name    */
/*    destination_unicode_name              Pointer to destination name   */
/*    destination_unicode_length            Destination for name length   */
/*    unicode_name_buffer_length            Buffer length of unicode name */
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
UINT  _fx_unicode_name_get_extended(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length, ULONG unicode_name_buffer_length)
{

UINT                   status;
FX_DIR_ENTRY           dir_entry;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
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
    /* Check if media format is exFAT.  */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Return the not implemented error.  */
        return(FX_NOT_IMPLEMENTED);
    }
#endif

    /* Set the destination unicode length to zero to indicate there is nothing in terms of a match.  */
    *destination_unicode_length =  0;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_UNICODE_NAME_GET, media_ptr, source_short_name, destination_unicode_name, 0, FX_TRACE_FILE_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied short file name and return the unicode name if there is a match.  */
    status =  _fx_unicode_directory_search(media_ptr, &dir_entry, (UCHAR *)source_short_name, 0, destination_unicode_name, destination_unicode_length, unicode_name_buffer_length);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Update the trace event with the unicode length.  */
    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_UNICODE_NAME_GET, 0, 0, 0, *destination_unicode_length)

    /* Return successful completion status.  */
    return(FX_SUCCESS);
}

