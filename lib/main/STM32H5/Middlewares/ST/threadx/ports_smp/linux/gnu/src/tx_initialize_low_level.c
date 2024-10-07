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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>


/* Define various Linux objects used by the ThreadX port.  */

TX_LINUX_MUTEX                  _tx_linux_mutex;
sem_t                           _tx_linux_scheduler_semaphore;
pthread_t                       _tx_linux_scheduler_id;
ULONG                           _tx_linux_global_int_disabled_flag;
struct timespec                 _tx_linux_time_stamp;
ULONG                           _tx_linux_system_error;
TX_THREAD_SMP_CORE_MAPPING      _tx_linux_virtual_cores[TX_THREAD_SMP_MAX_CORES];
extern UINT                     _tx_thread_preempt_disable;
extern TX_THREAD                *_tx_thread_current_ptr[TX_THREAD_SMP_MAX_CORES];
extern TX_THREAD                *_tx_thread_execute_ptr[TX_THREAD_SMP_MAX_CORES];
extern ULONG                    _tx_thread_system_state[TX_THREAD_SMP_MAX_CORES];
extern TX_THREAD_SMP_PROTECT    _tx_thread_smp_protection;

/* Define signals for linux thread. */
#define SUSPEND_SIG SIGUSR1
#define RESUME_SIG  SIGUSR2

static sigset_t     _tx_linux_thread_wait_mask;
static __thread int _tx_linux_thread_suspended;
static sem_t        _tx_linux_thread_timer_wait;
static sem_t        _tx_linux_thread_other_wait;
static sem_t        _tx_linux_sleep_sema;
__thread int        _tx_linux_threadx_thread = 0;

/* Define simulated timer interrupt.  This is done inside a thread, which is
   how other interrupts may be defined as well.  See code below for an
   example.  */

pthread_t           _tx_linux_timer_id;
sem_t               _tx_linux_timer_semaphore;
sem_t               _tx_linux_isr_semaphore;
void               *_tx_linux_timer_interrupt(void *p);


#ifdef TX_LINUX_DEBUG_ENABLE


/* Define the maximum size of the Linux debug array.  */

#ifndef TX_LINUX_DEBUG_EVENT_SIZE
#define TX_LINUX_DEBUG_EVENT_SIZE       400
#endif


/* Define debug log in order to debug Linux issues with this port.  */

typedef struct TX_LINUX_DEBUG_ENTRY_STRUCT
{
    char                    *tx_linux_debug_entry_action;
    pthread_t               tx_linux_debug_entry_running_id;
    UINT                    tx_linux_debug_entry_core;
    struct timespec         tx_linux_debug_entry_timestamp;
    char                    *tx_linux_debug_entry_file;
    unsigned long           tx_linux_debug_entry_line;
    TX_LINUX_MUTEX          tx_linux_debug_entry_mutex;
    TX_THREAD_SMP_PROTECT   tx_linux_debug_protection;
    unsigned long           tx_linux_debug_entry_int_disabled_flag;
    UINT                    tx_linux_debug_entry_preempt_disable;
    ULONG                   tx_linux_debug_entry_system_state[TX_THREAD_SMP_MAX_CORES];
    TX_THREAD               *tx_linux_debug_entry_current_thread[TX_THREAD_SMP_MAX_CORES];
    pthread_t               tx_linux_debug_entry_current_thread_id[TX_THREAD_SMP_MAX_CORES];
    TX_THREAD               *tx_linux_debug_entry_execute_thread[TX_THREAD_SMP_MAX_CORES];
    pthread_t               tx_linux_debug_entry_execute_thread_id[TX_THREAD_SMP_MAX_CORES];
} TX_LINUX_DEBUG_ENTRY;


/* Define the circular array of Linux debug entries.  */

TX_LINUX_DEBUG_ENTRY    _tx_linux_debug_entry_array[TX_LINUX_DEBUG_EVENT_SIZE];


/* Define the Linux debug index.  */

unsigned long           _tx_linux_debug_entry_index =  0;


