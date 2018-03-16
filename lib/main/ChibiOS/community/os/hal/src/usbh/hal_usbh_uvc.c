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
#include "hal_usbh.h"

#if HAL_USBH_USE_UVC

#if !HAL_USE_USBH
#error "USBHUVC needs HAL_USE_USBH"
#endif

#if !HAL_USBH_USE_IAD
#error "USBHUVC needs HAL_USBH_USE_IAD"
#endif

#if USBHUVC_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


static usbh_baseclassdriver_t *uvc_load(usbh_device_t *dev,
		const uint8_t *descriptor, uint16_t rem);
static void uvc_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	uvc_load,
	uvc_unload
};
const usbh_classdriverinfo_t usbhuvcClassDriverInfo = {
	0x0e, 0x03, 0x00, "UVC", &class_driver_vmt
};


static usbh_baseclassdriver_t *uvc_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	(void)dev;
	(void)descriptor;
	(void)rem;
	return NULL;
}

static void uvc_unload(usbh_baseclassdriver_t *drv) {
	(void)drv;
}

#endif

