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
/**   IAR Multithreaded Library Support                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Define IAR library for tools prior to version 8.  */

#if (__VER__ < 8000000)


/* IAR version 7 and below.  */

/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_mutex.h"


/* This implementation requires that the following macros are defined in the 
   tx_port.h file and <yvals.h> is included with the following code segments:
   
#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#include <yvals.h>
#endif

#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#define TX_THREAD_EXTENSION_2           VOID    *tx_thread_iar_tls_pointer;          
#else
#define TX_THREAD_EXTENSION_2          
#endif

#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                      thread_ptr -> tx_thread_iar_tls_pointer =  __iar_dlib_perthread_allocate();                                  
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                      __iar_dlib_perthread_deallocate(thread_ptr -> tx_thread_iar_tls_pointer); \
                                                                    thread_ptr -> tx_thread_iar_tls_pointer =  TX_NULL;            
#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION               __iar_dlib_perthread_access(0);
#else
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                                  
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                                  
#endif

    This should be done automatically if TX_ENABLE_IAR_LIBRARY_SUPPORT is defined while building the ThreadX library and the 
    application.  

    Finally, the project options General Options -> Library Configuration should have the "Enable thread support in library" box selected.
*/

#ifdef TX_ENABLE_IAR_LIBRARY_SUPPORT

#include <yvals.h>


#if _MULTI_THREAD

TX_MUTEX    __tx_iar_system_lock_mutexes[_MAX_LOCK];
UINT        __tx_iar_system_lock_next_free_mutex =  0;


/* Define error counters, just for debug purposes.  */

UINT        __tx_iar_system_lock_no_mutexes;
UINT        __tx_iar_system_lock_internal_errors;
UINT        __tx_iar_system_lock_isr_caller;


/* Define the TLS access function for the IAR library.  */

void _DLIB_TLS_MEMORY *__iar_dlib_perthread_access(void _DLIB_TLS_MEMORY *symbp)
{

char _DLIB_TLS_MEMORY   *p = 0;
    
    /* Is there a current thread?  */
    if (_tx_thread_current_ptr)
      p = (char _DLIB_TLS_MEMORY *) _tx_thread_current_ptr -> tx_thread_iar_tls_pointer;
    else
      p = (void _DLIB_TLS_MEMORY *) __segment_begin("__DLIB_PERTHREAD");
    p += __IAR_DLIB_PERTHREAD_SYMBOL_OFFSET(symbp);
    return (void _DLIB_TLS_MEMORY *) p;
}


/* Define mutexes for IAR library.  */

void __iar_system_Mtxinit(__iar_Rmtx *m)
{

UINT        i;
UINT        status;
TX_MUTEX    *mutex_ptr;


    /* First, find a free mutex in the list.  */
    for (i = 0; i < _MAX_LOCK; i++)
    {

        /* Setup a pointer to the start of the next free mutex.  */
        mutex_ptr =  &__tx_iar_system_lock_mutexes[__tx_iar_system_lock_next_free_mutex++];
    
        /* Check for wrap-around on the next free mutex.  */
        if (__tx_iar_system_lock_next_free_mutex >= _MAX_LOCK)
        {
        
            /* Yes, set the free index back to 0.  */
            __tx_iar_system_lock_next_free_mutex =  0;
        }
    
        /* Is this mutex free?  */
        if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
        {
        
            /* Yes, this mutex is free, get out of the loop!  */
            break;
        }
    }

    /* Determine if a free mutex was found.   */
    if (i >= _MAX_LOCK)
    {
    
        /* Error!  No more free mutexes!  */
        
        /* Increment the no mutexes error counter.  */
        __tx_iar_system_lock_no_mutexes++;
        
        /* Set return pointer to NULL.  */
        *m =  TX_NULL;
        
        /* Return.  */
        return;
    }
    
    /* Now create the ThreadX mutex for the IAR library.  */
    status =  _tx_mutex_create(mutex_ptr, "IAR System Library Lock", TX_NO_INHERIT);
    
    /* Determine if the creation was successful.  */
    if (status == TX_SUCCESS)
    {
    
        /* Yes, successful creation, return mutex pointer.  */
        *m =  (VOID *) mutex_ptr;
    }
    else
    {
    
        /* Increment the internal error counter.  */
        __tx_iar_system_lock_internal_errors++;
    
        /* Return a NULL pointer to indicate an error.  */
        *m =  TX_NULL;
    }
}

void __iar_system_Mtxdst(__iar_Rmtx *m)
{

    /* Simply delete the mutex.  */
    _tx_mutex_delete((TX_MUTEX *) *m);
}

