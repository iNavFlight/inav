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
/*    _fx_directory_attributes_read                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified directory.  If   */
/*    found, the attribute read request is valid and the directory entry  */
/*    will be read in order to pickup the directory's attributes.         */
/*    Otherwise, if the directory is not found, the appropriate error     */
/*    code is returned to the caller.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Directory name pointer        */
/*    attributes_ptr                        Return attributes pointer     */
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
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_attributes_read(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes_ptr)
{

UINT                   status;
FX_DIR_ENTRY           dir_entry;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_attributes_reads++;
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
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_ATTRIBUTES_READ, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Search the system for the supplied file name.  */
    status =  _fx_directory_search(media_ptr, directory_name, &dir_entry, FX_NULL, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check to make sure the found entry is a file.  */
    if ((dir_entry.fx_dir_entry_attributes & (UCHAR)(FX_DIRECTORY)) == 0)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a file error code.  */
        return(FX_NOT_DIRECTORY);
    }

    /* Place the current attributes into the destination.  */
    *attributes_ptr =  (UINT)dir_entry.fx_dir_entry_attributes;

    /* Update the trace event with the attributes read.  */
    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_DIRECTORY_ATTRIBUTES_READ, 0, 0, dir_entry.fx_dir_entry_attributes, 0)

    /* Release media protection.  */
    FX_UNPROTECT

    /* File attribute read is complete, return successful status.  */
    return(FX_SUCCESS);
}

