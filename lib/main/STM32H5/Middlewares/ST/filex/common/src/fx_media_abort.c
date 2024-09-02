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
#include "fx_file.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_abort                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function marks all open files for the specified media as       */
/*    aborted and then removes the media control block from the open      */
/*    media list and marks it as aborted as well.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_delete                       Delete the mutex              */
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
UINT  _fx_media_abort(FX_MEDIA  *media_ptr)
{

FX_INT_SAVE_AREA
ULONG    open_count;
FX_FILE *file_ptr;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_aborts++;
#endif

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_ABORT, media_ptr, 0, 0, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Loop through the media's open files.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    file_ptr =    media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Mark the file as aborted.  */
        file_ptr -> fx_file_id =  FX_FILE_ABORTED_ID;

        /* Adjust the pointer and decrement the file opened count.  */
        file_ptr =  file_ptr -> fx_file_opened_next;
        open_count--;
    }

    /* Build the "abort" I/O driver request.  */
    media_ptr -> fx_media_driver_request =      FX_DRIVER_ABORT;
    media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_ABORT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Call the specified I/O driver with the abort request.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Now remove this media from the open list.  */

    /* Lockout interrupts for media removal.  */
    FX_DISABLE_INTS

    /* See if the media is the only one on the media opened list.  */
    if (_fx_system_media_opened_count == ((ULONG) 1))
    {

        /* Only opened media, just set the opened list to NULL.  */
        _fx_system_media_opened_ptr =  FX_NULL;
    }
    else
    {

        /* Otherwise, not the only opened media, link-up the neighbors.  */
        (media_ptr -> fx_media_opened_next) -> fx_media_opened_previous =
            media_ptr -> fx_media_opened_previous;
        (media_ptr -> fx_media_opened_previous) -> fx_media_opened_next =
            media_ptr -> fx_media_opened_next;

        /* See if we have to update the opened list head pointer.  */
        if (_fx_system_media_opened_ptr == media_ptr)
        {

            /* Yes, move the head pointer to the next opened media. */
            _fx_system_media_opened_ptr =  media_ptr -> fx_media_opened_next;
        }
    }

    /* Decrement the opened media counter.  */
    _fx_system_media_opened_count--;

    /* Finally, Indicate that this media is aborted.  */
    media_ptr -> fx_media_id =  FX_MEDIA_ABORTED_ID;

    /* Restore interrupt posture.  */
    FX_RESTORE_INTS

    /* Delete the media protection structure if FX_SINGLE_THREAD is not
       defined.  */
#ifndef FX_SINGLE_THREAD

#ifndef FX_DONT_CREATE_MUTEX

    /* Note that the protection is never released. The mutex delete
       service will handle all threads waiting access to this media
       control block.  */
    tx_mutex_delete(& (media_ptr -> fx_media_protect));
#endif
#endif

#ifdef FX_DONT_CREATE_MUTEX

    /* Release media protection.  */
    FX_UNPROTECT
#endif

    /* Return status to the caller.  */
    return(FX_SUCCESS);
}

