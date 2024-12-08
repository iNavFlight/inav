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
/**   Module                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TXM_MODULE
#include "txm_module.h"
#ifndef TXM_THREAD_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_thread_info_get                                PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the thread information get       */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to thread control block   */
/*    name                              Destination for the thread name   */
/*    state                             Destination for thread state      */
/*    run_count                         Destination for thread run count  */
/*    priority                          Destination for thread priority   */
/*    preemption_threshold              Destination for thread preemption-*/
/*                                        threshold                       */
/*    time_slice                        Destination for thread time-slice */
/*    next_thread                       Destination for next created      */
/*                                        thread                          */
/*    next_suspended_thread             Destination for next suspended    */
/*                                        thread                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                   Invalid thread pointer            */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_kernel_call_dispatcher                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Module application code                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  01-31-2022      Scott Larson            Modified comments and added   */
/*                                            CALL_NOT_USED option,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _txe_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count, UINT *priority, UINT *preemption_threshold, ULONG *time_slice, TX_THREAD **next_thread, TX_THREAD **next_suspended_thread)
{

UINT return_value;
ALIGN_TYPE extra_parameters[7];

    extra_parameters[0] = (ALIGN_TYPE) state;
    extra_parameters[1] = (ALIGN_TYPE) run_count;
    extra_parameters[2] = (ALIGN_TYPE) priority;
    extra_parameters[3] = (ALIGN_TYPE) preemption_threshold;
    extra_parameters[4] = (ALIGN_TYPE) time_slice;
    extra_parameters[5] = (ALIGN_TYPE) next_thread;
    extra_parameters[6] = (ALIGN_TYPE) next_suspended_thread;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_THREAD_INFO_GET_CALL, (ALIGN_TYPE) thread_ptr, (ALIGN_TYPE) name, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
