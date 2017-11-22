/*
* This file is part of Cleanflight.
*
* Cleanflight is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Cleanflight is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

BUSDEV_REGISTER_SPI_TAG(busdev_mpu9250,     DEVHW_MPU9250,      MPU9250_SPI_BUS,    MPU9250_CS_PIN,     MPU9250_EXTI_PIN,       0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_SPI_TAG(busdev_icm20608,    DEVHW_MPU6500,      ICM20608_SPI_BUS,   ICM20608_CS_PIN,    ICM20608_EXTI_PIN,      1,  DEVFLAGS_NONE);

BUSDEV_REGISTER_SPI_TAG(busdev_ms5611,      DEVHW_MS5611,       MS5611_SPI_BUS,     MS5611_CS_PIN,      NONE,                   0,   DEVFLAGS_USE_RAW_REGISTERS);

BUSDEV_REGISTER_I2C_TAG(busdev_hmc5883,     DEVHW_HMC5883,      MAG_I2C_BUS,        0x1E,               NONE,                   0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C_TAG(busdev_qmc5883,     DEVHW_QMC5883,      MAG_I2C_BUS,        0x0D,               NONE,                   0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C_TAG(busdev_mag3110,     DEVHW_MAG3110,      MAG_I2C_BUS,        0x0E,               NONE,                   0,  DEVFLAGS_NONE);

// PixRacer has built-in HMC5983 compass on the same SPI bus as MPU9250
BUSDEV_REGISTER_SPI_TAG(busdev_hmc5983_spi, DEVHW_HMC5883,      MPU9250_SPI_BUS,    PE15,               NONE,                   1,  DEVFLAGS_NONE);

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    { TIM3, IO_TAG(PB0),  TIM_Channel_3, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_PPM },  // PPM shared uart6 pc7
    { TIM1, IO_TAG(PE14), TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  // S1_OUT
    { TIM1, IO_TAG(PE13), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  // S2_OUT
    { TIM1, IO_TAG(PE11), TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  // S3_OUT
    { TIM1, IO_TAG(PE9),  TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  // S4_OUT
    { TIM4, IO_TAG(PD13), TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  // S5_OUT
    { TIM4, IO_TAG(PD14), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  // S6_OUT
};
