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

#ifndef USBH_ADDITIONAL_H_
#define USBH_ADDITIONAL_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS

/* Declarations */
extern const usbh_classdriverinfo_t usbhCustomClassDriverInfo;



/* Comma separated list of additional class drivers */
#define HAL_USBH_ADDITIONAL_CLASS_DRIVERS	\
	&usbhCustomClassDriverInfo,



#endif

#endif /* USBH_ADDITIONAL_H_ */
