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

#if HAL_USBH_USE_HID

#if !HAL_USE_USBH
#error "USBHHID needs USBH"
#endif

#include <string.h>
#include "usbh/dev/hid.h"
#include "usbh/internal.h"

#if USBHHID_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHHID_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHHID_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHHID_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif



#define USBH_HID_REQ_GET_REPORT		0x01
#define USBH_HID_REQ_GET_IDLE		0x02
#define USBH_HID_REQ_GET_PROTOCOL	0x03
#define USBH_HID_REQ_SET_REPORT		0x09
#define USBH_HID_REQ_SET_IDLE		0x0A
#define USBH_HID_REQ_SET_PROTOCOL	0x0B

/*===========================================================================*/
/* USB Class driver loader for HID								 		 	 */
/*===========================================================================*/

USBHHIDDriver USBHHIDD[HAL_USBHHID_MAX_INSTANCES];

static void _hid_init(void);
static usbh_baseclassdriver_t *_hid_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _hid_unload(usbh_baseclassdriver_t *drv);
static void _stop_locked(USBHHIDDriver *hidp);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_hid_init,
	_hid_load,
	_hid_unload
};

const usbh_classdriverinfo_t usbhhidClassDriverInfo = {
	"HID", &class_driver_vmt
};

static usbh_baseclassdriver_t *_hid_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHHIDDriver *hidp;

	if (_usbh_match_descriptor(descriptor, rem, USBH_DT_INTERFACE,
			0x03, -1, -1) != HAL_SUCCESS)
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;

	if ((ifdesc->bAlternateSetting != 0)
			|| (ifdesc->bNumEndpoints < 1)) {
		return NULL;
	}


	/* alloc driver */
	for (i = 0; i < HAL_USBHHID_MAX_INSTANCES; i++) {
		if (USBHHIDD[i].dev == NULL) {
			hidp = &USBHHIDD[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc HID driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	hidp->epin.status = USBH_EPSTATUS_UNINITIALIZED;
#if HAL_USBHHID_USE_INTERRUPT_OUT
	hidp->epout.status = USBH_EPSTATUS_UNINITIALIZED;
#endif
	hidp->ifnum = ifdesc->bInterfaceNumber;
	usbhEPSetName(&dev->ctrl, "HID[CTRL]");

	/* parse the configuration descriptor */
	if_iterator_t iif;
	generic_iterator_t iep;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			uinfof("INT IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&hidp->epin, dev, epdesc);
			usbhEPSetName(&hidp->epin, "HID[IIN ]");
#if HAL_USBHHID_USE_INTERRUPT_OUT
		} else if (((epdesc->bEndpointAddress & 0x80) == 0)
				&& (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			uinfof("INT OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&hidp->epout, dev, epdesc);
			usbhEPSetName(&hidp->epout, "HID[IOUT]");
#endif
		} else {
			uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
					epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}
	if (hidp->epin.status != USBH_EPSTATUS_CLOSED) {
		goto deinit;
	}

	if (ifdesc->bInterfaceSubClass != 0x01) {
		hidp->type = USBHHID_DEVTYPE_GENERIC;
		uinfof("HID: bInterfaceSubClass=%02x, generic HID", ifdesc->bInterfaceSubClass);
		if (ifdesc->bInterfaceSubClass != 0x00) {
			uinfof("HID: bInterfaceSubClass=%02x is an invalid bInterfaceSubClass value",
					ifdesc->bInterfaceSubClass);
		}
	} else if (ifdesc->bInterfaceProtocol == 0x01) {
		hidp->type = USBHHID_DEVTYPE_BOOT_KEYBOARD;
		uinfo("HID: BOOT protocol keyboard found");
	} else if (ifdesc->bInterfaceProtocol == 0x02) {
		hidp->type = USBHHID_DEVTYPE_BOOT_MOUSE;
		uinfo("HID: BOOT protocol mouse found");
	} else {
		uerrf("HID: bInterfaceProtocol=%02x is an invalid boot protocol, abort",
				ifdesc->bInterfaceProtocol);
		goto deinit;
	}

	hidp->state = USBHHID_STATE_ACTIVE;

	return (usbh_baseclassdriver_t *)hidp;

deinit:
	/* Here, the enpoints are closed, and the driver is unlinked */
	return NULL;
}

static void _hid_unload(usbh_baseclassdriver_t *drv) {
	USBHHIDDriver *const hidp = (USBHHIDDriver *)drv;
	chSemWait(&hidp->sem);
	_stop_locked(hidp);
	hidp->state = USBHHID_STATE_STOP;
	chSemSignal(&hidp->sem);
}

static void _in_cb(usbh_urb_t *urb) {
	USBHHIDDriver *const hidp = (USBHHIDDriver *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		if (hidp->config->cb_report) {
			hidp->config->cb_report(hidp, urb->actualLength);
		}
		break;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("HID: URB IN disconnected");

		return;
	case USBH_URBSTATUS_TIMEOUT:
		//no data
		break;
	default:
		uerrf("HID: URB IN status unexpected = %d", urb->status);
		break;
	}
	usbhURBObjectResetI(&hidp->in_urb);
	usbhURBSubmitI(&hidp->in_urb);
}

void usbhhidStart(USBHHIDDriver *hidp, const USBHHIDConfig *cfg) {
	osalDbgCheck(hidp && cfg);
	osalDbgCheck(cfg->report_buffer && (cfg->protocol <= USBHHID_PROTOCOL_REPORT));

	chSemWait(&hidp->sem);
	if (hidp->state == USBHHID_STATE_READY) {
		chSemSignal(&hidp->sem);
		return;
	}
	osalDbgCheck(hidp->state == USBHHID_STATE_ACTIVE);

	hidp->config = cfg;

	/* init the URBs */
	uint32_t report_len = hidp->epin.wMaxPacketSize;
	if (report_len > cfg->report_len)
		report_len = cfg->report_len;
	usbhURBObjectInit(&hidp->in_urb, &hidp->epin, _in_cb, hidp,
			cfg->report_buffer, report_len);

	/* open the int IN/OUT endpoints */
	usbhEPOpen(&hidp->epin);
#if HAL_USBHHID_USE_INTERRUPT_OUT
	if (hidp->epout.status == USBH_EPSTATUS_CLOSED) {
		usbhEPOpen(&hidp->epout);
	}
#endif

	usbhhidSetProtocol(hidp, cfg->protocol);

	usbhURBSubmit(&hidp->in_urb);

	hidp->state = USBHHID_STATE_READY;
	chSemSignal(&hidp->sem);
}

static void _stop_locked(USBHHIDDriver *hidp) {
	if (hidp->state == USBHHID_STATE_ACTIVE)
		return;

	osalDbgCheck(hidp->state == USBHHID_STATE_READY);

	usbhEPClose(&hidp->epin);
#if HAL_USBHHID_USE_INTERRUPT_OUT
	if (hidp->epout.status != USBH_EPSTATUS_UNINITIALIZED) {
		usbhEPClose(&hidp->epout);
	}
#endif
	hidp->state = USBHHID_STATE_ACTIVE;
}

void usbhhidStop(USBHHIDDriver *hidp) {
	chSemWait(&hidp->sem);
	_stop_locked(hidp);
	chSemSignal(&hidp->sem);
}

usbh_urbstatus_t usbhhidGetReport(USBHHIDDriver *hidp,
		uint8_t report_id, usbhhid_reporttype_t report_type,
		void *data, uint16_t len) {
	osalDbgCheck(hidp);
	osalDbgAssert((uint8_t)report_type <= USBHHID_REPORTTYPE_FEATURE, "wrong report type");
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSIN(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_GET_REPORT,
			((uint8_t)report_type << 8) | report_id, hidp->ifnum, len, data);
}

usbh_urbstatus_t usbhhidSetReport(USBHHIDDriver *hidp,
		uint8_t report_id, usbhhid_reporttype_t report_type,
		const void *data, uint16_t len) {
	osalDbgCheck(hidp);
	osalDbgAssert((uint8_t)report_type <= USBHHID_REPORTTYPE_FEATURE, "wrong report type");
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSOUT(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_SET_REPORT,
			((uint8_t)report_type << 8) | report_id, hidp->ifnum, len, (void *)data);
}

usbh_urbstatus_t usbhhidGetIdle(USBHHIDDriver *hidp, uint8_t report_id, uint8_t *duration) {
	osalDbgCheck(hidp);
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSIN(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_GET_IDLE,
			report_id, hidp->ifnum, 1, duration);
}

usbh_urbstatus_t usbhhidSetIdle(USBHHIDDriver *hidp, uint8_t report_id, uint8_t duration) {
	osalDbgCheck(hidp);
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSOUT(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_SET_IDLE,
			(duration << 8) | report_id, hidp->ifnum, 0, NULL);
}

usbh_urbstatus_t usbhhidGetProtocol(USBHHIDDriver *hidp, uint8_t *protocol) {
	osalDbgCheck(hidp);
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSIN(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_GET_PROTOCOL,
			0, hidp->ifnum, 1, protocol);
}

usbh_urbstatus_t usbhhidSetProtocol(USBHHIDDriver *hidp, uint8_t protocol) {
	osalDbgCheck(hidp);
	osalDbgAssert(protocol <= 1, "invalid protocol");
	return usbhControlRequest(hidp->dev,
			USBH_REQTYPE_CLASSOUT(USBH_REQTYPE_RECIP_INTERFACE), USBH_HID_REQ_SET_PROTOCOL,
			protocol, hidp->ifnum, 0, NULL);
}

static void _hid_object_init(USBHHIDDriver *hidp) {
	osalDbgCheck(hidp != NULL);
	memset(hidp, 0, sizeof(*hidp));
	hidp->info = &usbhhidClassDriverInfo;
	hidp->state = USBHHID_STATE_STOP;
	chSemObjectInit(&hidp->sem, 1);
}

static void _hid_init(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHHID_MAX_INSTANCES; i++) {
		_hid_object_init(&USBHHIDD[i]);
	}
}

#endif
