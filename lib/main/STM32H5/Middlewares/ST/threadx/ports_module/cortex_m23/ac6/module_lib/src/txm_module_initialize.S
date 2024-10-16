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
/*    _txm_module_initialize                            Cortex-M23/AC6    */
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
/*  04-02-2021      Scott Larson            Initial Version 6.1.6         */
/*  01-31-2022      Scott Larson            Modified comments, fixed      */
/*                                            scatterload, and made       */
/*                                            heap user configurable,     */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
// VOID   _txm_module_initialize(VOID)
    .global  _txm_module_initialize
    .thumb_func
_txm_module_initialize:
    PUSH    {r0-r3}                             // Save r0-r3
    MOV     r0, r8                              // Copy r8-r11
    MOV     r1, r9
    MOV     r2, r10
    MOV     r3, r11
    PUSH    {r0-r7}                             // Save r4-r11
    MOV     r0, r12
    MOV     r1, lr
    PUSH    {r0-r1}                             // Save r12, lr

    B       __scatterload                       // Call ARM func to initialize variables

// Override the __rt_exit function.
    .global  __rt_exit
    .thumb_func
__rt_exit:
    POP     {r0-r1}                             // Restore r4-r12 and LR
    MOV     r12, r0
    MOV     lr, r1
    POP     {r0-r7}
    MOV     r8, r0
    MOV     r9, r1
    MOV     r10, r2
    MOV     r11, r3
    BX      lr                                  // Return to caller

// Override the __rt_entry function.
    .global __rt_entry
    .type   __rt_entry, %function
__rt_entry:
    MOV     r0,sp
    ADDS    r0,r0,#40
    LDM     r0,{r0-r1}
    BL      __rt_lib_init                       // Call ARM func to initialize library
    POP     {r0-r1}                             // Restore dregs and LR
    MOV     r12, r0                             
    MOV     lr, r1
    POP     {r0-r7}
    MOV     r8, r0
    MOV     r9, r1
    MOV     r10, r2
    MOV     r11, r3
    POP     {r0-r3}
    BX      lr                                  // Return to caller
