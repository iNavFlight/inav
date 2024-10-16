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

/* The thread execution profile kit is designed to track thread execution time 
   based on the hardware timer defined by TX_EXECUTION_TIME_SOURCE and 
   TX_EXECUTION_MAX_TIME_SOURCE below. When the thread's total time reaches 
   the maximum value, it remains there until the time is reset to 0 via a call
   to tx_thread_execution_time_reset. There are several assumptions to the 
   operation of this kit, as follows:

   1. In tx_port.h replace:
   
        #define TX_THREAD_EXTENSION_3" 
        
      with:
      
        #define TX_THREAD_EXTENSION_3           unsigned long long  tx_thread_execution_time_total; \
                                                unsigned long       tx_thread_execution_time_last_start; 
   
        Note: if 64-bit time source is present, the tx_thread_execution_time_last_start type should be unsigned long long.

   2. The TX_EXECUTION_TIME_SOURCE and TX_EXECUTION_MAX_TIME_SOURCE macros are 
      defined to utilize a local hardware time source.
   
   3. ThreadX 5.4 (or later) is being used, with the assembly code enabled to
      call the following routines from assembly code:
      
            VOID  _tx_execution_thread_enter(void);
            VOID  _tx_execution_thread_exit(void);
            VOID  _tx_execution_isr_enter(void);
            VOID  _tx_execution_isr_exit(void);
      
    4. The ThreadX library assembly code must be rebuilt with TX_ENABLE_EXECUTION_CHANGE_NOTIFY so 
       that these macros are expanded in the TX_THREAD structure and so the assembly code macros 
       are enabled to call the execution profile routines.  

    5. Add tx_execution_profile.c to the application build.  */


/* Externally reference several internal ThreadX variables.  */
   
extern ULONG                            _tx_thread_system_state[TX_THREAD_SMP_MAX_CORES];
extern UINT                             _tx_thread_preempt_disable;
extern TX_THREAD                        *_tx_thread_current_ptr[TX_THREAD_SMP_MAX_CORES];
extern TX_THREAD                        *_tx_thread_execute_ptr[TX_THREAD_SMP_MAX_CORES];
extern TX_THREAD                        *_tx_thread_created_ptr;
extern ULONG                            _tx_thread_created_count;


/* Define the total time for all threads.  This is accumulated as each thread's total time is accumulated.  */

EXECUTION_TIME                      _tx_execution_thread_time_total[TX_THREAD_SMP_MAX_CORES];


/* Define the ISR time gathering information. This is setup to track total ISR time presently, but
   could easily be expanded to track different ISRs. Also, only ISRs that utilize _tx_thread_context_save
   and _tx_thread_context_restore are tracked by this utility.  */

EXECUTION_TIME                      _tx_execution_isr_time_total[TX_THREAD_SMP_MAX_CORES];
EXECUTION_TIME_SOURCE_TYPE          _tx_execution_isr_time_last_start[TX_THREAD_SMP_MAX_CORES];          


/* Define the system idle time gathering information. For idle time that exceeds the range of the timer
   source, another timer source may be needed. In addition, the total thread execution time added to the 
   total ISR time, less the total system time is also a measure of idle time.  */

EXECUTION_TIME                      _tx_execution_idle_time_total[TX_THREAD_SMP_MAX_CORES];
EXECUTION_TIME_SOURCE_TYPE          _tx_execution_idle_time_last_start[TX_THREAD_SMP_MAX_CORES];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_enter                         PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
UINT                        core;


    /* Pickup the current time.  */
    current_time =  TX_EXECUTION_TIME_SOURCE;

    /* Pickup the core index.  */
    core =  TX_SMP_CORE_ID;

    /* Pickup the current thread control block.  */
    thread_ptr =  _tx_thread_current_ptr[core];

    /* This thread is being scheduled.  Simply setup the last start time in the
       thread control block.  */
    thread_ptr -> tx_thread_execution_time_last_start =  current_time;
    
    /* Pickup the last idle start time.  */
    last_start_time =  _tx_execution_idle_time_last_start[core];
    
    /* Determine if idle time is being measured.  */
    if (last_start_time)
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
            delta_time =  (EXECUTION_TIME) (current_time + (((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) - last_start_time));
        }
    
        /* Pickup the total time.  */
        total_time =  _tx_execution_idle_time_total[core];

        /* Now compute the new total time.  */
        new_total_time =  total_time + delta_time;
        
        /* Determine if a rollover on the total time is present.  */
        if (new_total_time < total_time)
        {
        
            /* Rollover. Set the total time to max value.  */
            new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
        }
    
        /* Now store back the total idle time.  */
        _tx_execution_idle_time_total[core] =  new_total_time;    
        
        /* Disable the idle time measurement.  */
        _tx_execution_idle_time_last_start[core] =  0;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_exit                          PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
