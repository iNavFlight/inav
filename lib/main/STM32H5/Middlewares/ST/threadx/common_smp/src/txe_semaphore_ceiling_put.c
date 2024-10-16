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
#include "tx_semaphore.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_semaphore_ceiling_put                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the semaphore ceiling put        */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                         Pointer to semaphore          */
/*    ceiling                               Maximum value of semaphore    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SEMAPHORE_ERROR                    Invalid semaphore pointer     */
/*    TX_INVALID_CEILING                    Invalid semaphore ceiling     */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_semaphore_ceiling_put             Actual semaphore ceiling put  */
/*                                            function                    */
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
UINT  _txe_semaphore_ceiling_put(TX_SEMAPHORE *semaphore_ptr, ULONG ceiling)
{

UINT        status;


    /* Check for an invalid semaphore pointer.  */
    if (semaphore_ptr == TX_NULL)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }

    /* Now check for a valid semaphore ID.  */
    else if (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }

    /* Determine if the ceiling is valid - must be greater than 1.  */
    else if (ceiling == ((ULONG) 0))
    {

        /* Invalid ceiling, return error.  */
        status =  TX_INVALID_CEILING;
    }
    else
    {

        /* Call actual semaphore ceiling put function.  */
        status =  _tx_semaphore_ceiling_put(semaphore_ptr, ceiling);
    }

    /* Return completion status.  */
    return(status);
}

