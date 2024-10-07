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
/*    tx_posix.h                                          PORTABLE C      */
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
/*  10-31-2022      Scott Larson            Update WORK_REQ_SIZE value,   */
/*                                            update pthread_t typedef,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef TX_POSIX
#define TX_POSIX

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/************************************************************************/
/* Macros to convert between semaphore, queue, scheduler                */
/************************************************************************/
#define  MAKE_TX_SEM(sem)             ((TX_SEMAPHORE *)sem)
#define  MAKE_POSIX_QUEUE(queue)      ((POSIX_MSG_QUEUE *)queue)
#define  MAKE_POSIX_SEM(sem)          ((sem_t *)sem)

/************************************************************************/
/* Define max values for message queue services                         */
/************************************************************************/
#define  MQ_MAXMSG                      125             /* MQ_MAXMSG 1024 (POSIX value).   */
#define  MQ_MSGSIZE                     500             /* MQ_MSGSIZE 4096 (POSIX value)   */
#define  MQ_FLAGS                       0
#define  MQ_PRIO_MAX                    32              /* Maximum priority of message.    */

#ifdef TX_64_BIT
#define TX_POSIX_MESSAGE_SIZE           5
#define TX_POSIX_QUEUE_PRIORITY_OFFSET  3
#else
#define TX_POSIX_MESSAGE_SIZE           4
#define TX_POSIX_QUEUE_PRIORITY_OFFSET  2
#endif
/************************************************************************/
/*                          Global Variables                            */
/************************************************************************/

/* to errno.h
#ifndef TX_POSIX_SOURCE
#define errno posix_errno
#endif
*/

/* Define the system configuration constants for the Evacuation Kit for 
   POSIX Users.This is where the number of system objects 
   (pthreads, message queues, semaphores etc.)are defined.              */

/************************************************************************/
/*               SYSTEM CONFIGURATION PARAMETERS                        */
/************************************************************************/

/* Define the maximum number of simultaneous POSIX semaphores 
    supported.  */
#define  SEM_NSEMS_MAX                  16

/* Define the maximum length of name of semaphore .  */
#define  SEM_NAME_MAX                   10

/* Max value of semaphore while initialization.  */
#define  SEM_VALUE_MAX                  100

/* Define the maximum number of simultaneous POSIX message queues supported.  */
    
#define  POSIX_MAX_QUEUES               16

/* Define the maximum number of simultaneous POSIX pthreads supported.  */
#define  PTHREAD_THREADS_MAX            16

/* Define the maximum number of simultaneous POSIX mutexes supported.  */
    
#define  POSIX_MAX_MUTEX                16

/* Define the maximum length of name of message queue.  */
#define  PATH_MAX                       10


/* Define size of the posix heap memory segment.                              */
/* NOTE:  This region should be large enough to supply the memory       */
/*        for all pthread stacks, pthread control blocks in the system  */

#define  TX_DEFAULT_THREAD_STACK_SIZE   2048
#define  TX_REGION0_CONSTANT            14
#define  TX_REGION0_SIZE                ( (TX_DEFAULT_THREAD_STACK_SIZE+16) * TX_REGION0_CONSTANT)

#define  POSIX_HEAP_SIZE_IN_BYTES       (TX_REGION0_SIZE * 4)




/* Define number of CPU ticks per second */
#define  CPU_TICKS_PER_SECOND           100  /* assuming 10 mSec tick */  
#define  NANOSECONDS_IN_CPU_TICK        10000000  /* assuming 10 mSec tick */ 

/* Define queue control specific data definitions.  */

#define  TX_SEMAPHORE_ID                0x53454D41UL
#define  TX_QUEUE_ID                    0x51554555UL
#define  PX_QUEUE_ID                    0x51554555UL
#define  TX_MUTEX_ID                    0x4D555445UL

/************************************************************************/
/*             Misc. POSIX-related definitions .                        */
/************************************************************************/
#define  POSIX_STACK_PADDING            1024
#define  POSIX_SYSTEM_STACK_SIZE        1024
#define  POSIX_PTHREAD_STACK_SIZE       1024  

/************************************************************************/
/*               ARCHITECTURE DEFINITIONS                               */
/************************************************************************/
/* Define all supported architectures here.  */

#define  POSIX_POWERPC                  1
#define  POSIX_68K                      2
#define  POSIX_ARM                      3
#define  POSIX_MIPS                     4

/* Define POSIX_ARCH as one of the above list.  */

#define  POSIX_ARCH                     POSIX_POWERPC

/* Make sure POSIX_ARCH is defined.  */

#ifndef  POSIX_ARCH
#error   Must define symbol POSIX_ARCH to *something*!
#endif

/* Define the minimum stack size for each supported architecture here.  */

#define  MIN_STACKSIZE_POWERPC          2048

