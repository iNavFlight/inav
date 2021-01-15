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

#ifndef USBH_INTERNAL_H_
#define USBH_INTERNAL_H_

#include "hal_usbh.h"

#if HAL_USE_USBH

/*===========================================================================*/
/* These declarations are not part of the public API.                        */
/*===========================================================================*/

#if HAL_USBH_USE_FTDI
extern const usbh_classdriverinfo_t usbhftdiClassDriverInfo;
#endif
#if HAL_USBH_USE_AOA
extern const usbh_classdriverinfo_t usbhaoaClassDriverInfo;
#endif
#if HAL_USBH_USE_MSD
extern const usbh_classdriverinfo_t usbhmsdClassDriverInfo;
#endif
#if HAL_USBH_USE_HID
extern const usbh_classdriverinfo_t usbhhidClassDriverInfo;
#endif
#if HAL_USBH_USE_UVC
extern const usbh_classdriverinfo_t usbhuvcClassDriverInfo;
#endif
#if HAL_USBH_USE_HUB
extern const usbh_classdriverinfo_t usbhhubClassDriverInfo;
void _usbhub_port_object_init(usbh_port_t *port, USBHDriver *usbh,
		USBHHubDriver *hub, uint8_t number);
#else
void _usbhub_port_object_init(usbh_port_t *port, USBHDriver *usbh, uint8_t number);
#endif

void _usbh_port_disconnected(usbh_port_t *port);
void _usbh_urb_completeI(usbh_urb_t *urb, usbh_urbstatus_t status);
bool _usbh_urb_abortI(usbh_urb_t *urb, usbh_urbstatus_t status);
void _usbh_urb_abort_and_waitS(usbh_urb_t *urb, usbh_urbstatus_t status);

bool _usbh_match_vid_pid(usbh_device_t *dev, int32_t vid, int32_t pid);
bool _usbh_match_descriptor(const uint8_t *descriptor, uint16_t rem,
		int16_t type, int16_t _class, int16_t subclass, int16_t protocol);

#define USBH_REQTYPE_CLASSIN(type)	\
	(USBH_REQTYPE_DIR_IN | type | USBH_REQTYPE_TYPE_CLASS)

#define USBH_REQTYPE_CLASSOUT(type)	\
	(USBH_REQTYPE_DIR_OUT | type | USBH_REQTYPE_TYPE_CLASS)

#define USBH_REQTYPE_STANDARDIN(type)	\
	(USBH_REQTYPE_DIR_IN | type | USBH_REQTYPE_TYPE_STANDARD)

#define USBH_REQTYPE_STANDARDOUT(type)	\
	(USBH_REQTYPE_DIR_OUT | type | USBH_REQTYPE_TYPE_STANDARD)


#define USBH_PID_DATA0            0
#define USBH_PID_DATA2            1
#define USBH_PID_DATA1            2
#define USBH_PID_MDATA            3
#define USBH_PID_SETUP            3


/* GetBusState and SetHubDescriptor are optional, omitted */
#define ClearHubFeature   (((USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_DEVICE) << 8) \
							| USBH_REQ_CLEAR_FEATURE)
#define SetHubFeature     (((USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_DEVICE) << 8) \
							| USBH_REQ_SET_FEATURE)
#define ClearPortFeature   (((USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_OTHER) << 8) \
							| USBH_REQ_CLEAR_FEATURE)
#define SetPortFeature     (((USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_OTHER) << 8) \
							| USBH_REQ_SET_FEATURE)
#define GetHubDescriptor  (((USBH_REQTYPE_DIR_IN | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_DEVICE) << 8) \
							| USBH_REQ_GET_DESCRIPTOR)
#define GetHubStatus      (((USBH_REQTYPE_DIR_IN | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_DEVICE) << 8) \
							| USBH_REQ_GET_STATUS)
#define GetPortStatus     (((USBH_REQTYPE_DIR_IN | USBH_REQTYPE_TYPE_CLASS | USBH_REQTYPE_RECIP_OTHER) << 8) \
							| USBH_REQ_GET_STATUS)


#define USBH_PORTSTATUS_CONNECTION        0x0001
#define USBH_PORTSTATUS_ENABLE            0x0002
#define USBH_PORTSTATUS_SUSPEND           0x0004
#define USBH_PORTSTATUS_OVERCURRENT       0x0008
#define USBH_PORTSTATUS_RESET             0x0010
/* bits 5 to 7 are reserved */
#define USBH_PORTSTATUS_POWER             0x0100
#define USBH_PORTSTATUS_LOW_SPEED         0x0200
#define USBH_PORTSTATUS_HIGH_SPEED        0x0400
#define USBH_PORTSTATUS_TEST              0x0800
#define USBH_PORTSTATUS_INDICATOR         0x1000
/* bits 13 to 15 are reserved */

#define USBH_PORTSTATUS_C_CONNECTION      0x0001
#define USBH_PORTSTATUS_C_ENABLE          0x0002
#define USBH_PORTSTATUS_C_SUSPEND         0x0004
#define USBH_PORTSTATUS_C_OVERCURRENT     0x0008
#define USBH_PORTSTATUS_C_RESET           0x0010

#define USBH_HUBSTATUS_C_HUB_LOCAL_POWER       0x0001
#define USBH_HUBSTATUS_C_HUB_OVER_CURRENT      0x0002

/*
 * Port feature numbers
 * See USB 2.0 spec Table 11-17
 */
#define USBH_HUB_FEAT_C_HUB_LOCAL_POWER  0
#define USBH_HUB_FEAT_C_HUB_OVER_CURRENT 1
#define USBH_PORT_FEAT_CONNECTION        0
#define USBH_PORT_FEAT_ENABLE            1
#define USBH_PORT_FEAT_SUSPEND           2
#define USBH_PORT_FEAT_OVERCURRENT     	 3
#define USBH_PORT_FEAT_RESET             4
#define USBH_PORT_FEAT_POWER             8
#define USBH_PORT_FEAT_LOWSPEED          9
#define USBH_PORT_FEAT_C_CONNECTION      16
#define USBH_PORT_FEAT_C_ENABLE          17
#define USBH_PORT_FEAT_C_SUSPEND         18
#define USBH_PORT_FEAT_C_OVERCURRENT     19
#define USBH_PORT_FEAT_C_RESET           20
#define USBH_PORT_FEAT_TEST              21
#define USBH_PORT_FEAT_INDICATOR         22

#define sizeof_array(x) 	(sizeof(x)/sizeof(*(x)))

#include "usbh/desciter.h"	/* descriptor iterators */
#include "usbh/debug.h"

#endif

#endif /* USBH_INTERNAL_H_ */
