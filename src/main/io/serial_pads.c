/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_SERIAL_PADS

#include "config/feature.h"
#include "config/parameter_group_ids.h"

#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/timer.h"

#include "fc/config.h"

#include "io/serial_pads.h"

// usage byte of MSP2_INAV_SERIAL_PADS
typedef enum {
    PAD_USAGE_NONE = 0,
    PAD_USAGE_MOTOR,
    PAD_USAGE_SERVO,
    PAD_USAGE_LED,
} padUsage_e;

PG_REGISTER(serialPadConfig_t, serialPadConfig, PG_SERIAL_PAD_CONFIG, 0);

static ioTag_t ownPins[SERIAL_PAD_UART_COUNT][SERIAL_PAD_DIRECTION_COUNT];

// the outputs MSP2_INAV_OUTPUT_MAPPING_EXT2 lists, in its order
static bool isPad(const timerHardware_t *timHw)
{
    return !(timHw->usageFlags & (TIM_USE_PPM | TIM_USE_PWM));
}

static const timerHardware_t *padHardware(uint8_t pad)
{
    for (int i = 0; pad && i < timerHardwareCount; i++) {
        if (isPad(&timerHardware[i]) && --pad == 0) {
            return &timerHardware[i];
        }
    }
    return NULL;
}

uint8_t serialPadFind(ioTag_t tag)
{
    uint8_t pad = 0;
    for (int i = 0; tag && i < timerHardwareCount; i++) {
        if (isPad(&timerHardware[i])) {
            pad++;
            if (timerHardware[i].tag == tag) {
                return pad;
            }
        }
    }
    return 0;
}

static ioTag_t currentPin(int uart, int direction)
{
    serialPortPins_t pins;
    uartGetPortPins(uart, &pins);
    return direction == SERIAL_PAD_TX ? pins.txPin : pins.rxPin;
}

// what checkPwmTimerConflicts() keeps off the outputs, and a UART would break the same way
static bool isReserved(ioTag_t tag)
{
#ifdef USE_SOFTSERIAL1
    if (feature(FEATURE_SOFTSERIAL) && (tag == IO_TAG(SOFTSERIAL_1_RX_PIN) || tag == IO_TAG(SOFTSERIAL_1_TX_PIN))) {
        return true;
    }
#endif
#ifdef USE_SOFTSERIAL2
    if (feature(FEATURE_SOFTSERIAL) && (tag == IO_TAG(SOFTSERIAL_2_RX_PIN) || tag == IO_TAG(SOFTSERIAL_2_TX_PIN))) {
        return true;
    }
#endif
#ifdef USE_ADC
    const ioTag_t adcPins[] = {
#ifdef ADC_CHANNEL_1_PIN
        IO_TAG(ADC_CHANNEL_1_PIN),
#endif
#ifdef ADC_CHANNEL_2_PIN
        IO_TAG(ADC_CHANNEL_2_PIN),
#endif
#ifdef ADC_CHANNEL_3_PIN
        IO_TAG(ADC_CHANNEL_3_PIN),
#endif
#ifdef ADC_CHANNEL_4_PIN
        IO_TAG(ADC_CHANNEL_4_PIN),
#endif
#ifdef ADC_CHANNEL_5_PIN
        IO_TAG(ADC_CHANNEL_5_PIN),
#endif
#ifdef ADC_CHANNEL_6_PIN
        IO_TAG(ADC_CHANNEL_6_PIN),
#endif
        IO_TAG(NONE)
    };
    for (unsigned i = 0; i < ARRAYLEN(adcPins); i++) {
        if (tag == adcPins[i]) {
            return true;
        }
    }
#endif
    return false;
}

static bool isHeldByUart(ioTag_t tag)
{
    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        if (doesConfigurationUsePort(i) && (currentPin(i, SERIAL_PAD_TX) == tag || currentPin(i, SERIAL_PAD_RX) == tag)) {
            return true;
        }
    }
    return false;
}

void serialPadsInit(void)
{
    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        for (int dir = 0; dir < SERIAL_PAD_DIRECTION_COUNT; dir++) {
            ownPins[i][dir] = currentPin(i, dir);
        }
    }

    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        for (int dir = 0; dir < SERIAL_PAD_DIRECTION_COUNT; dir++) {
            const ioTag_t tag = serialPadConfig()->pin[i][dir];
            // a pin this board has no output on, or one already in use, leaves the UART where it was
            if (doesConfigurationUsePort(i) && timerGetByTag(tag, TIM_USE_ANY) && !isReserved(tag) && !isHeldByUart(tag)) {
                uartRoutePin(i, dir == SERIAL_PAD_TX, tag, true);
            }
        }
    }
}

