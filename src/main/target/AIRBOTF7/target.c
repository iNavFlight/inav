/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"
#include "drivers/sensor.h"

// Board hardware definitions
BUSDEV_REGISTER_SPI_TAG(busdev_imu0_mpu6000,    DEVHW_MPU6000,  GYRO_0_SPI_BUS,  GYRO_0_CS_PIN,   GYRO_0_EXTI_PIN,     0,  DEVFLAGS_NONE,   GYRO_0_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_imu0_mpu6500,    DEVHW_MPU6500,  GYRO_0_SPI_BUS,  GYRO_0_CS_PIN,   GYRO_0_EXTI_PIN,     0,  DEVFLAGS_NONE,   GYRO_0_ALIGN);

// OMNIBUSF7NANOV7 doesn't have a second gyro
#ifndef OMNIBUSF7NANOV7
BUSDEV_REGISTER_SPI_TAG(busdev_imu1_mpu6000,    DEVHW_MPU6000,  GYRO_1_SPI_BUS,  GYRO_1_CS_PIN,   GYRO_1_EXTI_PIN,     1,  DEVFLAGS_NONE,   GYRO_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_imu1_mpu6500,    DEVHW_MPU6500,  GYRO_1_SPI_BUS,  GYRO_1_CS_PIN,   GYRO_1_EXTI_PIN,     1,  DEVFLAGS_NONE,   GYRO_1_ALIGN);
#endif

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_LED,              0, 0), // LED
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_ANY,              0, 0), // Cam control, SS, UNUSED

    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MC_MOTOR,         0, 0), //S1
    DEF_TIM(TIM4,  CH1, PB6,  TIM_USE_MC_MOTOR,         0, 0), //S2
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_MC_MOTOR,         0, 0), //S3
    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_MC_MOTOR,         0, 0), //S4
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
