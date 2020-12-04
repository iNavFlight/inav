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

BUSDEV_REGISTER_SPI_TAG(busdev_imu1_6000,   DEVHW_MPU6000,  IMU1_SPI_BUS,   IMU1_CS_PIN,    IMU1_EXTI_PIN,  0,  DEVFLAGS_NONE,  IMU1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_imu2_6500,   DEVHW_MPU6500,  IMU2_SPI_BUS,   IMU2_CS_PIN,    IMU2_EXTI_PIN,  1,  DEVFLAGS_NONE,  IMU2_ALIGN);

//BUSDEV_REGISTER_SPI_TAG(busdev_imu1_6500,   DEVHW_MPU6500,  IMU1_SPI_BUS,   IMU1_CS_PIN,    IMU1_EXTI_PIN,  0,  DEVFLAGS_NONE,  IMU1_ALIGN);
//BUSDEV_REGISTER_SPI_TAG(busdev_imu2_6000,   DEVHW_MPU6000,  IMU2_SPI_BUS,   IMU2_CS_PIN,    IMU2_EXTI_PIN,  1,  DEVFLAGS_NONE,  IMU2_ALIGN);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM2,  CH1, PA0,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),   // S1
    DEF_TIM(TIM2,  CH2, PA1,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),   // S2
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
