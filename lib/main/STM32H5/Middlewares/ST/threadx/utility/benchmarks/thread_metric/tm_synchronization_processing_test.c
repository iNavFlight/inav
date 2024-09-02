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
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Synchronization Processing Test                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    tm_synchronization_processing_test                  PORTABLE C      */ 
/*                                                           6.1.7        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Microsoft Corporation                             */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the Semaphore get/put processing test.            */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  10-15-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */ 
/**************************************************************************/ 
#include "tm_api.h"


/* Define the counters used in the demo application...  */

unsigned long   tm_synchronization_processing_counter;


/* Define the test thread prototypes.  */

void            tm_synchronization_processing_thread_0_entry(void);


/* Define the reporting thread prototype.  */

void            tm_synchronization_processing_thread_report(void);


/* Define the initialization prototype.  */

void            tm_synchronization_processing_initialize(void);


/* Define main entry point.  */

void tm_main()
{

    /* Initialize the test.  */
    tm_initialize(tm_synchronization_processing_initialize);
}


/* Define the synchronization processing test initialization.  */

void  tm_synchronization_processing_initialize(void)
{

    /* Create thread 0 at priority 10.  */
    tm_thread_create(0, 10, tm_synchronization_processing_thread_0_entry);

    /* Resume thread 0.  */
    tm_thread_resume(0);

    /* Create a semaphore for the test.  */
    tm_semaphore_create(0);

    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    tm_thread_create(5, 2, tm_synchronization_processing_thread_report);
    tm_thread_resume(5);
}


/* Define the synchronization processing thread.  */
void  tm_synchronization_processing_thread_0_entry(void)
{

int status;

    while(1)
    {

        /* Get the semaphore.  */
        tm_semaphore_get(0);

        /* Release the semaphore.  */
        status = tm_semaphore_put(0);

        /* Check for semaphore put error.  */
        if (status != TM_SUCCESS)
            break;

        /* Increment the number of semaphore get/puts.  */
        tm_synchronization_processing_counter++;
    }
}


/* Define the synchronization test reporting thread.  */
void  tm_synchronization_processing_thread_report(void)
{

unsigned long   last_counter;
unsigned long   relative_time;


    /* Initialize the last counter.  */
    last_counter =  0;

    /* Initialize the relative time.  */
    relative_time =  0;

    while(1)
    {

        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);

        /* Increment the relative time.  */
        relative_time =  relative_time + TM_TEST_DURATION;

        /* Print results to the stdio window.  */
        printf("**** Thread-Metric Synchronization Processing Test **** Relative Time: %lu\n", relative_time);

        /* See if there are any errors.  */
        if (tm_synchronization_processing_counter == last_counter)
        {

            printf("ERROR: Invalid counter value(s). Error getting/putting semaphore!\n");
        }

        /* Show the time period total.  */
        printf("Time Period Total:  %lu\n\n", tm_synchronization_processing_counter - last_counter);

        /* Save the last counter.  */
        last_counter =  tm_synchronization_processing_counter;
    }
}
