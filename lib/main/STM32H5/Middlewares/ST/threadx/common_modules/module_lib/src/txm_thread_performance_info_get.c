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
#ifndef TXM_THREAD_PERFORMANCE_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_performance_info_get                     PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    thread.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to thread control block   */
/*    resumptions                       Destination for number of times   */
/*                                        thread was resumed              */
/*    suspensions                       Destination for number of times   */
/*                                        thread was suspended            */
/*    solicited_preemptions             Destination for number of times   */
/*                                        thread called another service   */
/*                                        that resulted in preemption     */
/*    interrupt_preemptions             Destination for number of times   */
/*                                        thread was preempted by another */
/*                                        thread made ready in Interrupt  */
/*                                        Service Routine (ISR)           */
/*    priority_inversions               Destination for number of times   */
/*                                        a priority inversion was        */
/*                                        detected for this thread        */
/*    time_slices                       Destination for number of times   */
/*                                        thread was time-sliced          */
/*    relinquishes                      Destination for number of thread  */
/*                                        relinquishes                    */
/*    timeouts                          Destination for number of timeouts*/
/*                                        for thread                      */
/*    wait_aborts                       Destination for number of wait    */
/*                                        aborts for thread               */
/*    last_preempted_by                 Destination for pointer of the    */
/*                                        thread that last preempted this */
/*                                        thread                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
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
UINT _tx_thread_performance_info_get(TX_THREAD *thread_ptr, ULONG *resumptions, ULONG *suspensions, ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions, ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts, TX_THREAD **last_preempted_by)
{

UINT return_value;
ALIGN_TYPE extra_parameters[9];

    extra_parameters[0] = (ALIGN_TYPE) suspensions;
    extra_parameters[1] = (ALIGN_TYPE) solicited_preemptions;
    extra_parameters[2] = (ALIGN_TYPE) interrupt_preemptions;
    extra_parameters[3] = (ALIGN_TYPE) priority_inversions;
    extra_parameters[4] = (ALIGN_TYPE) time_slices;
    extra_parameters[5] = (ALIGN_TYPE) relinquishes;
    extra_parameters[6] = (ALIGN_TYPE) timeouts;
    extra_parameters[7] = (ALIGN_TYPE) wait_aborts;
    extra_parameters[8] = (ALIGN_TYPE) last_preempted_by;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_THREAD_PERFORMANCE_INFO_GET_CALL, (ALIGN_TYPE) thread_ptr, (ALIGN_TYPE) resumptions, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
