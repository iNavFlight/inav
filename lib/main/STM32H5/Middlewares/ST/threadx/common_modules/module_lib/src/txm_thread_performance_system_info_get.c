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
#ifndef TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_performance_system_info_get              PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves thread system performance information.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    resumptions                       Destination for total number of   */
/*                                        thread resumptions              */
/*    suspensions                       Destination for total number of   */
/*                                        thread suspensions              */
/*    solicited_preemptions             Destination for total number of   */
/*                                        thread preemption from thread   */
/*                                        API calls                       */
/*    interrupt_preemptions             Destination for total number of   */
/*                                        thread preemptions as a result  */
/*                                        of threads made ready inside of */
/*                                        Interrupt Service Routines      */
/*    priority_inversions               Destination for total number of   */
/*                                        priority inversions             */
/*    time_slices                       Destination for total number of   */
/*                                        time-slices                     */
/*    relinquishes                      Destination for total number of   */
/*                                        relinquishes                    */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
/*    wait_aborts                       Destination for total number of   */
/*                                        wait aborts                     */
/*    non_idle_returns                  Destination for total number of   */
/*                                        times threads return when       */
/*                                        another thread is ready         */
/*    idle_returns                      Destination for total number of   */
/*                                        times threads return when no    */
/*                                        other thread is ready           */
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
UINT _tx_thread_performance_system_info_get(ULONG *resumptions, ULONG *suspensions, ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions, ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts, ULONG *non_idle_returns, ULONG *idle_returns)
{

UINT return_value;
ALIGN_TYPE extra_parameters[9];

    extra_parameters[0] = (ALIGN_TYPE) solicited_preemptions;
    extra_parameters[1] = (ALIGN_TYPE) interrupt_preemptions;
    extra_parameters[2] = (ALIGN_TYPE) priority_inversions;
    extra_parameters[3] = (ALIGN_TYPE) time_slices;
    extra_parameters[4] = (ALIGN_TYPE) relinquishes;
    extra_parameters[5] = (ALIGN_TYPE) timeouts;
    extra_parameters[6] = (ALIGN_TYPE) wait_aborts;
    extra_parameters[7] = (ALIGN_TYPE) non_idle_returns;
    extra_parameters[8] = (ALIGN_TYPE) idle_returns;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL, (ALIGN_TYPE) resumptions, (ALIGN_TYPE) suspensions, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
