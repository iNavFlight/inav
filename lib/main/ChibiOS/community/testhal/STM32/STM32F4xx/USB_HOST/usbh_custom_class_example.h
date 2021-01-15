/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2015..2017 Diego Ismirlian, (dismirlian (at) google's mail)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef USBH_CUSTOM_H_
#define USBH_CUSTOM_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
#define USBH_CUSTOM_CLASS_MAX_INSTANCES		1

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

typedef enum {
	USBHCUSTOM_STATE_UNINIT = 0,
	USBHCUSTOM_STATE_STOP = 1,
	USBHCUSTOM_STATE_ACTIVE = 2,
	USBHCUSTOM_STATE_READY = 3
} usbhcustom_state_t;

typedef struct USBHCustomDriver USBHCustomDriver;

struct USBHCustomDriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	uint8_t ifnum;

	usbhcustom_state_t state;
};


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern USBHCustomDriver USBHCUSTOMD[USBH_CUSTOM_CLASS_MAX_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
	/* API goes here */

#ifdef __cplusplus
}
#endif

#endif

#endif /* USBH_CUSTOM_H_ */
