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

    .global     _txm_module_manager_kernel_dispatch
    .global     _txm_system_mode_enter
    .global     _txm_system_mode_exit
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_user_mode_entry             Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
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
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
    .text
    .align 12
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  _txm_module_manager_user_mode_entry
    .type    _txm_module_manager_user_mode_entry, "function"
_txm_module_manager_user_mode_entry:
_txm_system_mode_enter:
    SVC     1                               // Get out of user mode
_txm_module_priv:
    // At this point, we are in system mode.
    // Save LR (and r3 for 8 byte aligned stack) and call the kernel dispatch function.
    PUSH    {r3, lr}
    BL      _txm_module_manager_kernel_dispatch
    POP     {r3, lr}

    .global _txm_system_mode_exit
_txm_system_mode_exit:
    // Trap to restore user mode while inside of ThreadX
    SVC     2

    BX      lr                              // Return to the caller
    NOP
    NOP

    // Fill up 4kB page.
    .align 12
_txm_module_manager_user_mode_end:
