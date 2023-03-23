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

#include "platform.h"

#include "build/assert.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/rcc.h"

// io ports defs are stored in array by index now
struct ioPortDef_s {
    rccPeriphTag_t rcc;
};

#if defined(STM32F4)
const struct ioPortDef_s ioPortDefs[] = {
    { RCC_AHB1(GPIOA) },
    { RCC_AHB1(GPIOB) },
    { RCC_AHB1(GPIOC) },
    { RCC_AHB1(GPIOD) },
    { RCC_AHB1(GPIOE) },
    { RCC_AHB1(GPIOF) },
};
#elif defined(STM32F7)
const struct ioPortDef_s ioPortDefs[] = {
    { RCC_AHB1(GPIOA) },
    { RCC_AHB1(GPIOB) },
    { RCC_AHB1(GPIOC) },
    { RCC_AHB1(GPIOD) },
    { RCC_AHB1(GPIOE) },
    { RCC_AHB1(GPIOF) },
};
#elif defined(STM32H7)
const struct ioPortDef_s ioPortDefs[] = {
    { RCC_AHB4(GPIOA) },
    { RCC_AHB4(GPIOB) },
    { RCC_AHB4(GPIOC) },
    { RCC_AHB4(GPIOD) },
    { RCC_AHB4(GPIOE) },
    { RCC_AHB4(GPIOF) },
    { RCC_AHB4(GPIOG) },
    { RCC_AHB4(GPIOH) },
    { RCC_AHB4(GPIOI) },
};
#elif defined(AT32F43x)   
const struct ioPortDef_s ioPortDefs[] = {
    { RCC_AHB1(GPIOA) },
    { RCC_AHB1(GPIOB) },
    { RCC_AHB1(GPIOC) },
    { RCC_AHB1(GPIOD) },
    { RCC_AHB1(GPIOE) },
    { RCC_AHB1(GPIOF) },
    { RCC_AHB1(GPIOG) },
    { RCC_AHB1(GPIOH) },
};
# endif

ioRec_t* IO_Rec(IO_t io)
{
    ASSERT(io != NULL);
    ASSERT((ioRec_t*)io >= &ioRecs[0]);
    ASSERT((ioRec_t*)io < &ioRecs[DEFIO_IO_USED_COUNT]);

    return io;
}

#if defined(AT32F43x)   
gpio_type * IO_GPIO(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->gpio;
}
#else
GPIO_TypeDef * IO_GPIO(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->gpio;
}
# endif
 
uint16_t IO_Pin(IO_t io)
{
    const ioRec_t *ioRec = IO_Rec(io);
    return ioRec->pin;
}

// port index, GPIOA == 0
int IO_GPIOPortIdx(IO_t io)
{
    if (!io) {
        return -1;
    }
    return (((size_t)IO_GPIO(io) - GPIOA_BASE) >> 10);     // ports are 0x400 apart
}

int IO_EXTI_PortSourceGPIO(IO_t io)
{
    return IO_GPIOPortIdx(io);
}

int IO_GPIO_PortSource(IO_t io)
{
    return IO_GPIOPortIdx(io);
}

// zero based pin index
int IO_GPIOPinIdx(IO_t io)
{
    if (!io) {
        return -1;
    }
    return 31 - __builtin_clz(IO_Pin(io));  // CLZ is a bit faster than FFS
}

int IO_EXTI_PinSource(IO_t io)
{
    return IO_GPIOPinIdx(io);
}

int IO_GPIO_PinSource(IO_t io)
{
    return IO_GPIOPinIdx(io);
}

uint32_t IO_EXTI_Line(IO_t io)
{
    if (!io) {
        return 0;
    }
#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7) || defined(AT32F43x)
    return 1 << IO_GPIOPinIdx(io);
#else
# error "Unknown target type"
#endif
}

bool IORead(IO_t io)
{
    if (!io) {
        return false;
    }
#if defined(USE_HAL_DRIVER)
    return !! HAL_GPIO_ReadPin(IO_GPIO(io),IO_Pin(io));
#elif defined(AT32F43x)
    return !! (IO_GPIO(io)->idt & IO_Pin(io));
#else
    return !! (IO_GPIO(io)->IDR & IO_Pin(io));
#endif
}

