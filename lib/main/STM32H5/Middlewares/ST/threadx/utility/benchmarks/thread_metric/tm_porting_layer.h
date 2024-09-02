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

#ifndef TM_PORTING_LAYER_H
#define TM_PORTING_LAYER_H

#include <stdio.h>


/* Define the TRAP instruction. This is used by the Interrupt Processing and Interrupt Preemption Processing tests. 
   The SVC instruction below is for Cortex-M architectures using IAR tools. This will likely need to be modified 
   for different processors and/or development tools. 

   Note also that for the Interrupt Processing test there is the assumption that the SVC ISR looks like:

    PUBLIC  SVC_Handler
SVC_Handler:
    PUSH    {lr}
    BL      tm_interrupt_handler
    POP     {lr}
    BX      LR

    And that for the Interrupt Preemption Processing test the SVC ISR looks like:

    PUBLIC  SVC_Handler
SVC_Handler:
    PUSH    {lr}
    BL      tm_interrupt_preemption_handler
    POP     {lr}
    BX      LR

   Again, this is very processor/tool specific so changes are likely needed for non Cortex-M/IAR
   environments.  */

#define TM_CAUSE_INTERRUPT    asm("SVC #0");



#endif

