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
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

// Gyro 1: ICM42688P on SPI1, tag=0
BUSDEV_REGISTER_SPI_TAG(busdev_icm42688_1, DEVHW_ICM42605, ICM42605_1_SPI_BUS, ICM42605_1_CS_PIN, NONE, 0, DEVFLAGS_NONE, IMU_ICM42605_ALIGN);
// Gyro 2: ICM42688P on SPI4, tag=1
BUSDEV_REGISTER_SPI_TAG(busdev_icm42688_2, DEVHW_ICM42605, ICM42605_2_SPI_BUS, ICM42605_2_CS_PIN, NONE, 1, DEVFLAGS_NONE, IMU_ICM42605_2_ALIGN);

timerHardware_t timerHardware[] = {
    // ---- Motors (TIM1 x4) ----
    DEF_TIM(TIM1,  CH1,  PE9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M1
    DEF_TIM(TIM1,  CH2,  PE11, TIM_USE_OUTPUT_AUTO, 0, 1),  // M2
    DEF_TIM(TIM1,  CH3,  PE13, TIM_USE_OUTPUT_AUTO, 0, 2),  // M3
    DEF_TIM(TIM1,  CH4,  PE14, TIM_USE_OUTPUT_AUTO, 0, 3),  // M4

    // ---- Motors (TIM8 x2) ----
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_OUTPUT_AUTO, 0, 4),  // M5
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_OUTPUT_AUTO, 0, 5),  // M6

    // ---- Motors (TIM3 x2) ----
    DEF_TIM(TIM3,  CH1,  PA6,  TIM_USE_OUTPUT_AUTO, 0, 6),  // M7
    DEF_TIM(TIM3,  CH2,  PA7,  TIM_USE_OUTPUT_AUTO, 0, 7),  // M8

    // ---- LED Strip (TIM5 CH1) ----
    DEF_TIM(TIM5,  CH1,  PA0,  TIM_USE_LED, 0, 8),          // WS2812 LED Strip

    // ---- Camera Control (PA1 / TIM15_CH1N) ----
    DEF_TIM(TIM15, CH1N, PA1,  TIM_USE_ANY, 0, 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