void __iar_system_Mtxlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex locks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Get the mutex.  */
        status =  _tx_mutex_get((TX_MUTEX *) *m, TX_WAIT_FOREVER);

        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_system_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_system_lock_isr_caller++;
    }
}

void __iar_system_Mtxunlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex unlocks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Release the mutex.  */
        status =  _tx_mutex_put((TX_MUTEX *) *m);
        
        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_system_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_system_lock_isr_caller++;
    }
}


#if _DLIB_FILE_DESCRIPTOR

TX_MUTEX    __tx_iar_file_lock_mutexes[_MAX_FLOCK];
UINT        __tx_iar_file_lock_next_free_mutex =  0;


/* Define error counters, just for debug purposes.  */

UINT        __tx_iar_file_lock_no_mutexes;
UINT        __tx_iar_file_lock_internal_errors;
UINT        __tx_iar_file_lock_isr_caller;


void __iar_file_Mtxinit(__iar_Rmtx *m)
{

UINT        i;
UINT        status;
TX_MUTEX    *mutex_ptr;


    /* First, find a free mutex in the list.  */
    for (i = 0; i < _MAX_FLOCK; i++)
    {

        /* Setup a pointer to the start of the next free mutex.  */
        mutex_ptr =  &__tx_iar_file_lock_mutexes[__tx_iar_file_lock_next_free_mutex++];
    
        /* Check for wrap-around on the next free mutex.  */
        if (__tx_iar_file_lock_next_free_mutex >= _MAX_LOCK)
        {
        
            /* Yes, set the free index back to 0.  */
            __tx_iar_file_lock_next_free_mutex =  0;
        }
    
        /* Is this mutex free?  */
        if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
        {
        
            /* Yes, this mutex is free, get out of the loop!  */
            break;
        }
    }

    /* Determine if a free mutex was found.   */
    if (i >= _MAX_LOCK)
    {
    
        /* Error!  No more free mutexes!  */
        
        /* Increment the no mutexes error counter.  */
        __tx_iar_file_lock_no_mutexes++;
        
        /* Set return pointer to NULL.  */
        *m =  TX_NULL;
        
        /* Return.  */
        return;
    }
    
    /* Now create the ThreadX mutex for the IAR library.  */
    status =  _tx_mutex_create(mutex_ptr, "IAR File Library Lock", TX_NO_INHERIT);
    
    /* Determine if the creation was successful.  */
    if (status == TX_SUCCESS)
    {
    
        /* Yes, successful creation, return mutex pointer.  */
        *m =  (VOID *) mutex_ptr;
    }
    else
    {
    
        /* Increment the internal error counter.  */
        __tx_iar_file_lock_internal_errors++;
    
        /* Return a NULL pointer to indicate an error.  */
        *m =  TX_NULL;
    }
}

void __iar_file_Mtxdst(__iar_Rmtx *m)
{

    /* Simply delete the mutex.  */
    _tx_mutex_delete((TX_MUTEX *) *m);
}

void __iar_file_Mtxlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex locks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Get the mutex.  */
        status =  _tx_mutex_get((TX_MUTEX *) *m, TX_WAIT_FOREVER);

        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_file_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_file_lock_isr_caller++;
    }
}

void __iar_file_Mtxunlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex unlocks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Release the mutex.  */
        status =  _tx_mutex_put((TX_MUTEX *) *m);
        
        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_file_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_file_lock_isr_caller++;
    }
}
#endif  /* _DLIB_FILE_DESCRIPTOR */

#endif  /* _MULTI_THREAD  */

#endif  /* TX_ENABLE_IAR_LIBRARY_SUPPORT  */

#else   /* IAR version 8 and above.  */


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_mutex.h"

/* This implementation requires that the following macros are defined in the 
   tx_port.h file and <yvals.h> is included with the following code segments:
   
#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#include <yvals.h>
#endif

#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#define TX_THREAD_EXTENSION_2           VOID    *tx_thread_iar_tls_pointer;          
#else
#define TX_THREAD_EXTENSION_2          
#endif

#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
void    *_tx_iar_create_per_thread_tls_area(void);
void    _tx_iar_destroy_per_thread_tls_area(void *tls_ptr);
void    __iar_Initlocks(void);

#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                      thread_ptr -> tx_thread_iar_tls_pointer =  __iar_dlib_perthread_allocate();                                  
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                      do {__iar_dlib_perthread_deallocate(thread_ptr -> tx_thread_iar_tls_pointer); \
                                                                        thread_ptr -> tx_thread_iar_tls_pointer =  TX_NULL; } while(0);
#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION               do {__iar_Initlocks();} while(0);
#else
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                                  
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                                  
#endif

    This should be done automatically if TX_ENABLE_IAR_LIBRARY_SUPPORT is defined while building the ThreadX library and the 
    application.  

    Finally, the project options General Options -> Library Configuration should have the "Enable thread support in library" box selected.
*/

