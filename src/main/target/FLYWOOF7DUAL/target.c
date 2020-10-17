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
#include <stdbool.h>
#include <platform.h>

#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"
#include "drivers/pwm_mapping.h"

#include "fc/fc_msp_box.h"

#include "io/piniobox.h"

// IMU 1
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000_1,   DEVHW_MPU6000,  GYRO_1_SPI_BUS, GYRO_1_CS_PIN,  GYRO_1_EXTI_PIN,    0,  DEVFLAGS_NONE,  GYRO_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500_1,   DEVHW_MPU6500,  GYRO_1_SPI_BUS, GYRO_1_CS_PIN,  GYRO_1_EXTI_PIN,    0,  DEVFLAGS_NONE,  GYRO_1_ALIGN);

// IMU 2
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000_2,   DEVHW_MPU6000,  GYRO_2_SPI_BUS, GYRO_2_CS_PIN,  GYRO_2_EXTI_PIN,    1,  DEVFLAGS_NONE,  GYRO_2_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500_2,   DEVHW_MPU6500,  GYRO_2_SPI_BUS, GYRO_2_CS_PIN,  GYRO_2_EXTI_PIN,    1,  DEVFLAGS_NONE,  GYRO_2_ALIGN);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8,  CH1, PC6,  TIM_USE_PPM,                          0, 0),  // PPM&SBUS  

    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),      // S1 - D(1,2)
    DEF_TIM(TIM3,  CH1, PB4,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),      // S2 - D(1,4)
    DEF_TIM(TIM2,  CH2, PB3,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),      // S3 - D(1,6)
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),      // S4 - D(1,5)  
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),      // S5 - D(2,4) 
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),      // S6 - D(2,1)

    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED,                          0, 0),  // LED STRIP(1,5)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
}
