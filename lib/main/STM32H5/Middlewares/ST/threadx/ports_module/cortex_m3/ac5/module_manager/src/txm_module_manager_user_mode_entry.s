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
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

    IMPORT  _txm_module_manager_kernel_dispatch
    IMPORT  _tx_thread_current_ptr

    AREA ||.text||, CODE, READONLY, ALIGN=5
    THUMB
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_user_mode_entry               Cortex-M3/AC5     */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allows modules to enter kernel mode.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    SVC 1                                 Enter kernel mode             */
/*    SVC 2                                 Exit kernel mode              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Modules in user mode                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
// VOID   _txm_module_manager_user_mode_entry(VOID)
// {
    EXPORT  _txm_module_manager_user_mode_entry
_txm_module_manager_user_mode_entry
    SVC     1                                       // Enter kernel
    EXPORT  _txm_module_priv
_txm_module_priv
    // At this point, we are out of user mode. The original LR has been saved in the
    // thread control block. Simply call the kernel dispatch function.
    BL      _txm_module_manager_kernel_dispatch

    // Pickup the original LR value while still in privileged mode
    LDR     r2, =_tx_thread_current_ptr             // Build current thread pointer address
    LDR     r3, [r2]                                // Pickup current thread pointer
    LDR     lr, [r3, #0xA0]                         // Pickup saved LR from original call

    SVC     2                                       // Exit kernel and return to user mode
    EXPORT  _txm_module_user_mode_exit
_txm_module_user_mode_exit
    BX      lr                                      // Return to the caller
// }
    ALIGN 32
    END
