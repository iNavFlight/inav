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
#include "tx_trace.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_info_get                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information from the specified mutex.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*    name                              Destination for the mutex name    */
/*    count                             Destination for the owner count   */
/*    owner                             Destination for the owner's       */
/*                                        thread control block pointer    */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on the mutex   */
/*    suspended_count                   Destination for suspended count   */
/*    next_mutex                        Destination for pointer to next   */
/*                                        mutex on the created list       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
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
UINT  _tx_mutex_info_get(TX_MUTEX *mutex_ptr, CHAR **name, ULONG *count, TX_THREAD **owner,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_MUTEX **next_mutex)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_INFO_GET, mutex_ptr, 0, 0, 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the mutex.  */
    if (name != TX_NULL)
    {

        *name =  mutex_ptr -> tx_mutex_name;
    }

    /* Retrieve the current ownership count of the mutex.  */
    if (count != TX_NULL)
    {

        *count =  ((ULONG) mutex_ptr -> tx_mutex_ownership_count);
    }

    /* Retrieve the current owner of the mutex.  */
    if (owner != TX_NULL)
    {

        *owner =  mutex_ptr -> tx_mutex_owner;
    }

    /* Retrieve the first thread suspended on this mutex.  */
    if (first_suspended != TX_NULL)
    {

        *first_suspended =  mutex_ptr -> tx_mutex_suspension_list;
    }

    /* Retrieve the number of threads suspended on this mutex.  */
    if (suspended_count != TX_NULL)
    {

        *suspended_count =  (ULONG) mutex_ptr -> tx_mutex_suspended_count;
    }

    /* Retrieve the pointer to the next mutex created.  */
    if (next_mutex != TX_NULL)
    {

        *next_mutex =  mutex_ptr -> tx_mutex_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

