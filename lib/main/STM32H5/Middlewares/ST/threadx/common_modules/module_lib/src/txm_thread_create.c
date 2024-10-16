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
#ifndef TXM_THREAD_CREATE_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_thread_create                                  PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the thread create function call. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*    name                                  Pointer to thread name string */
/*    entry_function                        Entry function of the thread  */
/*    entry_input                           32-bit input value to thread  */
/*    stack_start                           Pointer to start of stack     */
/*    stack_size                            Stack size in bytes           */
/*    priority                              Priority of thread (0-31)     */
/*    preempt_threshold                     Preemption threshold          */
/*    time_slice                            Thread time-slice value       */
/*    auto_start                            Automatic start selection     */
/*    thread_control_block_size             Size of thread control block  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                       Invalid thread pointer        */
/*    TX_PTR_ERROR                          Invalid entry point or stack  */
/*                                            address                     */
/*    TX_SIZE_ERROR                         Invalid stack size -too small */
/*    TX_PRIORITY_ERROR                     Invalid thread priority       */
/*    TX_THRESH_ERROR                       Invalid preemption threshold  */
/*    status                                Actual completion status      */
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
UINT _txe_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr, VOID (*entry_function)(ULONG entry_input), ULONG entry_input, VOID *stack_start, ULONG stack_size, UINT priority, UINT preempt_threshold, ULONG time_slice, UINT auto_start, UINT thread_control_block_size)
{

UINT return_value;
ALIGN_TYPE extra_parameters[9];

    extra_parameters[0] = (ALIGN_TYPE) entry_function;
    extra_parameters[1] = (ALIGN_TYPE) entry_input;
    extra_parameters[2] = (ALIGN_TYPE) stack_start;
    extra_parameters[3] = (ALIGN_TYPE) stack_size;
    extra_parameters[4] = (ALIGN_TYPE) priority;
    extra_parameters[5] = (ALIGN_TYPE) preempt_threshold;
    extra_parameters[6] = (ALIGN_TYPE) time_slice;
    extra_parameters[7] = (ALIGN_TYPE) auto_start;
    extra_parameters[8] = (ALIGN_TYPE) thread_control_block_size;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_THREAD_CREATE_CALL, (ALIGN_TYPE) thread_ptr, (ALIGN_TYPE) name_ptr, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
