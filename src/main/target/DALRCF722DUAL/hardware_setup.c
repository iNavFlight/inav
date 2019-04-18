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
#include "platform.h"

#include "drivers/io_impl.h"

#ifdef USE_HARDWARE_PREBOOT_SETUP


void initialisePreBootHardware(void)
{
    //DALRC F722 have dual GYRO:mpu6000 and icm20602,they use same SPI but different CS pin.Just use mpu6000 for INAV.
    //So set icm20602 CS pin high to block icm20602 SPI .
    IOInit(DEFIO_IO(PA4), OWNER_SYSTEM, RESOURCE_OUTPUT, 0);
    IOConfigGPIO(DEFIO_IO(PA4), IOCFG_OUT_PP);
    IOHi(DEFIO_IO(PA4));
}
#endif


