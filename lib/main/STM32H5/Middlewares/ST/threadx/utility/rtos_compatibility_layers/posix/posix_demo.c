/* This is a small demo of the POSIX Compliancy Wrapper for the high-performance ThreadX kernel.     */
/* It includes examples of six pthreads of different priorities, using a message queue, semaphore and mutex.  */

#include   "pthread.h"

#define     DEMO_STACK_SIZE         2048
#define     MAX_MESSAGE_SIZE        50
#define     DEMO_BYTE_POOL_SIZE     9120

/* Define the POSIX pthread object control blocks ... */

pthread_t               pthread_0;
pthread_t               pthread_1;
pthread_t               pthread_2;
pthread_t               pthread_3;
pthread_t               pthread_4;
pthread_t               pthread_5;

/* Define pthread attributes objects */

pthread_attr_t          ptattr0;
pthread_attr_t          ptattr1;
pthread_attr_t          ptattr2;
pthread_attr_t          ptattr3;
pthread_attr_t          ptattr4;
pthread_attr_t          ptattr5;


/* Define the message queue attribute.  */

struct mq_attr          queue_atrr;

/* Define a queue descriptor.          */

mqd_t                   q_des;

/* Define a semaphore.                 */

sem_t                   *sem;

/* Define a mutex                     */

pthread_mutex_t         mutex1;

/* Define a mutex attributes object   */

pthread_mutexattr_t     mta1;


/* Define the counters used in this demo application...  */

ULONG     pthread_0_counter;
ULONG     pthread_1_counter;
ULONG     pthread_2_counter;
ULONG     pthread_3_counter;
ULONG     pthread_4_counter;
ULONG     pthread_5_counter;
ULONG     pthread_1_message_sent;
ULONG     pthread_2_message_received;


/* Define pthread function prototypes.  */

VOID    *pthread_0_entry(VOID *);
VOID    *pthread_1_entry(VOID *);
VOID    *pthread_2_entry(VOID *);
VOID    *pthread_3_entry(VOID *);
VOID    *pthread_4_entry(VOID *);
VOID    *pthread_5_entry(VOID *);


/* Message to be sent.  */
CHAR *msg0 = "This is a test message";

/* Define main entry point.  */

INT main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}

ULONG free_memory[192*1024 / sizeof(ULONG)];
/* Define what the initial system looks like.  */
VOID tx_application_define(VOID *first_unused_memory)
{

VOID* storage_ptr;


struct sched_param  param;

    queue_atrr.mq_maxmsg  = 124;
    queue_atrr.mq_msgsize = MAX_MESSAGE_SIZE;

   
    /* Init POSIX Wrapper */
    storage_ptr = (VOID*) posix_initialize(free_memory);

    /* Put system definition stuff in here, e.g. pthread creates and other assoerted
       create information. */

    /* Create pthread attributes for pthread 0 to pthread 5 */
    pthread_attr_init(&ptattr0);
    pthread_attr_init(&ptattr1);
    pthread_attr_init(&ptattr2);
    pthread_attr_init(&ptattr3);
    pthread_attr_init(&ptattr4);
    pthread_attr_init(&ptattr5);  

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
    
    /* Create pthread 1.  */
    param.sched_priority = 15;
    pthread_attr_setschedparam(&ptattr1, &param);
    pthread_attr_setstackaddr(&ptattr1, (VOID*) storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_1, &ptattr1,pthread_1_entry,NULL);
    
    /* Create pthread 2.  */
    param.sched_priority = 20;
    pthread_attr_setschedparam(&ptattr2, &param);
    pthread_attr_setstackaddr(&ptattr2, (VOID*) storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_2, &ptattr2,pthread_2_entry,NULL);

    /* Create pthread 3.  */
    param.sched_priority = 25;
    pthread_attr_setschedparam(&ptattr3, &param);
    pthread_attr_setstackaddr(&ptattr3, (VOID*) storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_3, &ptattr3,pthread_3_entry,NULL);
  
    /* Create pthread 4.  */
    param.sched_priority = 30;
    pthread_attr_setschedparam(&ptattr4, &param);
    pthread_attr_setstackaddr(&ptattr4, (VOID*) storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_4, &ptattr4,pthread_4_entry,NULL);
  
    /* Create pthread 5.  */
    param.sched_priority = 5;
    pthread_attr_setschedparam(&ptattr5, &param);
    pthread_attr_setstackaddr(&ptattr5, (VOID*) storage_ptr );
    storage_ptr = (int *) storage_ptr + DEMO_STACK_SIZE;    
    pthread_create (&pthread_5, &ptattr5,pthread_5_entry,NULL);

    /* Create a Message queue.  */
    q_des = mq_open("Queue",O_CREAT|O_RDWR,0,&queue_atrr);

    /* Create a Semaphore.  */
    sem = sem_open("Sem0", O_CREAT | O_EXCL,0,1);

    /* Create a Mutex */
    pthread_mutex_init(&mutex1, NULL);
  
}

