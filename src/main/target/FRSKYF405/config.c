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

#include <platform.h>

#include "rx/rx.h"
#include "fc/config.h"

void targetConfiguration(void)
{
    // SBUS is received via a hardware inverter (2N7002E MOSFET, Q1).
    // The signal arrives at UART2 RX already de-inverted (active high),
    // so the UART must NOT apply additional software inversion.
    rxConfigMutable()->serialrx_inverted = 1;
}
