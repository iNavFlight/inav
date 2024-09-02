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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_semaphore.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_semaphore_info_get                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information from the specified semaphore.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                     Pointer to semaphore control block*/
/*    name                              Destination for the semaphore name*/
/*    current_value                     Destination for current value of  */
/*                                        the semaphore                   */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on semaphore   */
/*    suspended_count                   Destination for suspended count   */
/*    next_semaphore                    Destination for pointer to next   */
/*                                        semaphore on the created list   */
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
UINT  _tx_semaphore_info_get(TX_SEMAPHORE *semaphore_ptr, CHAR **name, ULONG *current_value,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_SEMAPHORE **next_semaphore)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_INFO_GET, semaphore_ptr, 0, 0, 0, TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the semaphore.  */
    if (name != TX_NULL)
    {

        *name =  semaphore_ptr -> tx_semaphore_name;
    }

    /* Retrieve the current value of the semaphore.  */
    if (current_value != TX_NULL)
    {

        *current_value =  semaphore_ptr -> tx_semaphore_count;
    }

    /* Retrieve the first thread suspended on this semaphore.  */
    if (first_suspended != TX_NULL)
    {

        *first_suspended =  semaphore_ptr -> tx_semaphore_suspension_list;
    }

    /* Retrieve the number of threads suspended on this semaphore.  */
    if (suspended_count != TX_NULL)
    {

        *suspended_count =  (ULONG) semaphore_ptr -> tx_semaphore_suspended_count;
    }

    /* Retrieve the pointer to the next semaphore created.  */
    if (next_semaphore != TX_NULL)
    {

        *next_semaphore =  semaphore_ptr -> tx_semaphore_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

