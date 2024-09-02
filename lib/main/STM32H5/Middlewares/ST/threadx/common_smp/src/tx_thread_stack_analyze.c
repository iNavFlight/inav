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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_stack_analyze                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function analyzes the stack to calculate the highest stack     */
/*    pointer in the thread's stack. This can then be used to derive the  */
/*    minimum amount of stack left for any given thread.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX internal code                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_stack_analyze(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

ULONG       *stack_ptr;
ULONG       *stack_lowest;
ULONG       *stack_highest;
ULONG       size;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if the thread pointer is NULL.  */
    if (thread_ptr != TX_NULL)
    {

        /* Determine if the thread ID is invalid.  */
        if (thread_ptr -> tx_thread_id == TX_THREAD_ID)
        {

            /* Pickup the current stack variables.  */
            stack_lowest =   TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_stack_start);

            /* Determine if the pointer is null.  */
            if (stack_lowest != TX_NULL)
            {

                /* Pickup the highest stack pointer.  */
                stack_highest =  TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_stack_highest_ptr);

                /* Determine if the pointer is null.  */
                if (stack_highest != TX_NULL)
                {

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* We need to binary search the remaining stack for missing 0xEFEFEFEF 32-bit data pattern.
                       This is a best effort algorithm to find the highest stack usage. */
                    do
                    {

                        /* Calculate the size again. */
                        size =  (ULONG) (TX_ULONG_POINTER_DIF(stack_highest, stack_lowest))/((ULONG) 2);
                        stack_ptr =  TX_ULONG_POINTER_ADD(stack_lowest, size);

                        /* Determine if the pattern is still there.  */
                        if (*stack_ptr != TX_STACK_FILL)
                        {

                            /* Update the stack highest, since we need to look in the upper half now.  */
                            stack_highest =  stack_ptr;
                        }
                        else
                        {

                            /* Update the stack lowest, since we need to look in the lower half now.  */
                            stack_lowest =  stack_ptr;
                        }

                    } while(size > ((ULONG) 1));

                    /* Position to first used word - at this point we are within a few words.  */
                    while (*stack_ptr == TX_STACK_FILL)
                    {

                        /* Position to next word in stack.  */
                        stack_ptr =  TX_ULONG_POINTER_ADD(stack_ptr, 1);
                    }

                    /* Optional processing extension.  */
                    TX_THREAD_STACK_ANALYZE_EXTENSION

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* Check to see if the thread is still created.  */
                    if (thread_ptr -> tx_thread_id == TX_THREAD_ID)
                    {

                        /* Yes, thread is still created.  */

                        /* Now check the new highest stack pointer is past the stack start.  */
                        if (stack_ptr > (TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_stack_start)))
                        {

                            /* Yes, now check that the new highest stack pointer is less than the previous highest stack pointer.  */
                            if (stack_ptr < (TX_VOID_TO_ULONG_POINTER_CONVERT(thread_ptr -> tx_thread_stack_highest_ptr)))
                            {

                                /* Yes, is the current highest stack pointer pointing at used memory?  */
                                if (*stack_ptr != TX_STACK_FILL)
                                {

                                    /* Yes, setup the highest stack usage.  */
                                    thread_ptr -> tx_thread_stack_highest_ptr =  stack_ptr;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE
}

