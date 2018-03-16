/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Diego Ismirlian, TISA, (dismirlian (at) google's mail)

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

#if HAL_USE_USBH

#include "usbh/dev/hub.h"
#include "usbh/internal.h"
#include <string.h>

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

#if STM32_USBH_USE_OTG1
USBHDriver USBHD1;
#endif
#if STM32_USBH_USE_OTG2
USBHDriver USBHD2;
#endif


static void _classdriver_process_device(usbh_device_t *dev);
static bool _classdriver_load(usbh_device_t *dev, uint8_t class,
		uint8_t subclass, uint8_t protocol, uint8_t *descbuff, uint16_t rem);


/*===========================================================================*/
/* Checks.								                                     */
/*===========================================================================*/

static inline void _check_dev(usbh_device_t *dev) {
	osalDbgCheck(dev);
	//TODO: add more checks.
}

static inline void _check_ep(usbh_ep_t *ep) {
	osalDbgCheck(ep != 0);
	_check_dev(ep->device);
	osalDbgCheck(ep->type <= 3);
	//TODO: add more checks.
}

static inline void _check_urb(usbh_urb_t *urb) {
	osalDbgCheck(urb != 0);
	_check_ep(urb->ep);
	osalDbgCheck((urb->buff != NULL) || (urb->requestedLength == 0));
	//TODO: add more checks.
}

/*===========================================================================*/
/* Main driver API.						                                     */
/*===========================================================================*/

void usbhObjectInit(USBHDriver *usbh) {
	memset(usbh, 0, sizeof(*usbh));
	usbh->status = USBH_STATUS_STOPPED;
#if HAL_USBH_USE_HUB
	INIT_LIST_HEAD(&usbh->hubs);
	_usbhub_port_object_init(&usbh->rootport, usbh, 0, 1);
#else
	_usbhub_port_object_init(&usbh->rootport, usbh, 1);
#endif
}

void usbhInit(void) {
#if HAL_USBH_USE_HUB
	uint8_t i;
	for (i = 0; i < HAL_USBHHUB_MAX_INSTANCES; i++) {
		usbhhubObjectInit(&USBHHUBD[i]);
	}
#endif
	usbh_lld_init();
}

void usbhStart(USBHDriver *usbh) {
	usbDbgInit(usbh);

	osalSysLock();
	osalDbgAssert((usbh->status == USBH_STATUS_STOPPED) || (usbh->status == USBH_STATUS_STARTED),
				"invalid state");
	usbh_lld_start(usbh);
	usbh->status = USBH_STATUS_STARTED;
	osalOsRescheduleS();
	osalSysUnlock();
}


void usbhStop(USBHDriver *usbh) {
	//TODO: implement
	(void)usbh;
}
void usbhSuspend(USBHDriver *usbh) {
	//TODO: implement
	(void)usbh;
}
void usbhResume(USBHDriver *usbh) {
	//TODO: implement
	(void)usbh;
}

/*===========================================================================*/
/* Endpoint API.						                                     */
/*===========================================================================*/

void usbhEPObjectInit(usbh_ep_t *ep, usbh_device_t *dev, const usbh_endpoint_descriptor_t *desc) {
	osalDbgCheck(ep);
	_check_dev(dev);
	osalDbgCheck(desc);

	memset(ep, 0, sizeof(*ep));
	ep->device = dev;
	ep->wMaxPacketSize = desc->wMaxPacketSize;
	ep->address = desc->bEndpointAddress & 0x0F;
	ep->type = (usbh_eptype_t) (desc->bmAttributes & 0x03);
	if (ep->type != USBH_EPTYPE_CTRL) {
		ep->in = (desc->bEndpointAddress & 0x80) ? TRUE : FALSE;
	}
	ep->bInterval = desc->bInterval;

	/* low-level part */
	usbh_lld_ep_object_init(ep);

	ep->status = USBH_EPSTATUS_CLOSED;
}


static void _ep0_object_init(usbh_device_t *dev, uint16_t wMaxPacketSize) {
	const usbh_endpoint_descriptor_t ep0_descriptor = {
		7,	//bLength
		5,	//bDescriptorType
		0,	//bEndpointAddress
		0,	//bmAttributes
		wMaxPacketSize,
		0,	//bInterval
	};
	usbhEPObjectInit(&dev->ctrl, dev, &ep0_descriptor);
	usbhEPSetName(&dev->ctrl, "DEV[CTRL]");
}


/*===========================================================================*/
/* URB API.				    	                                     		 */
/*===========================================================================*/

void usbhURBObjectInit(usbh_urb_t *urb, usbh_ep_t *ep, usbh_completion_cb callback,
		void *user, void *buff, uint32_t len) {

	osalDbgCheck(urb != 0);
	_check_ep(ep);

	/* initialize the common part: */
	urb->ep = ep;
	urb->callback = callback;
	urb->userData = user;
	urb->buff = buff;
	urb->requestedLength = len;
	urb->actualLength = 0;
	urb->status = USBH_URBSTATUS_INITIALIZED;
	urb->waitingThread = 0;
	urb->abortingThread = 0;

	/* initialize the ll part: */
	usbh_lld_urb_object_init(urb);
}

void usbhURBObjectResetI(usbh_urb_t *urb) {
	osalDbgAssert(!usbhURBIsBusy(urb), "invalid status");

	osalDbgCheck((urb->waitingThread == 0) && (urb->abortingThread == 0));

	urb->actualLength = 0;
	urb->status = USBH_URBSTATUS_INITIALIZED;

	/* reset the ll part: */
	usbh_lld_urb_object_reset(urb);
}

void usbhURBSubmitI(usbh_urb_t *urb) {
	osalDbgCheckClassI();
	_check_urb(urb);
	osalDbgAssert(urb->status == USBH_URBSTATUS_INITIALIZED, "invalid status");
	usbh_ep_t *const ep = urb->ep;
	if (ep->status == USBH_EPSTATUS_HALTED) {
		_usbh_urb_completeI(urb, USBH_URBSTATUS_STALL);
		return;
	}
	if (ep->status != USBH_EPSTATUS_OPEN) {
		_usbh_urb_completeI(urb, USBH_URBSTATUS_DISCONNECTED);
		return;
	}
	if (!(usbhDeviceGetPort(ep->device)->status & USBH_PORTSTATUS_ENABLE)) {
		_usbh_urb_completeI(urb, USBH_URBSTATUS_DISCONNECTED);
		return;
	}
	urb->status = USBH_URBSTATUS_PENDING;
	usbh_lld_urb_submit(urb);
}

bool _usbh_urb_abortI(usbh_urb_t *urb, usbh_urbstatus_t status) {
	osalDbgCheckClassI();
	_check_urb(urb);

	switch (urb->status) {
/*	case USBH_URBSTATUS_UNINITIALIZED:
 * 	case USBH_URBSTATUS_INITIALIZED:
 *	case USBH_URBSTATUS_ERROR:
 *	case USBH_URBSTATUS_TIMEOUT:
 *	case USBH_URBSTATUS_CANCELLED:
 *	case USBH_URBSTATUS_STALL:
 *	case USBH_URBSTATUS_DISCONNECTED:
 *	case USBH_URBSTATUS_OK: */
	default:
		/* already finished */
		_usbh_urb_completeI(urb, status);
		return TRUE;

//	case USBH_URBSTATUS_QUEUED:
	case USBH_URBSTATUS_PENDING:
		return usbh_lld_urb_abort(urb, status);
	}
}

void _usbh_urb_abort_and_waitS(usbh_urb_t *urb, usbh_urbstatus_t status) {
	osalDbgCheckClassS();
	_check_urb(urb);

	if (_usbh_urb_abortI(urb, status) == FALSE) {
		uwarn("URB wasn't aborted immediately, suspend");
		osalThreadSuspendS(&urb->abortingThread);
		urb->abortingThread = 0;
	} else {
		osalOsRescheduleS();
	}
	uwarn("URB aborted");
}

bool usbhURBCancelI(usbh_urb_t *urb) {
	return _usbh_urb_abortI(urb, USBH_URBSTATUS_CANCELLED);
}

void usbhURBCancelAndWaitS(usbh_urb_t *urb) {
	_usbh_urb_abort_and_waitS(urb, USBH_URBSTATUS_CANCELLED);
}

msg_t usbhURBWaitTimeoutS(usbh_urb_t *urb, systime_t timeout) {
	msg_t ret;

	osalDbgCheckClassS();
	_check_urb(urb);

	switch (urb->status) {
	case USBH_URBSTATUS_INITIALIZED:
	case USBH_URBSTATUS_PENDING:
//	case USBH_URBSTATUS_QUEUED:
		ret = osalThreadSuspendTimeoutS(&urb->waitingThread, timeout);
		urb->waitingThread = 0;
		break;

	case USBH_URBSTATUS_OK:
		ret = MSG_OK;
		osalOsRescheduleS();
		break;

/*	case USBH_URBSTATUS_UNINITIALIZED:
 *	case USBH_URBSTATUS_ERROR:
 *	case USBH_URBSTATUS_TIMEOUT:
 *	case USBH_URBSTATUS_CANCELLED:
 *	case USBH_URBSTATUS_STALL:
 *	case USBH_URBSTATUS_DISCONNECTED: */
	default:
		ret = MSG_RESET;
		osalOsRescheduleS();
		break;
	}
	return ret;
}

msg_t usbhURBSubmitAndWaitS(usbh_urb_t *urb, systime_t timeout) {
	msg_t ret;

	osalDbgCheckClassS();
	_check_urb(urb);

	usbhURBSubmitI(urb);
	ret = usbhURBWaitTimeoutS(urb, timeout);
	if (ret == MSG_TIMEOUT)
		_usbh_urb_abort_and_waitS(urb, USBH_URBSTATUS_TIMEOUT);

	return ret;
}

static inline msg_t _wakeup_message(usbh_urbstatus_t status) {
	if (status == USBH_URBSTATUS_OK) return MSG_OK;
	if (status == USBH_URBSTATUS_TIMEOUT) return MSG_TIMEOUT;
	return MSG_RESET;
}

void _usbh_urb_completeI(usbh_urb_t *urb, usbh_urbstatus_t status) {
	osalDbgCheckClassI();
	_check_urb(urb);
	urb->status = status;
	osalThreadResumeI(&urb->waitingThread, _wakeup_message(status));
	osalThreadResumeI(&urb->abortingThread, MSG_RESET);
	if (urb->callback)
		urb->callback(urb);
}

/*===========================================================================*/
/* Synchronous API.		    	                                     		 */
/*===========================================================================*/

usbh_urbstatus_t usbhBulkTransfer(usbh_ep_t *ep,
		void *data,
		uint32_t len,
		uint32_t *actual_len,
		systime_t timeout) {

	osalDbgCheck(ep != NULL);
	osalDbgCheck((data != NULL) || (len == 0));
	osalDbgAssert(ep->type == USBH_EPTYPE_BULK, "wrong ep");

	usbh_urb_t urb;
	usbhURBObjectInit(&urb, ep, 0, 0, data, len);

	osalSysLock();
	usbhURBSubmitAndWaitS(&urb, timeout);
	osalSysUnlock();

	if (actual_len != NULL)
		*actual_len = urb.actualLength;

	return urb.status;
}

usbh_urbstatus_t usbhControlRequestExtended(usbh_device_t *dev,
		const usbh_control_request_t *req,
		uint8_t *buff,
		uint32_t *actual_len,
		systime_t timeout) {

	_check_dev(dev);
	osalDbgCheck(req != NULL);

	usbh_urb_t urb;

	usbhURBObjectInit(&urb, &dev->ctrl, 0, 0, buff, req->wLength);
	urb.setup_buff = req;

	osalSysLock();
	usbhURBSubmitAndWaitS(&urb, timeout);
	osalSysUnlock();

	if (actual_len != NULL)
		*actual_len = urb.actualLength;

	return urb.status;
}

usbh_urbstatus_t usbhControlRequest(usbh_device_t *dev,
		uint8_t bmRequestType,
		uint8_t bRequest,
		uint16_t wValue,
		uint16_t wIndex,
		uint16_t wLength,
		uint8_t *buff) {

	const USBH_DEFINE_BUFFER(usbh_control_request_t, req) = {
			bmRequestType,
			bRequest,
			wValue,
			wIndex,
			wLength
	};
	return usbhControlRequestExtended(dev, &req, buff, NULL, MS2ST(1000));
}

/*===========================================================================*/
/* Standard request helpers.   	                                     		 */
/*===========================================================================*/

#define USBH_GET_DESCRIPTOR(type, value, index)	\
	USBH_STANDARDIN(type, \
	USBH_REQ_GET_DESCRIPTOR, \
	value, \
	index) \

#define USBH_GETDEVICEDESCRIPTOR \
	USBH_GET_DESCRIPTOR(USBH_REQTYPE_DEVICE, (USBH_DT_DEVICE << 8) | 0, 0)

#define USBH_GETCONFIGURATIONDESCRIPTOR(index) \
	USBH_GET_DESCRIPTOR(USBH_REQTYPE_DEVICE, (USBH_DT_CONFIG << 8) | index, 0)

#define USBH_GETSTRINGDESCRIPTOR(index, langID) \
	USBH_GET_DESCRIPTOR(USBH_REQTYPE_DEVICE, (USBH_DT_STRING << 8) | index, langID)

bool usbhStdReqGetDeviceDescriptor(usbh_device_t *dev,
		uint16_t wLength,
		uint8_t *buf) {
	usbh_device_descriptor_t *desc;
	usbh_urbstatus_t ret = usbhControlRequest(dev, USBH_GETDEVICEDESCRIPTOR, wLength, buf);
	desc = (usbh_device_descriptor_t *)buf;
	if ((ret != USBH_URBSTATUS_OK)
			|| (desc->bLength != USBH_DT_DEVICE_SIZE)
			|| (desc->bDescriptorType != USBH_DT_DEVICE)) {
		return HAL_FAILED;
	}
	return HAL_SUCCESS;
}

bool usbhStdReqGetConfigurationDescriptor(usbh_device_t *dev,
		uint8_t index,
		uint16_t wLength,
		uint8_t *buf) {
	usbh_urbstatus_t ret = usbhControlRequest(dev, USBH_GETCONFIGURATIONDESCRIPTOR(index), wLength, buf);
	usbh_config_descriptor_t *const desc = (usbh_config_descriptor_t *)buf;
	if ((ret != USBH_URBSTATUS_OK)
			|| (desc->bLength < USBH_DT_CONFIG_SIZE)
			|| (desc->bDescriptorType != USBH_DT_CONFIG)) {
		return HAL_FAILED;
	}
	return HAL_SUCCESS;
}

bool usbhStdReqGetStringDescriptor(usbh_device_t *dev,
		uint8_t index,
		uint16_t langID,
		uint16_t wLength,
		uint8_t *buf) {

	osalDbgAssert(wLength >= USBH_DT_STRING_SIZE, "wrong size");
	usbh_string_descriptor_t *desc = (usbh_string_descriptor_t *)buf;
	usbh_urbstatus_t ret = usbhControlRequest(dev, USBH_GETSTRINGDESCRIPTOR(index, langID), wLength, buf);
	if ((ret != USBH_URBSTATUS_OK)
			|| (desc->bLength < USBH_DT_STRING_SIZE)
			|| (desc->bDescriptorType != USBH_DT_STRING)) {
		return HAL_FAILED;
	}
	return HAL_SUCCESS;
}



#define USBH_SET_INTERFACE(interface, alt)	\
	USBH_STANDARDOUT(USBH_REQTYPE_INTERFACE, \
	USBH_REQ_SET_INTERFACE, \
	alt, \
	interface) \

#define USBH_GET_INTERFACE(interface)	\
	USBH_STANDARDIN(USBH_REQTYPE_INTERFACE, \
	USBH_REQ_GET_INTERFACE, \
	0, \
	interface) \

bool usbhStdReqSetInterface(usbh_device_t *dev,
		uint8_t bInterfaceNumber,
		uint8_t bAlternateSetting) {

	usbh_urbstatus_t ret = usbhControlRequest(dev, USBH_SET_INTERFACE(bInterfaceNumber, bAlternateSetting), 0, NULL);
	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	return HAL_SUCCESS;
}

bool usbhStdReqGetInterface(usbh_device_t *dev,
		uint8_t bInterfaceNumber,
		uint8_t *bAlternateSetting) {

	USBH_DEFINE_BUFFER(uint8_t, alt);

	usbh_urbstatus_t ret = usbhControlRequest(dev, USBH_GET_INTERFACE(bInterfaceNumber), 1, &alt);
	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	*bAlternateSetting = alt;
	return HAL_SUCCESS;
}


/*===========================================================================*/
/* Device-related functions.   	                                     		 */
/*===========================================================================*/

static uint8_t _find_address(USBHDriver *host) {
	uint8_t addr, i, j;
	for (i = 0; i < sizeof_array(host->address_bitmap); i++) {
		addr = host->address_bitmap[i];
		for (j = 0; j < 8; j++) {
			if ((addr & (1 << j)) == 0) {
				//found:
				addr = i * 8 + j + 1;
				host->address_bitmap[i] |= (1 << j);
				return addr;
			}
		}
	}
	return 0;
}

static void _free_address(USBHDriver *host, uint8_t addr) {
	uinfof("Free address %d", addr);
	host->address_bitmap[addr / 8] &= ~(1 << ((addr - 1) & 7));
}

static void _device_initialize(usbh_device_t *dev, usbh_devspeed_t speed) {
	dev->address = 0;
	dev->speed = speed;
	dev->status = USBH_DEVSTATUS_DEFAULT;
	dev->langID0 = 0;
	dev->keepFullCfgDesc = 0;
	_ep0_object_init(dev, 64);
}

static bool _device_setaddress(usbh_device_t *dev, uint8_t address) {
	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_STANDARDOUT(USBH_REQTYPE_DEVICE, USBH_REQ_SET_ADDRESS, address, 0),
			0,
			0);
	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	dev->address = address;
	return HAL_SUCCESS;
}