/* Now define the debug entry function.  */
void    _tx_linux_debug_entry_insert(char *action, char *file, unsigned long line)
{
UINT    i;

    /* Get the time stamp.  */
    clock_gettime(CLOCK_REALTIME, &_tx_linux_time_stamp);

    /* Setup the debug entry.  */
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_action =             action;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_core =               _tx_thread_smp_core_get();
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_timestamp =          _tx_linux_time_stamp;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_file =               file;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_line =               line;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_protection =               _tx_thread_smp_protection;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_preempt_disable =    _tx_thread_preempt_disable;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_mutex =              _tx_linux_mutex;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_int_disabled_flag =  _tx_linux_global_int_disabled_flag;
    _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_running_id =         pthread_self();
    for (i = 0; i < TX_THREAD_SMP_MAX_CORES; i++)
    {

        _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_system_state[i] =           _tx_thread_system_state[i];
        _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_current_thread[i] =         _tx_thread_current_ptr[i];
        if (_tx_thread_current_ptr[i])
            _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_current_thread_id[i] =  _tx_thread_current_ptr[i] -> tx_thread_linux_thread_id;
        else
            _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_current_thread_id[i] =  0;
        _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_execute_thread[i] =         _tx_thread_execute_ptr[i];
        if (_tx_thread_execute_ptr[i])
            _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_execute_thread_id[i] =  _tx_thread_execute_ptr[i] -> tx_thread_linux_thread_id;
        else
            _tx_linux_debug_entry_array[_tx_linux_debug_entry_index].tx_linux_debug_entry_execute_thread_id[i] =  0;
    }

    /* Now move to the next entry.  */
    _tx_linux_debug_entry_index++;

    /* Determine if we need to wrap the list.  */
    if (_tx_linux_debug_entry_index >= TX_LINUX_DEBUG_EVENT_SIZE)
    {

        /* Yes, wrap the list!  */
        _tx_linux_debug_entry_index =  0;
    }
}

#endif


/* Define the ThreadX timer interrupt handler.  */

void    _tx_timer_interrupt(void);


/* Define other external function references.  */

VOID    _tx_initialize_low_level(VOID);
VOID    _tx_thread_context_save(VOID);
VOID    _tx_thread_context_restore(VOID);


/* Define other external variable references.  */

extern VOID     *_tx_initialize_unused_memory;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_low_level                          SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for any low-level processor            */
/*    initialization, including setting up interrupt vectors, setting     */
/*    up a periodic timer interrupt source, saving the system stack       */
/*    pointer for use in ISR processing later, and finding the first      */
/*    available RAM memory address for tx_application_define.             */
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
/*    sched_setaffinity                                                   */
/*    getpid                                                              */
/*    _tx_linux_thread_init                                               */
/*    pthread_setschedparam                                               */
/*    pthread_mutexattr_init                                              */
/*    pthread_mutex_init                                                  */
/*    _tx_linux_thread_suspend                                            */
/*    sem_init                                                            */
/*    pthread_create                                                      */
/*    printf                                                              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter           ThreadX entry function        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_initialize_low_level(VOID)
{
UINT i;
struct sched_param sp;
pthread_mutexattr_t attr;

#ifdef TX_LINUX_MULTI_CORE
cpu_set_t mask;

    /* Limit this ThreadX simulation on Linux to a single core.  */
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(getpid(), sizeof(mask), &mask) != 0)
    {

        /* Error restricting the process to one core.  */
        printf("ThreadX Linux error restricting the process to one core!\n");
        while(1)
        {
        }
    }
#endif

    /* Pickup the first available memory address.  */

    /* Save the first available memory address.  */
    _tx_initialize_unused_memory =  malloc(TX_LINUX_MEMORY_SIZE);

    /* Pickup the unique Id of the current thread, which will also be the Id of the scheduler.  */
    _tx_linux_scheduler_id = pthread_self();

    /* Init Linux thread. */
    _tx_linux_thread_init();

    /* Set priority and schedual of main thread. */
    sp.sched_priority = TX_LINUX_PRIORITY_SCHEDULE;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);

    /* Create the system mutex.  This is used by the
       scheduler thread (which is the main thread) to block all
       other stuff out.  */
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&_tx_linux_mutex.tx_linux_mutex, &attr);
    sem_init(&_tx_linux_scheduler_semaphore, 0, 0);

    /* Loop to clear the virtual core array, which is how we map threads to cores.  */
    for (i = 0; i < TX_THREAD_SMP_MAX_CORES; i++)
    {

        /* Clear this mapping entry.  */
        _tx_linux_virtual_cores[i].tx_thread_smp_core_mapping_thread =          TX_NULL;
        _tx_linux_virtual_cores[i].tx_thread_smp_core_mapping_linux_thread_id = 0;
    }

    /* Initialize the global interrupt disabled flag.  */
    _tx_linux_global_int_disabled_flag =  TX_FALSE;

    /* Create semaphore for timer thread. */
    sem_init(&_tx_linux_timer_semaphore, 0, 0);

    /* Create semaphore for ISR thread. */
    sem_init(&_tx_linux_isr_semaphore, 0, 0);

    /* Setup periodic timer interrupt.  */
    if(pthread_create(&_tx_linux_timer_id, NULL, _tx_linux_timer_interrupt, &_tx_linux_timer_id))
    {

        /* Error creating the timer interrupt.  */
        printf("ThreadX Linux error creating timer interrupt thread!\n");
        while(1)
        {
        }
    }

    /* Otherwise, we have a good thread create.  Now set the priority to
       a level lower than the system thread but higher than the application
       threads.  */
    sp.sched_priority = TX_LINUX_PRIORITY_ISR;
    pthread_setschedparam(_tx_linux_timer_id, SCHED_FIFO, &sp);

    /* Done, return to caller.  */
}