/* Define the test pthreads */
INT pt0_status=0;

VOID    *pthread_0_entry(VOID *pthread0_input)
{

    struct timespec thread_0_sleep_time={0,0};

    /* This pthread simply sits in while-forever-sleep loop */ 
    while(1) 
    {
        /* Increment the pthread counter.*/
        pthread_0_counter++;
        /* sleep for a while  */
        thread_0_sleep_time.tv_nsec =  999999999;
         thread_0_sleep_time.tv_sec =  4;
    pt0_status=nanosleep(&thread_0_sleep_time,0);
    if(pt0_status)
        break;
    }

    return(&pt0_status);
}


INT    pt1_status=0;

VOID    *pthread_1_entry(VOID *pthread1_input)
{
    
    struct timespec thread_1_sleep_time={0,0};

    /* This thread simply sends a messages to a queue shared by pthread 2.  */
    while(1)
    {
    
        /* Increment the thread counter.  */
        pthread_1_counter++;
    /* Send message to queue 0.  */
        pt1_status = mq_send(q_des,msg0,strlen(msg0),3);

        /* check status.  */
        if(pt1_status)
           break;

        /* Increment the message sent.  */
        pthread_1_message_sent++;

     /* sleep for a while  */
        thread_1_sleep_time.tv_nsec = 200000000;
    nanosleep(&thread_1_sleep_time,0);
    }
    return(&pt1_status);
}


INT     pt2_status;

VOID    *pthread_2_entry(VOID *pthread2_input)
{

CHAR    msgr0[MAX_MESSAGE_SIZE]; 
ULONG   priority;
struct timespec thread_2_sleep_time={0,0};

    /* This pthread retrieves messages placed on the queue by pthread 1.  */
    while(1 )
    {
        /* Increment the thread counter.  */
        pthread_2_counter++;
        pt2_status = mq_receive(q_des,msgr0,MAX_MESSAGE_SIZE,&priority);
        
        if(pt2_status == ERROR)
           break;

        /* Otherwise, it is OK to increment the received message count.  */
        pthread_2_message_received++;
        /* sleep for a while  */
        thread_2_sleep_time.tv_nsec = 200000000;
    nanosleep(&thread_2_sleep_time,0);
    }
   return(&pt2_status);
}

INT    pt3_status;
VOID    *pthread_3_entry(VOID *pthread3_input)
{


struct timespec thread_3_sleep_time={0,0};


    /* This function compete for ownership of semaphore_0.  */
    while(1)
    {

        /* Increment the thread counter.  */
        pthread_3_counter++;
        /* Get the semaphore with suspension.  */ 
        pt3_status = sem_wait(sem);

        /* Check status.  */
        if (pt3_status)
          break;

        /* Sleep for a while to hold the semaphore.  */
        thread_3_sleep_time.tv_nsec = 200000000;
        nanosleep(&thread_3_sleep_time,0);

        /* Release the semaphore.  */
        pt3_status = sem_post(sem);

        /* Check status.  */
        if (pt3_status )
            break;
    }
    return(&pt3_status);
}

INT  pt4_status;

VOID    *pthread_4_entry(VOID *pthread4_input)
{

struct timespec thread_4_sleep_time={0,0};

    while(1)
    {

       /* Increment the thread counter.  */
       pthread_4_counter++;
       /* now lock the mutex */
       pt4_status = pthread_mutex_lock(&mutex1);
       if (pt4_status != OK)
       break;

       /* sleep for a while  */
       thread_4_sleep_time.tv_nsec = 200000000;
       nanosleep(&thread_4_sleep_time,0);

       pt4_status = pthread_mutex_unlock(&mutex1);
       if (pt4_status != OK)
       break;
    }
    return(&pt4_status);
}

INT     pt5_status;

VOID    *pthread_5_entry(VOID *pthread5_input)
{

struct timespec thread_5_sleep_time={0,0};

    while(1)
    {
        /* Increment the thread counter. */
        pthread_5_counter++;
        /* now lock the mutex */
        pt5_status = pthread_mutex_lock(&mutex1);
        if (pt5_status != OK)
            break;

        /* sleep for a while  */
        thread_5_sleep_time.tv_nsec = 20000000;
    nanosleep(&thread_5_sleep_time,0);
        pt5_status = pthread_mutex_unlock(&mutex1);
          if (pt5_status != OK)
          break;
    }
    return(&pt5_status);
}


