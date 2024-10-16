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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_media.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_read                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the specified raw logical sector from the       */
/*    specified media.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Logical sector to read        */
/*    buffer_ptr                            Pointer to caller's buffer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_read       Logical sector read utility   */
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
UINT  _fx_media_read(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr)
{

UINT                   status;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_reads++;
#endif

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_READ, media_ptr, logical_sector, buffer_ptr, 0, FX_TRACE_MEDIA_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Read the logical sector.  */
    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector, buffer_ptr, ((ULONG) 1), FX_DATA_SECTOR);

#ifdef TX_ENABLE_EVENT_TRACE

    /* Check for successful status.  */
    if (status == FX_SUCCESS)
    {

        /* Update the trace event with the bytes read.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_READ, 0, 0, 0, media_ptr -> fx_media_bytes_per_sector)
    }
#endif

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return status to the caller.  */
    return(status);
}

