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

#if defined(USE_IRLOCK)

void irlockInit(void);
bool irlockHasBeenDetected(void);

// Regular sample rate of the IRLock task and the retry delay while a transfer is on the bus
#define IRLOCK_UPDATE_RATE_HZ       100
#define IRLOCK_UPDATE_PERIOD_US     (1000000 / IRLOCK_UPDATE_RATE_HZ)
#define IRLOCK_READ_RETRY_US        1000

uint32_t irlockUpdate(void);    // returns the delay until the next call
bool irlockMeasurementIsValid(void);
bool irlockGetPosition(float *distX, float *distY);

#endif /* USE_IRLOCK */
