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
/** FileX Component                                                       */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */ 
/*                                                                        */ 
/*    fx_port.h                                            Generic        */ 
/*                                                           6.1.5        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file contains data type definitions that make the FileX FAT    */ 
/*    compatible file system function identically on a variety of         */ 
/*    different processor architectures.  For example, the byte offset of */ 
/*    various entries in the boot record, and directory entries are       */ 
/*    defined in this file.                                               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  11-09-2020     William E. Lamie         Initial Version 6.1.2         */
/*  03-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/

#ifndef FX_PORT_H
#define FX_PORT_H


/* Determine if the optional FileX user define file should be used.  */

#ifdef FX_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in fx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "fx_user.h"
#endif


/* Include the ThreadX api file.  */

#ifndef FX_STANDALONE_ENABLE
#include "tx_api.h"


/* Define ULONG64 typedef, if not already defined.  */

#ifndef ULONG64_DEFINED
#define ULONG64_DEFINED
typedef unsigned long long  ULONG64;
#endif

#else

/* Define compiler library include files.  */

#include <stdint.h>
#include <stdlib.h>

#define VOID                                    void
typedef char                                    CHAR;
typedef char                                    BOOL;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;

#ifndef ULONG64_DEFINED
#define ULONG64_DEFINED
typedef unsigned long long                      ULONG64;
#endif

/* Define basic alignment type used in block and byte pool operations. This data type must
   be at least 32-bits in size and also be large enough to hold a pointer type.  */

#ifndef ALIGN_TYPE_DEFINED
#define ALIGN_TYPE_DEFINED
#define ALIGN_TYPE                              ULONG
#endif

#endif

/* Define FileX internal protection macros.  If FX_SINGLE_THREAD is defined,
   these protection macros are effectively disabled.  However, for multi-thread
   uses, the macros are setup to utilize a ThreadX mutex for multiple thread 
   access control into an open media.  */

#if defined(FX_SINGLE_THREAD) || defined(FX_STANDALONE_ENABLE)
#define FX_PROTECT                   
#define FX_UNPROTECT
#else
#define FX_PROTECT                      if (media_ptr -> fx_media_id != FX_MEDIA_ID) return(FX_MEDIA_NOT_OPEN); \
                                        else if (tx_mutex_get(&(media_ptr -> fx_media_protect), TX_WAIT_FOREVER) != TX_SUCCESS) return(FX_MEDIA_NOT_OPEN);
#define FX_UNPROTECT                    tx_mutex_put(&(media_ptr -> fx_media_protect));
#endif


/* Define interrupt lockout constructs to protect the system date/time from being updated
   while they are being read.  */
#ifndef FX_STANDALONE_ENABLE
#ifndef FX_INT_SAVE_AREA
#define FX_INT_SAVE_AREA                unsigned int  old_interrupt_posture;
#endif

#ifndef FX_DISABLE_INTS
#define FX_DISABLE_INTS                 old_interrupt_posture =  tx_interrupt_control(TX_INT_DISABLE);
#endif

#ifndef FX_RESTORE_INTS
#define FX_RESTORE_INTS                 tx_interrupt_control(old_interrupt_posture);
#endif
#else
/* Disable use of ThreadX protection in standalone mode for FileX */
#ifndef FX_LEGACY_INTERRUPT_PROTECTION
#define FX_LEGACY_INTERRUPT_PROTECTION
#endif
#define FX_INT_SAVE_AREA
#define FX_DISABLE_INTS
#define FX_RESTORE_INTS
#endif

/* Define the error checking logic to determine if there is a caller error in the FileX API.  
   The default definitions assume ThreadX is being used.  This code can be completely turned 
   off by just defining these macros to white space.  */

#ifndef FX_STANDALONE_ENABLE
#ifndef TX_TIMER_PROCESS_IN_ISR

#define FX_CALLER_CHECKING_EXTERNS      extern  TX_THREAD      *_tx_thread_current_ptr; \
                                        extern  TX_THREAD       _tx_timer_thread; \
                                        extern  volatile ULONG  _tx_thread_system_state;

#define FX_CALLER_CHECKING_CODE         if ((TX_THREAD_GET_SYSTEM_STATE()) || \
                                            (_tx_thread_current_ptr == TX_NULL) || \
                                            (_tx_thread_current_ptr == &_tx_timer_thread)) \
                                            return(FX_CALLER_ERROR);

#else
#define FX_CALLER_CHECKING_EXTERNS      extern  TX_THREAD      *_tx_thread_current_ptr; \
                                        extern  volatile ULONG  _tx_thread_system_state;

#define FX_CALLER_CHECKING_CODE         if ((TX_THREAD_GET_SYSTEM_STATE()) || \
                                            (_tx_thread_current_ptr == TX_NULL)) \
                                            return(FX_CALLER_ERROR);
#endif
#else
#define FX_CALLER_CHECKING_EXTERNS
#define FX_CALLER_CHECKING_CODE
#endif


/* Define the update rate of the system timer.  These values may also be defined at the command
   line when compiling the fx_system_initialize.c module in the FileX library build.  Alternatively, they can
   be modified in this file or fx_user.h. Note: the update rate must be an even number of seconds greater
   than or equal to 2, which is the minimal update rate for FAT time. */

/* Define the number of seconds the timer parameters are updated in FileX.  The default
   value is 10 seconds.  This value can be overwritten externally. */

#ifndef FX_UPDATE_RATE_IN_SECONDS
#define FX_UPDATE_RATE_IN_SECONDS 10
#endif


/* Defines the number of ThreadX timer ticks required to achieve the update rate specified by 
   FX_UPDATE_RATE_IN_SECONDS defined previously. By default, the ThreadX timer tick is 10ms, 
   so the default value for this constant is 1000.  If TX_TIMER_TICKS_PER_SECOND is defined,
   this value is derived from TX_TIMER_TICKS_PER_SECOND.  */
 
#ifndef FX_UPDATE_RATE_IN_TICKS
#if (defined(TX_TIMER_TICKS_PER_SECOND) && (!defined(FX_STANDALONE_ENABLE)))
#define FX_UPDATE_RATE_IN_TICKS         (TX_TIMER_TICKS_PER_SECOND * FX_UPDATE_RATE_IN_SECONDS)
#else
#define FX_UPDATE_RATE_IN_TICKS         1000 
#endif
#endif


/* Define the version ID of FileX.  This may be utilized by the application.  */

#ifdef FX_SYSTEM_INIT
CHAR                            _fx_version_id[] = 
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  FileX Generic Version 6.2.0 *";
#else
extern  CHAR                    _fx_version_id[];
#endif

#endif

