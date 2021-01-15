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

#ifndef HAL_USBH_H_
#define HAL_USBH_H_

#include "hal.h"

#ifndef HAL_USE_USBH
#define HAL_USE_USBH FALSE
#endif

#ifndef HAL_USBH_USE_FTDI
#define HAL_USBH_USE_FTDI FALSE
#endif

#ifndef HAL_USBH_USE_HUB
#define HAL_USBH_USE_HUB FALSE
#endif

#ifndef HAL_USBH_USE_MSD
#define HAL_USBH_USE_MSD FALSE
#endif

#ifndef HAL_USBH_USE_UVC
#define HAL_USBH_USE_UVC FALSE
#endif

#ifndef HAL_USBH_USE_AOA
#define HAL_USBH_USE_AOA FALSE
#endif

#ifndef HAL_USBH_USE_HID
#define HAL_USBH_USE_HID FALSE
#endif

#ifndef HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS
#define HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS	FALSE
#endif

#define HAL_USBH_USE_IAD     HAL_USBH_USE_UVC

#if (HAL_USE_USBH == TRUE) || defined(__DOXYGEN__)

#include "osal.h"
#include "usbh/list.h"
#include "usbh/defs.h"

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USBH_USE_HUB
#define USBH_MAX_ADDRESSES				1
#else
#define USBH_MAX_ADDRESSES				(HAL_USBHHUB_MAX_PORTS + 1)
#endif

enum usbh_status {
	USBH_STATUS_STOPPED = 0,
	USBH_STATUS_STARTED,
	USBH_STATUS_SUSPENDED,
};

/* These correspond to the USB spec */
enum usbh_devstatus {
	USBH_DEVSTATUS_DISCONNECTED = 0,
	USBH_DEVSTATUS_ATTACHED,
	USBH_DEVSTATUS_CONNECTED,
	USBH_DEVSTATUS_DEFAULT,
	USBH_DEVSTATUS_ADDRESS,
	USBH_DEVSTATUS_CONFIGURED,
};

enum usbh_devspeed {
	USBH_DEVSPEED_LOW = 0,
	USBH_DEVSPEED_FULL,
	USBH_DEVSPEED_HIGH,
};

enum usbh_epdir {
	USBH_EPDIR_IN		= 0x80,
	USBH_EPDIR_OUT		= 0
};

enum usbh_eptype {
	USBH_EPTYPE_CTRL	= 0,
	USBH_EPTYPE_ISO		= 1,
	USBH_EPTYPE_BULK	= 2,
	USBH_EPTYPE_INT		= 3,
};

enum usbh_epstatus {
	USBH_EPSTATUS_UNINITIALIZED = 0,
	USBH_EPSTATUS_CLOSED,
	USBH_EPSTATUS_OPEN,
	USBH_EPSTATUS_HALTED,
};

enum usbh_urbstatus {
	USBH_URBSTATUS_UNINITIALIZED = 0,
	USBH_URBSTATUS_INITIALIZED,
	USBH_URBSTATUS_PENDING,
	USBH_URBSTATUS_ERROR,
	USBH_URBSTATUS_TIMEOUT,
	USBH_URBSTATUS_CANCELLED,
	USBH_URBSTATUS_STALL,
	USBH_URBSTATUS_DISCONNECTED,
	USBH_URBSTATUS_OK,
};

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/* forward declarations */
typedef struct USBHDriver USBHDriver;
typedef struct usbh_port usbh_port_t;
typedef	struct usbh_device usbh_device_t;
typedef struct usbh_ep usbh_ep_t;
typedef struct usbh_urb usbh_urb_t;
typedef struct usbh_baseclassdriver usbh_baseclassdriver_t;
typedef struct usbh_classdriverinfo usbh_classdriverinfo_t;
#if HAL_USBH_USE_HUB
typedef struct USBHHubDriver USBHHubDriver;
#endif

/* typedefs */
typedef enum usbh_status usbh_status_t;
typedef enum usbh_devspeed usbh_devspeed_t;
typedef enum usbh_devstatus usbh_devstatus_t;
typedef enum usbh_epdir usbh_epdir_t;
typedef enum usbh_eptype usbh_eptype_t;
typedef enum usbh_epstatus usbh_epstatus_t;
typedef enum usbh_urbstatus usbh_urbstatus_t;
typedef uint16_t usbh_portstatus_t;
typedef uint16_t usbh_portcstatus_t;
typedef void (*usbh_completion_cb)(usbh_urb_t *);

/* include the low level driver; the required definitions are above */
#include "hal_usbh_lld.h"

