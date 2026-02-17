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
 *
 * RP2350 GPIO abstraction for INAV.
 *
 * RP2350 has a flat GPIO model (GPIO 0-29, single port).
 * We map these into INAV's two-port model:
 *   Port A (gpioid 0) = GPIO 0-15
 *   Port B (gpioid 1) = GPIO 16-29
 *
 * Unlike STM32, the pin field in ioRec_t stores the raw GPIO number
 * (not a bitmask), and IO operations use Pico SDK gpio functions.
 */

#include "platform.h"

#include "build/assert.h"
#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"

#include "hardware/gpio.h"

// Port definition tables (from io_def_generated.h via TARGET_IO_PORTx)
#if DEFIO_PORT_USED_COUNT > 0
static const uint16_t ioDefUsedMask[DEFIO_PORT_USED_COUNT] = { DEFIO_PORT_USED_LIST };
static const uint8_t ioDefUsedOffset[DEFIO_PORT_USED_COUNT] = { DEFIO_PORT_OFFSET_LIST };
#else
static const uint16_t ioDefUsedMask[1] = {0};
static const uint8_t ioDefUsedOffset[1] = {0};
#endif

ioRec_t ioRecs[DEFIO_IO_USED_COUNT];

// Convert port index + pin-within-port to raw GPIO number
static inline uint8_t portPinToGpio(unsigned port, unsigned pin)
{
    return (uint8_t)(port * 16 + pin);
}

void IOInitGlobal(void)
{
    ioRec_t *ioRec = ioRecs;

    for (unsigned port = 0; port < ARRAYLEN(ioDefUsedMask); port++) {
        for (unsigned pin = 0; pin < sizeof(ioDefUsedMask[0]) * 8; pin++) {
            if (ioDefUsedMask[port] & (1 << pin)) {
                ioRec->gpio = NULL;
                // Store raw GPIO number (not a bitmask)
                ioRec->pin = portPinToGpio(port, pin);
                ioRec++;
            }
        }
    }
}

ioRec_t *IO_Rec(IO_t io)
{
    ASSERT(io != NULL);
    ASSERT((ioRec_t *)io >= &ioRecs[0]);
    ASSERT((ioRec_t *)io < &ioRecs[DEFIO_IO_USED_COUNT]);
    return io;
}

GPIO_TypeDef *IO_GPIO(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->gpio;
}

uint16_t IO_Pin(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->pin;
}

int IO_GPIOPortIdx(IO_t io)
{
    if (!io) {
        return -1;
    }
    return 0;
}

int IO_GPIOPinIdx(IO_t io)
{
    if (!io) {
        return -1;
    }
    return IO_Pin(io);
}

int IO_GPIO_PortSource(IO_t io)
{
    return IO_GPIOPortIdx(io);
}

int IO_GPIO_PinSource(IO_t io)
{
    return IO_GPIOPinIdx(io);
}

uint32_t IO_EXTI_Line(IO_t io)
{
    UNUSED(io);
    return 0;
}

// --- GPIO read/write via Pico SDK ---

bool IORead(IO_t io)
{
    if (!io) {
        return false;
    }
    return gpio_get(IO_Pin(io));
}

void IOWrite(IO_t io, bool hi)
{
    if (!io) {
        return;
    }
    gpio_put(IO_Pin(io), hi);
}

void IOHi(IO_t io)
{
    if (!io) {
        return;
    }
    gpio_put(IO_Pin(io), 1);
}

void IOLo(IO_t io)
{
    if (!io) {
        return;
    }
    gpio_put(IO_Pin(io), 0);
}

void IOToggle(IO_t io)
{
    if (!io) {
        return;
    }
    gpio_put(IO_Pin(io), !gpio_get(IO_Pin(io)));
}

// --- Pin ownership ---

void IOInit(IO_t io, resourceOwner_e owner, resourceType_e resource, uint8_t index)
{
    if (!io) {
        return;
    }
    ioRec_t *ioRec = IO_Rec(io);
    ioRec->owner = owner;
    ioRec->resource = resource;
    ioRec->index = index;
}

void IORelease(IO_t io)
{
    if (!io) {
        return;
    }
    ioRec_t *ioRec = IO_Rec(io);
    ioRec->owner = OWNER_FREE;
}

resourceOwner_e IOGetOwner(IO_t io)
{
    if (!io) {
        return OWNER_FREE;
    }
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->owner;
}

resourceType_e IOGetResource(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->resource;
}

// --- GPIO configuration ---

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    if (!io) {
        return;
    }

    uint16_t ioPin = IO_Pin(io);

    // Only call gpio_init if pin hasn't been configured yet.
    // This prevents resetting output levels on already-configured SPI CS pins.
    gpio_function_t currentFunction = gpio_get_function(ioPin);
    if (currentFunction == GPIO_FUNC_NULL) {
        gpio_init(ioPin);
    }

    gpio_set_dir(ioPin, (cfg & 0x01));
    gpio_set_pulls(ioPin, (cfg >> 5) & 0x01, (cfg >> 6) & 0x01);
}

void IOConfigGPIOAF(IO_t io, ioConfig_t cfg, uint8_t af)
{
    UNUSED(af);
    IOConfigGPIO(io, cfg);
}

// --- Tag-based lookup ---

IO_t IOGetByTag(ioTag_t tag)
{
    const int portIdx = DEFIO_TAG_GPIOID(tag);
    const int pinIdx = DEFIO_TAG_PIN(tag);

    if (portIdx < 0 || portIdx >= DEFIO_PORT_USED_COUNT) {
        return NULL;
    }

    if (!(ioDefUsedMask[portIdx] & (1 << pinIdx))) {
        return NULL;
    }

    int offset = __builtin_popcount(((1 << pinIdx) - 1) & ioDefUsedMask[portIdx]);
    offset += ioDefUsedOffset[portIdx];
    return ioRecs + offset;
}
