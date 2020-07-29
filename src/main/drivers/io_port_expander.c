/*
 * This file is part of INAV.
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

#include <stdbool.h>
#include "platform.h"
#include "drivers/io_port_expander.h"
#include "drivers/io_pcf8574.h"

#ifdef USE_I2C_IO_EXPANDER

static ioPortExpanderState_t ioPortExpanderState;

void ioPortExpanderInit(void)
{

    ioPortExpanderState.active = pcf8574Init();

    if (ioPortExpanderState.active) {
        ioPortExpanderState.state = 0x00;
        pcf8574Write(ioPortExpanderState.state); //Set all ports to OFF
    }

}

void ioPortExpanderSet(uint8_t pin, uint8_t value)
{
    if (pin > 7) {
        return;
    }

    //Cast to 0/1
    value = (bool) value;

    ioPortExpanderState.state ^= (-value ^ ioPortExpanderState.state) & (1UL << pin);
    ioPortExpanderState.shouldSync = true;
}

void ioPortExpanderSync(void)
{
    if (ioPortExpanderState.active && ioPortExpanderState.shouldSync) {
        pcf8574Write(ioPortExpanderState.state);
        ioPortExpanderState.shouldSync = false;;
    }
}

#endif