#define USBH_DEFINE_BUFFER(var)	USBH_LLD_DEFINE_BUFFER(var)
#define USBH_DECLARE_STRUCT_MEMBER(member) USBH_LLD_DECLARE_STRUCT_MEMBER(member)

struct usbh_urb {
	usbh_ep_t *ep;

	void *userData;
	usbh_completion_cb callback;

	const void *setup_buff;
	void *buff;
	uint32_t requestedLength;
	uint32_t actualLength;

	usbh_urbstatus_t status;

	thread_reference_t waitingThread;
	thread_reference_t abortingThread;

	/* Low level part */
	_usbh_urb_ll_data
};

struct usbh_ep {
	usbh_device_t		*device;
	usbh_ep_t	 		*next;

	usbh_epstatus_t		status;
	uint8_t 			address;
	bool				in;
	usbh_eptype_t		type;
	uint16_t			wMaxPacketSize;
	uint8_t				bInterval;

	/* debug */
	const char			*name;

	/* Low-level part */
	_usbh_ep_ll_data
};

struct usbh_device {
	USBHDriver *host;	/* shortcut to host */

	usbh_ep_t ctrl;
	usbh_ep_t *endpoints;

	usbh_baseclassdriver_t *drivers;

	uint16_t langID0;

	usbh_devstatus_t status;
	usbh_devspeed_t speed;

	USBH_DECLARE_STRUCT_MEMBER(usbh_device_descriptor_t devDesc);
	USBH_DECLARE_STRUCT_MEMBER(usbh_config_descriptor_t basicConfigDesc);

	uint8_t *fullConfigurationDescriptor;
	uint8_t keepFullCfgDesc;

	uint8_t address;
	uint8_t bConfiguration;

	/* Low level part */
	_usbh_device_ll_data
};


struct usbh_port {
#if HAL_USBH_USE_HUB
	USBHHubDriver *hub;
#endif

	usbh_portstatus_t status;
	usbh_portcstatus_t c_status;

	usbh_port_t *next;

	uint8_t number;

	usbh_device_t device;

	/* Low level part */
	_usbh_port_ll_data
};

struct USBHDriver {
	usbh_status_t status;
	uint8_t address_bitmap[(USBH_MAX_ADDRESSES + 7) / 8];

	usbh_port_t rootport;

#if HAL_USBH_USE_HUB
	struct list_head hubs;
#endif

	/* Low level part */
	_usbhdriver_ll_data

#if USBH_DEBUG_ENABLE
	/* debug */
	uint8_t dbg_buff[USBH_DEBUG_BUFFER];
	THD_WORKING_AREA(waDebug, 512);
	input_queue_t iq;
#endif
};



/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/


/*===========================================================================*/
/* Main driver API.		                                                     */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	/* Main functions */
	void usbhObjectInit(USBHDriver *usbh);
	void usbhInit(void);
	void usbhStart(USBHDriver *usbh);
	void usbhStop(USBHDriver *usbh);
	void usbhSuspend(USBHDriver *usbh);
	void usbhResume(USBHDriver *usbh);

	/* Device-related */
#if	USBH_DEBUG_ENABLE && USBH_DEBUG_ENABLE_INFO
	void usbhDevicePrintInfo(usbh_device_t *dev);
	void usbhDevicePrintConfiguration(const uint8_t *descriptor, uint16_t rem);
