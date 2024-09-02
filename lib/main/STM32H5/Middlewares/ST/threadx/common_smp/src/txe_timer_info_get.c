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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_timer_info_get                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the timer information get        */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*    name                              Destination for the timer name    */
/*    active                            Destination for active flag       */
/*    remaining_ticks                   Destination for remaining ticks   */
/*                                        before expiration               */
/*    reschedule_ticks                  Destination for reschedule ticks  */
/*    next_timer                        Destination for next timer on the */
/*                                        created list                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TIMER_ERROR                    Invalid timer pointer             */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_timer_info_get                Actual info get call              */
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
UINT  _txe_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks,
                ULONG *reschedule_ticks, TX_TIMER **next_timer)
{

UINT    status;


    /* Check for an invalid timer pointer.  */
    if (timer_ptr == TX_NULL)
    {

        /* Timer pointer is invalid, return appropriate error code.  */
        status =  TX_TIMER_ERROR;
    }

    /* Now check for invalid timer ID.  */
    else if (timer_ptr -> tx_timer_id != TX_TIMER_ID)
    {

        /* Timer pointer is invalid, return appropriate error code.  */
        status =  TX_TIMER_ERROR;
    }
    else
    {

        /* Otherwise, call the actual timer information get service.  */
        status =  _tx_timer_info_get(timer_ptr, name, active, remaining_ticks, reschedule_ticks, next_timer);
    }

    /* Return completion status.  */
    return(status);
}

