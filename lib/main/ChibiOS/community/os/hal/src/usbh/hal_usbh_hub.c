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

#if HAL_USBH_USE_HUB

#if !HAL_USE_USBH
#error "USBHHUB needs HAL_USE_USBH"
#endif

#include <string.h>
#include "usbh/dev/hub.h"
#include "usbh/internal.h"

#if USBHHUB_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHHUB_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHHUB_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHHUB_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


USBHHubDriver USBHHUBD[HAL_USBHHUB_MAX_INSTANCES];
static usbh_port_t USBHPorts[HAL_USBHHUB_MAX_PORTS];

static void _hub_init(void);
static usbh_baseclassdriver_t *_hub_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _hub_unload(usbh_baseclassdriver_t *drv);
static const usbh_classdriver_vmt_t usbhhubClassDriverVMT = {
	_hub_init,
	_hub_load,
	_hub_unload
};

const usbh_classdriverinfo_t usbhhubClassDriverInfo = {
	"HUB", &usbhhubClassDriverVMT
};


void _usbhub_port_object_init(usbh_port_t *port, USBHDriver *usbh,
		USBHHubDriver *hub, uint8_t number) {
	memset(port, 0, sizeof(*port));
	port->number = number;
	port->device.host = usbh;
	port->hub = hub;
}

usbh_urbstatus_t usbhhubControlRequest(USBHDriver *host, USBHHubDriver *hub,
											uint8_t bmRequestType,
											uint8_t bRequest,
											uint16_t wValue,
											uint16_t wIndex,
											uint16_t wLength,
											uint8_t *buf) {
	if (hub == NULL)
		return usbh_lld_root_hub_request(host, bmRequestType, bRequest, wValue, wIndex, wLength, buf);

	return usbhControlRequest(hub->dev,
			bmRequestType, bRequest, wValue, wIndex, wLength, buf);
}


static void _urb_complete(usbh_urb_t *urb) {

	USBHHubDriver *const hubdp = (USBHHubDriver *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_TIMEOUT:
		/* the device NAKed */
		udbg("HUB: no info");
		//hubdp->statuschange = 0;
		break;
	case USBH_URBSTATUS_OK: {
		uint8_t len = hubdp->hubDesc.bNbrPorts / 8 + 1;
		if (urb->actualLength != len) {
			uwarnf("Expected %d status change bytes but got %d", len, urb->actualLength);
		}

		if (urb->actualLength < len)
			len = urb->actualLength;

		if (len > 4)
			len = 4;

		uint8_t *sc = (uint8_t *)&hubdp->statuschange;
		uint8_t *r = hubdp->scbuff;
		while (len--)
			*sc++ |= *r++;

		uinfof("HUB: change, %08x", hubdp->statuschange);
	}	break;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("HUB: URB disconnected, aborting poll");
		return;
	default:
		uerrf("HUB: URB status unexpected = %d", urb->status);
		break;
	}

	usbhURBObjectResetI(urb);
	usbhURBSubmitI(urb);
}