static inline bool _device_read_basic_cfgdesc(usbh_device_t *dev, uint8_t bConfiguration) {
	/* get configuration descriptor */
	return usbhStdReqGetConfigurationDescriptor(dev, bConfiguration,
			sizeof(dev->basicConfigDesc), (uint8_t *)&dev->basicConfigDesc);
}

static void _device_read_full_cfgdesc(usbh_device_t *dev, uint8_t bConfiguration) {
	_check_dev(dev);

	uint8_t i;

	if (dev->fullConfigurationDescriptor != NULL) {
		chHeapFree(dev->fullConfigurationDescriptor);
	}

	dev->fullConfigurationDescriptor =
			(uint8_t *)chHeapAlloc(0, dev->basicConfigDesc.wTotalLength);

	if (!dev->fullConfigurationDescriptor)
		return;

	for (i = 0; i < 3; i++) {
		if (usbhStdReqGetConfigurationDescriptor(dev, bConfiguration,
				dev->basicConfigDesc.wTotalLength,
				dev->fullConfigurationDescriptor) == HAL_SUCCESS) {
			return;
		}
		osalThreadSleepMilliseconds(200);
	}

	/* error */
	chHeapFree(dev->fullConfigurationDescriptor);
	dev->fullConfigurationDescriptor = NULL;
}

