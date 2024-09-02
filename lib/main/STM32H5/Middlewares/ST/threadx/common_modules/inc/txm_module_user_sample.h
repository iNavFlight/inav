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
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    txm_module_user.h                                   PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring the Module Manager  */
/*    in specific ways. This file will have an effect only if the Module  */
/*    Manager library is built with TXM_MODULE_INCLUDE_USER_DEFINE_FILE   */
/*    defined. Note that all the defines in this file may also be made on */
/*    the command line when building Modules library and application      */
/*    objects.                                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  01-31-2022      Scott Larson            Modified comments and added   */
/*                                            CALL_NOT_USED defines,      */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_USER_H
#define TXM_MODULE_USER_H

/* Defines the kernel stack size for a module thread. The default is 512, which is
   sufficient for applications only using ThreadX, however, if other libraries are
   used i.e. FileX, NetX, etc., then this value will most likely need to be increased.  */

/* #define TXM_MODULE_KERNEL_STACK_SIZE    2048 */


/* Uncomment any of these defines to prevent modules from being able to make that system call. */

/* #define TXM_BLOCK_ALLOCATE_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_CREATE_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_DELETE_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_INFO_GET_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_BLOCK_POOL_PRIORITIZE_CALL_NOT_USED */
/* #define TXM_BLOCK_RELEASE_CALL_NOT_USED */
/* #define TXM_BYTE_ALLOCATE_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_CREATE_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_DELETE_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_INFO_GET_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_BYTE_POOL_PRIORITIZE_CALL_NOT_USED */
/* #define TXM_BYTE_RELEASE_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_CREATE_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_DELETE_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_GET_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_INFO_GET_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_SET_CALL_NOT_USED */
/* #define TXM_EVENT_FLAGS_SET_NOTIFY_CALL_NOT_USED */
/* #define TXM_MUTEX_CREATE_CALL_NOT_USED */
/* #define TXM_MUTEX_DELETE_CALL_NOT_USED */
/* #define TXM_MUTEX_GET_CALL_NOT_USED */
/* #define TXM_MUTEX_INFO_GET_CALL_NOT_USED */
/* #define TXM_MUTEX_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_MUTEX_PRIORITIZE_CALL_NOT_USED */
/* #define TXM_MUTEX_PUT_CALL_NOT_USED */
/* #define TXM_QUEUE_CREATE_CALL_NOT_USED */
/* #define TXM_QUEUE_DELETE_CALL_NOT_USED */
/* #define TXM_QUEUE_FLUSH_CALL_NOT_USED */
/* #define TXM_QUEUE_FRONT_SEND_CALL_NOT_USED */
/* #define TXM_QUEUE_INFO_GET_CALL_NOT_USED */
/* #define TXM_QUEUE_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_QUEUE_PRIORITIZE_CALL_NOT_USED */
/* #define TXM_QUEUE_RECEIVE_CALL_NOT_USED */
/* #define TXM_QUEUE_SEND_CALL_NOT_USED */
/* #define TXM_QUEUE_SEND_NOTIFY_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_CEILING_PUT_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_CREATE_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_DELETE_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_GET_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_INFO_GET_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_PRIORITIZE_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_PUT_CALL_NOT_USED */
/* #define TXM_SEMAPHORE_PUT_NOTIFY_CALL_NOT_USED */
/* #define TXM_THREAD_CREATE_CALL_NOT_USED */
/* #define TXM_THREAD_DELETE_CALL_NOT_USED */
/* #define TXM_THREAD_ENTRY_EXIT_NOTIFY_CALL_NOT_USED */
/* #define TXM_THREAD_IDENTIFY_CALL_NOT_USED */
/* #define TXM_THREAD_INFO_GET_CALL_NOT_USED */
/* #define TXM_THREAD_INTERRUPT_CONTROL_CALL_NOT_USED */
/* #define TXM_THREAD_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_THREAD_PREEMPTION_CHANGE_CALL_NOT_USED */
/* #define TXM_THREAD_PRIORITY_CHANGE_CALL_NOT_USED */
/* #define TXM_THREAD_RELINQUISH_CALL_NOT_USED */
/* #define TXM_THREAD_RESET_CALL_NOT_USED */
/* #define TXM_THREAD_RESUME_CALL_NOT_USED */
/* #define TXM_THREAD_SLEEP_CALL_NOT_USED */
/* #define TXM_THREAD_STACK_ERROR_NOTIFY_CALL_NOT_USED */
/* #define TXM_THREAD_SUSPEND_CALL_NOT_USED */
/* thread system suspend is needed in _txm_module_thread_shell_entry */
/* #define TXM_THREAD_TERMINATE_CALL_NOT_USED */
/* #define TXM_THREAD_TIME_SLICE_CHANGE_CALL_NOT_USED */
/* #define TXM_THREAD_WAIT_ABORT_CALL_NOT_USED */
/* #define TXM_TIME_GET_CALL_NOT_USED */
/* #define TXM_TIME_SET_CALL_NOT_USED */
/* #define TXM_TIMER_ACTIVATE_CALL_NOT_USED */
/* #define TXM_TIMER_CHANGE_CALL_NOT_USED */
/* #define TXM_TIMER_CREATE_CALL_NOT_USED */
/* #define TXM_TIMER_DEACTIVATE_CALL_NOT_USED */
/* #define TXM_TIMER_DELETE_CALL_NOT_USED */
/* #define TXM_TIMER_INFO_GET_CALL_NOT_USED */
/* #define TXM_TIMER_PERFORMANCE_INFO_GET_CALL_NOT_USED */
/* #define TXM_TIMER_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED */
/* #define TXM_TRACE_BUFFER_FULL_NOTIFY_CALL_NOT_USED */
/* #define TXM_TRACE_DISABLE_CALL_NOT_USED */
/* #define TXM_TRACE_ENABLE_CALL_NOT_USED */
/* #define TXM_TRACE_EVENT_FILTER_CALL_NOT_USED */
/* #define TXM_TRACE_EVENT_UNFILTER_CALL_NOT_USED */
/* #define TXM_TRACE_INTERRUPT_CONTROL_CALL_NOT_USED */
/* #define TXM_TRACE_ISR_ENTER_INSERT_CALL_NOT_USED */
/* #define TXM_TRACE_ISR_EXIT_INSERT_CALL_NOT_USED */
/* #define TXM_TRACE_USER_EVENT_INSERT_CALL_NOT_USED */
/* #define TXM_MODULE_APPLICATION_REQUEST_CALL_NOT_USED */
/* #define TXM_MODULE_OBJECT_ALLOCATE_CALL_NOT_USED */
/* #define TXM_MODULE_OBJECT_DEALLOCATE_CALL_NOT_USED */
/* #define TXM_MODULE_OBJECT_POINTER_GET_CALL_NOT_USED */
/* #define TXM_MODULE_OBJECT_POINTER_GET_EXTENDED_CALL_NOT_USED */


#endif
