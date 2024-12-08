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
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_mutex_info_get                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the mutex information get        */
/*    service.                                                            */
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
/*    TX_MUTEX_ERROR                    Invalid mutex pointer             */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_info_get                Actual mutex info get service     */
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
UINT  _txe_mutex_info_get(TX_MUTEX *mutex_ptr, CHAR **name, ULONG *count, TX_THREAD **owner,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_MUTEX **next_mutex)
{

UINT        status;


    /* Check for an invalid mutex pointer.  */
    if (mutex_ptr == TX_NULL)
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
    }

    /* Now check for invalid mutex ID.  */
    else if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
    }
    else
    {

        /* Otherwise, call the actual mutex information get service.  */
        status =  _tx_mutex_info_get(mutex_ptr, name, count, owner, first_suspended,
                                                            suspended_count, next_mutex);
    }

    /* Return completion status.  */
    return(status);
}

