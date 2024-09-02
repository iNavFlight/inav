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
#ifndef TXM_TIMER_CREATE_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_timer_create                                   PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the create application timer     */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*    name_ptr                          Pointer to timer name             */
/*    expiration_function               Application expiration function   */
/*    initial_ticks                     Initial expiration ticks          */
/*    reschedule_ticks                  Reschedule ticks                  */
/*    auto_activate                     Automatic activation flag         */
/*    timer_control_block_size          Size of timer control block       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TIMER_ERROR                    Invalid timer control block       */
/*    TX_TICK_ERROR                     Invalid initial expiration count  */
/*    TX_ACTIVATE_ERROR                 Invalid timer activation option   */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    status                            Actual completion status          */
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
UINT _txe_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr, VOID (*expiration_function)(ULONG), ULONG expiration_input, ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate, UINT timer_control_block_size)
{

UINT return_value;
ALIGN_TYPE extra_parameters[6];

    extra_parameters[0] = (ALIGN_TYPE) expiration_function;
    extra_parameters[1] = (ALIGN_TYPE) expiration_input;
    extra_parameters[2] = (ALIGN_TYPE) initial_ticks;
    extra_parameters[3] = (ALIGN_TYPE) reschedule_ticks;
    extra_parameters[4] = (ALIGN_TYPE) auto_activate;
    extra_parameters[5] = (ALIGN_TYPE) timer_control_block_size;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_TIMER_CREATE_CALL, (ALIGN_TYPE) timer_ptr, (ALIGN_TYPE) name_ptr, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
