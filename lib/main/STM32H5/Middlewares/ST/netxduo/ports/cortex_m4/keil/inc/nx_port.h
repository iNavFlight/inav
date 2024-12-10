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
/** NetX Component                                                        */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */ 
/*                                                                        */ 
/*    nx_port.h                                         Cortex-M4/Keil    */ 
/*                                                          6.1.11        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file contains data type definitions that make the NetX         */ 
/*    real-time TCP/IP function identically on a variety of different     */ 
/*    processor architectures.                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            renamed temporary variable, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_PORT_H
#define NX_PORT_H

/* Determine if the optional NetX user define file should be used.  */

#ifdef NX_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in nx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "nx_user.h"
#endif


/* Default to little endian, since this is what most ARM targets are.  */

#define NX_LITTLE_ENDIAN    1


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Define various constants for the port.  */ 

#ifndef NX_IP_PERIODIC_RATE
#define NX_IP_PERIODIC_RATE 100             /* Default IP periodic rate of 1 second for 
                                               ports with 10ms timer interrupts.  This 
                                               value may be defined instead at the 
                                               command line and this value will not be
                                               used.  */
#endif


/* Define macros that swap the endian for little endian ports.  */
#ifdef NX_LITTLE_ENDIAN
#define NX_CHANGE_ULONG_ENDIAN(arg)                         \
    {                                                       \
        ULONG _i;                                           \
        ULONG _tmp;                                         \
        _i = (UINT)arg;                                     \
        /* _i = A, B, C, D */                               \
        _tmp = _i ^ (((_i) >> 16) | (_i << 16));            \
        /* _tmp = _i ^ (_i ROR 16) = A^C, B^D, C^A, D^B */  \
        _tmp &= 0xff00ffff;                                 \
        /* _tmp = A^C, 0, C^A, D^B */                       \
        _i = ((_i) >> 8) | (_i<<24);                        \
        /* _i = D, A, B, C */                               \
        _i = _i ^ ((_tmp) >> 8);                            \
        /* _i = D, C, B, A */                               \
        arg = _i;                                           \
    }
#define NX_CHANGE_USHORT_ENDIAN(a)      a = (((a >> 8) | (a << 8)) & 0xFFFF)


#define __SWAP32__(val) ((((val) & 0xFF000000) >> 24 ) | (((val) & 0x00FF0000) >> 8) \
			 | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24))

#define __SWAP16__(val) ((((val) & 0xFF00) >> 8) | (((val) & 0x00FF) << 8))


#ifndef htonl
#define htonl(val)  __SWAP32__(val)
#endif /* htonl */
#ifndef ntohl
#define ntohl(val)  __SWAP32__(val)
#endif /* htonl */

#ifndef htons
#define htons(val)  __SWAP16__(val)
#endif /*htons */

#ifndef ntohs
#define ntohs(val)  __SWAP16__(val)
#endif /*htons */


#else
#define NX_CHANGE_ULONG_ENDIAN(a)
#define NX_CHANGE_USHORT_ENDIAN(a)

#ifndef htons
#define htons(val) (val)
#endif /* htons */

#ifndef ntohs
#define ntohs(val) (val)
#endif /* ntohs */

#ifndef ntohl
#define ntohl(val) (val)
#endif

#ifndef htonl
#define htonl(val) (val)
#endif /* htonl */
#endif


/* Define several macros for the error checking shell in NetX.  */

#ifndef TX_TIMER_PROCESS_IN_ISR

#define NX_CALLER_CHECKING_EXTERNS          extern  TX_THREAD           *_tx_thread_current_ptr; \
                                            extern  TX_THREAD           _tx_timer_thread; \
                                            extern  volatile ULONG      _tx_thread_system_state;

#define NX_THREADS_ONLY_CALLER_CHECKING     if ((TX_THREAD_GET_SYSTEM_STATE()) || \
                                                (_tx_thread_current_ptr == TX_NULL) || \
                                                (_tx_thread_current_ptr == &_tx_timer_thread)) \
                                                return(NX_CALLER_ERROR);

#define NX_INIT_AND_THREADS_CALLER_CHECKING if (((TX_THREAD_GET_SYSTEM_STATE()) && (TX_THREAD_GET_SYSTEM_STATE() < ((ULONG) 0xF0F0F0F0))) || \
                                                (_tx_thread_current_ptr == &_tx_timer_thread)) \
                                                return(NX_CALLER_ERROR);


#define NX_NOT_ISR_CALLER_CHECKING          if ((TX_THREAD_GET_SYSTEM_STATE()) && (TX_THREAD_GET_SYSTEM_STATE() < ((ULONG) 0xF0F0F0F0))) \
                                                return(NX_CALLER_ERROR);

#define NX_THREAD_WAIT_CALLER_CHECKING      if ((wait_option) && \
                                               ((_tx_thread_current_ptr == NX_NULL) || (TX_THREAD_GET_SYSTEM_STATE()) || (_tx_thread_current_ptr == &_tx_timer_thread))) \
                                            return(NX_CALLER_ERROR);


#else



#define NX_CALLER_CHECKING_EXTERNS          extern  TX_THREAD           *_tx_thread_current_ptr; \
                                            extern  volatile ULONG      _tx_thread_system_state;

#define NX_THREADS_ONLY_CALLER_CHECKING     if ((TX_THREAD_GET_SYSTEM_STATE()) || \
                                                (_tx_thread_current_ptr == TX_NULL)) \
                                                return(NX_CALLER_ERROR);

#define NX_INIT_AND_THREADS_CALLER_CHECKING if (((TX_THREAD_GET_SYSTEM_STATE()) && (TX_THREAD_GET_SYSTEM_STATE() < ((ULONG) 0xF0F0F0F0)))) \
                                                return(NX_CALLER_ERROR);

#define NX_NOT_ISR_CALLER_CHECKING          if ((TX_THREAD_GET_SYSTEM_STATE()) && (TX_THREAD_GET_SYSTEM_STATE() < ((ULONG) 0xF0F0F0F0))) \
                                                return(NX_CALLER_ERROR);

#define NX_THREAD_WAIT_CALLER_CHECKING      if ((wait_option) && \
                                               ((_tx_thread_current_ptr == NX_NULL) || (TX_THREAD_GET_SYSTEM_STATE()))) \
                                            return(NX_CALLER_ERROR);

#endif


/* Define the version ID of NetX.  This may be utilized by the application.  */

#ifdef NX_SYSTEM_INIT
CHAR                            _nx_version_id[] = 
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  NetX Duo Cortex-M4/Keil Version 6.2.0 *";
#else
extern  CHAR                    _nx_version_id[];
#endif

#endif