void IOWrite(IO_t io, bool hi)
{
    if (!io) {
        return;
    }
#if defined(USE_HAL_DRIVER)
    if (hi) {
        HAL_GPIO_WritePin(IO_GPIO(io),IO_Pin(io),GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(IO_GPIO(io),IO_Pin(io),GPIO_PIN_RESET);
    }
#elif defined(STM32F4)  
    if (hi) {
        IO_GPIO(io)->BSRRL = IO_Pin(io);   
    } else {
        IO_GPIO(io)->BSRRH = IO_Pin(io);   
    }
#elif defined(AT32F43x)
    IO_GPIO(io)->scr = IO_Pin(io) << (hi ? 0 : 16); 
#else
    IO_GPIO(io)->BSRR = IO_Pin(io) << (hi ? 0 : 16);  
#endif
}

void IOHi(IO_t io)
{
    if (!io) {
        return;
    }
#if defined(USE_HAL_DRIVER)
    HAL_GPIO_WritePin(IO_GPIO(io),IO_Pin(io),GPIO_PIN_SET);
#elif defined(STM32F4)
    IO_GPIO(io)->BSRRL = IO_Pin(io);
#elif defined(AT32F43x)
    IO_GPIO(io)->scr = IO_Pin(io);
#else
    IO_GPIO(io)->BSRR = IO_Pin(io);
#endif
}

void IOLo(IO_t io)
{
    if (!io) {
        return;
    }
#if defined(USE_HAL_DRIVER)
    HAL_GPIO_WritePin(IO_GPIO(io),IO_Pin(io),GPIO_PIN_RESET);
#elif defined(STM32F4)  
    IO_GPIO(io)->BSRRH = IO_Pin(io);
#elif defined(AT32F43x)
    IO_GPIO(io)->clr = IO_Pin(io);  
#else
    IO_GPIO(io)->BRR = IO_Pin(io);
#endif
}

void IOToggle(IO_t io)
{
    if (!io) {
        return;
    }

    uint32_t mask = IO_Pin(io);
    // Read pin state from ODR but write to BSRR because it only changes the pins
    // high in the mask value rather than all pins. XORing ODR directly risks
    // setting other pins incorrectly because it change all pins' state.
#if defined(USE_HAL_DRIVER)
    (void)mask;
    HAL_GPIO_TogglePin(IO_GPIO(io),IO_Pin(io));
#elif defined(STM32F4)
    if (IO_GPIO(io)->ODR & mask) {
        IO_GPIO(io)->BSRRH = mask;
    } else {
        IO_GPIO(io)->BSRRL = mask;
    }
#elif defined(AT32F43x)
 if (IO_GPIO(io)->odt & mask)
        mask <<= 16;   // bit is set, shift mask to reset half 
    
    IO_GPIO(io)->scr = IO_Pin(io);
#else
    if (IO_GPIO(io)->ODR & mask)
        mask <<= 16;   // bit is set, shift mask to reset half
    
    IO_GPIO(io)->BSRR = mask;
#endif
}

// claim IO pin, set owner and resources
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

#if defined(STM32F7) || defined(STM32H7)

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE);

    GPIO_InitTypeDef init = {
        .Pin = IO_Pin(io),
        .Mode = (cfg >> 0) & 0x13,
        .Speed = (cfg >> 2) & 0x03,
        .Pull = (cfg >> 5) & 0x03,
    };
    HAL_GPIO_Init(IO_GPIO(io), &init);
}

void IOConfigGPIOAF(IO_t io, ioConfig_t cfg, uint8_t af)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE);
    GPIO_InitTypeDef init = {
        .Pin = IO_Pin(io),
        .Mode = (cfg >> 0) & 0x13,
        .Speed = (cfg >> 2) & 0x03,
        .Pull = (cfg >> 5) & 0x03,
        .Alternate = af
    };
    HAL_GPIO_Init(IO_GPIO(io), &init);
}

