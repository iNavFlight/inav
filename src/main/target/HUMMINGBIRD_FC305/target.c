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

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"
#include "drivers/sensor.h"

BUSDEV_REGISTER_SPI_TAG(busdev_icm42688, DEVHW_ICM42605, ICM42605_SPI_BUS, ICM42605_CS_PIN, NONE, 0, DEVFLAGS_NONE, IMU_ICM42605_ALIGN);
BUSDEV_REGISTER_SPI(busdev_sdcard_spi,  DEVHW_SDCARD,       SDCARD_SPI_BUS,     SDCARD_CS_PIN,      NONE,           DEVFLAGS_SPI_MODE_0,  0);
BUSDEV_REGISTER_SPI(busdev_max7456,    DEVHW_MAX7456,  MAX7456_SPI_BUS,    MAX7456_CS_PIN, NONE, DEVFLAGS_USE_RAW_REGISTERS,  0);
BUSDEV_REGISTER_I2C(busdev_spl06,      DEVHW_SPL06,        SPL06_I2C_BUS,      SPL06_I2C_ADDR,     NONE,           DEVFLAGS_NONE,      0);

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3,  CH3,  PB0, TIM_USE_OUTPUT_AUTO, 0, 0), // S1
    DEF_TIM(TIM3,  CH4,  PB1, TIM_USE_OUTPUT_AUTO, 0, 0), // S2
    DEF_TIM(TIM3,  CH2,  PB5, TIM_USE_OUTPUT_AUTO, 0, 0), // S3
    DEF_TIM(TIM3,  CH1,  PB4, TIM_USE_OUTPUT_AUTO, 0, 0), // S4

    DEF_TIM(TIM1,  CH1,  PA8, TIM_USE_LED,                         0, 1),//WS2812B
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
