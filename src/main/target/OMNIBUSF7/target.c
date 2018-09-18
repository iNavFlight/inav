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
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000,     DEVHW_MPU6000,      MPU6000_SPI_BUS,    MPU6000_CS_PIN,     MPU6000_EXTI_PIN,       0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_SPI_BUS,    MPU6500_CS_PIN,     MPU6500_EXTI_PIN,       1,  DEVFLAGS_NONE);

BUSDEV_REGISTER_SPI(    busdev_bmp280,      DEVHW_BMP280,       BMP280_SPI_BUS,     BMP280_CS_PIN,      NONE,                       DEVFLAGS_NONE);

BUSDEV_REGISTER_I2C(    busdev_hmc5883,     DEVHW_HMC5883,      MAG_I2C_BUS,        0x1E,               NONE,                       DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C(    busdev_qmc5883,     DEVHW_QMC5883,      MAG_I2C_BUS,        0x0D,               NONE,                       DEVFLAGS_NONE);
BUSDEV_REGISTER_I2C(    busdev_mag3110,     DEVHW_MAG3110,      MAG_I2C_BUS,        0x0E,               NONE,                       DEVFLAGS_NONE);

BUSDEV_REGISTER_SPI(    busdev_max7456,     DEVHW_MAX7456,      MAX7456_SPI_BUS,    MAX7456_CS_PIN,     NONE,                       DEVFLAGS_USE_RAW_REGISTERS);

const timerHardware_t timerHardware[] = {

    DEF_TIM(TIM2, CH4, PA3,  TIM_USE_PPM,           0, 0 ), // UART2_RX, joined with PE13
    // DEF_TIM(TIM1, CH3, PE13, TIM_USE_NONE,          0, 0 ), // RC1 / PPM, unusable

    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_MC_MOTOR    | TIM_USE_FW_MOTOR,         0, 0 ), // M1
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_MC_MOTOR    | TIM_USE_FW_MOTOR,         0, 0 ), // M2
    DEF_TIM(TIM1, CH1, PE9,  TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO,         0, 2 ), // M3
    DEF_TIM(TIM1, CH2, PE11, TIM_USE_MC_MOTOR    | TIM_USE_FW_SERVO,         0, 1 ), // M4

    DEF_TIM(TIM4, CH1, PD12, TIM_USE_LED,           0, 0 ), // LED
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