#else
#	define usbhDevicePrintInfo(dev) do {} while(0)
#	define usbhDevicePrintConfiguration(descriptor, rem) do {} while(0)
#endif
	bool usbhDeviceReadString(usbh_device_t *dev, char *dest, uint8_t size,
			uint8_t index, uint16_t langID);
	static inline usbh_port_t *usbhDeviceGetPort(usbh_device_t *dev) {
		return container_of(dev, usbh_port_t, device);
	}

	/* Synchronous API */
	usbh_urbstatus_t usbhBulkTransfer(usbh_ep_t *ep,
			void *data,
			uint32_t len,
			uint32_t *actual_len,
			systime_t timeout);
	usbh_urbstatus_t usbhControlRequest(usbh_device_t *dev,
			uint8_t bmRequestType,
			uint8_t bRequest,
			uint16_t wValue,
			uint16_t wIndex,
			uint16_t wLength,
			uint8_t *buff);
	usbh_urbstatus_t usbhControlRequestExtended(usbh_device_t *dev,
			const usbh_control_request_t *req,
			uint8_t *buff,
			uint32_t *actual_len,
			systime_t timeout);

	/* Standard request helpers */
	bool usbhStdReqGetDeviceDescriptor(usbh_device_t *dev,
			uint16_t wLength,
			uint8_t *buf);
	bool usbhStdReqGetConfigurationDescriptor(usbh_device_t *dev,
			uint8_t index,
			uint16_t wLength,
			uint8_t *buf);
	bool usbhStdReqGetStringDescriptor(usbh_device_t *dev,
			uint8_t index,
			uint16_t langID,
			uint16_t wLength,
			uint8_t *buf);
	bool usbhStdReqSetInterface(usbh_device_t *dev,
			uint8_t bInterfaceNumber,
			uint8_t bAlternateSetting);
	bool usbhStdReqGetInterface(usbh_device_t *dev,
			uint8_t bInterfaceNumber,
			uint8_t *bAlternateSetting);

	/* Endpoint/pipe management */
	void usbhEPObjectInit(usbh_ep_t *ep, usbh_device_t *dev, const usbh_endpoint_descriptor_t *desc);
	static inline void usbhEPOpen(usbh_ep_t *ep) {
		osalDbgCheck(ep != 0);
		osalSysLock();
		osalDbgAssert(ep->status == USBH_EPSTATUS_CLOSED, "invalid state");
		usbh_lld_ep_open(ep);
		ep->next = ep->device->endpoints;
		ep->device->endpoints = ep;
		osalSysUnlock();
	}
	static inline void usbhEPCloseS(usbh_ep_t *ep) {
		osalDbgCheck(ep != 0);
		osalDbgCheckClassS();
		osalDbgAssert(ep->status != USBH_EPSTATUS_UNINITIALIZED, "invalid state");
		if (ep->status == USBH_EPSTATUS_CLOSED)
			return;
		usbh_lld_ep_close(ep);
	}
	static inline void usbhEPClose(usbh_ep_t *ep) {
		osalSysLock();
		usbhEPCloseS(ep);
		osalSysUnlock();
	}
	bool usbhEPReset(usbh_ep_t *ep);
	static inline bool usbhEPIsPeriodic(usbh_ep_t *ep) {
		osalDbgCheck(ep != NULL);
		return (ep->type & 1) != 0;
	}
	static inline bool usbhURBIsBusy(usbh_urb_t *urb) {
		osalDbgCheck(urb != NULL);
		return (urb->status == USBH_URBSTATUS_PENDING);
	}
	static inline void usbhEPSetName(usbh_ep_t *ep, const char *name) {
		ep->name = name;
	}

	/* URB management */
	void usbhURBObjectInit(usbh_urb_t *urb, usbh_ep_t *ep, usbh_completion_cb callback,
			void *user, void *buff, uint32_t len);
	void usbhURBObjectResetI(usbh_urb_t *urb);
	void usbhURBSubmitI(usbh_urb_t *urb);
	bool usbhURBCancelI(usbh_urb_t *urb);
	msg_t usbhURBSubmitAndWaitS(usbh_urb_t *urb, systime_t timeout);
	void usbhURBCancelAndWaitS(usbh_urb_t *urb);
	msg_t usbhURBWaitTimeoutS(usbh_urb_t *urb, systime_t timeout);

	static inline void usbhURBSubmit(usbh_urb_t *urb) {
		osalSysLock();
		usbhURBSubmitI(urb);
		osalOsRescheduleS();
		osalSysUnlock();
	}

	static inline bool usbhURBCancel(usbh_urb_t *urb) {
		bool ret;
		osalSysLock();
		ret = usbhURBCancelI(urb);
		osalOsRescheduleS();
		osalSysUnlock();
		return ret;
	}

	/* Main loop */
	void usbhMainLoop(USBHDriver *usbh);

#ifdef __cplusplus
}
#endif


/*===========================================================================*/
/* Class driver definitions and API.                                         */
/*===========================================================================*/

typedef struct usbh_classdriver_vmt usbh_classdriver_vmt_t;
struct usbh_classdriver_vmt {
	void (*init)(void);
	usbh_baseclassdriver_t *(*load)(usbh_device_t *dev,	const uint8_t *descriptor, uint16_t rem);
	void (*unload)(usbh_baseclassdriver_t *drv);
	/* TODO: add power control, suspend, etc */
};

struct usbh_classdriverinfo {
	const char *name;
	const usbh_classdriver_vmt_t *vmt;
};

#define _usbh_base_classdriver_data		\
	const usbh_classdriverinfo_t *info;	\
	usbh_device_t *dev;					\
	usbh_baseclassdriver_t *next;

struct usbh_baseclassdriver {
	_usbh_base_classdriver_data
};

#endif

#endif /* HAL_USBH_H_ */
