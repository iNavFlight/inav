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
/**   Execution Profile Kit                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#ifndef TX_EXECUTION_PROFILE_H
#define TX_EXECUTION_PROFILE_H


/*  The thread execution profile kit is designed to track thread execution time 
    based on the hardware timer defined by TX_EXECUTION_TIME_SOURCE and 
    TX_EXECUTION_MAX_TIME_SOURCE below. When the thread's total time reaches 
    the maximum value, it remains there until the time is reset to 0 via a call
    to tx_thread_execution_time_reset. There are several assumptions to the 
    operation of this kit, as follows:

    1.  The TX_EXECUTION_TIME_SOURCE and TX_EXECUTION_MAX_TIME_SOURCE macros are 
        defined to utilize a local hardware time source.

    2.  The following routines are called from assembly code:
            VOID  _tx_execution_thread_enter(void);
            VOID  _tx_execution_thread_exit(void);
            VOID  _tx_execution_isr_enter(void);
            VOID  _tx_execution_isr_exit(void);

    3.  The ThreadX library must be rebuilt with TX_EXECUTION_PROFILE_ENABLE so 
        the assembly code macros are enabled to call the execution profile routines.  

    4.  Add tx_execution_profile.c to the application build.  */

/* Define the basic time typedefs for 64-bit accumulation and a 32-bit timer source, which is the
   most common configuration.  */

typedef unsigned long long              EXECUTION_TIME;
typedef unsigned long                   EXECUTION_TIME_SOURCE_TYPE;
/* For 64-bit time source, the typedef would be:  */
/* typedef unsigned long long              EXECUTION_TIME_SOURCE_TYPE;  */

/* Define basic constants for the execution profile kit.  */

/*  Example for Cortex-M targets:  */
#ifndef TX_EXECUTION_TIME_SOURCE
#define TX_EXECUTION_TIME_SOURCE         (EXECUTION_TIME_SOURCE_TYPE) *((volatile ULONG *) 0xE0001004)
#endif
#ifndef TX_EXECUTION_MAX_TIME_SOURCE
#define TX_EXECUTION_MAX_TIME_SOURCE     0xFFFFFFFF
#endif

/* For 64-bit time source, the constant would be:  */
/*#define TX_EXECUTION_TIME_SOURCE         (EXECUTION_TIME_SOURCE_TYPE) *((volatile unsigned long long *) 0xE0001004)  */
/*#define TX_EXECUTION_MAX_TIME_SOURCE     0xFFFFFFFFFFFFFFFF  */


/* Define APIs of the execution profile kit.  */

struct TX_THREAD_STRUCT;
VOID  _tx_execution_initialize(void);
VOID  _tx_execution_thread_enter(void);
VOID  _tx_execution_thread_exit(void);
VOID  _tx_execution_isr_enter(void);
VOID  _tx_execution_isr_exit(void);
UINT  _tx_execution_thread_time_reset(struct TX_THREAD_STRUCT *thread_ptr);
UINT  _tx_execution_thread_total_time_reset(void);
UINT  _tx_execution_isr_time_reset(void);
UINT  _tx_execution_idle_time_reset(void);
UINT  _tx_execution_thread_time_get(struct TX_THREAD_STRUCT *thread_ptr, EXECUTION_TIME *total_time);
UINT  _tx_execution_thread_total_time_get(EXECUTION_TIME *total_time);
UINT  _tx_execution_isr_time_get(EXECUTION_TIME *total_time);
UINT  _tx_execution_idle_time_get(EXECUTION_TIME *total_time);

#endif
