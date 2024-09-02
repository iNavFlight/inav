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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_event_flags_info_get                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the event flag information get   */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to event flag group       */
/*    name                              Destination for the event flags   */
/*                                        group name                      */
/*    current_flags                     Current event flags               */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on event flags */
/*    suspended_count                   Destination for suspended count   */
/*    next_group                        Destination for pointer to next   */
/*                                        event flag group on the created */
/*                                        list                            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_GROUP_ERROR                    Invalid event flag group pointer  */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_event_flags_info_get          Actual event flags group info     */
/*                                        get service                     */
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
UINT  _txe_event_flags_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR **name, ULONG *current_flags,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_EVENT_FLAGS_GROUP **next_group)
{

UINT        status;


    /* Check for an invalid event flag group pointer.  */
    if (group_ptr == TX_NULL)
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }

    /* Now check for invalid event flag group ID.  */
    else if (group_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID)
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }
    else
    {

        /* Otherwise, call the actual event flags group information get service.  */
        status =  _tx_event_flags_info_get(group_ptr, name, current_flags, first_suspended,
                                                            suspended_count, next_group);
    }

    /* Return completion status.  */
    return(status);
}

