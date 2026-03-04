/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * RP2350 USB VCP serial driver for INAV
 *
 * Implements INAV's serialPort_t vtable using TinyUSB CDC class.
 * TinyUSB is initialized in systemInit() via tusb_init(); USB event processing
 * is driven exclusively by a 1ms repeating hardware alarm in systemInit().
 * tud_task() must only be called from that single timer callback — calling it
 * from foreground code while the timer IRQ can preempt creates re-entrancy
 * that corrupts TinyUSB shared state.
 *
 * This file also provides the three mandatory TinyUSB descriptor callbacks
 * (tud_descriptor_device_cb, tud_descriptor_configuration_cb,
 * tud_descriptor_string_cb).  They were previously supplied by pico_stdio_usb's
 * stdio_usb_descriptors.c, but that file is guarded by
 * "#if !defined(LIB_TINYUSB_DEVICE)" and is therefore inactive now that we
 * define LIB_TINYUSB_DEVICE=1 to prevent pico_stdio_usb from spawning a
 * competing tud_task() timer.
 */

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#ifdef USE_VCP

#include "build/build_config.h"

#include "common/utils.h"
#include "drivers/time.h"

#include "serial.h"
#include "serial_usb_vcp.h"

#include "tusb.h"

#define USB_TIMEOUT  50

static vcpPort_t vcpPort;

static void usbVcpSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}

static void usbVcpSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

static void usbVcpSetOptions(serialPort_t *instance, portOptions_t options)
{
    UNUSED(instance);
    UNUSED(options);
}

static bool isUsbVcpTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

static uint32_t usbVcpAvailable(const serialPort_t *instance)
{
    UNUSED(instance);
    return tud_cdc_available();
}

static uint8_t usbVcpRead(serialPort_t *instance)
{
    UNUSED(instance);
    // Callers must check serialRxBytesWaiting() > 0 before calling serialRead().
    // Do not spin here — that would block the cooperative scheduler.
    uint8_t ch = 0;
    tud_cdc_read(&ch, 1);
    return ch;
}

static bool usbVcpIsConnected(const serialPort_t *instance)
{
    (void)instance;
    return tud_cdc_connected();
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
        uint32_t written = tud_cdc_write(p, count);
        if (written > 0) {
            count -= written;
            p += written;
        }
        tud_cdc_write_flush();
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

    if (!tud_cdc_connected()) {
        return false;
    }

    uint32_t start = millis();
    uint8_t *p = port->txBuf;
    while (count > 0) {
        uint32_t written = tud_cdc_write(p, count);
        if (written > 0) {
            count -= written;
            p += written;
        }
        tud_cdc_write_flush();
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
    return tud_cdc_write_available();
}

static void usbVcpEndWrite(serialPort_t *instance)
{
    vcpPort_t *port = container_of(instance, vcpPort_t, port);
    port->buffering = false;
    usbVcpFlush(port);
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

void usbVcpInitHardware(void)
{
    // tusb_init() is called in systemInit() before fc_init().
    // tud_task() is driven by pico_stdio_usb's 1ms repeating alarm.
    // Nothing additional needed here.
}

serialPort_t *usbVcpOpen(void)
{
    vcpPort_t *s = &vcpPort;
    s->port.vTable = usbVTable;
    return (serialPort_t *)s;
}

uint32_t usbVcpGetBaudRate(serialPort_t *instance)
{
    UNUSED(instance);

    cdc_line_coding_t coding;
    tud_cdc_get_line_coding(&coding);
    return coding.bit_rate;
}

// ── TinyUSB descriptor callbacks ─────────────────────────────────────────────
//
// These three callbacks are mandatory for TinyUSB device mode.  They were
// previously provided by pico_stdio_usb/stdio_usb_descriptors.c, but that
// file is compiled only when LIB_TINYUSB_DEVICE is NOT defined.  Since we
// now define LIB_TINYUSB_DEVICE=1 (to stop pico_stdio_usb from spawning a
// competing tud_task() timer), we must provide the callbacks here instead.
//
// USB identifiers:
//   VID 0x2E8A = Raspberry Pi
//   PID 0x000B = Pico 2 (RP2350) CDC serial application
//   Product string from USBD_PRODUCT_STRING defined in target.h

#include "pico/unique_id.h"

#ifndef USBD_VID
#define USBD_VID 0x2E8A  // Raspberry Pi
#endif
#ifndef USBD_PID
#define USBD_PID 0x000B  // Pico 2 (RP2350) CDC application
#endif
#ifndef USBD_MANUFACTURER
#define USBD_MANUFACTURER "Raspberry Pi"
#endif
// USBD_PRODUCT_STRING comes from target.h (e.g. "RP2350_PICO")
#ifndef USBD_PRODUCT_STRING
#define USBD_PRODUCT_STRING "INAV"
#endif

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

#define USBD_ITF_CDC        0   // two interfaces: control + data
#define USBD_ITF_MAX        2

#define USBD_CDC_EP_CMD     0x81
#define USBD_CDC_EP_OUT     0x02
#define USBD_CDC_EP_IN      0x82
#define USBD_CDC_CMD_MAX_SIZE   8
#define USBD_CDC_IN_OUT_MAX_SIZE 64

#define USBD_STR_LANGID     0x00
#define USBD_STR_MANUF      0x01
#define USBD_STR_PRODUCT    0x02
#define USBD_STR_SERIAL     0x03
#define USBD_STR_CDC        0x04

static const tusb_desc_device_t usbd_desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USBD_VID,
    .idProduct          = USBD_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = USBD_STR_MANUF,
    .iProduct           = USBD_STR_PRODUCT,
    .iSerialNumber      = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGID, USBD_DESC_LEN,
        0, 250),
    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN,
        USBD_CDC_IN_OUT_MAX_SIZE),
};

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *const usbd_desc_str[] = {
    [USBD_STR_MANUF]   = USBD_MANUFACTURER,
    [USBD_STR_PRODUCT] = USBD_PRODUCT_STRING,
    [USBD_STR_SERIAL]  = usbd_serial_str,
    [USBD_STR_CDC]     = "INAV MSP",
};

const uint8_t *tud_descriptor_device_cb(void)
{
    return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return usbd_desc_cfg;
}

#define USBD_DESC_STR_MAX 20

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    static uint16_t desc_str[USBD_DESC_STR_MAX];

    if (!usbd_serial_str[0]) {
        pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
    }

    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409;  // English
        len = 1;
    } else {
        if (index >= ARRAYLEN(usbd_desc_str) || !usbd_desc_str[index]) {
            return NULL;
        }
        const char *str = usbd_desc_str[index];
        for (len = 0; len < USBD_DESC_STR_MAX - 1 && str[len]; len++) {
            desc_str[1 + len] = str[len];
        }
    }

    desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
    return desc_str;
}

#endif // USE_VCP
