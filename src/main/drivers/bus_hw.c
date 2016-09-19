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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/assert.h"
#include "build/debug.h"

#include "bus.h"
#include "bus_spi.h"

const BusDescriptor_t busHwDesc[MAX_BUS_COUNT] = {
#ifdef USE_SPI_DEVICE_1
    [BUS_SPI1] = { SPI1, &busSpiInit, &busSpiProcessTxn, &busSpiSetSpeed },
#endif
#ifdef USE_SPI_DEVICE_2
    [BUS_SPI2] = { SPI2, &busSpiInit, &busSpiProcessTxn, &busSpiSetSpeed },
#endif
#ifdef USE_SPI_DEVICE_3
    [BUS_SPI3] = { SPI3, &busSpiInit, &busSpiProcessTxn, &busSpiSetSpeed },
#endif
};