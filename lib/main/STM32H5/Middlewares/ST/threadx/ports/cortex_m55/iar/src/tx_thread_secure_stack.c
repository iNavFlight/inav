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


#include "tx_api.h"

/* If TX_SINGLE_MODE_SECURE or TX_SINGLE_MODE_NON_SECURE is defined,
   no secure stack functionality is needed. */
#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)

#define TX_SOURCE_CODE

#include <cmsis_compiler.h>             /* For intrinsic functions. */
#include "tx_secure_interface.h"        /* Interface for NS code. */

/* Minimum size of secure stack. */
#ifndef TX_THREAD_SECURE_STACK_MINIMUM
#define TX_THREAD_SECURE_STACK_MINIMUM     256
#endif
/* Maximum size of secure stack. */
#ifndef TX_THREAD_SECURE_STACK_MAXIMUM
#define TX_THREAD_SECURE_STACK_MAXIMUM     1024
#endif

/* 8 bytes added to stack size to "seal" stack. */
#define TX_THREAD_STACK_SEAL_SIZE           8
#define TX_THREAD_STACK_SEAL_VALUE          0xFEF5EDA5

/* max number of Secure context */
#ifndef TX_MAX_SECURE_CONTEXTS
#define TX_MAX_SECURE_CONTEXTS              32
#endif
#define TX_INVALID_SECURE_CONTEXT_IDX       (-1)

/* Secure stack info struct to hold stack start, stack limit,
   current stack pointer, and pointer to owning thread.
   This will be allocated for each thread with a secure stack. */
typedef struct TX_THREAD_SECURE_STACK_INFO_STRUCT
{
    VOID        *tx_thread_secure_stack_ptr;        /* Thread's secure stack current pointer */
    VOID        *tx_thread_secure_stack_start;      /* Thread's secure stack start address */
    VOID        *tx_thread_secure_stack_limit;      /* Thread's secure stack limit */
    TX_THREAD   *tx_thread_ptr;                     /* Keep track of thread for error handling */
    INT          tx_next_free_index;                /* Next free index of free secure context */
} TX_THREAD_SECURE_STACK_INFO;

/* Static secure contexts */
static TX_THREAD_SECURE_STACK_INFO tx_thread_secure_context[TX_MAX_SECURE_CONTEXTS];
/* Head of free secure context */
static INT tx_head_free_index = 0U;



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_mode_stack_initialize           Cortex-M55/IAR    */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes secure mode to use PSP stack.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    __get_CONTROL                         Intrinsic to get CONTROL      */
/*    __set_CONTROL                         Intrinsic to set CONTROL      */
/*    __set_PSPLIM                          Intrinsic to set PSP limit    */
/*    __set_PSP                             Intrinsic to set PSP          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  10-16-2020      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.1  */
/*  06-02-2021      Scott Larson            Change name, execute in       */
/*                                            handler mode,               */
/*                                            resulting in version 6.1.7  */
/*  01-31-2022      Himanshu Gupta          Modified comments(s), updated */
/*                                            secure stack allocation,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
__attribute__((cmse_nonsecure_entry))
UINT    _tx_thread_secure_mode_stack_initialize(void)
{
UINT    status;
INT     index;

    /* Make sure function is called from interrupt (threads should not call). */
    if (__get_IPSR() == 0)
    {
        status = TX_CALLER_ERROR;
    }
    else
    {
        /* Set secure mode to use PSP. */
        __set_CONTROL(__get_CONTROL() | 2);

        /* Set process stack pointer and stack limit to 0 to throw exception when a thread
           without a secure stack calls a secure function that tries to use secure stack. */
        __set_PSPLIM(0);
        __set_PSP(0);

        for (index = 0; index < TX_MAX_SECURE_CONTEXTS; index++)
        {

            /* Check last index and mark next free to invalid index */
            if(index == (TX_MAX_SECURE_CONTEXTS - 1))
            {
                tx_thread_secure_context[index].tx_next_free_index = TX_INVALID_SECURE_CONTEXT_IDX;
            }
            else
            {
                tx_thread_secure_context[index].tx_next_free_index = index + 1;
            }
        }

        status = TX_SUCCESS;
    }
    return status;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_mode_stack_allocate             Cortex-M55/IAR    */
