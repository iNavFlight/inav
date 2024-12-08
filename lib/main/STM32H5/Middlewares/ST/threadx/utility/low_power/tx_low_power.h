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
/**   Low Power Timer Management                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_low_power.h                                      PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines prototypes for the low-power timer additions      */
/*    required for sleep mode.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-02-2021      William E. Lamie        Initial Version 6.1.5         */
/*                                                                        */
/**************************************************************************/

#ifndef  TX_LOW_POWER_H
#define  TX_LOW_POWER_H

/* Declare low-power function prototypes.  */

VOID        tx_low_power_enter(VOID);
VOID        tx_low_power_exit(VOID);
VOID        tx_time_increment(ULONG time_increment);
ULONG       tx_timer_get_next(ULONG *next_timer_tick_ptr);

#endif
