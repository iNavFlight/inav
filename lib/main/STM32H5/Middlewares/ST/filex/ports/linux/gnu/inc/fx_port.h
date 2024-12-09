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
/*    fx_port.h                                           Linux/GCC       */ 
/*                                                           6.1.8        */
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
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  03-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.5  */
/*  08-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.8  */
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
#include <stdio.h>


/* Include the ThreadX api file.  */

#ifndef FX_STANDALONE_ENABLE

#include "tx_api.h"

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

#ifdef FX_REGRESSION_TEST
/* Define parameters for regression test suite.  */

#define FX_MAX_SECTOR_CACHE                    256
#define FX_MAX_FAT_CACHE                       64
#define FX_FAT_MAP_SIZE                        1


/* Define variables and macros used to introduce errors for the regression test suite.  */

extern  ULONG   _fx_ram_driver_io_error_request;
extern  ULONG   _fx_ram_driver_io_request_count;
extern  ULONG   _fx_file_open_max_file_size_request;
extern  ULONG   _fx_directory_entry_read_count;
extern  ULONG   _fx_directory_entry_read_error_request;
extern  ULONG   _fx_directory_entry_write_count;
extern  ULONG   _fx_directory_entry_write_error_request;
extern  ULONG   _fx_utility_fat_entry_write_count;
extern  ULONG   _fx_utility_fat_entry_write_error_request;
extern  ULONG   _fx_utility_fat_entry_read_count;
extern  ULONG   _fx_utility_fat_entry_read_error_request;
extern  ULONG   _fx_utility_logical_sector_flush_count;
extern  ULONG   _fx_utility_logical_sector_flush_error_request;
extern  ULONG   _fx_utility_logical_sector_write_count;
extern  ULONG   _fx_utility_logical_sector_write_error_request;
extern  ULONG   _fx_utility_logical_sector_read_count;
extern  ULONG   _fx_utility_logical_sector_read_error_request;
extern  ULONG   _fx_utility_logical_sector_read_1_count;
extern  ULONG   _fx_utility_logical_sector_read_1_error_request;

#ifdef FX_ENABLE_FAULT_TOLERANT
struct FX_MEDIA_STRUCT;
extern VOID fault_tolerant_enable_callback(struct FX_MEDIA_STRUCT *media_ptr, 
                                           UCHAR *fault_tolerant_memory_buffer,
                                           ULONG log_size);
extern VOID fault_tolerant_apply_log_callback(struct FX_MEDIA_STRUCT *media_ptr, 
                                              UCHAR *fault_tolerant_memory_buffer,
                                              ULONG log_size);
#endif /* FX_ENABLE_FAULT_TOLERANT */


