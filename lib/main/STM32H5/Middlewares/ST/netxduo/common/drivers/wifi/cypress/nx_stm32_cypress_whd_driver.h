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
/*    Cypress CHIP WiFi driver for the STM32 family of microprocessors    */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

#ifndef NX_STM32_CYPRESS_WHD_DRIVER_H
#define NX_STM32_CYPRESS_WHD_DRIVER_H

#ifdef   __cplusplus
extern   "C" {
#endif

#include "whd.h"
#include "nx_api.h"

/* Mode for the WiFi module, i.e. Station or Access Point. */
typedef enum
{
  WIFI_MODE_STA,
  WIFI_MODE_AP
} wifi_mode_t;


/* Public API */
VOID nx_driver_cypress_whd_entry(NX_IP_DRIVER *driver_req_ptr);
UINT cypress_whd_alloc_init(VOID);

extern whd_interface_t *Ifp;
extern wifi_mode_t WifiMode;

#ifdef   __cplusplus
}
#endif

#endif /* NX_STM32_CYPRESS_WHD_DRIVER_H */
