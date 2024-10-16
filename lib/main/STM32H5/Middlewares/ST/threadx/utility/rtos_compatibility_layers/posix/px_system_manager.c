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
/** POSIX wrapper for THREADX                                             */ 
/**                                                                       */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"     /* Threadx API */
#include "pthread.h"    /* Posix API */
#include "px_int.h"     /* Posix helper functions */

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    posix_system_manager_entry                          PORTABLE C      */ 
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This is the System Manager thread for the POSIX> The system         */
/*    manager thread cleans up terminated threads and releases            */
/*    the stack memory associated with the thread                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    input                                       Not used                */  
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_queue_receive                            Receive system request  */ 
/*    posix_do_pthread_delete                     Delete a pthread        */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Start-up code                                                       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Add 64-bit support,           */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID  posix_system_manager_entry(ULONG input)
{

UINT        status;
ULONG       request[WORK_REQ_SIZE];

POSIX_TCB   *pthread_ptr;
VOID        *value_ptr;

    /* Avoid compiler warning.  */
    TX_PARAMETER_NOT_USED(input);

    /* Loop forever, waiting for work requests.  */ 
    while(1)
    {
        /* Wait forever for the next work request.  */
        status = tx_queue_receive(&posix_work_queue, &request, TX_WAIT_FOREVER);
        /* Make sure we didn't encounter any trouble.  */ 
        if (status != TX_SUCCESS)
        {
            /* Get the next message.  */
            continue;
        }
        
        #ifdef TX_64_BIT
        pthread_ptr = (POSIX_TCB *)((((ALIGN_TYPE)request[0]) << 32) | request[1]);
        value_ptr = (VOID *)((((ALIGN_TYPE)request[2]) << 32) | request[3]);
        #else
        pthread_ptr = (POSIX_TCB *)request[0];
        value_ptr = (VOID *)request[1];
        #endif

        /* Delete the pthread  */
        posix_do_pthread_delete(pthread_ptr, value_ptr);
        
    } /* System Manager forever loop  */
}