static usbh_baseclassdriver_t *_hub_load(usbh_device_t *dev,
		const uint8_t *descriptor, uint16_t rem) {
	int i;

	USBHHubDriver *hubdp;

	if (_usbh_match_descriptor(descriptor, rem, USBH_DT_DEVICE,
			0x09, 0x00, 0x00) != HAL_SUCCESS)
		return NULL;

	generic_iterator_t iep, icfg;
	if_iterator_t iif;

	cfg_iter_init(&icfg, dev->fullConfigurationDescriptor,
			dev->basicConfigDesc.wTotalLength);

	if_iter_init(&iif, &icfg);
	if (!iif.valid)
		return NULL;

	if (_usbh_match_descriptor(iif.curr, iif.rem, USBH_DT_INTERFACE,
			0x09, 0x00, 0x00) != HAL_SUCCESS)
		return NULL;

	ep_iter_init(&iep, &iif);
	if (!iep.valid)
		return NULL;
	const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
	if ((epdesc->bmAttributes & 0x03) != USBH_EPTYPE_INT) {
		return NULL;
	}


	/* alloc driver */
	for (i = 0; i < HAL_USBHHUB_MAX_INSTANCES; i++) {
		if (USBHHUBD[i].dev == NULL) {
			hubdp = &USBHHUBD[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc HUB driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	hubdp->epint.status = USBH_EPSTATUS_UNINITIALIZED;
	hubdp->dev = dev;
	hubdp->ports = 0;

	usbhEPSetName(&dev->ctrl, "HUB[CTRL]");

	/* read Hub descriptor */
	uinfo("Read Hub descriptor");
	if (usbhhubControlRequest(dev->host, hubdp,
			USBH_REQTYPE_DIR_IN | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_DEVICE,
			USBH_REQ_GET_DESCRIPTOR,
			(USBH_DT_HUB << 8), 0, sizeof(hubdp->hubDesc),
			(uint8_t *)&hubdp->hubDesc) != USBH_URBSTATUS_OK) {
		hubdp->dev = NULL;
		return NULL;
	}

	const usbh_hub_descriptor_t *const hubdesc = &hubdp->hubDesc;

	uinfof("Hub descriptor loaded; %d ports, wHubCharacteristics=%04x, bPwrOn2PwrGood=%d, bHubContrCurrent=%d",
			hubdesc->bNbrPorts,
			hubdesc->wHubCharacteristics,
			hubdesc->bPwrOn2PwrGood,
			hubdesc->bHubContrCurrent);

	/* Alloc ports */
	uint8_t ports = hubdesc->bNbrPorts;
	for (i = 0; (ports > 0) && (i < HAL_USBHHUB_MAX_PORTS); i++) {
		if (USBHPorts[i].hub == NULL) {
			uinfof("Alloc port %d", ports);
			_usbhub_port_object_init(&USBHPorts[i], dev->host, hubdp, ports);
			USBHPorts[i].next = hubdp->ports;
			hubdp->ports = &USBHPorts[i];
			--ports;
		}
	}

	if (ports) {
		uwarn("Could not alloc all ports");
	}

	/* link hub to the host's list */
	list_add_tail(&hubdp->node, &dev->host->hubs);

	/* enable power to ports */
	usbh_port_t *port = hubdp->ports;
	while (port) {
		uinfof("Enable power for port %d", port->number);
		usbhhubSetFeaturePort(port, USBH_PORT_FEAT_POWER);
		port = port->next;
	}

	if (hubdesc->bPwrOn2PwrGood)
		osalThreadSleepMilliseconds(2 * hubdesc->bPwrOn2PwrGood);

	/* initialize the status change endpoint and trigger the first transfer */
	usbhEPObjectInit(&hubdp->epint, dev, epdesc);
	usbhEPSetName(&hubdp->epint, "HUB[INT ]");
	usbhEPOpen(&hubdp->epint);

	usbhURBObjectInit(&hubdp->urb, &hubdp->epint,
			_urb_complete, hubdp, hubdp->scbuff,
			(hubdesc->bNbrPorts + 8) / 8);

	usbhURBSubmit(&hubdp->urb);

	hubdp->dev = NULL;
	return (usbh_baseclassdriver_t *)hubdp;
}

static void _hub_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	USBHHubDriver *const hubdp = (USBHHubDriver *)drv;

	/* close the status change endpoint (this cancels ongoing URBs) */
	usbhEPClose(&hubdp->epint);

	/* de-alloc ports and unload drivers */
	usbh_port_t *port = hubdp->ports;
	while (port) {
		_usbh_port_disconnected(port);
		port->hub = NULL;
		port = port->next;
	}

	/* unlink the hub from the host's list */
	list_del(&hubdp->node);

}

static void _object_init(USBHHubDriver *hubdp) {
	osalDbgCheck(hubdp != NULL);
	memset(hubdp, 0, sizeof(*hubdp));
	hubdp->info = &usbhhubClassDriverInfo;
}

static void _hub_init(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHHUB_MAX_INSTANCES; i++) {
		_object_init(&USBHHUBD[i]);
	}
}

#else

#if HAL_USE_USBH
#include <string.h>
void _usbhub_port_object_init(usbh_port_t *port, USBHDriver *usbh, uint8_t number) {
	memset(port, 0, sizeof(*port));
	port->number = number;
	port->device.host = usbh;
}
#endif

#endif
