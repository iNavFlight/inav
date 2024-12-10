                         POSIX Compliancy Wrapper for Azure RTOS ThreadX



1.0 POSIX Compliancy Wrapper Overview

The POSIX Compliancy Wrapper supports many of the basic POSIX calls, with
some limitations, and utilizes ThreadX® primitives underneath. This POSIX
compatibility layer should have good performance since it utilizes internal
ThreadX primitives and bypasses basic ThreadX error checking.


1.1 POSIX Compliancy Wrapper Source

The Wrapper source code is designed for simplicity and is comprised of separate source 
files for most functions.  Including the supplied pthread.h file will import 
all the necessary POSIX constants and subroutine prototypes.


1.2 POSIX Compliancy Wrapper Documentation

This document itself serves as a POSIX Compliancy Wrapper User Guide by
providing an overview of the porting process, including various caveats and 
pitfalls to watch out for. In addition, each covered POSIX call is documented, 
including information about supported/unsupported options, limitations, deviations, 
and suggestions on how to work-around any limitations.


2.0 Installation

The POSIX Compliancy Wrapper is easily installed by adding the
the posix library to your current application build. Make sure your application build
references the same header files as the ones the posix library has been built with.
The file pthread.h must be included in your application source where POSIX 
calls are required.
Since the POSIX compliancy wrapper does not cover the complete standard, not all prototypes
are provided. Most notably is the header file tx_px_time.h.

2.1 Initialization

The POSIX Compliancy Wrapper requires that a special initialization function is called 
prior to accessing any POSIX calls.  The function to call and its prototype is:

VOID    *posix_initialize(VOID * posix_memory);

This function is usually called from the application specific ThreadX
initialization routine, tx_application_define().  The memory pointer supplied 
to posix_initialize must be a contiguouis reserved section of memory 
that has at least the following number of bytes:


        POSIX_SYSTEM_STACK_SIZE +
        TX_REGION0_SIZE_IN_BYTES +           /* Region0 size         */
        (WORK_QUEUE_DEPTH * WORK_REQ_SIZE) + /* system queue size    */
        POSIX_HEAP_SIZE_IN_BYTES                     


These equates are defined in tx_posix.h.  The following additional equates
define the number of POSIX objects supported by the POSIX Wrapper (default 
value is shown):

       SEM_NSEMS_MAX          100        /* simultaneous POSIX semaphores */

       SEM_NAME_MAX           10         /* maximum length of name of semaphore */

       SEM_VALUE_MAX          100        /* max value of semaphore while initialization */

       POSIX_MAX_QUEUES       32         /* maximum number of simultaneous POSIX
                                                message queues supported  */

       PATH_MAX               10         /* maximum length of name of a message queue  */

       PTHREAD_THREADS_MAX    256        /* define the maximum number of simultaneous
                                                POSIX Pthreads supported.  */

       POSIX_MAX_MUTEX        32         /* define the maximum number of simultaneous
                                                POSIX mutexes sported.     */


The function posix_initialize will return a pointer to the next free 
available memory location for the application.


3.0  POSIX Calls

Once posix_initialize returns, POSIX calls can be made.
The Threadx POSIX Compliancy Wrapper supports the following POSIX
calls:

/***********************************************************************/
/*              CALLS RELATED TO POSIX MESSAGE QUEUE                     */
/***********************************************************************/

INT                   mq_send(mqd_t mqdes, const char * msg_ptr,
                                ssize_t msg_len,ULONG msg_prio );
ssize_t               mq_receive(mqd_t mqdes, VOID *pMsg, ssize_t msgLen,
                                   ULONG *pMsgPrio );
INT                   mq_unlink(const char * mqName);
INT                   mq_close(mqd_t mqdes);
mqd_t                 mq_open(const CHAR * mqName, ULONG oflags,...);


/***********************************************************************/
/*              CALLS RELATED TO POSIX SEMAPHORE                         */
/***********************************************************************/