UINT                        core;


    /* Pickup the core index.  */
    core =  TX_SMP_CORE_ID;

    /* Pickup the current thread control block.  */
    thread_ptr =  _tx_thread_current_ptr[core];

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
                delta_time =  (EXECUTION_TIME) (current_time + (((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) - last_start_time));
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
            
            /* Now accumulate this thread's execution time into the total thread execution time.   */
            new_total_time =  _tx_execution_thread_time_total[core] + delta_time;
            
            /* Determine if a rollover on the total time is present.  */
            if (new_total_time < _tx_execution_thread_time_total[core])
            {
        
                /* Rollover. Set the total time to max value.  */
                new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
            }            

            /* Store back the new total time.  */
            _tx_execution_thread_time_total[core] =  new_total_time;
        }
        
        /* Is the system now idle?  */
        if (_tx_thread_execute_ptr[core] == TX_NULL)
        {
        
            /* Yes, idle system. Pickup the start of idle time.  */
            _tx_execution_idle_time_last_start[core] =  TX_EXECUTION_TIME_SOURCE;
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_enter                            PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
UINT                        core;


    /* Pickup the core index.  */
    core =  TX_SMP_CORE_ID;

    /* Determine if this is the first interrupt. Nested interrupts are all treated as 
       general interrupt processing.  */
    if (_tx_thread_system_state[core] == 1)
    {
        /* Pickup the current time.  */
        current_time =  TX_EXECUTION_TIME_SOURCE;

        /* Pickup the current thread control block.  */
        thread_ptr =  _tx_thread_current_ptr[core];

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
                    delta_time =  (EXECUTION_TIME) (current_time + (((EXECUTION_TIME_SOURCE_TYPE) TX_EXECUTION_MAX_TIME_SOURCE) - last_start_time));
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

                /* Now accumulate this thread's execution time into the total thread execution time.   */
                new_total_time =  _tx_execution_thread_time_total[core] + delta_time;
            
                /* Determine if a rollover on the total time is present.  */
                if (new_total_time < _tx_execution_thread_time_total[core])
                {
        
                    /* Rollover. Set the total time to max value.  */
                    new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
                }            

                /* Store back the new total time.  */
                _tx_execution_thread_time_total[core] =  new_total_time;
            }
        }
        
        /* Has idle time started?  */
        else if (_tx_execution_idle_time_last_start[core])
        {
        
            /* Pickup the last idle start time.  */
            last_start_time =  _tx_execution_idle_time_last_start[core];
    
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
            total_time =  _tx_execution_idle_time_total[core];

            /* Now compute the new total time.  */
            new_total_time =  total_time + delta_time;
        
            /* Determine if a rollover on the total time is present.  */
            if (new_total_time < total_time)
            {
        
                /* Rollover. Set the total time to max value.  */
                new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
            }
    
            /* Now store back the total idle time.  */
            _tx_execution_idle_time_total[core] =  new_total_time;    
        
            /* Disable the idle time measurement.  */
            _tx_execution_idle_time_last_start[core] =  0;
        }

        /* Save the ISR start time.  */
        _tx_execution_isr_time_last_start[core] =  current_time;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_isr_exit                             PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