static void _device_free_full_cfgdesc(usbh_device_t *dev) {
	osalDbgCheck(dev);
	if (dev->fullConfigurationDescriptor != NULL) {
		chHeapFree(dev->fullConfigurationDescriptor);
		dev->fullConfigurationDescriptor = NULL;
	}
}


#define USBH_SET_CONFIGURATION(type, value, index)	\
	USBH_STANDARDOUT(type, \
	USBH_REQ_SET_CONFIGURATION, \
	value, \
	index) \

#define USBH_SETDEVICECONFIGURATION(index) \
	USBH_SET_CONFIGURATION(USBH_REQTYPE_DEVICE, index, 0)


static bool _device_set_configuration(usbh_device_t *dev, uint8_t configuration) {
	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_SETDEVICECONFIGURATION(configuration),
			0,
			0);
	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;
	return HAL_SUCCESS;
}

static bool _device_configure(usbh_device_t *dev, uint8_t bConfiguration) {
	uint8_t i;

	uinfof("Reading basic configuration descriptor %d", bConfiguration);
	for (i = 0; i < 3; i++) {
		if (!_device_read_basic_cfgdesc(dev, bConfiguration))
			break;
	}

	if (i == 3) {
		uerrf("Could not read basic configuration descriptor %d; "
					"won't configure device", bConfiguration);
		return HAL_FAILED;
	}

	uinfof("Selecting configuration %d", bConfiguration);
	for (i = 0; i < 3; i++) {
		if (!_device_set_configuration(dev, dev->basicConfigDesc.bConfigurationValue)) {
			/* TODO: check if correctly configured using GET_CONFIGURATION */
			dev->status = USBH_DEVSTATUS_CONFIGURED;
			dev->bConfiguration = bConfiguration;

			uinfo("Device configured.");
			return HAL_SUCCESS;
		}
	}

	return HAL_FAILED;
}

