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

BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000,   DEVHW_MPU6000,  MPU6000_SPI_BUS,   MPU6000_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_MPU6000_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_bmi270,   DEVHW_BMI270,  BMI270_SPI_BUS,   BMI270_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_BMI270_ALIGN);

#ifdef MAMBAH743_2022B
BUSDEV_REGISTER_SPI_TAG(busdev_icm42605,    DEVHW_ICM42605, ICM42605_SPI_BUS,   ICM42605_CS_PIN,    ICM42605_EXTI_PIN,  0,  DEVFLAGS_NONE,  IMU_ICM42605_ALIGN);
#endif

timerHardware_t timerHardware[] = {

    DEF_TIM(TIM5, CH1, PA0, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S1  
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 1),   // S2
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 2),   // S3
    DEF_TIM(TIM5, CH4, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 3),   // S4

    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 4),   // S5
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 5),   // S6
    DEF_TIM(TIM8, CH3, PC8, TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR, 0, 6),   // S7
    DEF_TIM(TIM8, CH4, PC9, TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR, 0, 7),   // S8

    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED, 0, 9),    // LED_2812
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
