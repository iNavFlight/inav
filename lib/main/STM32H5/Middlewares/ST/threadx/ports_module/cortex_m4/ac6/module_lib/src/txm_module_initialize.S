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

    .global     __use_two_region_memory
    .global     __scatterload

    .eabi_attribute Tag_ABI_PCS_RO_data, 1
    .eabi_attribute Tag_ABI_PCS_R9_use,  1
    .eabi_attribute Tag_ABI_PCS_RW_data, 2

    .text
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_initialize                            Cortex-M4/AC6     */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the module c runtime.                     */
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
/*    __scatterload                         Initialize C runtime          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_thread_shell_entry        Start module thread           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*  01-31-2022      Scott Larson            Modified comments and made    */
/*                                            heap user configurable,     */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
// VOID   _txm_module_initialize(VOID)
    .global  _txm_module_initialize
    .thumb_func
_txm_module_initialize:
    PUSH    {r0-r12,lr}                         // Save dregs and LR
    B       __scatterload                       // Call ARM func to initialize variables

// Override the __rt_exit function.
    .global  __rt_exit
    .thumb_func
__rt_exit:
    POP     {r4-r12,lr}                         // Restore dregs and LR
    BX      lr                                  // Return to caller

    .global __rt_entry
    .type   __rt_entry, %function
__rt_entry:
    POP     {r0-r1}
    BL      __rt_lib_init
    POP     {r2-r12,lr}                         // Restore dregs and LR
    BX      lr                                  // Return to caller
