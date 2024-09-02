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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_extended_relative_seek                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function positions the internal file pointers to the specified */
/*    byte relative offset such that the next read or write operation is  */
/*    performed there.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    byte_offset                           Byte offset of the seek       */
/*    seek_from                             Direction for relative seek,  */
/*                                          legal values are:             */
/*                                                                        */
/*                                              FX_SEEK_BEGIN             */
/*                                              FX_SEEK_END               */
/*                                              FX_SEEK_FORWARD           */
/*                                              FX_SEEK_BACK              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_extended_seek                Seek to specified position    */
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
UINT  _fx_file_extended_relative_seek(FX_FILE *file_ptr, ULONG64 byte_offset, UINT seek_from)
{

#ifndef FX_MEDIA_STATISTICS_DISABLE
FX_MEDIA *media_ptr;

    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

    /* Setup pointer to media structure.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_relative_seeks++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_RELATIVE_SEEK, file_ptr, byte_offset, seek_from, file_ptr -> fx_file_current_file_offset, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Determine if seeking from the beginning is requested.  */
    if (seek_from == FX_SEEK_BEGIN)
    {

        /* Yes, use the base file seek routine to seek from the beginning of
           the file.  */
        return(_fx_file_extended_seek(file_ptr, byte_offset));
    }
    /* Otherwise, determine if seeking from the end is requested.  */
    else if (seek_from == FX_SEEK_END)
    {

        /* Yes, seek from the end of the file.  */

        /* Determine if the requested seek offset is greater than
           the file size.  */
        if (byte_offset >= file_ptr -> fx_file_current_file_size)
        {

            /* Yes, just seek to the beginning of the file.  */
            return(_fx_file_extended_seek(file_ptr, ((ULONG64) 0)));
        }
        else
        {

            /* Logically seek from the end of the file.  */
            return(_fx_file_extended_seek(file_ptr, file_ptr -> fx_file_current_file_size - byte_offset));
        }
    }
    /* Otherwise, determine if seeking from the current position is requested.  */
    else if (seek_from == FX_SEEK_FORWARD)
    {

        /* Yes, just seek ahead from the current file position.  */
        return(_fx_file_extended_seek(file_ptr, file_ptr -> fx_file_current_file_offset + byte_offset));
    }
    /* Otherwise, seeking backward from the current position is assumed.  */
    else
    {

        /* Determine if the backward offset is greater than the current file offset.  */
        if (byte_offset >= file_ptr -> fx_file_current_file_offset)
        {

            /* Yes, just position the file to the beginning.  */
            return(_fx_file_extended_seek(file_ptr, ((ULONG64) 0)));
        }
        else
        {

            /* Seek backward relative to the current position.  */
            return(_fx_file_extended_seek(file_ptr, file_ptr -> fx_file_current_file_offset - byte_offset));
        }
    }
}