bool serialPadInEffect(int uart, serialPadDirection_e direction)
{
    const ioTag_t tag = serialPadConfig()->pin[uart][direction];
    return !tag || currentPin(uart, direction) == tag;
}

bool serialPadIsRouted(ioTag_t tag)
{
    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        for (int dir = 0; dir < SERIAL_PAD_DIRECTION_COUNT; dir++) {
            if (tag && currentPin(i, dir) == tag && ownPins[i][dir] != tag) {
                return true;
            }
        }
    }
    return false;
}

void serialPadGetOwnPins(int uart, serialPortPins_t *pins)
{
    pins->txPin = ownPins[uart][SERIAL_PAD_TX];
    pins->rxPin = ownPins[uart][SERIAL_PAD_RX];
}

static bool isOffered(int uart, int direction, ioTag_t tag)
{
    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        // the own pins of a port in use; pads moved to one stay on offer, see serialPadSet()
        if ((i == uart || doesConfigurationUsePort(i)) && (ownPins[i][SERIAL_PAD_TX] == tag || ownPins[i][SERIAL_PAD_RX] == tag)) {
            return false;
        }
    }
    return !isReserved(tag) && uartRoutePin(uart, direction == SERIAL_PAD_TX, tag, false);
}

bool serialPadIsValid(serialPortIdentifier_e identifier, serialPadDirection_e direction, uint8_t pad)
{
    if (identifier < 0 || identifier >= SERIAL_PAD_UART_COUNT || direction >= SERIAL_PAD_DIRECTION_COUNT
            || !serialIsPortAvailable(identifier)) {
        return false;
    }
    const timerHardware_t *timHw = padHardware(pad);
    return pad == 0 || (timHw && isOffered(identifier, direction, timHw->tag));
}

void serialPadSet(serialPortIdentifier_e identifier, serialPadDirection_e direction, uint8_t pad)
{
    const timerHardware_t *timHw = padHardware(pad);
    const ioTag_t tag = timHw ? timHw->tag : IO_TAG(NONE);

    for (int i = 0; tag && i < SERIAL_PAD_UART_COUNT; i++) {
        for (int dir = 0; dir < SERIAL_PAD_DIRECTION_COUNT; dir++) {
            if (serialPadConfig()->pin[i][dir] == tag) {
                serialPadConfigMutable()->pin[i][dir] = IO_TAG(NONE);
            }
        }
    }
    serialPadConfigMutable()->pin[identifier][direction] = tag;
}

static padUsage_e padUsage(const timerHardware_t *timHw, uint8_t *number)
{
    switch (pwmGetPadFunction(timHw, number)) {
        case TIM_USE_MOTOR: return PAD_USAGE_MOTOR;
        case TIM_USE_SERVO: return PAD_USAGE_SERVO;
        case TIM_USE_LED:   return PAD_USAGE_LED;
        default:            return PAD_USAGE_NONE;
    }
}

void serialPadsWriteList(sbuf_t *dst)
{
    uint8_t *count = sbufPtr(dst);
    sbufWriteU8(dst, 0);

    for (int i = 0; i < SERIAL_PAD_UART_COUNT; i++) {
        if (!serialIsPortAvailable(i)) {
            continue;
        }
        for (int dir = 0; dir < SERIAL_PAD_DIRECTION_COUNT; dir++) {
            uint8_t pad = 0;
            for (int t = 0; t < timerHardwareCount; t++) {
                const timerHardware_t *timHw = &timerHardware[t];
                if (!isPad(timHw)) {
                    continue;
                }
                pad++;
                // a pin listed twice in timerHardware[] is offered once
                if (serialPadFind(timHw->tag) != pad || !isOffered(i, dir, timHw->tag) || sbufBytesRemaining(dst) < 6) {
                    continue;
                }
                uint8_t number;
                const padUsage_e usage = padUsage(timHw, &number);
                sbufWriteU8(dst, i);
                sbufWriteU8(dst, dir);
                sbufWriteU8(dst, pad);
                sbufWriteU8(dst, serialPadConfig()->pin[i][dir] == timHw->tag);
                sbufWriteU8(dst, usage);
                sbufWriteU8(dst, number);
                (*count)++;
            }
        }
    }
}

#endif
