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

#ifndef USBH_DEFS_H_
#define USBH_DEFS_H_

#include "hal.h"

#if HAL_USE_USBH

#include "osal.h"

#ifdef __IAR_SYSTEMS_ICC__
#define PACKED_STRUCT typedef PACKED_VAR struct
#else
#define PACKED_STRUCT typedef struct PACKED_VAR
#endif

PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} usbh_device_descriptor_t;
#define USBH_DT_DEVICE                   0x01
#define USBH_DT_DEVICE_SIZE              18

PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  bMaxPower;
} usbh_config_descriptor_t;
#define USBH_DT_CONFIG                   0x02
#define USBH_DT_CONFIG_SIZE              9

PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wData[1];
} usbh_string_descriptor_t;
#define USBH_DT_STRING                   0x03
#define USBH_DT_STRING_SIZE              2

PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  bInterfaceProtocol;
	uint8_t  iInterface;
} usbh_interface_descriptor_t;
#define USBH_DT_INTERFACE                0x04
#define USBH_DT_INTERFACE_SIZE           9

PACKED_STRUCT {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bEndpointAddress;
	uint8_t	bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t	bInterval;
} usbh_endpoint_descriptor_t;
#define USBH_DT_ENDPOINT                 0x05
#define USBH_DT_ENDPOINT_SIZE            7

PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bFirstInterface;
	uint8_t  bInterfaceCount;
	uint8_t  bFunctionClass;
	uint8_t  bFunctionSubClass;
	uint8_t  bFunctionProtocol;
	uint8_t  iFunction;
} usbh_ia_descriptor_t;
#define USBH_DT_INTERFACE_ASSOCIATION    	0x0b
#define USBH_DT_INTERFACE_ASSOCIATION_SIZE	8

PACKED_STRUCT {
	uint8_t  bDescLength;
	uint8_t  bDescriptorType;
	uint8_t  bNbrPorts;
	uint16_t wHubCharacteristics;
	uint8_t  bPwrOn2PwrGood;
	uint8_t  bHubContrCurrent;
	uint32_t DeviceRemovable;
} usbh_hub_descriptor_t;
#define USBH_DT_HUB				    	0x29
#define USBH_DT_HUB_SIZE		    	(7 + 4)

PACKED_STRUCT {
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} usbh_control_request_t;


#define USBH_REQ_GET_STATUS              0x00
#define USBH_REQ_CLEAR_FEATURE           0x01
#define USBH_REQ_SET_FEATURE             0x03
#define USBH_REQ_SET_ADDRESS             0x05
#define USBH_REQ_GET_DESCRIPTOR          0x06
#define USBH_REQ_SET_DESCRIPTOR          0x07
#define USBH_REQ_GET_CONFIGURATION       0x08
#define USBH_REQ_SET_CONFIGURATION       0x09
#define USBH_REQ_GET_INTERFACE           0x0A
#define USBH_REQ_SET_INTERFACE           0x0B
#define USBH_REQ_SYNCH_FRAME             0x0C


#define USBH_REQTYPE_IN					0x80
#define USBH_REQTYPE_OUT				0x00

#define USBH_REQTYPE_STANDARD			0x00
#define USBH_REQTYPE_CLASS				0x20
#define USBH_REQTYPE_VENDOR				0x40

#define USBH_REQTYPE_DEVICE				0x00
#define USBH_REQTYPE_INTERFACE			0x01
#define USBH_REQTYPE_ENDPOINT			0x02
#define USBH_REQTYPE_OTHER				0x03

#endif


#endif /* USBH_DEFS_H_ */
