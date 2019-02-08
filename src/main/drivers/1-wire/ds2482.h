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

#pragma once

//#include "drivers/io_types.h"
#include <stdbool.h>
#include "drivers/1-wire.h"

#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482)

bool ds2482Detect(owDev_t *owDev);

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */
