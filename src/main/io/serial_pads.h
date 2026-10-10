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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/streambuf.h"
#include "config/parameter_group.h"
#include "drivers/io_types.h"
#include "drivers/serial.h"
#include "io/serial.h"

#define SERIAL_PAD_UART_COUNT   (SERIAL_PORT_USART8 + 1)

typedef enum {
    SERIAL_PAD_TX = 0,
    SERIAL_PAD_RX,
    SERIAL_PAD_DIRECTION_COUNT
} serialPadDirection_e;

typedef struct serialPadConfig_s {
    // the pad's pin rather than its number, so a target that renumbers its outputs keeps the wiring
    ioTag_t pin[SERIAL_PAD_UART_COUNT][SERIAL_PAD_DIRECTION_COUNT];
} serialPadConfig_t;

PG_DECLARE(serialPadConfig_t, serialPadConfig);

// Pads are numbered from 1 in the order of the Outputs tab (S1, S2, ...); 0 is the UART's own pin
void serialPadsInit(void);
bool serialPadIsRouted(ioTag_t tag);
void serialPadGetOwnPins(int uart, serialPortPins_t *pins);
uint8_t serialPadFind(ioTag_t tag);
bool serialPadIsValid(serialPortIdentifier_e identifier, serialPadDirection_e direction, uint8_t pad);
void serialPadSet(serialPortIdentifier_e identifier, serialPadDirection_e direction, uint8_t pad);
void serialPadsWriteList(sbuf_t *dst);
