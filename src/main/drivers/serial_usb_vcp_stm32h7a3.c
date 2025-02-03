/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#ifdef USE_VCP

#include "usb_io.h"

#include "build/build_config.h"


#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/time.h"

#include "serial.h"
#include "serial_usb_vcp.h"

#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

#include "vcp_hal/usbd_cdc_interface_stm32h7a3.h"
#include "vcp_hal/usbd_desc_stm32h7a3.h"

USBD_HandleTypeDef hUsbDeviceHS;

#ifdef DEBUG
static inline void Error_Handler(void)
{
    while (1) {
        asm volatile("nop");
    }
}
#else
#define Error_Handler()
#endif

void usbVcpInitHardware(void)
 {
    usbGenerateDisconnectPulse();

    IOInit(IOGetByTag(IO_TAG(PA11)), OWNER_USB, RESOURCE_INPUT, 0);
    IOInit(IOGetByTag(IO_TAG(PA12)), OWNER_USB, RESOURCE_OUTPUT, 0);

    // MX_USB_DEVICE_Init code bellow

    //if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK) {
    if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_Start(&hUsbDeviceHS) != USBD_OK) {
        Error_Handler();
    }

    HAL_PWREx_EnableUSBVoltageDetector();

    // END OF MX_USB_Device_Init
    //delay(100); // Cold boot failures observed without this, even when USB cable is not connected
}

#endif