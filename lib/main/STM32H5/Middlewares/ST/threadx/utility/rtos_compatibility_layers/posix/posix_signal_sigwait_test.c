/* Simple signal sigwait test.  */

#include   "pthread.h"

#define     DEMO_STACK_SIZE         2048

/* Define the POSIX pthread object control blocks ... */

pthread_t               pthread_0;
pthread_t               pthread_1;


/* Define pthread attributes objects */


pthread_attr_t          ptattr0;
pthread_attr_t          ptattr1;


/* Define the counters used in this test application...  */

ULONG     pthread_0_counter;
ULONG     pthread_0_signal_counter15;
ULONG     pthread_0_signal_counter14;
ULONG     pthread_0_signal_counter13;

ULONG     pthread_1_counter;


/* Define pthread function prototypes.  */

VOID    *pthread_0_entry(VOID *);
VOID    *pthread_1_entry(VOID *);


/* Define signal handlers.  */

VOID    pthread_0_signal_handler15(int);
VOID    pthread_0_signal_handler14(int);
VOID    pthread_0_signal_handler13(int);
ULONG free_memory[192*1024 / sizeof(ULONG)];

/* Define main entry point.  */

INT main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */
VOID tx_application_define(VOID *first_unused_memory)
{

    VOID* storage_ptr;


struct sched_param  param;

   
    /* Init POSIX Wrapper */
    storage_ptr = (VOID*) posix_initialize((VOID*)free_memory);

    /* Put system definition stuff in here, e.g. pthread creates and other assoerted
       create information. */

    /* Create pthread attributes.  */
    pthread_attr_init(&ptattr0);
    pthread_attr_init(&ptattr1);

    /* Create a sched_param structure */
    memset(&param, 0, sizeof(param));
   
    /* Now create all pthreads , firstly modify respective ptheread
       attribute with desired priority and stack start address and then create the pthread */
    
    /* Create pthread 0.  */
    param.sched_priority = 15;
    pthread_attr_setschedparam(&ptattr0, &param);
    pthread_attr_setstackaddr(&ptattr0,  storage_ptr );
    storage_ptr = (UINT *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_0, &ptattr0,pthread_0_entry,NULL);


    /* Create pthread 1.  */
    param.sched_priority = 10;
    pthread_attr_setschedparam(&ptattr1, &param);
    pthread_attr_setstackaddr(&ptattr1, (VOID*) storage_ptr );
    storage_ptr = (UINT *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_1, &ptattr1,pthread_1_entry,NULL);
}

VOID    error_handler(void)
{

    while(1)
    {
    }
}



/* Define the signal handlers.  */


VOID  pthread_0_signal_handler13(int signo)
{

sigset_t    wait_set;
int         signal_received;
int         status;

    /* Check for pthread self call not pthread 0. The signal handler should appear to be
       called from pthread 0.  */
    if (pthread_self() != pthread_0)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Check for proper signal.  */
    if (signo != 13)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Just increment the signal counter for this test.  */
    pthread_0_signal_counter13++;

    /* Wait on signal 12.  */
    sigemptyset(&wait_set);
    sigaddset(&wait_set, 12);
    status =  sigwait(&wait_set, &signal_received);
    
    /* Check for an error.  */
    if ((status) || (signal_received != 12))
    {
    
        /* Call error handler.  */
        error_handler();
    }
}


VOID  pthread_0_signal_handler14(int signo)
{

sigset_t    wait_set;
int         signal_received;
int         status;


    /* Check for pthread self call not pthread 0. The signal handler should appear to be
       called from pthread 0.  */
    if (pthread_self() != pthread_0)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Check for proper signal.  */
    if (signo != 14)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Just increment the signal counter for this test.  */
    pthread_0_signal_counter14++;

    /* Wait on signal 13.  */
    sigemptyset(&wait_set);
    sigaddset(&wait_set, 13);
    status =  sigwait(&wait_set, &signal_received);
    
    /* Check for an error.  */
    if ((status) || (signal_received != 13))
    {
    
        /* Call error handler.  */
        error_handler();
    }
}


VOID  pthread_0_signal_handler15(int signo)
{

sigset_t    wait_set;
int         signal_received;
int         status;


    /* Check for pthread self call not pthread 0. The signal handler should appear to be
       called from pthread 0.  */
    if (pthread_self() != pthread_0)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Check for proper signal.  */
    if (signo != 15)
    {
    
        /* Call error handler.  */
        error_handler();
    }

    /* Just increment the signal counter for this test.  */
    pthread_0_signal_counter15++;

    /* Wait on signal 14.  */
    sigemptyset(&wait_set);
    sigaddset(&wait_set, 14);
    status =  sigwait(&wait_set, &signal_received);
    
    /* Check for an error.  */
    if ((status) || (signal_received != 14))
    {
    
        /* Call error handler.  */
        error_handler();
    }
}


/* Define the test pthreads */
INT pt0_status=0;


/* Self signal test.  */

VOID    *pthread_0_entry(VOID *pthread0_input)
{

sigset_t    wait_set;
int         signal_received;


    /* Register the signal handlers.  */
    pt0_status =  signal(15, pthread_0_signal_handler15);

    /* Check for error.  */
    if (pt0_status)
        error_handler();

    pt0_status =  signal(14, pthread_0_signal_handler14);

    /* Check for error.  */
    if (pt0_status)
        error_handler();

    pt0_status =  signal(13, pthread_0_signal_handler13);

    /* Check for error.  */
    if (pt0_status)
        error_handler();

    /* This pthread simply sits in while-forever-sleep loop */ 
    while(1) 
    {
        /* Increment the pthread counter.*/
        pthread_0_counter++;

        /* Wait on signal 15.  */
        sigemptyset(&wait_set);
        sigaddset(&wait_set, 15);

        /* Signal wait.  */        
        pt0_status =  sigwait(&wait_set, &signal_received);

        /* Check for errors.  */
        if ((pt0_status) || 
            (signal_received != 15) ||
            (pthread_0_counter != pthread_0_signal_counter15) || 
            (pthread_0_counter != pthread_0_signal_counter14) ||
            (pthread_0_counter != pthread_0_signal_counter13))
        {
        
            /* In this test, this thread should never resume!  */
            error_handler();

            /* Break out of the loop.  */
            break;            
        }
    }

    return(&pt0_status);
}


INT    pt1_status=0;

VOID    *pthread_1_entry(VOID *pthread1_input)
{
    

    /* This thread simply sends a messages to a queue shared by pthread 2.  */
    while(1)
    {
    
        /* Increment the thread counter.  */
        pthread_1_counter++;

        /* Raise the first signal for pthread 0.  */
        pt1_status =   pthread_kill(pthread_0, 15);

        pt1_status +=  pthread_kill(pthread_0, 14);

        pt1_status +=  pthread_kill(pthread_0, 13);

        pt1_status +=  pthread_kill(pthread_0, 12);

        /* Check for errors.  */
        if ((pt1_status) || 
            (pthread_0_counter != (pthread_1_counter+1)) ||
            (pthread_1_counter != pthread_0_signal_counter15) || 
            (pthread_1_counter != pthread_0_signal_counter14) ||
            (pthread_1_counter != pthread_0_signal_counter13))
        {

            error_handler();
            break;
        }
    }
    
    return(&pt1_status);
}