INT                   sem_close(sem_t  * sem);
INT                   sem_getvalue(sem_t * sem,ULONG * sval);
sem_t                *sem_open(const char * name, ULONG oflag, ...);
INT                   sem_post(sem_t * sem);
INT                   sem_trywait(sem_t * sem);
INT                   sem_unlink(const char * name);
INT                   sem_wait( sem_t * sem );
INT                   sem_init(sem_t *sem , INT pshared, UINT value);
INT                   sem_destroy(sem_t *sem);

/***********************************************************************/
/*              CALLS RELATED TO POSIX pthreads                          */
/***********************************************************************/

INT                   sched_yield(VOID);
INT                   pthread_create (pthread_t *thread,  
					                  pthread_attr_t *attr,
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
INT                   phread_attr_getstackaddr( pthread_attr_t *attr,VOID **stackaddr);
INT                   phread_attr_setstackaddr(pthread_attr_t *attr,VOID **stackaddr);
INT                   pthread_attr_getstacksize( pthread_attr_t *attr, ssize_t *stacksize);
INT                   pthread_attr_setstacksize(pthread_attr_t *attr, ssize_t stacksize);
INT                   phread_attr_getstack( pthread_attr_t *attr,VOID **stackaddr,
                                           ssize_t *stacksize);
INT                   phread_attr_setstack( pthread_attr_t *attr,VOID *stackaddr,
                                            ssize_t stacksize);

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
VOID                  pthread_testcancel(VOID);
INT                   pthread_getschedparam(pthread_t thread, INT *policy, struct sched_param *param);
INT                   pthread_setschedparam(pthread_t thread, INT policy, const struct sched_param *param);

INT                   sched_get_priority_max(INT policy)
INT                   sched_get_priority_min(INT policy)

INT                   pthread_once (pthread_once_t * once_control, VOID (*init_routine) (VOID))

INT                   pthread_kill(ALIGN_TYPE thread_id, int sig)
INT                   pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask)


/***********************************************************************/
/*              CALLS RELATED TO POSIX CONDITION VARIABLE                */
/***********************************************************************/

INT                   pthread_cond_destroy(pthread_cond_t *cond);
INT                   pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
INT                   pthread_cond_broadcast(pthread_cond_t *cond);
INT                   pthread_cond_signal(pthread_cond_t *cond);
INT                   pthread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex,
                                                 struct timespec *abstime);
INT                   pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);


/***********************************************************************/
/*              CALLS RELATED TO Timer                                 */
/***********************************************************************/

INT                   nanosleep(struct timespec *req, struct timespec *rem)
INT                   sleep(ULONG seconds)
INT                   clock_gettime(clockid_t t, struct timespec * tspec)
INT                   clock_settime(clockid_t t, const struct timespec * tspec)
INT                   clock_getres(clockid_t t, struct timespec * tspec)

/***********************************************************************/
/*              CALLS RELATED TO Signal                                */
/***********************************************************************/

INT                   sigwait(const sigset_t *set, int *sig)
INT                   sigaddset(sigset_t *set, int signo)
INT                   sigdelset(sigset_t *set, int signo)
INT                   sigemptyset(sigset_t *set)
INT                   signal(int signo, void (*func)(int))
INT                   sigfillset(sigset_t *set)


4.0  POSIX Compliancy Wrapper Error Handling

There are two "error handling" functions defined in tx_posix.c and used
throughout the Wrapper, as follows:

          posix_error_handler
          posix_internal_error

In general these routines are called when basic usage errors occur.  These 
routines may also be used as a place to catch errors that are not detected if the 
application source is not checking the return status. The default processing for each of 
these is a simple spin loop.

Most functions can provide an error number. The means by which each function
provides its error numbers is specified in its description.  Some functions 
provide the error number in a variable accessed through the symbol posix_errno.  
While other functions return an error number directly as the function value. Functions 
return a value of zero to indicate success. If more than one error occurs in 
processing a function call, any one of the possible errors may be returned, as the order 
of detection is undefined.

Some functions may return [ENOSYS] suggesting that an attempt was made to
use a function that is not available in this implementation.