/************************************************************************/ 
/*               MISCELLANEOUS CONSTANTS                                */ 
/************************************************************************/ 
/* Requests/commands to SysMgr task.  */

#define   SYSMGR_DELETE_TASK            0

/* pthread name length */
#define   PTHREAD_NAME_LEN              4

#define   PTHREAD_CREATE_DETACHED       1
#define   PTHREAD_CREATE_JOINABLE       0 

/* scheduler related constants */

#define   SCHED_PRIO_MAX                31
#define   SCHED_PRIO_MIN                1

/* time slice value in ticks for round robin scheduler */
#define   SCHED_RR_TIME_SLICE           20

#define   PTHREAD_MUTEX_NORMAL          1
#define   PTHREAD_MUTEX_RECURSIVE       2
#define   PTHREAD_MUTEX_ERRORCHECK      3
#define   PTHREAD_MUTEX_DEFAULT         PTHREAD_MUTEX_RECURSIVE

#define   PTHREAD_PRIO_INHERIT          1

#define   PTHREAD_PROCESS_PRIVATE       1
#define   PTHREAD_PROCESS_SHARED        2

#define   PTHREAD_CANCEL_ENABLE         0           /* default */

#define   PTHREAD_CANCEL_DISABLE        1

#define   PTHREAD_CANCEL_DEFERRED       0          /* default */

#define   PTHREAD_CANCEL_ASYNCHRONOUS   1

#define   PTHREAD_INHERIT_SCHED         1

#define   PTHREAD_EXPLICIT_SCHED        0

#define   PTHREAD_ONCE_INIT             {0, 0, {0,NULL,0,0,NULL,0,NULL,NULL}}

enum pth_once_state {
  PTH_ONCE_INIT      = 0x0,
  PTH_ONCE_DONE      = 0x1,
  PTH_ONCE_STARTED   = 0x2,
  PTH_ONCE_CANCELLED = 0x3
};

/************************************************************************/ 
/*               ERROR CODES (those defined outside of POSIX)           */ 
/************************************************************************/ 

#ifdef   ERROR
#undef   ERROR
#define  ERROR                  -1
#else
#define  ERROR                  -1
#endif

#define NO_ERROR        0

/* From semaphore.h, when px_sem_open fails: */
#define SEM_FAILED	((sem_t *) 0)

#ifndef _WIN32
typedef  ULONG                  BOOL;
#endif


#ifndef  OK
#define  OK                     0
#endif

#ifndef  FALSE
#define  FALSE                  0
#endif

#ifndef  TRUE
#define  TRUE                   1
#endif

#ifndef  NULL
#define  NULL                   0
#endif

/* these constants control internal working of the systemmanager thread */

#define   WORK_REQ_SIZE         (TX_2_ULONG * (sizeof(ALIGN_TYPE)/sizeof(ULONG)))
#define   WORK_QUEUE_DEPTH      10

#define   SYSMGR_PRIORITY       0
#define   SYSMGR_THRESHOLD      0


/* STRUCTURES RELATED TO pthreads  */



typedef struct pthread_attr_obj
{
     ULONG                pthread_flags;
     INT                  detach_state; 
     INT                  inherit_sched;
     INT                  sched_policy;
     struct sched_param   sched_attr;
     VOID                *stack_address;
     ULONG                stack_size;
     INT                  inuse;
} pthread_attr_t; 


typedef  INT    ssize_t ;     /* this should be pulled in from sys\types.h  */
typedef  ALIGN_TYPE  pthread_t;
typedef  ULONG  mode_t;



/* Define POSIX Pthread control block tructure.  */

typedef struct pthread_control_block
{
    /* This pthread's ThreadX TCB.  */ 
    TX_THREAD            thread_info;
    /* This pthread's unique identifier */
    pthread_t            pthreadID;
    /* To check if posix Pthread is in use.  */
    UINT                 in_use;
    /* All pthread attributes contained in the a pthread_attr_t object */
    ULONG                pthread_flags;  
    INT                  detach_state;   
    INT                  inherit_sched;
    INT                  sched_policy;
    struct sched_param   sched_attr;
    VOID                *stack_address;
    ULONG                stack_size;
    INT                  cancel_state;
    INT                  cancel_type;     
    /* Identifier of the target thread to which this pthread is joined */
    pthread_t            joined_to_pthreadID;   
    /* Identifier of the caller thread which has joined to this thread*/
    pthread_t            joined_by_pthreadID;
    /* To check if posix pthread is joined to any other pthread */
        UINT             is_joined_to;
    /* To check if posix Pthread is joined by any other pthread */
        UINT             is_joined_by;
    /* To check if posix Pthread is in detached state or not */
        UINT             is_detached;
    /* Value returned by the terminating thread which is joined to this thread */
    VOID                 *value_ptr;
    /* Define the original pthread priority.  */ 
    ULONG                orig_priority;
    /* Define the current pthread priority.  */ 
    ULONG                current_priority;
    /* Define the pthread's pre-emption threshold.  */ 
    ULONG                threshold;
    /* Define the pthread's timeslice.  */ 
    ULONG                time_slice;
    /* specify pthread start routine */
    VOID                 *(*start_routine)(VOID *); 
    /* specify argument for start up routine */
    ULONG                *entry_parameter;
    /* to hold error code for this pthread */
    ULONG                perrno;
    /* to hold pthread cancel request */
    UINT                 cancel_request;

    /* Signal information follows.  */
    signal_info          signals;

}POSIX_TCB;

