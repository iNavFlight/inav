/*
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

/* GYRO */
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_SPI_BUS,    MPU6500_CS_PIN,     MPU6500_EXTI_PIN,       0,  DEVFLAGS_NONE);

BUSDEV_REGISTER_I2C(    busdev_bmp280,      DEVHW_BMP280,       BARO_I2C_BUS,       0x76,               NONE,                       DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C(    busdev_hmc5883,     DEVHW_HMC5883,      MAG_I2C_BUS,        0x1E,               NONE,                       DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C(    busdev_qmc5883,     DEVHW_QMC5883,      MAG_I2C_BUS,        0x0D,               NONE,                       DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C(    busdev_mag3110,     DEVHW_MAG3110,      MAG_I2C_BUS,        0x0E,               NONE,                       DEVFLAGS_NONE);

BUSDEV_REGISTER_SPI(    busdev_max7456,     DEVHW_MAX7456,      MAX7456_SPI_BUS,    MAX7456_CS_PIN,     NONE,                       DEVFLAGS_USE_RAW_REGISTERS);

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {

    { TIM3,  IO_TAG(PB0),  TIM_CHANNEL_3, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM3, TIM_USE_MC_MOTOR    | TIM_USE_FW_MOTOR  }, //Motor 1
    { TIM3,  IO_TAG(PB1),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM3, TIM_USE_MC_MOTOR    | TIM_USE_FW_MOTOR  }, //Motor 2
    { TIM1,  IO_TAG(PE9),  TIM_CHANNEL_1, 1, IOCFG_AF_PP_PD, GPIO_AF1_TIM1, TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO  }, //Motor 3
    { TIM1,  IO_TAG(PE11), TIM_CHANNEL_2, 1, IOCFG_AF_PP_PD, GPIO_AF1_TIM1, TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO  }, //Motor 4
    { TIM8,  IO_TAG(PC9),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF3_TIM8, TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO  }, //Motor 5
    { TIM5,  IO_TAG(PA3),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM5, TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO  }, //Motor 6
    { TIM4,  IO_TAG(PD12), TIM_CHANNEL_1, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM4, TIM_USE_LED}, //Motor 6
};