/* This routine is called after initialization is complete in order to start
   all interrupt threads.  Interrupt threads in addition to the timer may
   be added to this routine as well.  */

void    _tx_initialize_start_interrupts(void)
{

    /* Kick the timer thread off to generate the ThreadX periodic interrupt
       source.  */
    tx_linux_sem_post(&_tx_linux_timer_semaphore);
}


/* Define the ThreadX system timer interrupt.  Other interrupts may be simulated
   in a similar way.  */

void    *_tx_linux_timer_interrupt(void *p)
{
struct timespec ts;
long timer_periodic_sec;
long timer_periodic_nsec;
int err;

    /* Calculate periodic timer. */
    timer_periodic_sec = 1 / TX_TIMER_TICKS_PER_SECOND;
    timer_periodic_nsec = 1000000000 / TX_TIMER_TICKS_PER_SECOND;
    nice(10);

    /* Wait startup semaphore. */
    tx_linux_sem_wait(&_tx_linux_timer_semaphore);

    while(1)
    {

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += timer_periodic_nsec;
        if (ts.tv_nsec > 1000000000)
        {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
        do
        {
            if (sem_timedwait(&_tx_linux_timer_semaphore, &ts) == 0)
            {
                break;
            }
            err = errno;
        } while (err != ETIMEDOUT);

        /* Call ThreadX context save for interrupt preparation.  */
        _tx_thread_context_save();

        /* Call the ThreadX system timer interrupt processing.  */
        _tx_timer_interrupt();

        /* Call ThreadX context restore for interrupt completion.  */
        _tx_thread_context_restore();
    }
}

/* Define functions for linux thread. */
void    _tx_linux_thread_resume_handler(int sig)
{
}

void    _tx_linux_thread_suspend_handler(int sig)
{
    if(pthread_equal(pthread_self(), _tx_linux_timer_id))
        tx_linux_sem_post(&_tx_linux_thread_timer_wait);
    else
        tx_linux_sem_post(&_tx_linux_thread_other_wait);

    if(_tx_linux_thread_suspended)
        return;

    _tx_linux_thread_suspended = 1;
    sigsuspend(&_tx_linux_thread_wait_mask);
    _tx_linux_thread_suspended = 0;
}

void    _tx_linux_thread_suspend(pthread_t thread_id)
{

    /* Send signal. */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);
    pthread_kill(thread_id, SUSPEND_SIG);
    _tx_linux_mutex_release(&_tx_linux_mutex);

    /* Wait until signal is received. */
    if(pthread_equal(thread_id, _tx_linux_timer_id))
        tx_linux_sem_wait(&_tx_linux_thread_timer_wait);
    else
        tx_linux_sem_wait(&_tx_linux_thread_other_wait);
}

void    _tx_linux_thread_resume(pthread_t thread_id)
{

    /* Send signal. */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);
    pthread_kill(thread_id, RESUME_SIG);
    _tx_linux_mutex_release(&_tx_linux_mutex);
}

void    _tx_linux_thread_init()
{
struct sigaction sa;

    /* Create semaphore for linux thread. */
    sem_init(&_tx_linux_thread_timer_wait, 0, 0);
    sem_init(&_tx_linux_thread_other_wait, 0, 0);
    sem_init(&_tx_linux_sleep_sema, 0, 0);

    sigfillset(&_tx_linux_thread_wait_mask);
    sigdelset(&_tx_linux_thread_wait_mask, RESUME_SIG);

    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = _tx_linux_thread_resume_handler;
    sigaction(RESUME_SIG, &sa, NULL);

    sa.sa_handler = _tx_linux_thread_suspend_handler;
    sigaction(SUSPEND_SIG, &sa, NULL);
}

void    _tx_linux_thread_sleep(long ns)
{
struct timespec ts;
int err;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += ns;
    if (ts.tv_nsec > 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    do
    {
        if (sem_timedwait(&_tx_linux_sleep_sema, &ts) == 0)
        {
            break;
        }
        err = errno;
    } while (err != ETIMEDOUT);
}
