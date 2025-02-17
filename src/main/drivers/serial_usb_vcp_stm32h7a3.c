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

#ifdef STM32H7A3xx
#include "vcp_hal/stm32h7a3/inav_cdc.h"
#include "vcp_hal/stm32h7a3/usbd_cdc_if.h"
#include "vcp_hal/stm32h7a3/usbd_desc.h"
#else
#error "Unknown MCU"
#endif

extern USBD_HandleTypeDef hUsbDeviceHS;

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

void MX_USB_DEVICE_Init(void);

void usbVcpInitHardware(void)
 {
    MX_USB_DEVICE_Init();

/*
    //usbGenerateDisconnectPulse();

    IOInit(IOGetByTag(IO_TAG(PA11)), OWNER_USB, RESOURCE_INPUT, 0);
    IOInit(IOGetByTag(IO_TAG(PA12)), OWNER_USB, RESOURCE_OUTPUT, 0);

    // MX_USB_DEVICE_Init code bellow

    __HAL_RCC_SYSCFG_CLK_ENABLE();

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
    */
}
#define USB_TIMEOUT  50

static vcpPort_t vcpPort;

static void usbVcpSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);

    // TODO implement
}

static void usbVcpSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);

    // TODO implement
}

static void usbVcpSetOptions(serialPort_t *instance, portOptions_t options)
{
    UNUSED(instance);
    UNUSED(options);

    // TODO implement
}

static bool isUsbVcpTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

static uint32_t usbVcpAvailable(const serialPort_t *instance)
{
    UNUSED(instance);

    return CDC_Receive_BytesAvailable();
}

static uint8_t usbVcpRead(serialPort_t *instance)
{
    UNUSED(instance);

    uint8_t buf[1];

    while (true) {
        if (CDC_Receive_DATA(buf, 1))
            return buf[0];
    }
}

static bool usbVcpIsConnected(const serialPort_t *instance)
{
    (void)instance;
    return usbIsConnected() && usbIsConfigured();
}

static void usbVcpWriteBuf(serialPort_t *instance, const void *data, int count)
{
    UNUSED(instance);

    if (!usbVcpIsConnected(instance)) {
        return;
    }

    uint32_t start = millis();
    const uint8_t *p = data;
    while (count > 0) {
        uint32_t txed = CDC_Send_DATA(p, count);
        count -= txed;
        p += txed;

        if (millis() - start > USB_TIMEOUT) {
            break;
        }
    }
}

static bool usbVcpFlush(vcpPort_t *port)
{
    uint32_t count = port->txAt;
    port->txAt = 0;

    if (count == 0) {
        return true;
    }

    if (!usbIsConnected() || !usbIsConfigured()) {
        return false;
    }

    uint32_t start = millis();
    uint8_t *p = port->txBuf;
    while (count > 0) {
        uint32_t txed = CDC_Send_DATA(p, count);
        count -= txed;
        p += txed;

        if (millis() - start > USB_TIMEOUT) {
            break;
        }
    }
    return count == 0;
}

static void usbVcpWrite(serialPort_t *instance, uint8_t c)
{
    vcpPort_t *port = container_of(instance, vcpPort_t, port);

    port->txBuf[port->txAt++] = c;
    if (!port->buffering || port->txAt >= ARRAYLEN(port->txBuf)) {
        usbVcpFlush(port);
    }
}

static void usbVcpBeginWrite(serialPort_t *instance)
{
    vcpPort_t *port = container_of(instance, vcpPort_t, port);
    port->buffering = true;
}

static uint32_t usbTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return CDC_Send_FreeBytes();
}

static void usbVcpEndWrite(serialPort_t *instance)
{
    vcpPort_t *port = container_of(instance, vcpPort_t, port);
    port->buffering = false;
    usbVcpFlush(port);
}



uint32_t usbVcpGetBaudRate(serialPort_t *instance)
{
    UNUSED(instance);

    return CDC_BaudRate();
}

static const struct serialPortVTable usbVTable[] = {
    {
        .serialWrite = usbVcpWrite,
        .serialTotalRxWaiting = usbVcpAvailable,
        .serialTotalTxFree = usbTxBytesFree,
        .serialRead = usbVcpRead,
        .serialSetBaudRate = usbVcpSetBaudRate,
        .isSerialTransmitBufferEmpty = isUsbVcpTransmitBufferEmpty,
        .setMode = usbVcpSetMode,
        .setOptions = usbVcpSetOptions,
        .isConnected = usbVcpIsConnected,
        .writeBuf = usbVcpWriteBuf,
        .beginWrite = usbVcpBeginWrite,
        .endWrite = usbVcpEndWrite,
        .isIdle = NULL,
    }
};

serialPort_t *usbVcpOpen(void)
{
    vcpPort_t *s;

    s = &vcpPort;
    s->port.vTable = usbVTable;

    return (serialPort_t *)s;
}


#endif