static bool _device_enumerate(usbh_device_t *dev) {

	uinfo("Enumerate.");
	uinfo("Get first 8 bytes of device descriptor");

	/* get first 8 bytes of device descriptor */
	if (usbhStdReqGetDeviceDescriptor(dev, 8, (uint8_t *)&dev->devDesc)) {
		uerr("Error");
		return HAL_FAILED;
	}

	uinfof("Configure bMaxPacketSize0 = %d", dev->devDesc.bMaxPacketSize0);
	/* configure EP0 wMaxPacketSize */
	usbhEPClose(&dev->ctrl);
	_ep0_object_init(dev, dev->devDesc.bMaxPacketSize0);
	usbhEPOpen(&dev->ctrl);

	uint8_t addr = _find_address(dev->host);
	if (addr == 0) {
		uerr("No free addresses found");
		return HAL_FAILED;
	}

	/* set device address */
	uinfof("Set device address: %d", addr);
	if (_device_setaddress(dev, addr)) {
		uerr("Error");
		_free_address(dev->host, addr);
		return HAL_FAILED;
	}

	/* update EP because of the address change */
	usbhEPClose(&dev->ctrl);
	_ep0_object_init(dev, dev->devDesc.bMaxPacketSize0);
	usbhEPOpen(&dev->ctrl);

	uinfof("Wait stabilization...");
	osalThreadSleepMilliseconds(HAL_USBH_DEVICE_ADDRESS_STABILIZATION);

	/* address is set */
	dev->status = USBH_DEVSTATUS_ADDRESS;

	uinfof("Get full device desc");
	/* get full device descriptor */
	if (usbhStdReqGetDeviceDescriptor(dev, sizeof(dev->devDesc),
			(uint8_t *)&dev->devDesc)) {
		uerr("Error");
		_device_setaddress(dev, 0);
		_free_address(dev->host, addr);
		return HAL_FAILED;
	}

	uinfof("Enumeration finished.");
	return HAL_SUCCESS;
}

