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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

    SECTION `.text`:CODE:NOROOT(2)
    THUMB
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_stack_initialize                Cortex-M55/IAR    */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enters the SVC handler to initialize a secure stack.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    none                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    none                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    SVC 3                                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TX_PORT_SPECIFIC_PRE_INITIALIZATION                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      Scott Larson            Initial Version 6.1.7         */
/*  07-29-2022      Scott Larson            Modified comments and changed */
/*                                            secure stack initialization */
/*                                            macro to port-specific,     */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_thread_secure_stack_initialize(VOID)
// {
    EXPORT _tx_thread_secure_stack_initialize
_tx_thread_secure_stack_initialize:
#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
    CPSIE   i               // Enable interrupts for SVC call
    SVC     3
    CPSID   i               // Disable interrupts
#else
    MOV     r0, #0xFF       // Feature not enabled
#endif
    BX      lr
    END
