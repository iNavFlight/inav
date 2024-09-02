/* Simple nested-signaling test.  */

#include   "pthread.h"

#define     DEMO_STACK_SIZE         2048
#define     DEMO_BYTE_POOL_SIZE     9120


/* Define the POSIX pthread object control blocks ... */

pthread_t               pthread_0;


/* Define pthread attributes objects */


pthread_attr_t          ptattr0;


/* Define the counters used in this test application...  */

ULONG     pthread_0_counter;
ULONG     pthread_0_signal_counter15;
ULONG     pthread_0_signal_counter14;
ULONG     pthread_0_signal_counter13;


/* Define pthread function prototypes.  */

VOID    *pthread_0_entry(VOID *);


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
    storage_ptr = (VOID*) posix_initialize((VOID* )free_memory);

    /* Put system definition stuff in here, e.g. pthread creates and other assoerted
       create information. */

    /* Create pthread attributes.  */
    pthread_attr_init(&ptattr0);

    /* Create a sched_param structure */
    memset(&param, 0, sizeof(param));
   
    /* Now create all pthreads , firstly modify respective ptheread
       attribute with desired priority and stack start address and then create the pthread */
    
    /* Create pthread 0.  */
    param.sched_priority = 10;
    pthread_attr_setschedparam(&ptattr0, &param);
    pthread_attr_setstackaddr(&ptattr0,  storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_0, &ptattr0,pthread_0_entry,NULL);
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
}


VOID  pthread_0_signal_handler14(int signo)
{

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

    /* Raise another signal for nesting test.  */
    pthread_kill(pthread_0, 13);
}


VOID  pthread_0_signal_handler15(int signo)
{

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

    /* Raise another signal for nesting test.  */
    pthread_kill(pthread_0, 14);
}


/* Define the test pthreads */
INT pt0_status=0;


/* Self signal test.  */

VOID    *pthread_0_entry(VOID *pthread0_input)
{

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

        /* Raise the signal.  */
        pt0_status =  pthread_kill(pthread_0, 15);
  
        /* Check for errors.  */
        if ((pt0_status) || 
            (pthread_0_counter != pthread_0_signal_counter15) || 
            (pthread_0_counter != pthread_0_signal_counter14) ||
            (pthread_0_counter != pthread_0_signal_counter13))
        {

            error_handler();
            break;
        }
    }

    return(&pt0_status);
}