#if USBH_DEBUG_ENABLE && USBH_DEBUG_ENABLE_INFO
void usbhDevicePrintInfo(usbh_device_t *dev) {
	USBH_DEFINE_BUFFER(char, str[64]);
	usbh_device_descriptor_t *const desc = &dev->devDesc;

	uinfo("----- Device info -----");
	uinfo("Device descriptor:");
	uinfof("\tUSBSpec=%04x, #configurations=%d, langID0=%04x",
			desc->bcdUSB,
			desc->bNumConfigurations,
			dev->langID0);

	uinfof("\tClass=%02x, Subclass=%02x, Protocol=%02x",
			desc->bDeviceClass,
			desc->bDeviceSubClass,
			desc->bDeviceProtocol);

	uinfof("\tVID=%04x, PID=%04x, Release=%04x",
			desc->idVendor,
			desc->idProduct,
			desc->bcdDevice);

	if (dev->langID0) {
		usbhDeviceReadString(dev, str, sizeof(str), desc->iManufacturer, dev->langID0);
		uinfof("\tManufacturer: %s", str);
		usbhDeviceReadString(dev, str, sizeof(str), desc->iProduct, dev->langID0);
		uinfof("\tProduct: %s", str);
		usbhDeviceReadString(dev, str, sizeof(str), desc->iSerialNumber, dev->langID0);
		uinfof("\tSerial Number: %s", str);
	}

	if (dev->status == USBH_DEVSTATUS_CONFIGURED) {
		uinfo("Configuration descriptor (partial):");
		usbh_config_descriptor_t *const cfg = &dev->basicConfigDesc;
		uinfof("\tbConfigurationValue=%d, Length=%d, #interfaces=%d",
				cfg->bConfigurationValue,
				cfg->wTotalLength,
				cfg->bNumInterfaces);

		uinfof("\tCurrent=%dmA", cfg->bMaxPower * 2);
		uinfof("\tSelfPowered=%d, RemoteWakeup=%d",
				cfg->bmAttributes & 0x40 ? 1 : 0,
				cfg->bmAttributes & 0x20 ? 1 : 0);
		if (dev->langID0) {
			usbhDeviceReadString(dev, str, sizeof(str), cfg->iConfiguration, dev->langID0);
			uinfof("\tName: %s", str);
		}
	}

	uinfo("----- End Device info -----");

}

void usbhDevicePrintConfiguration(const uint8_t *descriptor, uint16_t rem) {
	generic_iterator_t iep, icfg, ics;
	if_iterator_t iif;

	uinfo("----- Configuration info -----");
	uinfo("Configuration descriptor:");
	cfg_iter_init(&icfg, descriptor, rem);
	const usbh_config_descriptor_t *const cfgdesc = cfg_get(&icfg);
	uinfof("Configuration %d, #IFs=%d", cfgdesc->bConfigurationValue, cfgdesc->bNumInterfaces);

	for (if_iter_init(&iif, &icfg); iif.valid; if_iter_next(&iif)) {
		const usbh_interface_descriptor_t *const ifdesc = if_get(&iif);

		uinfof("  Interface %d, alt=%d, #EPs=%d, "
				"Class=%02x, Subclass=%02x, Protocol=%02x",
				ifdesc->bInterfaceNumber, ifdesc->bAlternateSetting, ifdesc->bNumEndpoints,
				ifdesc->bInterfaceClass, ifdesc->bInterfaceSubClass, ifdesc->bInterfaceProtocol);

		for (cs_iter_init(&ics, (generic_iterator_t *)&iif); ics.valid; cs_iter_next(&ics)) {
			uinfof("    Class-Specific descriptor, Length=%d, Type=%02x",
					ics.curr[0], ics.curr[1]);
		}

		for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
			const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);

			uinfof("    Endpoint descriptor, Address=%02x, Type=%d, MaxPacket=%d, Interval=%d",
					epdesc->bEndpointAddress,
					epdesc->bmAttributes & 3,
					epdesc->wMaxPacketSize,
					epdesc->bInterval);

			for (cs_iter_init(&ics, &iep); ics.valid; cs_iter_next(&ics)) {
				uinfof("    Class-Specific descriptor, Length=%d, Type=%02x",
						ics.curr[0], ics.curr[1]);
			}
		}
	}
	uinfo("----- End Configuration info -----");
}
#endif

bool usbhDeviceReadString(usbh_device_t *dev, char *dest, uint8_t size,
		uint8_t index, uint16_t langID) {

	usbh_string_descriptor_t *const desc = (usbh_string_descriptor_t *)dest;
	osalDbgAssert(size >= 2, "wrong size");

	*dest = 0;
	if (index == 0)
		return HAL_SUCCESS;
	if (usbhStdReqGetStringDescriptor(dev, index, langID, size, (uint8_t *)dest))
		return HAL_FAILED;
	if (desc->bLength & 1)
		return HAL_FAILED;
	if (desc->bLength <= 2)
		return HAL_SUCCESS;

	uint8_t nchars = desc->bLength / 2;		/* including the trailing 0 */
	if (size < nchars)
		nchars = size;

	char *src = (char *)&desc->wData[0];
	while (--nchars) {
		*dest++ = *src;
		src += 2;
	}
	*dest = 0;
	return HAL_SUCCESS;
}




/*===========================================================================*/
/* Port processing functions.  	                                     		 */
/*===========================================================================*/

static void _port_connected(usbh_port_t *port);

static void _port_reset(usbh_port_t *port) {
	usbhhubControlRequest(port->device.host,
#if HAL_USBH_USE_HUB
			port->hub,
#endif
			USBH_REQTYPE_OUT | USBH_REQTYPE_CLASS | USBH_REQTYPE_OTHER,
			USBH_REQ_SET_FEATURE,
			USBH_PORT_FEAT_RESET,
			port->number,
			0,
			0);
}