typedef struct pthread_mutex_attr_obj
{
     INT                 type;
     INT                 protocol;
     INT                 pshared;
     INT                 in_use;
     
} pthread_mutexattr_t;

/* Define POSIX mutex structure.  */

typedef struct pthread_mutex_control_block
{
    /* This mutex's ThreadX Control block  */ 
    TX_MUTEX      mutex_info;   
    /* This mutex's attributes */
    INT           type;
    /* Is this Mutex object is in use?  */
    INT           in_use;

} pthread_mutex_t;


/*     STRUCTURES RELATED TO POSIX MESSAGE QUEUE   */


struct mq_attr
{
    /* No. of maximum messages.  */
    ULONG         mq_maxmsg;
    /* Size of the message.  */
    ULONG         mq_msgsize;
    /* Flags are ignored as these are passed separately in open().  */
    ULONG         mq_flags;
}; 

/* Define POSIX message queue structure.  */
typedef struct msg_que
{
    /* Define ThreadX queue.  */
    TX_QUEUE                      queue;
    /* To check if posix queue is in use.  */
    UINT                          in_use;
    /* To check if queue is unlinked.  */
    UINT                          unlink_flag;
    /* Name of queue.  */
    CHAR                        * name;
    /* Attribute of queue.  */
    struct mq_attr                q_attr;
    /* To check no. of times queue is opened.  */
    UINT                          open_count;
    /* Address for variable length message.  */
    VOID                        * storage;
    /* Byte pool for variable length message.  */
    TX_BYTE_POOL                  vq_message_area;
    /* POSIX queue ID.  */
    ULONG                         px_queue_id;

}POSIX_MSG_QUEUE;

/* Define Queue Descriptor.  */
typedef struct mq_des
{
    /* Queue FLAGS.  */ 
    ULONG                         f_flag;
    /* message Queue structure.  */
    POSIX_MSG_QUEUE             * f_data;
    
} *mqd_t;


/* STRUCTURES RELATED TO POSIX SEMAPHORES  */

typedef struct POSIX_SEMAPHORE_STRUCT         
{
    /* ThreadX semaphore.  */
    TX_SEMAPHORE                  sem;
    /* To check if semaphore is in use.  */
    UINT                          in_use;
    /* semaphore identifier  */
    ULONG                         psemId;
    /* number of attachments  */
    ULONG                         refCnt;        /* previously it was int  */
     /* name of semaphore  */
    char                        * sem_name;
    /* Open Count.  */
    ULONG                         count;
    /* For unlink flag.  */
    ULONG                         unlink_flag;

} sem_t;

typedef sem_t             *SEM_ID;         

typedef struct pthread_cond_obj
{
    /* This pthread condition variable's internal counting Semaphore  */ 
    TX_SEMAPHORE        cond_semaphore;
     
    INT                 type;
    INT                 in_use;
     
} pthread_cond_t;

typedef struct pthread_condattr_obj
{
/*     INT                 type; */
     INT                 in_use;
     
} pthread_condattr_t;



typedef struct  pthread_once_obj
{
  UINT          state;
  ULONG          flags;
  TX_EVENT_FLAGS_GROUP   event;
}pthread_once_t;


/* Define extern for errno variable.  */

extern unsigned int   posix_errno;



/* Define POSIX initialize prototype.  */

VOID                 *posix_initialize(VOID * posix_memory);

/* Define POSIX API function prototypes.  */ 

INT                   mq_send(mqd_t mqdes, const char * msg_ptr,
                                size_t msg_len,ULONG msg_prio ); 
ssize_t               mq_receive(mqd_t mqdes, VOID *pMsg, size_t msgLen,
                                   ULONG *pMsgPrio );
INT                   mq_unlink(const char * mqName);
INT                   mq_close(mqd_t mqdes);
mqd_t                 mq_open(const CHAR * mqName, ULONG oflags,...);
INT                   sem_close(sem_t  * sem);
INT                   sem_getvalue(sem_t * sem,ULONG * sval);
sem_t                *sem_open(const char * name, ULONG oflag, ...);
INT                   sem_post(sem_t * sem);
INT                   sem_trywait(sem_t * sem);
INT                   sem_unlink(const char * name);
INT                   sem_wait( sem_t * sem );
INT                   sem_init(sem_t *sem , INT pshared, UINT value);
INT                   sem_destroy(sem_t *sem);

