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

#ifndef USBH_HID_H_
#define USBH_HID_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_HID

/* TODO:
 *
 */


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
#if !defined(HAL_USBHHID_USE_INTERRUPT_OUT)
#define HAL_USBHHID_USE_INTERRUPT_OUT 				FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

typedef enum {
	USBHHID_STATE_UNINIT = 0,
	USBHHID_STATE_STOP = 1,
	USBHHID_STATE_ACTIVE = 2,
	USBHHID_STATE_READY = 3
} usbhhid_state_t;

typedef enum {
	USBHHID_DEVTYPE_GENERIC = 0,
	USBHHID_DEVTYPE_BOOT_KEYBOARD = 1,
	USBHHID_DEVTYPE_BOOT_MOUSE = 2,
} usbhhid_devtype_t;

typedef enum {
	USBHHID_REPORTTYPE_INPUT = 1,
	USBHHID_REPORTTYPE_OUTPUT = 2,
	USBHHID_REPORTTYPE_FEATURE = 3,
} usbhhid_reporttype_t;

typedef enum {
	USBHHID_PROTOCOL_BOOT = 0,
	USBHHID_PROTOCOL_REPORT = 1,
} usbhhid_protocol_t;

typedef struct USBHHIDDriver USBHHIDDriver;
typedef struct USBHHIDConfig USBHHIDConfig;

typedef void (*usbhhid_report_callback)(USBHHIDDriver *hidp, uint16_t len);

struct USBHHIDConfig {
	usbhhid_report_callback cb_report;
	void *report_buffer;
	uint16_t report_len;
	usbhhid_protocol_t protocol;
};

struct USBHHIDDriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	usbh_ep_t epin;
#if HAL_USBHHID_USE_INTERRUPT_OUT
	usbh_ep_t epout;
#endif
	uint8_t ifnum;

	usbhhid_devtype_t type;
	usbhhid_state_t state;

	usbh_urb_t in_urb;

	const USBHHIDConfig *config;

	semaphore_t sem;
};


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern USBHHIDDriver USBHHIDD[HAL_USBHHID_MAX_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
	/* HID Common API */
	usbh_urbstatus_t usbhhidGetReport(USBHHIDDriver *hidp,
			uint8_t report_id, usbhhid_reporttype_t report_type,
			void *data, uint16_t len);
	usbh_urbstatus_t usbhhidSetReport(USBHHIDDriver *hidp,
			uint8_t report_id, usbhhid_reporttype_t report_type,
			const void *data, uint16_t len);
	usbh_urbstatus_t usbhhidGetIdle(USBHHIDDriver *hidp, uint8_t report_id, uint8_t *duration);
	usbh_urbstatus_t usbhhidSetIdle(USBHHIDDriver *hidp, uint8_t report_id, uint8_t duration);
	usbh_urbstatus_t usbhhidGetProtocol(USBHHIDDriver *hidp, uint8_t *protocol);
	usbh_urbstatus_t usbhhidSetProtocol(USBHHIDDriver *hidp, uint8_t protocol);

	static inline uint8_t usbhhidGetType(USBHHIDDriver *hidp) {
		return hidp->type;
	}

	static inline usbhhid_state_t usbhhidGetState(USBHHIDDriver *hidp) {
		return hidp->state;
	}

	void usbhhidStart(USBHHIDDriver *hidp, const USBHHIDConfig *cfg);
#ifdef __cplusplus
}
#endif

#endif

#endif /* USBH_HID_H_ */