static void _port_update_status(usbh_port_t *port) {
	uint32_t stat;
	if (usbhhubControlRequest(port->device.host,
#if HAL_USBH_USE_HUB
			port->hub,
#endif
			USBH_REQTYPE_IN | USBH_REQTYPE_CLASS | USBH_REQTYPE_OTHER,
			USBH_REQ_GET_STATUS,
			0,
			port->number,
			4,
			(uint8_t *)&stat) != USBH_URBSTATUS_OK) {
		return;
	}
	port->status = stat & 0xffff;
	port->c_status |= stat >> 16;
}

static void _port_process_status_change(usbh_port_t *port) {

	_port_update_status(port);

	if (port->c_status & USBH_PORTSTATUS_C_CONNECTION) {
		/* port connected status changed */
		port->c_status &= ~USBH_PORTSTATUS_C_CONNECTION;
		usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_CONNECTION);
		if ((port->status & (USBH_PORTSTATUS_CONNECTION | USBH_PORTSTATUS_ENABLE))
				== USBH_PORTSTATUS_CONNECTION) {
			if (port->device.status != USBH_DEVSTATUS_DISCONNECTED) {
				_usbh_port_disconnected(port);
			}

			/* connected, disabled */
			_port_connected(port);
		} else {
			/* disconnected */
			_usbh_port_disconnected(port);
		}
	}

	if (port->c_status & USBH_PORTSTATUS_C_RESET) {
		port->c_status &= ~USBH_PORTSTATUS_C_RESET;
		usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_RESET);
	}

	if (port->c_status & USBH_PORTSTATUS_C_ENABLE) {
		port->c_status &= ~USBH_PORTSTATUS_C_ENABLE;
		usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_ENABLE);
	}

	if (port->c_status & USBH_PORTSTATUS_C_OVERCURRENT) {
		port->c_status &= ~USBH_PORTSTATUS_C_OVERCURRENT;
		usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_OVERCURRENT);
	}

	if (port->c_status & USBH_PORTSTATUS_C_SUSPEND) {
		port->c_status &= ~USBH_PORTSTATUS_C_SUSPEND;
		usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_SUSPEND);
	}

}


static void _port_connected(usbh_port_t *port) {
	/* connected */

	systime_t start;
	uint8_t i;
	uint8_t retries;
	usbh_devspeed_t speed;
	USBH_DEFINE_BUFFER(usbh_string_descriptor_t, strdesc);

	uinfof("Port %d connected, wait debounce...", port->number);

	port->device.status = USBH_DEVSTATUS_ATTACHED;

	/* wait for attach de-bounce */
	osalThreadSleepMilliseconds(HAL_USBH_PORT_DEBOUNCE_TIME);

	/* check disconnection */
	_port_update_status(port);
	if (port->c_status & USBH_PORTSTATUS_C_CONNECTION) {
		/* connection state changed; abort */
		goto abort;
	}

	port->device.status = USBH_DEVSTATUS_CONNECTED;
	retries = 3;

reset:
	for (i = 0; i < 3; i++) {
		uinfo("Try reset...");
		port->c_status &= ~(USBH_PORTSTATUS_C_RESET | USBH_PORTSTATUS_C_ENABLE);
		_port_reset(port);
		osalThreadSleepMilliseconds(20);	/* give it some time to reset (min. 10ms) */
		start = osalOsGetSystemTimeX();
		while (TRUE) {
			_port_update_status(port);

			/* check for disconnection */
			if (port->c_status & USBH_PORTSTATUS_C_CONNECTION)
				goto abort;

			/* check for reset completion */
			if (port->c_status & USBH_PORTSTATUS_C_RESET) {
				port->c_status &= ~USBH_PORTSTATUS_C_RESET;
				usbhhubClearFeaturePort(port, USBH_PORT_FEAT_C_RESET);

				if ((port->status & (USBH_PORTSTATUS_ENABLE | USBH_PORTSTATUS_CONNECTION))
						== (USBH_PORTSTATUS_ENABLE | USBH_PORTSTATUS_CONNECTION)) {
					goto reset_success;
				}
			}

			/* check for timeout */
			if (osalOsGetSystemTimeX() - start > HAL_USBH_PORT_RESET_TIMEOUT) break;
		}
	}

	/* reset procedure failed; abort */
	goto abort;

reset_success:

	uinfo("Reset OK, recovery...");

	/* reset recovery */
	osalThreadSleepMilliseconds(100);

	/* initialize object */
	if (port->status & USBH_PORTSTATUS_LOW_SPEED) {
		speed = USBH_DEVSPEED_LOW;
	} else if (port->status & USBH_PORTSTATUS_HIGH_SPEED) {
		speed = USBH_DEVSPEED_HIGH;
	} else {
		speed = USBH_DEVSPEED_FULL;
	}
	_device_initialize(&port->device, speed);
	usbhEPOpen(&port->device.ctrl);

	/* device with default address (0), try enumeration */
	if (_device_enumerate(&port->device)) {
		/* enumeration failed */
		usbhEPClose(&port->device.ctrl);

		if (!--retries)
			goto abort;

		/* retry reset & enumeration */
		goto reset;
	}

	/* load the default language ID */
	uinfo("Loading langID0...");
	if (!usbhStdReqGetStringDescriptor(&port->device, 0, 0,
			USBH_DT_STRING_SIZE, (uint8_t *)&strdesc)
		&& (strdesc.bLength >= 4)
		&& !usbhStdReqGetStringDescriptor(&port->device, 0, 0,
			4, (uint8_t *)&strdesc)) {

		port->device.langID0 = strdesc.wData[0];
		uinfof("langID0=%04x", port->device.langID0);
	}

	/* check if the device has only one configuration */
	if (port->device.devDesc.bNumConfigurations == 1) {
		uinfo("Device has only one configuration");
		_device_configure(&port->device, 0);
	}

	_classdriver_process_device(&port->device);
	return;

abort:
	uerr("Abort");
	port->device.status = USBH_DEVSTATUS_DISCONNECTED;
}

