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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_trace.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_create                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a mutex with optional priority inheritance as */
/*    specified in this call.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                             Pointer to mutex control block*/
/*    name_ptr                              Pointer to mutex name         */
/*    inherit                               Priority inheritance option   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit)
{

TX_INTERRUPT_SAVE_AREA

TX_MUTEX        *next_mutex;
TX_MUTEX        *previous_mutex;


    /* Initialize mutex control block to all zeros.  */
    TX_MEMSET(mutex_ptr, 0, (sizeof(TX_MUTEX)));

    /* Setup the basic mutex fields.  */
    mutex_ptr -> tx_mutex_name =             name_ptr;
    mutex_ptr -> tx_mutex_inherit =          inherit;

    /* Disable interrupts to place the mutex on the created list.  */
    TX_DISABLE

    /* Setup the mutex ID to make it valid.  */
    mutex_ptr -> tx_mutex_id =  TX_MUTEX_ID;

    /* Setup the thread mutex release function pointer.  */
    _tx_thread_mutex_release =  &(_tx_mutex_thread_release);

    /* Place the mutex on the list of created mutexes.  First,
       check for an empty list.  */
    if (_tx_mutex_created_count == TX_EMPTY)
    {

        /* The created mutex list is empty.  Add mutex to empty list.  */
        _tx_mutex_created_ptr =                   mutex_ptr;
        mutex_ptr -> tx_mutex_created_next =      mutex_ptr;
        mutex_ptr -> tx_mutex_created_previous =  mutex_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_mutex =      _tx_mutex_created_ptr;
        previous_mutex =  next_mutex -> tx_mutex_created_previous;

        /* Place the new mutex in the list.  */
        next_mutex -> tx_mutex_created_previous =  mutex_ptr;
        previous_mutex -> tx_mutex_created_next =  mutex_ptr;

        /* Setup this mutex's next and previous created links.  */
        mutex_ptr -> tx_mutex_created_previous =  previous_mutex;
        mutex_ptr -> tx_mutex_created_next =      next_mutex;
    }

    /* Increment the ownership count.  */
    _tx_mutex_created_count++;

    /* Optional mutex create extended processing.  */
    TX_MUTEX_CREATE_EXTENSION(mutex_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_MUTEX, mutex_ptr, name_ptr, inherit, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_CREATE, mutex_ptr, inherit, TX_POINTER_TO_ULONG_CONVERT(&next_mutex), 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

