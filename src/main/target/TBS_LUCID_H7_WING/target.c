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

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"

BUSDEV_REGISTER_SPI_TAG(busdev_gyro1_mpu6000,  DEVHW_MPU6000,  GYRO1_SPI_BUS,   GYRO1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_MPU6000_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_gyro1_icm42688,  DEVHW_ICM42605,  GYRO1_SPI_BUS,   GYRO1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ICM42605_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_gyro2_mpu6000,  DEVHW_MPU6000,  GYRO2_SPI_BUS,   GYRO2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_MPU6000_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_gyro2_icm42688,  DEVHW_ICM42605,  GYRO2_SPI_BUS,   GYRO2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ICM42605_ALIGN);

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_OUTPUT_AUTO, 0, 0),   // S1
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_OUTPUT_AUTO, 0, 1),   // S2

    DEF_TIM(TIM5, CH1, PA0, TIM_USE_OUTPUT_AUTO, 0, 2),   // S3  
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_OUTPUT_AUTO, 0, 3),   // S4
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_OUTPUT_AUTO, 0, 4),   // S5
    DEF_TIM(TIM5, CH4, PA3, TIM_USE_OUTPUT_AUTO, 0, 5),   // S6

    DEF_TIM(TIM4, CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 6),   // S7
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 7),   // S8
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 0),   // S9
    DEF_TIM(TIM4, CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 0),   // S10 DMA_NONE

    DEF_TIM(TIM15, CH1, PE5, TIM_USE_OUTPUT_AUTO, 0, 0),   // S11
    DEF_TIM(TIM15, CH2, PE6, TIM_USE_OUTPUT_AUTO, 0, 0),   // S12 DMA_NONE

    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 9),   // S13
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
