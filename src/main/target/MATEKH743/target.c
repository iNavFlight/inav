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

BUSDEV_REGISTER_SPI_TAG(busdev_imu0,   DEVHW_MPU6000,  MPU6000_SPI_BUS,   MPU6000_CS_PIN,   MPU6000_EXTI_PIN,   0,  DEVFLAGS_NONE,  IMU_MPU6000_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_imu1,   DEVHW_MPU6500,  MPU6500_SPI_BUS,   MPU6500_CS_PIN,   MPU6500_EXTI_PIN,   1,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_imu2,   DEVHW_ICM42605, ICM42605_SPI_BUS,  ICM42605_CS_PIN,  ICM42605_EXTI_PIN,  2,  DEVFLAGS_NONE,  IMU_ICM42605_ALIGN);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),   // S1
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 1),   // S2

    DEF_TIM(TIM5, CH1, PA0, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 2),   // S3  
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 3),   // S4
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 4),   // S5
    DEF_TIM(TIM5, CH4, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 5),   // S6

    DEF_TIM(TIM4, CH1, PD12, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 6),   // S7
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 7),   // S8
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S9
    DEF_TIM(TIM4, CH4, PD15, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S10 DMA_NONE

    DEF_TIM(TIM15, CH1, PE5, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 0),   // S11
    DEF_TIM(TIM15, CH2, PE6, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 0),   // S12 DMA_NONE

    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED, 0, 8),    // LED_2812
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_BEEPER, 0, 0),  // BEEPER PWM

    DEF_TIM(TIM8,  CH2, PC7,  TIM_USE_PPM, 0, 0),   // RX6 PPM
    DEF_TIM(TIM8,  CH1, PC6,  TIM_USE_ANY, 0, 0),   // TX6    
    DEF_TIM(TIM16, CH1, PB8,  TIM_USE_ANY, 0, 0),   // RX4
    DEF_TIM(TIM17, CH1, PB9,  TIM_USE_ANY, 0, 0),   // TX4
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