#ifdef TX_ENABLE_IAR_LIBRARY_SUPPORT

#include <DLib_threads.h>


void * __aeabi_read_tp();

void* _tx_iar_create_per_thread_tls_area();
void _tx_iar_destroy_per_thread_tls_area(void *tls_ptr);

#pragma section="__iar_tls$$DATA"

/* Define the TLS access function for the IAR library.  */
void * __aeabi_read_tp(void)
{
  void *p = 0;
  TX_THREAD *thread_ptr = _tx_thread_current_ptr;
  if (thread_ptr)
  {
    p = thread_ptr->tx_thread_iar_tls_pointer;      
  }
  else
  {
    p = __section_begin("__iar_tls$$DATA");
  }
  return p;
}

/* Define the TLS creation and destruction to use malloc/free.  */

void* _tx_iar_create_per_thread_tls_area()
{
  UINT tls_size = __iar_tls_size();  
  
  /* Get memory for TLS.  */  
  void *p = malloc(tls_size);

  /* Initialize TLS-area and run constructors for objects in TLS */
  __iar_tls_init(p);
  return p;
}

void _tx_iar_destroy_per_thread_tls_area(void *tls_ptr)
{
  /* Destroy objects living in TLS */
  __call_thread_dtors();
  free(tls_ptr);  
}

#ifndef _MAX_LOCK
#define _MAX_LOCK 4
#endif

static TX_MUTEX    __tx_iar_system_lock_mutexes[_MAX_LOCK];
static UINT        __tx_iar_system_lock_next_free_mutex =  0;


/* Define error counters, just for debug purposes.  */

UINT        __tx_iar_system_lock_no_mutexes;
UINT        __tx_iar_system_lock_internal_errors;
UINT        __tx_iar_system_lock_isr_caller;


/* Define mutexes for IAR library.  */

void __iar_system_Mtxinit(__iar_Rmtx *m)
{

UINT        i;
UINT        status;
TX_MUTEX    *mutex_ptr;


    /* First, find a free mutex in the list.  */
    for (i = 0; i < _MAX_LOCK; i++)
    {

        /* Setup a pointer to the start of the next free mutex.  */
        mutex_ptr =  &__tx_iar_system_lock_mutexes[__tx_iar_system_lock_next_free_mutex++];
    
        /* Check for wrap-around on the next free mutex.  */
        if (__tx_iar_system_lock_next_free_mutex >= _MAX_LOCK)
        {
        
            /* Yes, set the free index back to 0.  */
            __tx_iar_system_lock_next_free_mutex =  0;
        }
    
        /* Is this mutex free?  */
        if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
        {
        
            /* Yes, this mutex is free, get out of the loop!  */
            break;
        }
    }

    /* Determine if a free mutex was found.   */
    if (i >= _MAX_LOCK)
    {
    
        /* Error!  No more free mutexes!  */
        
        /* Increment the no mutexes error counter.  */
        __tx_iar_system_lock_no_mutexes++;
        
        /* Set return pointer to NULL.  */
        *m =  TX_NULL;
        
        /* Return.  */
        return;
    }
    
    /* Now create the ThreadX mutex for the IAR library.  */
    status =  _tx_mutex_create(mutex_ptr, "IAR System Library Lock", TX_NO_INHERIT);
    
    /* Determine if the creation was successful.  */
    if (status == TX_SUCCESS)
    {
    
        /* Yes, successful creation, return mutex pointer.  */
        *m =  (VOID *) mutex_ptr;
    }
    else
    {
    
        /* Increment the internal error counter.  */
        __tx_iar_system_lock_internal_errors++;
    
        /* Return a NULL pointer to indicate an error.  */
        *m =  TX_NULL;
    }
}

void __iar_system_Mtxdst(__iar_Rmtx *m)
{

    /* Simply delete the mutex.  */
    _tx_mutex_delete((TX_MUTEX *) *m);
}