UINT                        core;


    /* Pickup the core index.  */
    core =  TX_SMP_CORE_ID;

    /* Determine if this is the first interrupt. Nested interrupts are all treated as 
       general interrupt processing.  */
    if (_tx_thread_system_state[core] == 1)
    {

        /* Pickup the current time.  */
        current_time =  TX_EXECUTION_TIME_SOURCE;

        /* Pickup the last start time.  */
        last_start_time =  _tx_execution_isr_time_last_start[core];
        
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
        total_time =  _tx_execution_isr_time_total[core];

        /* Now compute the new total time.  */
        new_total_time =  total_time + delta_time;
        
        /* Determine if a rollover on the total time is present.  */
        if (new_total_time < total_time)
        {
        
            /* Rollover. Set the total time to max value.  */
            new_total_time =  (EXECUTION_TIME) TX_EXECUTION_MAX_TIME_SOURCE;
        }
    
        /* Store back the new total time.  */
        _tx_execution_isr_time_total[core] =  new_total_time;
        
        /* Pickup the current thread control block.  */
        thread_ptr =  _tx_thread_current_ptr[core];

        /* Was a thread interrupted?  */
        if (thread_ptr)
        {

            /* Now determine if the thread will execution is going to occur immediately.  */
            if ((thread_ptr == _tx_thread_execute_ptr[core]) || (_tx_thread_preempt_disable))
            {
            
                /* Yes, setup the thread last start time in the thread control block.  */
                thread_ptr -> tx_thread_execution_time_last_start =  current_time;
            }
        }
        
        /* Determine if the system is now idle.  */
        if (_tx_thread_execute_ptr[core] == TX_NULL)
        {
        
            /* Yes, idle system. Pickup the start of idle time.  */
            _tx_execution_idle_time_last_start[core] =  TX_EXECUTION_TIME_SOURCE;
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_thread_time_reset                    PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
/*    _tx_execution_thread_total_time_reset              PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_total_time_reset(void)
{

TX_INTERRUPT_SAVE_AREA
            
TX_THREAD       *thread_ptr;            
UINT            total_threads;
UINT            core;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Reset the total time for all cores.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {
    
        _tx_execution_thread_time_total[core] =  0;
    }

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
/*    _tx_execution_isr_time_reset                       PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_isr_time_reset(void)
{

TX_INTERRUPT_SAVE_AREA

UINT            core;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Reset the ISR total time to 0.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {

        _tx_execution_isr_time_total[core] =  0;
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
/*    _tx_execution_idle_time_reset                      PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_idle_time_reset(void)
{

TX_INTERRUPT_SAVE_AREA

UINT            core;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Reset the idle total time to 0.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {

        _tx_execution_idle_time_total[core] =  0;
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
/*    _tx_execution_thread_time_get                      PORTABLE SMP     */
/*                                                           6.1.12       */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
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
/*    _tx_execution_thread_total_time_get                PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of all threads.               */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_thread_total_time_get(EXECUTION_TIME *total_time)
{

TX_INTERRUPT_SAVE_AREA

UINT            core;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Return the total thread time.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {

        *total_time =  _tx_execution_thread_time_total[core];
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
/*    _tx_execution_isr_time_get                         PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of all ISRs.                  */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_isr_time_get(EXECUTION_TIME *total_time)
{

TX_INTERRUPT_SAVE_AREA

UINT            core;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Return the total ISR time.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {

        /* Return the total time.  */
        *total_time =  _tx_execution_isr_time_total[core];
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
/*    _tx_execution_idle_time_get                        PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the total system idle time.                      */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_idle_time_get(EXECUTION_TIME *total_time)
{

TX_INTERRUPT_SAVE_AREA

UINT            core;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Return the total time.  */
    for (core = 0; core < TX_THREAD_SMP_MAX_CORES; core++)
    {

        /* Return the total time.  */
        *total_time =  _tx_execution_idle_time_total[core];
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
/*    _tx_execution_core_thread_total_time_get           PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the total thread execution time of the           */
/*    specified core.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    core                              Specified core                    */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_core_thread_total_time_get(UINT core, EXECUTION_TIME *total_time)
{

    /* Determine if the core is valid.  */
    if (core >= TX_THREAD_SMP_MAX_CORES)
    {
    
        /* Invalid core, return an error.  */
        return(TX_NOT_DONE);
    }

    /* Return the total thread time for the core.  */
    *total_time =  _tx_execution_thread_time_total[core];
        
    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_core_isr_time_get                    PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the execution time of ISRs on the specified      */
/*    core.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    core                              Specified core                    */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_core_isr_time_get(UINT core, EXECUTION_TIME *total_time)
{

    /* Determine if the core is valid.  */
    if (core >= TX_THREAD_SMP_MAX_CORES)
    {
    
        /* Invalid core, return an error.  */
        return(TX_NOT_DONE);
    }

    /* Return the total ISR time for this core.  */
    *total_time =  _tx_execution_isr_time_total[core];

    /* Return success.  */
    return(TX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_execution_core_idle_time_get                   PORTABLE SMP     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the idle time of the specified core.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    core                              Specified core                    */
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
/*  07-29-2022      William E. Lamie        Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT  _tx_execution_core_idle_time_get(UINT core, EXECUTION_TIME *total_time)
{

    /* Determine if the core is valid.  */
    if (core >= TX_THREAD_SMP_MAX_CORES)
    {
    
        /* Invalid core, return an error.  */
        return(TX_NOT_DONE);
    }

    /* Return the total idle time for this core.  */
    *total_time =  _tx_execution_idle_time_total[core];

    /* Return success.  */
    return(TX_SUCCESS);
}