#elif defined(STM32F4)

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE);

    GPIO_InitTypeDef init = {
        .GPIO_Pin = IO_Pin(io),
        .GPIO_Mode = (cfg >> 0) & 0x03,
        .GPIO_Speed = (cfg >> 2) & 0x03,
        .GPIO_OType = (cfg >> 4) & 0x01,
        .GPIO_PuPd = (cfg >> 5) & 0x03,
    };
    GPIO_Init(IO_GPIO(io), &init);
}

void IOConfigGPIOAF(IO_t io, ioConfig_t cfg, uint8_t af)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE);
    GPIO_PinAFConfig(IO_GPIO(io), IO_GPIO_PinSource(io), af);

    GPIO_InitTypeDef init = {
        .GPIO_Pin = IO_Pin(io),
        .GPIO_Mode = (cfg >> 0) & 0x03,
        .GPIO_Speed = (cfg >> 2) & 0x03,
        .GPIO_OType = (cfg >> 4) & 0x01,
        .GPIO_PuPd = (cfg >> 5) & 0x03,
    };
    GPIO_Init(IO_GPIO(io), &init);
}
#elif defined(AT32F43x)

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE); 
 
    gpio_init_type init = {
        .gpio_pins = IO_Pin(io),
        .gpio_out_type = (cfg >> 4) & 0x01,
        .gpio_pull = (cfg >> 5) & 0x03,
        .gpio_mode = (cfg >> 0) & 0x03,
        .gpio_drive_strength =  (cfg >> 2) & 0x03,
     };
  
    gpio_init(IO_GPIO(io), &init);
}

void IOConfigGPIOAF(IO_t io, ioConfig_t cfg, uint8_t af)
{
    if (!io) {
        return;
    }
    const rccPeriphTag_t rcc = ioPortDefs[IO_GPIOPortIdx(io)].rcc;
    RCC_ClockCmd(rcc, ENABLE);
    // Must run configure the pin's muxing function
    gpio_pin_mux_config(IO_GPIO(io), IO_GPIO_PinSource(io), af);

    gpio_init_type init = {
        .gpio_pins = IO_Pin(io),
        .gpio_out_type = (cfg >> 4) & 0x01,
        .gpio_pull = (cfg >> 5) & 0x03,
        .gpio_mode = (cfg >> 0) & 0x03,
        .gpio_drive_strength =  (cfg >> 2) & 0x03,
     };
    gpio_init(IO_GPIO(io), &init);
}

#endif

static const uint16_t ioDefUsedMask[DEFIO_PORT_USED_COUNT] = { DEFIO_PORT_USED_LIST };
static const uint8_t ioDefUsedOffset[DEFIO_PORT_USED_COUNT] = { DEFIO_PORT_OFFSET_LIST };
ioRec_t ioRecs[DEFIO_IO_USED_COUNT];

// initialize all ioRec_t structures from ROM
// currently only bitmask is used, this may change in future
void IOInitGlobal(void)
{
    ioRec_t *ioRec = ioRecs;

    for (unsigned port = 0; port < ARRAYLEN(ioDefUsedMask); port++) {
        for (unsigned pin = 0; pin < sizeof(ioDefUsedMask[0]) * 8; pin++) {
            if (ioDefUsedMask[port] & (1 << pin)) {
#if defined(AT32F43x)
                ioRec->gpio = (gpio_type *)(GPIOA_BASE + (port << 10));   // ports are 0x400 apart
#else
                ioRec->gpio = (GPIO_TypeDef *)(GPIOA_BASE + (port << 10));   // ports are 0x400 apart 
# endif
                ioRec->pin = 1 << pin;
                ioRec++;
            }
        }
    }
}

IO_t IOGetByTag(ioTag_t tag)
{
    const int portIdx = DEFIO_TAG_GPIOID(tag);
    const int pinIdx = DEFIO_TAG_PIN(tag);

    if (portIdx < 0 || portIdx >= DEFIO_PORT_USED_COUNT) {
        return NULL;
    }
    // Check if pin exists
    if (!(ioDefUsedMask[portIdx] & (1 << pinIdx))) {
        return NULL;
    }
    // Count bits before this pin on single port
    int offset = __builtin_popcount(((1 << pinIdx) - 1) & ioDefUsedMask[portIdx]);
    // Add port offset
    offset += ioDefUsedOffset[portIdx];
    return ioRecs + offset;
}
