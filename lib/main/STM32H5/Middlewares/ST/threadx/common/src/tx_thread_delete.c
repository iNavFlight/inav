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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_delete                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles application delete thread requests.  The      */
/*    thread to delete must be in a terminated or completed state,        */
/*    otherwise this function just returns an error code.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_delete(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Lockout interrupts while the thread is being deleted.  */
    TX_DISABLE

    /* Check for proper status of this thread to delete.  */
    if (thread_ptr -> tx_thread_state != TX_COMPLETED)
    {

        /* Now check for terminated state.  */
        if (thread_ptr -> tx_thread_state != TX_TERMINATED)
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Thread not completed or terminated - return an error!  */
            status =  TX_DELETE_ERROR;
        }
    }

    /* Determine if the delete operation is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Yes, continue with deleting the thread.  */

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_DELETE_EXTENSION(thread_ptr)

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_DELETE, thread_ptr, TX_POINTER_TO_ULONG_CONVERT(&next_thread), 0, 0, TX_TRACE_THREAD_EVENTS)

        /* If trace is enabled, unregister this object.  */
        TX_TRACE_OBJECT_UNREGISTER(thread_ptr)

        /* Log this kernel call.  */
        TX_EL_THREAD_DELETE_INSERT

        /* Unregister thread in the thread array structure.  */
        TX_EL_THREAD_UNREGISTER(thread_ptr)

        /* Clear the thread ID to make it invalid.  */
        thread_ptr -> tx_thread_id =  TX_CLEAR_ID;

        /* Decrement the number of created threads.  */
        _tx_thread_created_count--;

        /* See if the thread is the only one on the list.  */
        if (_tx_thread_created_count == TX_EMPTY)
        {

            /* Only created thread, just set the created list to NULL.  */
            _tx_thread_created_ptr =  TX_NULL;
        }
        else
        {

            /* Otherwise, not the only created thread, link-up the neighbors.  */
            next_thread =                                thread_ptr -> tx_thread_created_next;
            previous_thread =                            thread_ptr -> tx_thread_created_previous;
            next_thread -> tx_thread_created_previous =  previous_thread;
            previous_thread -> tx_thread_created_next =  next_thread;

            /* See if we have to update the created list head pointer.  */
            if (_tx_thread_created_ptr == thread_ptr)
            {

                /* Yes, move the head pointer to the next link. */
                _tx_thread_created_ptr =  next_thread;
            }
        }

        /* Execute Port-Specific completion processing. If needed, it is typically defined in tx_port.h.  */
        TX_THREAD_DELETE_PORT_COMPLETION(thread_ptr)

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Return completion status.  */
    return(status);
}

