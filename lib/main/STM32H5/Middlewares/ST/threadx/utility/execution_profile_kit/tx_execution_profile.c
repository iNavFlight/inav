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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_execution_profile.h"

/* Note to developers upgrading from ThreadX version 5: In ThreadX 5, the instruction was to
   modify TX_THREAD_EXTENSION_3, and to define the symbol TX_ENABLE_EXECUTION_CHANGE_NOTIFY.

   For ThreadX 6, user no long need to modify TX_THREAD_EXTENSION_3, and shall use the symbol
   TX_EXECUTION_PROFILE_ENABLE instead of TX_ENABLE_EXECUTION_CHANGE_NOTIFY.

   For backward compatibiliy reasons, project upgraded from ThreadX 5 may still be able to use
   Execution Profile without changes to existing project, users are strongly recommended to
   make the change.  */


#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)

/* The thread execution profile kit is designed to track thread execution time
   based on the hardware timer defined by TX_EXECUTION_TIME_SOURCE and
   TX_EXECUTION_MAX_TIME_SOURCE below. When the thread's total time reaches
   the maximum value, it remains there until the time is reset to 0 via a call
   to tx_thread_execution_time_reset. There are several assumptions to the
   operation of this kit, as follows:

   1. The TX_EXECUTION_TIME_SOURCE and TX_EXECUTION_MAX_TIME_SOURCE macros are
      defined to utilize a local hardware time source.
   
   2. ThreadX 5.4 (or later) is being used, with the assembly code enabled to
      call the following routines from assembly code:
      
            VOID  _tx_execution_thread_enter(void);
            VOID  _tx_execution_thread_exit(void);
            VOID  _tx_execution_isr_enter(void);
            VOID  _tx_execution_isr_exit(void);
      
    3. The ThreadX library assembly code must be rebuilt with TX_EXECUTION_PROFILE_ENABLE so
       that these macros are expanded in the TX_THREAD structure and so the assembly code macros
       are enabled to call the execution profile routines.

    4. Add tx_execution_profile.c to the application build.  */


/* Externally reference several internal ThreadX variables.  */
   
extern ULONG                            _tx_thread_system_state;
extern UINT                             _tx_thread_preempt_disable;
extern TX_THREAD                        *_tx_thread_current_ptr;
extern TX_THREAD                        *_tx_thread_execute_ptr;
extern TX_THREAD                        *_tx_thread_created_ptr;
extern ULONG                            _tx_thread_created_count;


/* Define the total time for all threads.  This is accumulated as each thread's total time is accumulated.  */

EXECUTION_TIME                          _tx_execution_thread_time_total;


/* Define the ISR time gathering information. This is setup to track total ISR time presently, but
   could easily be expanded to track different ISRs. Also, only ISRs that utilize _tx_thread_context_save
   and _tx_thread_context_restore are tracked by this utility.  */

EXECUTION_TIME                          _tx_execution_isr_time_total;
EXECUTION_TIME_SOURCE_TYPE              _tx_execution_isr_time_last_start;


/* Define the system idle time gathering information. For idle time that exceeds the range of the timer
   source, another timer source may be needed. In addition, the total thread execution time added to the 
   total ISR time, less the total system time is also a measure of idle time.  */

EXECUTION_TIME                          _tx_execution_idle_time_total;
EXECUTION_TIME_SOURCE_TYPE              _tx_execution_idle_time_last_start;
UINT                                    _tx_execution_idle_active;