Each pthread has its own error number, which can be obtained through a 
function call:

INT   posix_get_pthread_errno(pthread_t ptid)

This call will return the last generated error code for the pthread having 
ptid as an ID.


5.1   POSIX Compliancy Wrapper Limitations

Due to performance and architecture issues, this POSIX Compliancy Wrapper
does not support all the POSIX calls. A summary of the POSIX Compliancy 
Wrapper limitations is as follows:

·   Configuration
·   Initialization
·   Driver and I/O model might require porting of current drivers.
·   Multi-processor extensions are not supported
·   Unsupported calls (please see below)
.   Calls supported with certain limitations (please see list below)

The POSIX Compliancy Wrapper supports a subset of POSIX calls.  In addition,
there are also certain limitations with respect to some services.  Below is the list of
such limitations:


LIMITATIONS

Following calls are implemented with some limitations:

1.) mq_open()

         LIMITATIONS :
                a.) The value of mode (mode_t) has no effect in this implementation.
                b.) If pAttr is NULL, the message queue is created with
                      implementation-defined default message queue attributes.
                      The default message queue attributes selected are :

                      #define MQ_MAXMSG 125             [MQ_MAXMSG 1024 (POSIX value)]
                      #define MQ_MSGSIZE 500            [MQ_MSGSIZE 4096 (POSIX value)]
                      #define MQ_FLAGS 0

                      This is due to limitation of size of posix_region0_byte_pool (64KB ).

2.) mq_send()

         LIMITATIONS :
                a.) In POSIX : If more than one mq_send() is blocked on a queue and
                    space becomes available in that queue, the message with the highest
                    priority will be unblocked. THIS FEATURE IS NOT IMPLEMENTED.

                b.) If a message is sent (or received) to a queue with out opening the named 
                    queue, in such a  case mqdes (message queue descriptor) pointer is 
                    invalid and may result in erratic behavior.

3.) mq_receive()

         LIMITATIONS :
                a.) If a receive (or send) message from queue with out it being opened, erratic
                     behavior may ensue.

4.) ULONG sem_close()

        LIMITATIONS :
                a.) This routine does not deallocate any system resources.

5.) POSIX SEMAPHORE

        LIMITATIONS :
                a.) If any operation (eg. sem_post, sem_wait, sem_trywait, sem_getvalue ) is done on a
                    semaphore before creating or opening (sem_open()) the named semaphore, erratic
                    behavior may result.

6.) ULONG sem_trywait(sem_t * sem)

        LIMITATIONS :

                a.) EDEADLKA :->[ This is a return value when deadlock condition is detected; i.e., two separate
                                  processes are waiting for an available resource to be released via a
                                  semaphore "held" by the other process.] This is not implemented.

                b.) EINTR    :->[ This is a return value when sem_wait() was interrupted by a signal.]
                                         This is not implemented.
7.) Thread Cancelation

pthread cancelation cleanup handlers are not supported which means
pthread_cleanup_push( ) and pthread_cleanup_pop( ) functions are not 
implemented.

When the pthread_cancel( ) function is called the target thread is canceled
with immediate effect. (provided cancelability is enabled for the target 
pthread)

The cancelation processing in the target thread shall run asynchronously
with respect to the ailing thread returning from pthread_cancel( ).

8.) Attributes for Condition Variable
    No attributes are supported for condition variable in this implementation.

9.) pthreads suspended by nanosleep() and sleep() calls can not be awakened
by signals, once in the suspension both these calls will complete the 
suspension period.

10.) pthread_once (pthread_once_t * once_control, VOID (*init_routine) (VOID))
There is no provision if the init_routine contains a cancellation point.


6.0  Demonstration System

The file posix_demo.c contains a demonstration system that utilizes POSIX
calls.  This Demo application will demonstrate some of the basic POSIX 
calls.  This demo application should be used as an example of how to integrate the POSIX 
Compliancy Wrapper into your application.


7.0  Future  POSIX Compliancy Wrapper Phases

Please get in touch with us for next phases of this POSIX Compliancy
Wrapper.




