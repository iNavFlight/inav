/*
 * This file is part of iNav.
 *
 * iNav is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>

// tristate_e represents something that can take a default AUTO
// value and two explicit ON and OFF values. To ease the transition
// from boolean settings (0 = OFF, 1 = ON), the 1 value has
// been picked as ON while OFF is represented by 2. AUTO is represented
// by 0.
typedef enum {
    TRISTATE_AUTO = 0,
    TRISTATE_ON = 1,
    TRISTATE_OFF = 2,
} tristate_e;

// tristateWithDefaultOnIsActive returns false is tristate is TRISTATE_OFF
// and true otherwise.
static inline bool tristateWithDefaultOnIsActive(tristate_e tristate)
{
    return tristate != TRISTATE_OFF;
}

// tristateWithDefaultOffIsActive returns true is tristate is TRISTATE_ON
// and false otherwise.
static inline bool tristateWithDefaultOffIsActive(tristate_e tristate)
{
    return tristate == TRISTATE_ON;
}

// tristateWithDefaultIsActive() calls tristateWithDefaultOnIsActive() when
// def is true, and tristateWithDefaultOffIsActive() otherwise.
// See tristateWithDefaultOnIsActive() and tristateWithDefaultOffIsActive()
static inline bool tristateWithDefaultIsActive(tristate_e tristate, bool def)
{
    return def ? tristateWithDefaultOnIsActive(tristate) : tristateWithDefaultOffIsActive(tristate);
}