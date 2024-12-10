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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_initialize.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX initialization component, including   */
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef TX_INITIALIZE_H
#define TX_INITIALIZE_H


/* Define constants that indicate initialization is in progress.  */

#define TX_INITIALIZE_IN_PROGRESS               ((ULONG) 0xF0F0F0F0UL)
#define TX_INITIALIZE_ALMOST_DONE               ((ULONG) 0xF0F0F0F1UL)
#define TX_INITIALIZE_IS_FINISHED               ((ULONG) 0x00000000UL)


/* Define internal initialization function prototypes.  */

VOID        _tx_initialize_high_level(VOID);
VOID        _tx_initialize_kernel_setup(VOID);
VOID        _tx_initialize_low_level(VOID);


/* Define the macro for adding additional port-specific global data. This macro is defined
   as white space, unless defined by tx_port.h.  */

#ifndef TX_PORT_SPECIFIC_DATA
#define TX_PORT_SPECIFIC_DATA
#endif


/* Define the macro for adding additional port-specific pre and post initialization processing.
   These macros is defined as white space, unless defined by tx_port.h.  */

#ifndef TX_PORT_SPECIFIC_PRE_INITIALIZATION
#define TX_PORT_SPECIFIC_PRE_INITIALIZATION
#endif

#ifndef TX_PORT_SPECIFIC_POST_INITIALIZATION
#define TX_PORT_SPECIFIC_POST_INITIALIZATION
#endif

#ifndef TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION
#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION
#endif


/* Initialization component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_INITIALIZE_INIT
#define INITIALIZE_DECLARE
#else
#define INITIALIZE_DECLARE extern
#endif


/* Define the unused memory pointer.  The value of the first available
   memory address is placed in this variable in the low-level
   initialization function.  The content of this variable is passed
   to the application's system definition function.  */

INITIALIZE_DECLARE VOID     *_tx_initialize_unused_memory;


#endif
