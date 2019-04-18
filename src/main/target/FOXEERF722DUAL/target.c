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

#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"

BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000,     DEVHW_MPU6000,      MPU6000_SPI_BUS,    MPU6000_CS_PIN,     MPU6000_EXTI_PIN,       0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_SPI_BUS,    MPU6500_CS_PIN,     MPU6500_EXTI_PIN,       1,  DEVFLAGS_NONE);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_PPM,                          0, 0),  // PPM&SBUS  

    DEF_TIM(TIM1,  CH2, PA9,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR, 0, 1),  // S1 - D(2, 1, 6)
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR, 0, 0),  // S2 - D(2, 6, 0)
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO, 0, 0),  // S3 - D(2, 7, 7)
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO, 0, 0),  // S4 - D(2, 4, 7)
    DEF_TIM(TIM3,  CH1, PC6,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO, 0, 0),  // S5 - D(1, 4, 5)
    DEF_TIM(TIM3,  CH2, PC7,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO, 0, 0),  // S6 - D(1, 5, 5)

    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_LED,                          0, 0),  // LED STRIP(1,5)

    //DEF_TIM(TIM2, CH2, PB3,  TIM_USE_CAMERA_CONTROL,                   0, 0),   // Reserved (Cam Control for BF)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