#define FX_DIRECTORY_ENTRY_READ_EXTENSION               _fx_directory_entry_read_count++;                               \
                                                        if (_fx_directory_entry_read_error_request)                     \
                                                        {                                                               \
                                                            _fx_directory_entry_read_error_request--;                   \
                                                            if (_fx_directory_entry_read_error_request == 0)            \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_DIRECTORY_ENTRY_WRITE_EXTENSION              _fx_directory_entry_write_count++;                              \
                                                        if (_fx_directory_entry_write_error_request)                    \
                                                        {                                                               \
                                                            _fx_directory_entry_write_error_request--;                  \
                                                            if (_fx_directory_entry_write_error_request == 0)           \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_FAT_ENTRY_READ_EXTENSION             _fx_utility_fat_entry_read_count++;                             \
                                                        if (_fx_utility_fat_entry_read_error_request)                   \
                                                        {                                                               \
                                                            _fx_utility_fat_entry_read_error_request--;                 \
                                                            if (_fx_utility_fat_entry_read_error_request == 0)          \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                            if (_fx_utility_fat_entry_read_error_request == 10000)      \
                                                            {                                                           \
                                                                *entry_ptr =  1;                                        \
                                                                _fx_utility_fat_entry_read_error_request =  0;          \
                                                                return(FX_SUCCESS);                                     \
                                                            }                                                           \
                                                            if (_fx_utility_fat_entry_read_error_request == 20000)      \
                                                            {                                                           \
                                                                *entry_ptr =  media_ptr -> fx_media_fat_reserved;       \
                                                                _fx_utility_fat_entry_read_error_request =  0;          \
                                                                return(FX_SUCCESS);                                     \
                                                            }                                                           \
                                                            if (_fx_utility_fat_entry_read_error_request == 30000)      \
                                                            {                                                           \
                                                                *entry_ptr =  cluster;                                  \
                                                                _fx_utility_fat_entry_read_error_request =  0;          \
                                                                return(FX_SUCCESS);                                     \
                                                            }                                                           \
                                                            if (_fx_utility_fat_entry_read_error_request == 40000)      \
                                                            {                                                           \
                                                                media_ptr -> fx_media_total_clusters =  0;              \
                                                                _fx_utility_fat_entry_read_error_request =  0;          \
                                                                return(FX_SUCCESS);                                     \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION            _fx_utility_fat_entry_write_count++;                            \
                                                        if (_fx_utility_fat_entry_write_error_request)                  \
                                                        {                                                               \
                                                            _fx_utility_fat_entry_write_error_request--;                \
                                                            if (_fx_utility_fat_entry_write_error_request == 0)         \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_LOGICAL_SECTOR_FLUSH_EXTENSION       _fx_utility_logical_sector_flush_count++;                       \
                                                        if (_fx_utility_logical_sector_flush_error_request)             \
                                                        {                                                               \
                                                            _fx_utility_logical_sector_flush_error_request--;           \
                                                            if (_fx_utility_logical_sector_flush_error_request == 0)    \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION       _fx_utility_logical_sector_read_count++;                         \
                                                        if (_fx_utility_logical_sector_read_error_request)              \
                                                        {                                                               \
                                                            _fx_utility_logical_sector_read_error_request--;            \
                                                            if (_fx_utility_logical_sector_read_error_request == 0)     \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1      _fx_utility_logical_sector_read_1_count++;                      \
                                                        if (_fx_utility_logical_sector_read_1_error_request)            \
                                                        {                                                               \
                                                            _fx_utility_logical_sector_read_1_error_request--;          \
                                                            if (_fx_utility_logical_sector_read_1_error_request == 0)   \
                                                            {                                                           \
                                                                cache_entry =  FX_NULL;                                 \
                                                            }                                                           \
                                                        }

#define FX_UTILITY_LOGICAL_SECTOR_WRITE_EXTENSION       _fx_utility_logical_sector_write_count++;                       \
                                                        if (_fx_utility_logical_sector_write_error_request)             \
                                                        {                                                               \
                                                            _fx_utility_logical_sector_write_error_request--;           \
                                                            if (_fx_utility_logical_sector_write_error_request == 0)    \
                                                            {                                                           \
                                                                return(FX_IO_ERROR);                                    \
                                                            }                                                           \
                                                        }

#define FX_FAULT_TOLERANT_ENABLE_EXTENSION              fault_tolerant_enable_callback(media_ptr, media_ptr -> fx_media_fault_tolerant_memory_buffer, total_size);
#define FX_FAULT_TOLERANT_APPLY_LOGS_EXTENSION          fault_tolerant_apply_log_callback(media_ptr, media_ptr -> fx_media_fault_tolerant_memory_buffer, size);
#endif /* FX_REGRESSION_TEST */



/* Define FileX internal protection macros.  If FX_SINGLE_THREAD is defined,
   these protection macros are effectively disabled.  However, for multi-thread
   uses, the macros are setup to utilize a ThreadX mutex for multiple thread 
   access control into an open media.  */

/* Reduce the mutex error checking for testing purpose.  */

#if defined(FX_SINGLE_THREAD) || defined(FX_STANDALONE_ENABLE)
#define FX_PROTECT                   
#define FX_UNPROTECT
#else
#define FX_PROTECT                      tx_mutex_get(&(media_ptr -> fx_media_protect), TX_WAIT_FOREVER);
#define FX_UNPROTECT                    tx_mutex_put(&(media_ptr -> fx_media_protect));
#endif


/* Define interrupt lockout constructs to protect the system date/time from being updated
   while they are being read.  */

#ifndef FX_STANDALONE_ENABLE
#define FX_INT_SAVE_AREA                unsigned int  old_interrupt_posture;
#define FX_DISABLE_INTS                 old_interrupt_posture =  tx_interrupt_control(TX_INT_DISABLE);
#define FX_RESTORE_INTS                 tx_interrupt_control(old_interrupt_posture);
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

#define FX_CALLER_CHECKING_CODE         if ((_tx_thread_system_state) || \
                                            (_tx_thread_current_ptr == TX_NULL) || \
                                            (_tx_thread_current_ptr == &_tx_timer_thread)) \
                                            return(FX_CALLER_ERROR);

#else
#define FX_CALLER_CHECKING_EXTERNS      extern  TX_THREAD      *_tx_thread_current_ptr; \
                                        extern  volatile ULONG  _tx_thread_system_state;

#define FX_CALLER_CHECKING_CODE         if ((_tx_thread_system_state) || \
                                            (_tx_thread_current_ptr == TX_NULL)) \
                                            return(FX_CALLER_ERROR);
#endif
#else
#define FX_CALLER_CHECKING_EXTERNS
#define FX_CALLER_CHECKING_CODE
#endif

/* Define the update rate of the system timer.  These values may also be defined at the command
   line when compiling the fx_system_initialize.c module in the FileX library build.  Alternatively, they can
   be modified in this file.  Note: the update rate must be an even number of seconds greater
   than or equal to 2, which is the minimal update rate for FAT time. */

#ifndef FX_UPDATE_RATE_IN_SECONDS
#define FX_UPDATE_RATE_IN_SECONDS       10     /* Update time at 10 second intervals */
#endif

#ifndef FX_UPDATE_RATE_IN_TICKS
#define FX_UPDATE_RATE_IN_TICKS         1000   /* Same update rate, but in ticks  */
#endif


/* Define the version ID of FileX.  This may be utilized by the application.  */

#ifdef FX_SYSTEM_INIT
CHAR                            _fx_version_id[] = 
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  FileX Linux/GCC Version 6.2.0 *";
#else
extern  CHAR                    _fx_version_id[];
#endif

#endif

