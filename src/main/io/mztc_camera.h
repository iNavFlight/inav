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

#include "drivers/time.h"
#include "drivers/serial.h"
#include "config/mztc_camera.h"

#ifdef USE_MZTC

// The driver API lives in config/mztc_camera.h alongside the data model. That
// keeps the parameter group, the limits and the functions that enforce them in
// one place. This header exists for the callers that only want the driver. It
// adds the accessors that are outside the data model.

// Identity bytes reported by the camera in its model or version reply. Returns
// NULL until the camera has answered a probe. len receives the byte count.
const uint8_t *mztcGetDeviceId(uint8_t *len);

#endif // USE_MZTC