/* For Cortex-M targets, we need to keep track of nested interrupts internally.  */
#ifdef TX_CORTEX_M_EPK
ULONG                                   _tx_execution_isr_nest_counter = 0;
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_initialize                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called at initialization.                          */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    xxx                               xxx                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022      Scott Larson            Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
VOID  _tx_execution_initialize(void)
{
    /* In idle mode until a thread is scheduled or ISR occurs.  */
    _tx_execution_idle_active = TX_TRUE;

    /* Pickup the start of idle time.  */
    _tx_execution_idle_time_last_start =  TX_EXECUTION_TIME_SOURCE;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_enter                          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called whenever thread execution starts.           */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_schedule               Thread scheduling                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  04-25-2022      Scott Larson            Modified comments and fixed   */
/*                                            wrap-around calculation,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_execution_thread_enter(void)
{

TX_THREAD                   *thread_ptr;
EXECUTION_TIME_SOURCE_TYPE  last_start_time;
EXECUTION_TIME_SOURCE_TYPE  current_time;
EXECUTION_TIME              delta_time;
EXECUTION_TIME              total_time;
EXECUTION_TIME              new_total_time;


    /* Pickup the current time.  */
    current_time =  TX_EXECUTION_TIME_SOURCE;

    /* Pickup the current thread control block.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* This thread is being scheduled.  Simply setup the last start time in the
       thread control block.  */
    thread_ptr -> tx_thread_execution_time_last_start =  current_time;
    
    /* Pickup the last idle start time.  */
    last_start_time =  _tx_execution_idle_time_last_start;
    
    /* Determine if idle time is being measured.  */
    if (_tx_execution_idle_active)
    {

        /* Determine how to calculate the difference.  */
        if (current_time >= last_start_time)
        {
        
            /* Simply subtract.  */
            delta_time =  (EXECUTION_TIME) (current_time - last_start_time);
        }
        else
        {
        
            /* Timer wrapped, compute the delta assuming incrementing time counter.  */
            delta_time =  (EXECUTION_TIME) (current_time + ((((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) + 1) - last_start_time));
        }
    
        /* Pickup the total time.  */
        total_time =  _tx_execution_idle_time_total;

        /* Now compute the new total time.  */
        new_total_time =  total_time + delta_time;
        
        /* Determine if a rollover on the total time is present.  */
        if (new_total_time < total_time)
        {
        
            /* Rollover. Set the total time to max value.  */
            new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
        }
    
        /* Now store back the total idle time.  */
        _tx_execution_idle_time_total =  new_total_time;
        
        /* Disable the idle time measurement.  */
        _tx_execution_idle_active = TX_FALSE;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_exit                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called whenever a thread execution ends.           */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_system_return          Thread exiting                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  04-25-2022      Scott Larson            Modified comments and fixed   */
/*                                            wrap-around calculation,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_execution_thread_exit(void)
{

TX_THREAD                   *thread_ptr;
EXECUTION_TIME              total_time;
EXECUTION_TIME              new_total_time;
EXECUTION_TIME_SOURCE_TYPE  last_start_time;
EXECUTION_TIME_SOURCE_TYPE  current_time;
EXECUTION_TIME              delta_time;


    /* Pickup the current thread control block.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Determine if there is a thread.  */
    if (thread_ptr)
    {
    
        /* Pickup the current time.  */
        current_time =  TX_EXECUTION_TIME_SOURCE;

        /* Pickup the last start time.  */
        last_start_time =  thread_ptr -> tx_thread_execution_time_last_start;

        /* Determine if there is an actual start time.  */
        if (last_start_time)
        {

            /* Clear the last start time.  */
            thread_ptr -> tx_thread_execution_time_last_start =  0;
      
            /* Determine how to calculate the difference.  */
            if (current_time >= last_start_time)
            {
        
                /* Simply subtract.  */
                delta_time =  (EXECUTION_TIME) (current_time - last_start_time);
            }        
            else
            {
        
                /* Timer wrapped, compute the delta assuming incrementing time counter.  */
                delta_time =  (EXECUTION_TIME) (current_time + ((((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) + 1) - last_start_time));
            }

            /* Pickup the total time.  */
            total_time =  thread_ptr -> tx_thread_execution_time_total;

            /* Now compute the new total time.  */
            new_total_time =  total_time + delta_time;
        
            /* Determine if a rollover on the total time is present.  */
            if (new_total_time < total_time)
            {
        
                /* Rollover. Set the total time to max value.  */
                new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
            }
    
            /* Store back the new total time.  */
            thread_ptr -> tx_thread_execution_time_total =  new_total_time;
            
            /* Now accumulate this thread's execution time into the total thread execution time.  */
            new_total_time =  _tx_execution_thread_time_total + delta_time;
            
            /* Determine if a rollover on the total time is present.  */
            if (new_total_time < _tx_execution_thread_time_total)
            {
        
                /* Rollover. Set the total time to max value.  */
                new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
            }

            /* Store back the new total time.  */
            _tx_execution_thread_time_total =  new_total_time;
        }

        /* Is the system now idle?  */
        if (_tx_thread_execute_ptr == TX_NULL)
        {
            /* Yes, idle system. Pickup the start of idle time.  */
            _tx_execution_idle_time_last_start =  TX_EXECUTION_TIME_SOURCE;
            _tx_execution_idle_active = TX_TRUE;
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_enter                             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called whenever ISR processing starts.             */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_context_save           ISR context save                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  04-25-2022      Scott Larson            Modified comments and fixed   */
/*                                            wrap-around calculation,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_execution_isr_enter(void)
{

TX_THREAD                   *thread_ptr;
EXECUTION_TIME_SOURCE_TYPE  current_time;
EXECUTION_TIME              total_time;
EXECUTION_TIME              new_total_time;
EXECUTION_TIME_SOURCE_TYPE  last_start_time;
EXECUTION_TIME              delta_time;

#ifdef TX_CORTEX_M_EPK
    /* Increment the nested interrupt counter.  */
    _tx_execution_isr_nest_counter++;
#endif

    /* Determine if this is the first interrupt. Nested interrupts are all treated as
       general interrupt processing.  */
#ifdef TX_CORTEX_M_EPK
    if ((TX_THREAD_GET_SYSTEM_STATE()) && (_tx_execution_isr_nest_counter == 1))
#else
    if (TX_THREAD_GET_SYSTEM_STATE() == 1)
#endif
    {
        /* Pickup the current time.  */
        current_time =  TX_EXECUTION_TIME_SOURCE;

        /* Pickup the current thread control block.  */
        thread_ptr =  _tx_thread_current_ptr;

        /* Determine if a thread was interrupted.  */
        if (thread_ptr)
        {
        
            /* Pickup the last start time.  */
            last_start_time =  thread_ptr -> tx_thread_execution_time_last_start;

            /* Determine if there is an actual start time.  */
            if (last_start_time)
            {

                /* Clear the last start time.  */
                thread_ptr -> tx_thread_execution_time_last_start =  0;
        
                /* Determine how to calculate the difference.  */
                if (current_time >= last_start_time)
                {
        
                    /* Simply subtract.  */
                    delta_time =  (EXECUTION_TIME) (current_time - last_start_time);
                }        
                else
                {
        
                    /* Timer wrapped, compute the delta assuming incrementing time counter.  */
                    delta_time =  (EXECUTION_TIME) (current_time + ((((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) + 1) - last_start_time));
                }

                /* Pickup the total time.  */
                total_time =  thread_ptr -> tx_thread_execution_time_total;

                /* Now compute the new total time.  */
                new_total_time =  total_time + delta_time;
        
                /* Determine if a rollover on the total time is present.  */
                if (new_total_time < total_time)
                {
        
                    /* Rollover. Set the total time to max value.  */
                    new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
                }
    
                /* Store back the new total time.  */
                thread_ptr -> tx_thread_execution_time_total =  new_total_time;

                /* Now accumulate this thread's execution time into the total thread execution time.  */
                new_total_time =  _tx_execution_thread_time_total + delta_time;
            
                /* Determine if a rollover on the total time is present.  */
                if (new_total_time < _tx_execution_thread_time_total)
                {
        
                    /* Rollover. Set the total time to max value.  */
                    new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
                }            

                /* Store back the new total time.  */
                _tx_execution_thread_time_total =  new_total_time;
            }
        }
        
        /* Has idle time started?  */
        else if (_tx_execution_idle_active)
        {
        
            /* Pickup the last idle start time.  */
            last_start_time =  _tx_execution_idle_time_last_start;
    
            /* Determine how to calculate the difference.  */
            if (current_time >= last_start_time)
            {
        
                /* Simply subtract.  */
                delta_time =  (EXECUTION_TIME) (current_time - last_start_time);
            }        
            else
            {
        
                /* Timer wrapped, compute the delta assuming incrementing time counter.  */
                delta_time =  (EXECUTION_TIME) (current_time + ((((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) + 1) - last_start_time));
            }
    
            /* Pickup the total time.  */
            total_time =  _tx_execution_idle_time_total;

            /* Now compute the new total time.  */
            new_total_time =  total_time + delta_time;
        
            /* Determine if a rollover on the total time is present.  */
            if (new_total_time < total_time)
            {
        
                /* Rollover. Set the total time to max value.  */
                new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
            }
    
            /* Now store back the total idle time.  */
            _tx_execution_idle_time_total =  new_total_time;    
        
            /* Disable the idle time measurement.  */
            _tx_execution_idle_active = TX_FALSE;
        }

        /* Save the ISR start time.  */
        _tx_execution_isr_time_last_start =  current_time;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_exit                              PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called whenever ISR processing ends.               */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_context_restore        Thread de-scheduling              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  04-25-2022      Scott Larson            Modified comments and fixed   */
/*                                            wrap-around calculation,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_execution_isr_exit(void)
{

TX_THREAD                   *thread_ptr;
EXECUTION_TIME              total_time;
EXECUTION_TIME              new_total_time;
EXECUTION_TIME_SOURCE_TYPE  last_start_time;
EXECUTION_TIME_SOURCE_TYPE  current_time;
EXECUTION_TIME              delta_time;


    /* Determine if this is the first interrupt. Nested interrupts are all treated as 
       general interrupt processing.  */
#ifdef TX_CORTEX_M_EPK
    if ((TX_THREAD_GET_SYSTEM_STATE()) && (_tx_execution_isr_nest_counter == 1))
#else
    if (TX_THREAD_GET_SYSTEM_STATE() == 1)
#endif
    {

        /* Pickup the current time.  */
        current_time =  TX_EXECUTION_TIME_SOURCE;

        /* Pickup the last start time.  */
        last_start_time =  _tx_execution_isr_time_last_start;
        
        /* Determine how to calculate the difference.  */
        if (current_time >= last_start_time)
        {
        
           /* Simply subtract.  */
           delta_time =  (EXECUTION_TIME) (current_time - last_start_time);
        }
        else
        {
        
            /* Timer wrapped, compute the delta assuming incrementing time counter.  */
            delta_time =  (EXECUTION_TIME) (current_time + (((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) - last_start_time));
        }

        /* Pickup the total time.  */
        total_time =  _tx_execution_isr_time_total;

        /* Now compute the new total time.  */
        new_total_time =  total_time + delta_time;
        
        /* Determine if a rollover on the total time is present.  */
        if (new_total_time < total_time)
        {
        
            /* Rollover. Set the total time to max value.  */
            new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
        }
    
        /* Store back the new total time.  */
        _tx_execution_isr_time_total =  new_total_time;
        
        /* Pickup the current thread control block.  */
        thread_ptr =  _tx_thread_current_ptr;

        /* Was a thread interrupted?  */
        if (thread_ptr)
        {

            /* Now determine if the thread will execution is going to occur immediately.  */
            if ((thread_ptr == _tx_thread_execute_ptr) || (_tx_thread_preempt_disable))
            {
            
                /* Yes, setup the thread last start time in the thread control block.  */
                thread_ptr -> tx_thread_execution_time_last_start =  current_time;
            }
        }
        
        /* Determine if the system is now idle.  */
        if (_tx_thread_execute_ptr == TX_NULL)
        {
        
            /* Yes, idle system. Pickup the start of idle time.  */
            _tx_execution_idle_time_last_start =  TX_EXECUTION_TIME_SOURCE;
            _tx_execution_idle_active = TX_TRUE;
        }
    }
    
#ifdef TX_CORTEX_M_EPK
    /* Decrement the nested interrupt counter.  */
    _tx_execution_isr_nest_counter--;
#endif
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_time_reset                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets the execution time of the specified thread.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to thread                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_time_reset(TX_THREAD *thread_ptr)
{

    /* Reset the total time to 0.  */
    thread_ptr -> tx_thread_execution_time_total =  0;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_total_time_reset               PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets the total thread execution time.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_total_time_reset(void)
{

TX_INTERRUPT_SAVE_AREA
            
TX_THREAD       *thread_ptr;            
UINT            total_threads;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Reset the total time to 0.  */
    _tx_execution_thread_time_total =  0;

    /* Loop through threads to clear their accumulated time.  */
    total_threads =      _tx_thread_created_count;
    thread_ptr =         _tx_thread_created_ptr;
    while (total_threads--)
    {
        thread_ptr -> tx_thread_execution_time_total =  0;
        thread_ptr =  thread_ptr -> tx_thread_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_time_reset                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets the execution time of the ISR calculation.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_isr_time_reset(void)
{

    /* Reset the total time to 0.  */
    _tx_execution_isr_time_total =  0;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_idle_time_reset                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets the idle execution time calculation.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_idle_time_reset(void)
{

    /* Reset the total time to 0.  */
    _tx_execution_idle_time_total =  0;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_time_get                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of the specified thread.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to the thread             */
/*    total_time                        Destination for total time        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_time_get(TX_THREAD *thread_ptr, EXECUTION_TIME *total_time)
{

    /* Return the total time.  */
    *total_time =  thread_ptr -> tx_thread_execution_time_total;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_total_time_get                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of the specified thread.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    total_time                        Destination for total time        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_total_time_get(EXECUTION_TIME *total_time)
{

    /* Return the total time.  */
    *total_time =  _tx_execution_thread_time_total;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_time_get                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of ISRs.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    total_time                        Destination for total time        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_isr_time_get(EXECUTION_TIME *total_time)
{

    /* Return the total time.  */
    *total_time =  _tx_execution_isr_time_total;

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_idle_time_get                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of ISRs.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    total_time                        Destination for total time        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_idle_time_get(EXECUTION_TIME *total_time)
{

    /* Return the total time.  */
    *total_time =  _tx_execution_idle_time_total;

    /* Return success.  */
    return(TX_SUCCESS);
}


#endif /* #if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE) */