void _usbh_port_disconnected(usbh_port_t *port) {
	if (port->device.status == USBH_DEVSTATUS_DISCONNECTED)
		return;

	uinfo("Port disconnected");

	/* unload drivers */
	while (port->device.drivers) {
		usbh_baseclassdriver_t *drv = port->device.drivers;

		/* unload */
		uinfof("Unload driver %s", drv->info->name);
		drv->info->vmt->unload(drv);

		/* unlink */
		drv->dev = 0;
		port->device.drivers = drv->next;
	}

	/* close control endpoint */
	osalSysLock();
	usbhEPCloseS(&port->device.ctrl);
	osalSysUnlock();

	/* free address */
	if (port->device.address)
		_free_address(port->device.host, port->device.address);

	_device_free_full_cfgdesc(&port->device);

	port->device.status = USBH_DEVSTATUS_DISCONNECTED;
}



/*===========================================================================*/
/* Hub processing functions.  	                                     		 */
/*===========================================================================*/

#if HAL_USBH_USE_HUB
static void _hub_update_status(USBHDriver *host, USBHHubDriver *hub) {
	uint32_t stat;
	if (usbhhubControlRequest(host,
			hub,
			USBH_REQTYPE_IN | USBH_REQTYPE_CLASS | USBH_REQTYPE_DEVICE,
			USBH_REQ_GET_STATUS,
			0,
			0,
			4,
			(uint8_t *)&stat) != USBH_URBSTATUS_OK) {
		return;
	}
	if (hub) {
		hub->status = stat & 0xffff;
		hub->c_status |= stat >> 16;
	}
}

static void _hub_process_status_change(USBHDriver *host, USBHHubDriver *hub) {
	uinfo("Hub status change. GET_STATUS.");
	_hub_update_status(host, hub);

	if (hub->c_status & USBH_HUBSTATUS_C_HUB_LOCAL_POWER) {
		hub->c_status &= ~USBH_HUBSTATUS_C_HUB_LOCAL_POWER;
		uinfo("Clear USBH_HUB_FEAT_C_HUB_LOCAL_POWER");
		usbhhubClearFeatureHub(host, hub, USBH_HUB_FEAT_C_HUB_LOCAL_POWER);
	}

	if (hub->c_status & USBH_HUBSTATUS_C_HUB_OVER_CURRENT) {
		hub->c_status &= ~USBH_HUBSTATUS_C_HUB_OVER_CURRENT;
		uinfo("Clear USBH_HUB_FEAT_C_HUB_OVER_CURRENT");
		usbhhubClearFeatureHub(host, hub, USBH_HUB_FEAT_C_HUB_OVER_CURRENT);
	}
}

static uint32_t _hub_get_status_change_bitmap(USBHDriver *host, USBHHubDriver *hub) {
	if (hub != NULL) {
		osalSysLock();
		uint32_t ret = hub->statuschange;
		hub->statuschange = 0;
		osalOsRescheduleS();
		osalSysUnlock();
		return ret;
	}
	return usbh_lld_roothub_get_statuschange_bitmap(host);
}

#else
//TODO: replace the functions above
#endif

#if HAL_USBH_USE_HUB
static void _hub_process(USBHDriver *host, USBHHubDriver *hub) {
	uint32_t bitmap = _hub_get_status_change_bitmap(host, hub);
	if (!bitmap)
		return;

	if (bitmap & 1) {
		_hub_process_status_change(host, hub);
		bitmap &= ~1;
	}

	usbh_port_t *port = (hub == NULL) ? &host->rootport : hub->ports;
	uint8_t i;
	for (i = 1; i < 32; i++) {
		if (!bitmap || !port)
			break;
		if (bitmap & (1 << i)) {
			bitmap &= ~(1 << i);
			_port_process_status_change(port);
		}
		port = port->next;
	}

}
#else
static void _hub_process(USBHDriver *host) {
	uint32_t bitmap = usbh_lld_roothub_get_statuschange_bitmap(host);

#if 0	//TODO: complete _hub_process_status_change for root hub
	if (bitmap & 1) {
		_hub_process_status_change(host, hub);
		bitmap &= ~1;
	}
#endif

	if (!bitmap)
		return;

	_port_process_status_change(&host->rootport);
}
#endif

/*===========================================================================*/
/* Main processing loop (enumeration, loading/unloading drivers, etc). 		 */
/*===========================================================================*/
void usbhMainLoop(USBHDriver *usbh) {

	if (usbh->status == USBH_STATUS_STOPPED)
		return;

#if HAL_USBH_USE_HUB
	/* process root hub */
	_hub_process(usbh, NULL);

	/* process connected hubs */
	USBHHubDriver *hub;
    list_for_each_entry(hub, USBHHubDriver, &usbh->hubs, node) {
		_hub_process(usbh, hub);
	}
#else
	/* process root hub */
	_hub_process(usbh);
#endif
}


/*===========================================================================*/
/* IAD class driver.											 		 	 */
/*===========================================================================*/
#if HAL_USBH_USE_IAD
static usbh_baseclassdriver_t *iad_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void iad_unload(usbh_baseclassdriver_t *drv);
static const usbh_classdriver_vmt_t usbhiadClassDriverVMT = {
	iad_load,
	iad_unload
};
static const usbh_classdriverinfo_t usbhiadClassDriverInfo = {
	0xef, 0x02, 0x01, "IAD", &usbhiadClassDriverVMT
};

