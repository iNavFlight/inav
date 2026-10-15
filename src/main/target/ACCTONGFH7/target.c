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

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/sensor.h"
#include "drivers/timer.h"

BUSDEV_REGISTER_SPI_TAG(busdev_gyro1_icm42688, DEVHW_ICM42605, GYRO_1_SPI_BUS, GYRO_1_CS_PIN, GYRO_1_EXTI_PIN, 0, DEVFLAGS_NONE, GYRO_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_gyro2_lsm6dsk320x, DEVHW_LSM6D, GYRO_2_SPI_BUS, GYRO_2_CS_PIN, GYRO_2_EXTI_PIN, 1, DEVFLAGS_NONE, GYRO_2_ALIGN);

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1, CH1, PE9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M1
    DEF_TIM(TIM1, CH2, PE11, TIM_USE_OUTPUT_AUTO, 0, 1),  // M2
    DEF_TIM(TIM1, CH3, PE13, TIM_USE_OUTPUT_AUTO, 0, 2),  // M3
    DEF_TIM(TIM1, CH4, PE14, TIM_USE_OUTPUT_AUTO, 0, 3),  // M4

    DEF_TIM(TIM8, CH1, PC6,  TIM_USE_OUTPUT_AUTO, 0, 4),  // M5
    DEF_TIM(TIM8, CH2, PC7,  TIM_USE_OUTPUT_AUTO, 0, 5),  // M6
    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 6),  // M7
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 7),  // M8

    // DMA2 Stream0 is reserved by ADC1.
    DEF_TIM(TIM2, CH3, PB10, TIM_USE_LED,         0, 9),  // LED strip
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
