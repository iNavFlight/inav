/*
    Copyright (C) 2016 Jonathan Struebel

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

/**
 * @file    usbcfg.h
 * @brief   USB driver config header.
 *
 * @addtogroup USB
 * @{
 */

#ifndef USBCFG_H
#define USBCFG_H

#include "hal_usb_hid.h"

extern const USBConfig usbcfg;
extern const USBHIDConfig usbhidcfg;
extern USBHIDDriver UHD1;

#ifdef __cplusplus
extern "C" {
#endif
  size_t hidGetReport(uint8_t id, uint8_t *bp, size_t n);
  msg_t hidSetReport(uint8_t id, uint8_t *bp, size_t n);
#ifdef __cplusplus
}
#endif

#endif  /* USBCFG_H */

/** @} */