static usbh_baseclassdriver_t *iad_load(usbh_device_t *dev,
		const uint8_t *descriptor, uint16_t rem) {
	(void)rem;

	if (descriptor[1] != USBH_DT_DEVICE)
		return 0;

	uinfo("Load a driver for each IF collection.");

	generic_iterator_t icfg;
	if_iterator_t iif;
	const usbh_ia_descriptor_t *last_iad = 0;

	cfg_iter_init(&icfg, dev->fullConfigurationDescriptor,
			dev->basicConfigDesc.wTotalLength);
	if (!icfg.valid) {
		uerr("Invalid configuration descriptor.");
		return 0;
	}

	for (if_iter_init(&iif, &icfg); iif.valid; if_iter_next(&iif)) {
		if (iif.iad && (iif.iad != last_iad)) {
			last_iad = iif.iad;
			if (_classdriver_load(dev, iif.iad->bFunctionClass,
					iif.iad->bFunctionSubClass,
					iif.iad->bFunctionProtocol,
					(uint8_t *)iif.iad,
					(uint8_t *)iif.curr - (uint8_t *)iif.iad + iif.rem) != HAL_SUCCESS) {
				uwarnf("No drivers found for IF collection #%d:%d",
						iif.iad->bFirstInterface,
						iif.iad->bFirstInterface + iif.iad->bInterfaceCount - 1);
			}
		}
	}

	return 0;
}

static void iad_unload(usbh_baseclassdriver_t *drv) {
	(void)drv;
}
#endif


/*===========================================================================*/
/* Class driver loader.											 		 	 */
/*===========================================================================*/

static const usbh_classdriverinfo_t *usbh_classdrivers_lookup[] = {
#if HAL_USBH_USE_FTDI
	&usbhftdiClassDriverInfo,
#endif
#if HAL_USBH_USE_IAD
	&usbhiadClassDriverInfo,
#endif
#if HAL_USBH_USE_UVC
	&usbhuvcClassDriverInfo,
#endif
#if HAL_USBH_USE_MSD
	&usbhmsdClassDriverInfo,
#endif
#if HAL_USBH_USE_HUB
	&usbhhubClassDriverInfo
#endif
};

static bool _classdriver_load(usbh_device_t *dev, uint8_t class,
		uint8_t subclass, uint8_t protocol, uint8_t *descbuff, uint16_t rem) {
	uint8_t i;
	usbh_baseclassdriver_t *drv = NULL;
	for (i = 0; i < sizeof_array(usbh_classdrivers_lookup); i++) {
		const usbh_classdriverinfo_t *const info = usbh_classdrivers_lookup[i];
		if (class == 0xff) {
			/* vendor specific */
			if (info->class == 0xff) {
				uinfof("Try load vendor-specific driver %s", info->name);
				drv = info->vmt->load(dev, descbuff, rem);
				if (drv != NULL)
					goto success;
			}
		} else if ((info->class < 0) || ((info->class == class)
			&& ((info->subclass < 0) || ((info->subclass == subclass)
			&& ((info->protocol < 0) || (info->protocol == protocol)))))) {
			uinfof("Try load driver %s", info->name);
			drv = info->vmt->load(dev, descbuff, rem);

#if HAL_USBH_USE_IAD
			/* special case: */
			if (info == &usbhiadClassDriverInfo)
				return HAL_SUCCESS;
#endif

			if (drv != NULL)
				goto success;
		}
	}
	return HAL_FAILED;

success:
	/* Link this driver to the device */
	drv->next = dev->drivers;
	dev->drivers = drv;
	drv->dev = dev;
	return HAL_SUCCESS;
}

static void _classdriver_process_device(usbh_device_t *dev) {
	uinfo("New device found.");
	const usbh_device_descriptor_t *const devdesc = &dev->devDesc;

	usbhDevicePrintInfo(dev);

	/* TODO: Support multiple configurations
	 *
	 * Windows doesn't support them, so it's unlikely that any commercial USB device
	 * will have multiple configurations.
	 */
	if (dev->status != USBH_DEVSTATUS_CONFIGURED) {
		uwarn("Multiple configurations not supported, selecting configuration #0");
		if (_device_configure(dev, 0) != HAL_SUCCESS) {
			uerr("Couldn't configure device; abort.");
			return;
		}
	}

	_device_read_full_cfgdesc(dev, dev->bConfiguration);
	if (dev->fullConfigurationDescriptor == NULL) {
		uerr("Couldn't read full configuration descriptor; abort.");
		return;
	}

	usbhDevicePrintConfiguration(dev->fullConfigurationDescriptor,
			dev->basicConfigDesc.wTotalLength);

	if (devdesc->bDeviceClass == 0) {
		/* each interface defines its own device class/subclass/protocol */
		uinfo("Load a driver for each IF.");

		generic_iterator_t icfg;
		if_iterator_t iif;
		uint8_t last_if = 0xff;

		cfg_iter_init(&icfg, dev->fullConfigurationDescriptor,
				dev->basicConfigDesc.wTotalLength);
		if (!icfg.valid) {
			uerr("Invalid configuration descriptor.");
			goto exit;
		}

		for (if_iter_init(&iif, &icfg); iif.valid; if_iter_next(&iif)) {
			const usbh_interface_descriptor_t *const ifdesc = if_get(&iif);
			if (ifdesc->bInterfaceNumber != last_if) {
				last_if = ifdesc->bInterfaceNumber;
				if (_classdriver_load(dev, ifdesc->bInterfaceClass,
						ifdesc->bInterfaceSubClass,
						ifdesc->bInterfaceProtocol,
						(uint8_t *)ifdesc, iif.rem) != HAL_SUCCESS) {
					uwarnf("No drivers found for IF #%d", ifdesc->bInterfaceNumber);
				}
			}
		}

	} else {
		if (_classdriver_load(dev, devdesc->bDeviceClass,
				devdesc->bDeviceSubClass,
				devdesc->bDeviceProtocol,
				(uint8_t *)devdesc, USBH_DT_DEVICE_SIZE) != HAL_SUCCESS) {
			uwarn("No drivers found.");
		}
	}

exit:
	if (dev->keepFullCfgDesc == 0) {
		_device_free_full_cfgdesc(dev);
	}
}


#endif