void __iar_system_Mtxlock(__iar_Rmtx *m)
{
  if (*m)
  {  
    UINT    status;

    /* Determine the caller's context. Mutex locks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Get the mutex.  */
        status =  _tx_mutex_get((TX_MUTEX *) *m, TX_WAIT_FOREVER);

        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_system_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_system_lock_isr_caller++;
    }
  }
}

void __iar_system_Mtxunlock(__iar_Rmtx *m)
{
  if (*m)
  {
    UINT    status;

    /* Determine the caller's context. Mutex unlocks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Release the mutex.  */
        status =  _tx_mutex_put((TX_MUTEX *) *m);
        
        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_system_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_system_lock_isr_caller++;
    }
  }
}


#if _DLIB_FILE_DESCRIPTOR

#include <stdio.h>                        /* Added to get access to FOPEN_MAX */
#ifndef _MAX_FLOCK
#define _MAX_FLOCK FOPEN_MAX              /* Define _MAX_FLOCK as the maximum number of open files */
#endif


TX_MUTEX    __tx_iar_file_lock_mutexes[_MAX_FLOCK];
UINT        __tx_iar_file_lock_next_free_mutex =  0;


/* Define error counters, just for debug purposes.  */

UINT        __tx_iar_file_lock_no_mutexes;
UINT        __tx_iar_file_lock_internal_errors;
UINT        __tx_iar_file_lock_isr_caller;


void __iar_file_Mtxinit(__iar_Rmtx *m)
{

UINT        i;
UINT        status;
TX_MUTEX    *mutex_ptr;


    /* First, find a free mutex in the list.  */
    for (i = 0; i < _MAX_FLOCK; i++)
    {

        /* Setup a pointer to the start of the next free mutex.  */
        mutex_ptr =  &__tx_iar_file_lock_mutexes[__tx_iar_file_lock_next_free_mutex++];
    
        /* Check for wrap-around on the next free mutex.  */
        if (__tx_iar_file_lock_next_free_mutex >= _MAX_LOCK)
        {
        
            /* Yes, set the free index back to 0.  */
            __tx_iar_file_lock_next_free_mutex =  0;
        }
    
        /* Is this mutex free?  */
        if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
        {
        
            /* Yes, this mutex is free, get out of the loop!  */
            break;
        }
    }

    /* Determine if a free mutex was found.   */
    if (i >= _MAX_LOCK)
    {
    
        /* Error!  No more free mutexes!  */
        
        /* Increment the no mutexes error counter.  */
        __tx_iar_file_lock_no_mutexes++;
        
        /* Set return pointer to NULL.  */
        *m =  TX_NULL;
        
        /* Return.  */
        return;
    }
    
    /* Now create the ThreadX mutex for the IAR library.  */
    status =  _tx_mutex_create(mutex_ptr, "IAR File Library Lock", TX_NO_INHERIT);
    
    /* Determine if the creation was successful.  */
    if (status == TX_SUCCESS)
    {
    
        /* Yes, successful creation, return mutex pointer.  */
        *m =  (VOID *) mutex_ptr;
    }
    else
    {
    
        /* Increment the internal error counter.  */
        __tx_iar_file_lock_internal_errors++;
    
        /* Return a NULL pointer to indicate an error.  */
        *m =  TX_NULL;
    }
}

void __iar_file_Mtxdst(__iar_Rmtx *m)
{

    /* Simply delete the mutex.  */
    _tx_mutex_delete((TX_MUTEX *) *m);
}

void __iar_file_Mtxlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex locks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Get the mutex.  */
        status =  _tx_mutex_get((TX_MUTEX *) *m, TX_WAIT_FOREVER);

        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_file_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_file_lock_isr_caller++;
    }
}

void __iar_file_Mtxunlock(__iar_Rmtx *m)
{

UINT    status;


    /* Determine the caller's context. Mutex unlocks are only available from initialization and 
       threads.  */
    if ((_tx_thread_system_state == 0) || (_tx_thread_system_state >= TX_INITIALIZE_IN_PROGRESS))
    {
    
        /* Release the mutex.  */
        status =  _tx_mutex_put((TX_MUTEX *) *m);
        
        /* Check the status of the mutex release.  */
        if (status)
        {
        
            /* Internal error, increment the counter.  */
            __tx_iar_file_lock_internal_errors++;
        }
    }
    else
    {
        
        /* Increment the ISR caller error.  */
        __tx_iar_file_lock_isr_caller++;
    }
}
#endif  /* _DLIB_FILE_DESCRIPTOR */

#endif  /* TX_ENABLE_IAR_LIBRARY_SUPPORT  */

#endif /* IAR version 8 and above.  */
