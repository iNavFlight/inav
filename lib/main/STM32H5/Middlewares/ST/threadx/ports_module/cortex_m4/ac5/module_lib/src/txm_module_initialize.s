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

    IMPORT      __use_two_region_memory
    IMPORT      __scatterload
    IMPORT      txm_heap

    AREA ||.text||, CODE, READONLY
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_initialize                            Cortex-M4/AC5     */
/*                                                           6.1.9        */
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
/*                                                                        */
/**************************************************************************/
// VOID   _txm_module_initialize(VOID)

    EXPORT  _txm_module_initialize
_txm_module_initialize
    PUSH    {r4-r12,lr}                         // Save dregs and LR

    B       __scatterload                       // Call ARM func to initialize variables


/* Override __rt_exit function. */

    EXPORT  __rt_exit
__rt_exit

    POP     {r4-r12,lr}                         // Restore dregs and LR
    BX      lr                                  // Return to caller

    EXPORT __user_setup_stackheap
    // returns heap start address in  R0
    // returns heap end address in    R2
    // does not touch SP, it is already set up before the module runs

__user_setup_stackheap
    LDR     r1, _tx_heap_offset                 // load heap offset
    ADD     r0, r9, r1                          // calculate heap base address
    MOV     r2, #TXM_MODULE_HEAP_SIZE           // load heap size
    ADD     r2, r2, r0                          // calculate heap end address
    BX      lr

    ALIGN 4
_tx_heap_offset
    DCDO        txm_heap
    AREA ||.arm_vfe_header||, DATA, READONLY, NOALLOC, ALIGN=2

    IMPORT txm_heap [DATA]


// Dummy main function

    AREA section_main, CODE, READONLY, ALIGN=2
    EXPORT main
main
    BX lr

    END
