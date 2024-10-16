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

#include "tx_api.h"    /* Threadx API */
#include "pthread.h"  /* Posix API */
#include "px_int.h"    /* Posix helper functions */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*  sleep                                                  PORTABLE C     */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function shall cause the calling thread to be suspended from   */
/*    execution until either the number of realtime seconds specified by  */
/*    the argument seconds has elapsed                                    */  
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*   seconds                      Is the number of real-time (as opposed  */
/*                                to CPU-time) seconds to suspend the     */
/*                                calling thread.                         */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*   number of seconds remaining to sleep                                 */
/*                                A nonzero value indicates sleep was     */
/*                                interrupted. Zero is successful         */
/*                                completion                              */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_thread_sleep             ThreadX thread sleep service            */
/*                                                                        */  
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT sleep(ULONG seconds)
{

UINT temp1, temp2, diff, result;

    temp1 = tx_time_get();
    tx_thread_sleep(seconds * CPU_TICKS_PER_SECOND);
    temp2 = tx_time_get();
    diff = temp2 - temp1;

    if (diff > (seconds * CPU_TICKS_PER_SECOND))
    {
        result = (diff - (seconds * CPU_TICKS_PER_SECOND));
    }
    else
    {
        result = ((seconds * CPU_TICKS_PER_SECOND) - diff);
    }

    return (result/CPU_TICKS_PER_SECOND);    
    
}

