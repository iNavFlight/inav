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
/**   ThreadX Component                                                   */
/**                                                                       */
/**   POSIX Compliancy Wrapper (POSIX)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  EKP DEFINITIONS                                        RELEASE        */
/*                                                                        */
/*    px_int.h                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc.needed to          */
/*    implement the Evacuation Kit for POSIX Users (POSIX)                */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Remove unneeded values,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef _TX_PX_INT_H
#define _TX_PX_INT_H

#include  "string.h"

/* Define several ThreadX equates for use inside of POSIX. */
#define  TX_INITIALIZE_IN_PROGRESS      0xF0F0F0F0UL
#define  TX_INITIALIZE_ALMOST_DONE      0xF0F0F0F1UL

/* Threadx min and max priority */
#define TX_HIGHEST_PRIORITY                 1
#define TX_LOWEST_PRIORITY                  31

#define PX_HIGHEST_PRIORITY                 31
#define PX_LOWEST_PRIORITY                  1

/************ Extern Variables **************/
extern TX_THREAD *     _tx_thread_current_ptr;
extern TX_THREAD *     _tx_thread_execute_ptr;



/* declares posix objects in the px_pth_init.c file */
#ifdef  PX_OBJECT_INIT
#define PX_OBJECT_DECLARE 
#else
#define PX_OBJECT_DECLARE extern
#endif
/**************************************************************************/
/*                             Global Definitions                         */
/**************************************************************************/

/* The ThreadX work queue for the POSIX System Manager.    */
PX_OBJECT_DECLARE TX_QUEUE                     posix_work_queue;

/* Define the thread for the System Manager functionality.  */
PX_OBJECT_DECLARE TX_THREAD                    posix_system_manager;


/**************************************************************************/
/*                       Local definitions                                 */
/**************************************************************************/

/* Define a byte pool control block for the heap memory used
   by POSIX.  */

PX_OBJECT_DECLARE TX_BYTE_POOL          posix_heap_byte_pool;

/* Define a static pool of pthread Control Blocks (TCB). If more pthreades are
   required the constant PTHREAD_THREADS_MAX (in tx_posix.h) may be modified.   */ 
PX_OBJECT_DECLARE POSIX_TCB             ptcb_pool[PTHREAD_THREADS_MAX];

#if POSIX_MAX_QUEUES!= 0
/* Define a static pool of queues.If more pthread message queues are required
   the constant POSIX_MAX_QUEUES (in tx_posix.h) may be modified.               */
PX_OBJECT_DECLARE POSIX_MSG_QUEUE       posix_queue_pool[POSIX_MAX_QUEUES];
/* Define a queue attribute.            */
PX_OBJECT_DECLARE struct mq_attr        posix_qattr_default;
#endif

#if  SEM_NSEMS_MAX != 0
/* Define a static pool of semaphores. If more pthread semaphores required the
   constant SEM_NSEMS_MAX (in tx_posix.h) may be modified.                      */
PX_OBJECT_DECLARE sem_t                 posix_sem_pool[SEM_NSEMS_MAX];

#endif

/* Define a default pthread attribute.  */
PX_OBJECT_DECLARE pthread_attr_t        posix_default_pthread_attr;

/* Define a default mutex attribute.    */
PX_OBJECT_DECLARE pthread_mutexattr_t   posix_default_mutex_attr;



/* Define a temporary posix errno.  */

PX_OBJECT_DECLARE unsigned int posix_errno;

   
/**************************************************************************/
/*                      Local prototypes                                  */
/**************************************************************************/
/* Define Evacuation Kit for POSIX function prototypes.  */ 

INT                   posix_get_pthread_errno(pthread_t ptid);

POSIX_MSG_QUEUE      *posix_mq_create ( const CHAR * mq_name,
                                                struct mq_attr * msgq_attr);

POSIX_MSG_QUEUE      *posix_find_queue(const CHAR * mq_name);

POSIX_MSG_QUEUE      *posix_get_new_queue(ULONG maxnum);

ULONG                 posix_arrange_msg( TX_QUEUE * Queue, ULONG * pMsgPrio );

ULONG                 posix_priority_search(mqd_t msgQId ,ULONG priority);

VOID                  posix_queue_init(VOID);

VOID                  posix_qattr_init(VOID);

VOID                  posix_pthread_init(VOID);

struct mq_des        *posix_get_queue_des(POSIX_MSG_QUEUE * q_ptr);

VOID                  posix_reset_queue(POSIX_MSG_QUEUE * q_ptr);

VOID                  posix_memory_release(VOID * memory_ptr);

VOID                  posix_internal_error(ULONG error_code);

VOID                  posix_error_handler(ULONG error_code);

INT                   posix_memory_allocate(ULONG size, VOID **memory_ptr);
 
INT                   posix_queue_delete(POSIX_MSG_QUEUE  * q_ptr);

VOID                  posix_putback_queue(TX_QUEUE * qid);

sem_t                *posix_find_sem(const CHAR * name);

VOID                  posix_set_sem_name(sem_t * sem, CHAR *name);

TX_SEMAPHORE         *posix_get_new_sem(VOID);

VOID                  posix_sem_reset(sem_t *sem);

ULONG                 posix_in_thread_context(VOID);

VOID                  posix_reset_pthread_t(POSIX_TCB *ptcb);

TX_THREAD            *posix_tid2thread(pthread_t ptid);

POSIX_TCB            *posix_tid2tcb(pthread_t ptid);

pthread_t             posix_thread2tid(TX_THREAD *thread_ptr);

POSIX_TCB            *posix_thread2tcb(TX_THREAD *thread_ptr);

TX_THREAD            *posix_tcb2thread (POSIX_TCB *pthread_ptr);

VOID                  posix_thread_wrapper(ULONG pthr_ptr);

VOID                  set_default_pthread_attr(pthread_attr_t *attr);

VOID                  set_default_mutexattr(pthread_mutexattr_t *attr);

INT                   posix_allocate_pthread_t(POSIX_TCB **ptcb_ptr);

VOID                  posix_copy_pthread_attr(POSIX_TCB *pthread_ptr,pthread_attr_t *attr);

VOID                  posix_destroy_pthread(POSIX_TCB *pthread_ptr, VOID *value_ptr);

VOID                  posix_do_pthread_delete(POSIX_TCB *pthread_ptr, VOID *value_ptr);

VOID                  posix_system_manager_entry(ULONG input);

INT                   posix_set_pthread_errno(ULONG errno_set);

ULONG                 posix_abs_time_to_rel_ticks(struct timespec *abs_timeout);

#endif
