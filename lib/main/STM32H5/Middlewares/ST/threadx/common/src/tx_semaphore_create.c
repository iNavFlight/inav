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
/*    _tx_semaphore_create                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a counting semaphore with the initial count   */
/*    specified in this call.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                     Pointer to semaphore control block*/
/*    name_ptr                          Pointer to semaphore name         */
/*    initial_count                     Initial semaphore count           */
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
UINT  _tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count)
{

TX_INTERRUPT_SAVE_AREA

TX_SEMAPHORE    *next_semaphore;
TX_SEMAPHORE    *previous_semaphore;


    /* Initialize semaphore control block to all zeros.  */
    TX_MEMSET(semaphore_ptr, 0, (sizeof(TX_SEMAPHORE)));

    /* Setup the basic semaphore fields.  */
    semaphore_ptr -> tx_semaphore_name =             name_ptr;
    semaphore_ptr -> tx_semaphore_count =            initial_count;

    /* Disable interrupts to place the semaphore on the created list.  */
    TX_DISABLE

    /* Setup the semaphore ID to make it valid.  */
    semaphore_ptr -> tx_semaphore_id =  TX_SEMAPHORE_ID;

    /* Place the semaphore on the list of created semaphores.  First,
       check for an empty list.  */
    if (_tx_semaphore_created_count == TX_EMPTY)
    {

        /* The created semaphore list is empty.  Add semaphore to empty list.  */
        _tx_semaphore_created_ptr =                       semaphore_ptr;
        semaphore_ptr -> tx_semaphore_created_next =      semaphore_ptr;
        semaphore_ptr -> tx_semaphore_created_previous =  semaphore_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_semaphore =      _tx_semaphore_created_ptr;
        previous_semaphore =  next_semaphore -> tx_semaphore_created_previous;

        /* Place the new semaphore in the list.  */
        next_semaphore -> tx_semaphore_created_previous =  semaphore_ptr;
        previous_semaphore -> tx_semaphore_created_next =  semaphore_ptr;

        /* Setup this semaphore's next and previous created links.  */
        semaphore_ptr -> tx_semaphore_created_previous =  previous_semaphore;
        semaphore_ptr -> tx_semaphore_created_next =      next_semaphore;
    }

    /* Increment the created count.  */
    _tx_semaphore_created_count++;

    /* Optional semaphore create extended processing.  */
    TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_SEMAPHORE, semaphore_ptr, name_ptr, initial_count, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_CREATE, semaphore_ptr, initial_count, TX_POINTER_TO_ULONG_CONVERT(&next_semaphore), 0, TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