INT                   pthread_create (pthread_t *thread,  pthread_attr_t *attr, 
                                      VOID *(*start_routine)(VOID*),VOID *arg);
INT                   pthread_detach(pthread_t thread);
INT                   pthread_join(pthread_t thread, VOID **value_ptr);
INT                   pthread_equal(pthread_t thread1, pthread_t thread2);
VOID                  pthread_exit(VOID *value_ptr);
pthread_t             pthread_self(VOID);
INT                   pthread_attr_destroy(pthread_attr_t *attr);
INT                   pthread_attr_getdetachstate( pthread_attr_t *attr,INT *detachstate);
INT                   pthread_attr_setdetachstate(pthread_attr_t *attr,INT detachstate);
INT                   pthread_attr_getinheritsched(pthread_attr_t *attr, INT *inheritsched);
INT                   pthread_attr_setinheritsched(pthread_attr_t *attr, INT inheritsched);
INT                   pthread_attr_getschedparam(pthread_attr_t *attr,struct sched_param *param);
INT                   pthread_attr_setschedparam(pthread_attr_t *attr,struct sched_param *param);
INT                   pthread_attr_getschedpolicy(pthread_attr_t *attr, INT *policy);
INT                   pthread_attr_setschedpolicy(pthread_attr_t *attr, INT policy);
INT                   pthread_attr_init(pthread_attr_t *attr);
INT                   pthread_attr_getstackaddr( pthread_attr_t *attr,VOID **stackaddr);
INT                   pthread_attr_setstackaddr(pthread_attr_t *attr,VOID *stackaddr);
INT                   pthread_attr_getstacksize( pthread_attr_t *attr, size_t *stacksize);
INT                   pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
INT                   pthread_attr_getstack( pthread_attr_t *attr,VOID **stackaddr,
                                           size_t *stacksize);
INT                   pthread_attr_setstack( pthread_attr_t *attr,VOID *stackaddr,
                                            size_t stacksize);
INT                   pthread_mutexattr_gettype(pthread_mutexattr_t *attr, INT *type);
INT                   pthread_mutexattr_settype(pthread_mutexattr_t *attr, INT type);
INT                   pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
INT                   pthread_mutexattr_init(pthread_mutexattr_t *attr);
INT                   pthread_mutex_destroy(pthread_mutex_t *mutex);
INT                   pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
INT                   pthread_mutex_lock(pthread_mutex_t *mutex );
INT                   pthread_mutex_unlock(pthread_mutex_t *mutex );
INT                   pthread_mutex_trylock(pthread_mutex_t *mutex);
INT                   pthread_mutexattr_getprotocol( pthread_mutexattr_t *attr, INT *protocol);
INT                   pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, INT protocol);
INT                   pthread_mutexattr_getpshared (pthread_mutexattr_t *attr, INT *pshared);
INT                   pthread_mutexattr_setpshared (pthread_mutexattr_t *attr, INT pshared);
INT                   pthread_mutex_timedlock(pthread_mutex_t *mutex, struct timespec *abs_timeout);
INT                   pthread_setcancelstate (INT state, INT *oldstate);
INT                   pthread_setcanceltype (INT type, INT *oldtype);
INT                   pthread_cancel(pthread_t thread);
VOID                  pthread_yield(VOID);
INT                   pthread_once (pthread_once_t * once_control, VOID (*init_routine) (VOID));
VOID                  pthread_testcancel(VOID);
INT                   pthread_setschedparam(pthread_t thread, INT policy, const struct sched_param *param);
INT                   pthread_getschedparam(pthread_t thread, INT *policy, struct sched_param *param);

INT                   pthread_cond_destroy(pthread_cond_t *cond);
INT                   pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
INT                   pthread_cond_broadcast(pthread_cond_t *cond);
INT                   pthread_cond_signal(pthread_cond_t *cond);
INT                   pthread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex,
                                                 struct timespec *abstime);
INT                   pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);






/* static mutex initializer */

#define PTHREAD_MUTEX_INITIALIZER  {{TX_MUTEX_ID, "PMTX", 0, NULL, 0, 0, 0,  NULL, 0 , NULL, NULL}, PTHREAD_MUTEX_RECURSIVE , TX_TRUE}

/* static conditional variable initializer */
#define PTHREAD_COND_INITIALIZER  {{TX_SEMAPHORE_ID, "CSEM", 0, NULL, 0, NULL, NULL}, TX_TRUE}






#endif      /* TX_POSIX  */
