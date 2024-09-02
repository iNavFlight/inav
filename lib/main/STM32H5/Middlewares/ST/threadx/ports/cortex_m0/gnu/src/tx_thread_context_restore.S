@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */
@/** ThreadX Component                                                     */
@/**                                                                       */
@/**   Thread                                                              */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@
    .global  _tx_thread_system_state
    .global  _tx_thread_current_ptr
    .global  _tx_thread_system_stack_ptr
    .global  _tx_thread_execute_ptr
    .global  _tx_timer_time_slice
    .global  _tx_thread_schedule
    .global  _tx_thread_preempt_disable
    .global _tx_execution_isr_exit
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))                                        
    .global  _tx_execution_isr_exit
#endif
@
@
    .text 32
    .align 4
    .syntax unified
@/**************************************************************************/
@/*                                                                        */
@/*  FUNCTION                                               RELEASE        */
@/*                                                                        */
@/*    _tx_thread_context_restore                        Cortex-M0/GNU     */
@/*                                                           6.1          */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */
@/*    This function is only needed for legacy applications and it should  */
@/*    not be called in any new development on a Cortex-M.                 */
@/*    This function restores the interrupt context if it is processing a  */
@/*    nested interrupt.  If not, it returns to the interrupt thread if no */
@/*    preemption is necessary.  Otherwise, if preemption is necessary or  */
@/*    if no thread was running, the function returns to the scheduler.    */
@/*                                                                        */
@/*  INPUT                                                                 */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  OUTPUT                                                                */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  CALLS                                                                 */
@/*                                                                        */
@/*    _tx_thread_schedule                   Thread scheduling routine     */
@/*                                                                        */
@/*  CALLED BY                                                             */
@/*                                                                        */
@/*    ISRs                                  Interrupt Service Routines    */
@/*                                                                        */
@/*  RELEASE HISTORY                                                       */
@/*                                                                        */
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
@/*  09-30-2020     Scott Larson             Modified comment(s), and      */
@/*                                            cleaned up whitespace,      */
@/*                                            resulting in version 6.1    */
@/*                                                                        */
@/**************************************************************************/
@VOID   _tx_thread_context_restore(VOID)
@{
    .global  _tx_thread_context_restore
    .thumb_func
_tx_thread_context_restore:
@
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    /* Call the ISR exit function to indicate an ISR is complete.  */
    PUSH    {r0, lr}                                // Save return address
    BL      _tx_execution_isr_exit                  // Call the ISR exit function
    POP     {r0, lr}                                // Recover return address
#endif
    BX      lr
@}
