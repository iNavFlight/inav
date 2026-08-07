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
#include <stdlib.h>

#include "platform.h"

#include "build/build_config.h"

#include "drivers/time.h"
#include "drivers/bus_spi.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"

void initialisePreBootHardware(void)
{
    // VDD_3V3_SENSORS_EN
    IOInit(DEFIO_IO(PB2), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PB2), IOCFG_OUT_PP);
    // IOLo(DEFIO_IO(PB2));
    // delay(100);
    IOHi(DEFIO_IO(PB2));

    // CAM Switch / User3
    IOInit(DEFIO_IO(PC13), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PC13), IOCFG_OUT_PP);
    IOLo(DEFIO_IO(PC13));

    // User1
    IOInit(DEFIO_IO(PD4), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PD4), IOCFG_OUT_PP);
    IOLo(DEFIO_IO(PD4));

    // VTx 9V Switch / User4
    IOInit(DEFIO_IO(PE3), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PE3), IOCFG_OUT_PP);
    IOHi(DEFIO_IO(PE3));

    // User2
    IOInit(DEFIO_IO(PE4), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PE4), IOCFG_OUT_PP);
    IOLo(DEFIO_IO(PE4));
}