/*                                                           6.1.11a      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a thread's secure stack.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*    stack_size                            Size of stack to allocates    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                       Invalid thread pointer        */
/*    TX_SIZE_ERROR                         Invalid stack size            */
/*    TX_CALLER_ERROR                       Invalid caller of function    */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    __get_IPSR                            Intrinsic to get IPSR         */
/*    malloc                                Compiler's malloc function    */
/*    __set_PSPLIM                          Intrinsic to set PSP limit    */
/*    __set_PSP                             Intrinsic to set PSP          */
/*    __TZ_get_PSPLIM_NS                    Intrinsic to get NS PSP       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    SVC Handler                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  10-16-2020      Scott Larson            Modified comment(s),          */
/*                                            added stack sealing,        */
/*                                            resulting in version 6.1.1  */
/*  01-31-2022      Himanshu Gupta          Modified comments(s), updated */
/*                                            secure stack allocation,    */
/*                                            resulting in version 6.1.10 */
/*  05-02-2022      Scott Larson            Modified comment(s), added    */
/*                                            TX_INTERRUPT_SAVE_AREA,     */
/*                                            resulting in version 6.1.11a*/
/*                                                                        */
/**************************************************************************/
__attribute__((cmse_nonsecure_entry))
UINT    _tx_thread_secure_mode_stack_allocate(TX_THREAD *thread_ptr, ULONG stack_size)
{
TX_INTERRUPT_SAVE_AREA
UINT    status;
TX_THREAD_SECURE_STACK_INFO *info_ptr;
UCHAR   *stack_mem;
INT     secure_context_index;

    status = TX_SUCCESS;

    /* Make sure function is called from interrupt (threads should not call). */
    if (__get_IPSR() == 0)
    {
        status = TX_CALLER_ERROR;
    }
    else if (stack_size < TX_THREAD_SECURE_STACK_MINIMUM || stack_size > TX_THREAD_SECURE_STACK_MAXIMUM)
    {
        status = TX_SIZE_ERROR;
    }

    /* Check if thread already has secure stack allocated. */
    else if (thread_ptr -> tx_thread_secure_stack_context != 0)
    {
        status = TX_THREAD_ERROR;
    }

    else
    {
        TX_DISABLE

        /* Allocate free index for secure stack info. */
        if(tx_head_free_index != TX_INVALID_SECURE_CONTEXT_IDX)
        {
            secure_context_index = tx_head_free_index;
            tx_head_free_index = tx_thread_secure_context[tx_head_free_index].tx_next_free_index;
            tx_thread_secure_context[secure_context_index].tx_next_free_index = TX_INVALID_SECURE_CONTEXT_IDX;
        }
        else
        {
            secure_context_index = TX_INVALID_SECURE_CONTEXT_IDX;
        }

        TX_RESTORE

        if(secure_context_index != TX_INVALID_SECURE_CONTEXT_IDX)
        {
            info_ptr = &tx_thread_secure_context[secure_context_index];

            /* If stack info allocated, allocate a stack & seal. */
            stack_mem = malloc(stack_size + TX_THREAD_STACK_SEAL_SIZE);

            if(stack_mem != TX_NULL)
            {
                /* Secure stack has been allocated, save in the stack info struct. */
                info_ptr -> tx_thread_secure_stack_limit = stack_mem;
                info_ptr -> tx_thread_secure_stack_start = stack_mem + stack_size;
                info_ptr -> tx_thread_secure_stack_ptr = info_ptr -> tx_thread_secure_stack_start;
                info_ptr -> tx_thread_ptr = thread_ptr;

                /* Seal bottom of stack. */
                *(ULONG*)info_ptr -> tx_thread_secure_stack_start = TX_THREAD_STACK_SEAL_VALUE;

                /* Save secure context id (i.e non-zero base index) in thread. */
                thread_ptr -> tx_thread_secure_stack_context = (VOID *)(secure_context_index + 1);

                /* Check if this thread is running by looking at its stack start and PSPLIM_NS */
                if(((ULONG) thread_ptr -> tx_thread_stack_start & 0xFFFFFFF8) == __TZ_get_PSPLIM_NS())
                {
                    /* If this thread is running, set Secure PSP and PSPLIM. */
                    __set_PSPLIM((ULONG)(info_ptr -> tx_thread_secure_stack_limit));
                    __set_PSP((ULONG)(info_ptr -> tx_thread_secure_stack_ptr));
                }
            }

            else
            {
                TX_DISABLE

                /* Stack not allocated, free the info struct. */
                tx_thread_secure_context[secure_context_index].tx_next_free_index = tx_head_free_index;
                tx_head_free_index = secure_context_index;
                TX_RESTORE

                status = TX_NO_MEMORY;
            }
        }

        else
        {
            status = TX_NO_MEMORY;
        }
    }

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_mode_stack_free                 Cortex-M55/IAR    */
/*                                                           6.1.11a      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function frees a thread's secure stack.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                       Invalid thread pointer        */
/*    TX_CALLER_ERROR                       Invalid caller of function    */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    __get_IPSR                            Intrinsic to get IPSR         */
/*    free                                  Compiler's free() function    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    SVC Handler                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  10-16-2020      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.1  */
/*  01-31-2022      Himanshu Gupta          Modified comments(s), updated */
/*                                            secure stack allocation,    */
/*                                            resulting in version 6.1.10 */
/*  05-02-2022      Scott Larson            Modified comment(s), added    */
/*                                            TX_INTERRUPT_SAVE_AREA,     */
/*                                            resulting in version 6.1.11a*/
/*                                                                        */
/**************************************************************************/
__attribute__((cmse_nonsecure_entry))
UINT    _tx_thread_secure_mode_stack_free(TX_THREAD *thread_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT    status;
TX_THREAD_SECURE_STACK_INFO *info_ptr;
INT     secure_context_index;

    status = TX_SUCCESS;

    /* Pickup stack info id from thread. */
    secure_context_index = (INT)thread_ptr -> tx_thread_secure_stack_context - 1;

    /* Make sure function is called from interrupt (threads should not call). */
    if (__get_IPSR() == 0)
    {
        status = TX_CALLER_ERROR;
    }

    /* Check if secure context index is in valid range. */
    else if (secure_context_index < 0 || secure_context_index >= TX_MAX_SECURE_CONTEXTS)
    {
        status = TX_THREAD_ERROR;
    }
    else
    {

        /* Pickup stack info from static array of secure contexts. */
        info_ptr = &tx_thread_secure_context[secure_context_index];

        /* Check that this secure context is for this thread. */
        if (info_ptr -> tx_thread_ptr != thread_ptr)
        {
            status = TX_THREAD_ERROR;
        }

        else
        {

            /* Free secure stack. */
            free(info_ptr -> tx_thread_secure_stack_limit);

            TX_DISABLE

            /* Free info struct. */
            tx_thread_secure_context[secure_context_index].tx_next_free_index = tx_head_free_index;
            tx_head_free_index = secure_context_index;
            TX_RESTORE

            /* Clear secure context from thread. */
            thread_ptr -> tx_thread_secure_stack_context = 0;
        }
    }

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_stack_context_save              Cortex-M55/IAR    */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function saves context of the secure stack.                    */
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
/*    __get_IPSR                            Intrinsic to get IPSR         */
/*    __get_PSP                             Intrinsic to get PSP          */
/*    __set_PSPLIM                          Intrinsic to set PSP limit    */
/*    __set_PSP                             Intrinsic to set PSP          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    PendSV Handler                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  10-16-2020      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.1  */
/*  06-02-2021      Scott Larson            Fix stack pointer save,       */
/*                                            resulting in version 6.1.7  */
/*  01-31-2022      Himanshu Gupta          Modified comments(s), updated */
/*                                            secure stack allocation,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
__attribute__((cmse_nonsecure_entry))
void _tx_thread_secure_stack_context_save(TX_THREAD *thread_ptr)
{
TX_THREAD_SECURE_STACK_INFO *info_ptr;
ULONG   sp;
INT secure_context_index = (INT)thread_ptr -> tx_thread_secure_stack_context - 1;

    /* This function should be called from scheduler only. */
    if (__get_IPSR() == 0)
    {
        return;
    }

    /* Check if secure context index is in valid range. */
    else if (secure_context_index < 0 || secure_context_index >= TX_MAX_SECURE_CONTEXTS)
    {
        return;
    }

    /* Pickup the secure context pointer. */
    info_ptr = &tx_thread_secure_context[secure_context_index];

    /* Check that this secure context is for this thread. */
    if (info_ptr -> tx_thread_ptr != thread_ptr)
    {
        return;
    }

    /* Check that stack pointer is in range */
    sp = __get_PSP();
    if ((sp < (ULONG)info_ptr -> tx_thread_secure_stack_limit) ||
        (sp > (ULONG)info_ptr -> tx_thread_secure_stack_start))
    {
        return;
    }

    /* Save stack pointer. */
    info_ptr -> tx_thread_secure_stack_ptr = (VOID *) sp;

    /* Set process stack pointer and stack limit to 0 to throw exception when a thread
       without a secure stack calls a secure function that tries to use secure stack. */
    __set_PSPLIM(0);
    __set_PSP(0);

    return;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_stack_context_restore           Cortex-M55/IAR    */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function restores context of the secure stack.                 */
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
/*    __get_IPSR                            Intrinsic to get IPSR         */
/*    __set_PSPLIM                          Intrinsic to set PSP limit    */
/*    __set_PSP                             Intrinsic to set PSP          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    PendSV Handler                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  10-16-2020      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.1  */
/*  01-31-2022      Himanshu Gupta          Modified comments(s), updated */
/*                                            secure stack allocation,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
__attribute__((cmse_nonsecure_entry))
void _tx_thread_secure_stack_context_restore(TX_THREAD *thread_ptr)
{
TX_THREAD_SECURE_STACK_INFO *info_ptr;
INT secure_context_index = (INT)thread_ptr -> tx_thread_secure_stack_context - 1;

    /* This function should be called from scheduler only. */
    if (__get_IPSR() == 0)
    {
        return;
    }

    /* Check if secure context index is in valid range. */
    else if (secure_context_index < 0 || secure_context_index >= TX_MAX_SECURE_CONTEXTS)
    {
        return;
    }

    /* Pickup the secure context pointer. */
    info_ptr = &tx_thread_secure_context[secure_context_index];

    /* Check that this secure context is for this thread. */
    if (info_ptr -> tx_thread_ptr != thread_ptr)
    {
        return;
    }

    /* Set stack pointer and limit. */
    __set_PSPLIM((ULONG)info_ptr -> tx_thread_secure_stack_limit);
    __set_PSP   ((ULONG)info_ptr -> tx_thread_secure_stack_ptr);

    return;
}

#endif
