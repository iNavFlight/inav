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
/*                                                                        */
/*                                                                        */
/*  NetX Component                                                        */
/*                                                                        */
/*    MX CHIP EMW3080 WiFi driver for the STM32 family of microprocessors */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

#ifndef NX_DRIVER_EMW3080_H
#define NX_DRIVER_EMW3080_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Indicate that driver source is being compiled. */
#define NX_DRIVER_SOURCE

/* Include driver framework include file. */
#include "nx_driver_framework.h"

/* Public API */
void nx_driver_emw3080_entry(NX_IP_DRIVER *driver_req_ptr);
void nx_driver_emw3080_interrupt(void);

extern uint8_t WifiMode;

#ifdef   __cplusplus
}
#endif /* __cplusplus */

#endif /* NX_DRIVER_EMW3080_H */
