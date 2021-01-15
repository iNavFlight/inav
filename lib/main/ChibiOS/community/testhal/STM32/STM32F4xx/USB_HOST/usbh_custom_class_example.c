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

#include "hal.h"

#if HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS

#include <string.h>
#include "usbh_custom_class_example.h"
#include "usbh/internal.h"

#if USBH_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBH_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBH_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBH_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif

/*===========================================================================*/
/* USB Class driver loader for Custom Class Example				 		 	 */
/*===========================================================================*/

USBHCustomDriver USBHCUSTOMD[USBH_CUSTOM_CLASS_MAX_INSTANCES];

static void _init(void);
static usbh_baseclassdriver_t *_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_init,
	_load,
	_unload
};

const usbh_classdriverinfo_t usbhCustomClassDriverInfo = {
	"CUSTOM", &class_driver_vmt
};

static usbh_baseclassdriver_t *_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHCustomDriver *custp;
	(void)dev;

	if (_usbh_match_vid_pid(dev, 0xABCD, 0x0123) != HAL_SUCCESS)
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;

	/* alloc driver */
	for (i = 0; i < USBH_CUSTOM_CLASS_MAX_INSTANCES; i++) {
		if (USBHCUSTOMD[i].dev == NULL) {
			custp = &USBHCUSTOMD[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc CUSTOM driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	custp->ifnum = ifdesc->bInterfaceNumber;

	/* parse the configuration descriptor */
	if_iterator_t iif;
	generic_iterator_t iep;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			/* ... */
		} else {
			uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
					epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}

	custp->state = USBHCUSTOM_STATE_ACTIVE;

	return (usbh_baseclassdriver_t *)custp;

}

static void _unload(usbh_baseclassdriver_t *drv) {
	(void)drv;
}

static void _object_init(USBHCustomDriver *custp) {
	osalDbgCheck(custp != NULL);
	memset(custp, 0, sizeof(*custp));
	custp->state = USBHCUSTOM_STATE_STOP;
}

static void _init(void) {
	uint8_t i;
	for (i = 0; i < USBH_CUSTOM_CLASS_MAX_INSTANCES; i++) {
		_object_init(&USBHCUSTOMD[i]);
	}
}

#endif
