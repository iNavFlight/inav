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

BUSDEV_REGISTER_SPI_TAG(busdev_icm42688,  DEVHW_ICM42605,  IMU1_SPI_BUS,   IMU1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_icm42688_2, DEVHW_ICM42605, IMU2_SPI_BUS,   IMU2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU2_ALIGN);

BUSDEV_REGISTER_SPI(busdev_max7456,    DEVHW_MAX7456,  MAX7456_SPI_BUS,    MAX7456_CS_PIN, NONE, DEVFLAGS_USE_RAW_REGISTERS,  0);

BUSDEV_REGISTER_I2C(busdev_dps310_0,    DEVHW_DPS310,     BARO_I2C_BUS,     0x76,               NONE,           DEVFLAGS_NONE, 0);
BUSDEV_REGISTER_I2C(busdev_dps310_1,    DEVHW_DPS310,     BARO_I2C_BUS,     0x77,               NONE,           DEVFLAGS_NONE, 0);
    
BUSDEV_REGISTER_I2C(busdev_lis2mdl,     DEVHW_LIS2MDL,      MAG_I2C_BUS,    0x1E,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_qmc5883,     DEVHW_QMC5883,      MAG_I2C_BUS,    0x0D,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_hmc5883,     DEVHW_HMC5883,      MAG_I2C_BUS,    0x1E,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_qmc5883p,     DEVHW_QMC5883P,      MAG_I2C_BUS,    0x2C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_ak8963,      DEVHW_AK8963,       MAG_I2C_BUS,     0x0C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_ak8975,      DEVHW_AK8975,       MAG_I2C_BUS,     0x0C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_mag3110,     DEVHW_MAG3110,      MAG_I2C_BUS,    0x0E,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_lis3mdl,     DEVHW_LIS3MDL,      MAG_I2C_BUS,    0x1E,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_ist8310_0,   DEVHW_IST8310_0,    MAG_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_ist8310_1,   DEVHW_IST8310_1,    MAG_I2C_BUS,    0x0E,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_ist8308,     DEVHW_IST8308,      MAG_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_vcm5883,     DEVHW_VCM5883,      MAG_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
BUSDEV_REGISTER_I2C(busdev_mlx90393,     DEVHW_MLX90393,      MAG_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);


BUSDEV_REGISTER_I2C(busdev_ms4525,      DEVHW_MS4525,       PITOT_I2C_BUS,     0x28,               NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);


timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0),    // S2
    DEF_TIM(TIM2, CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0),    // S1
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),    // S4
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),    // S3
    DEF_TIM(TIM5, CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 5),    // S7  
    DEF_TIM(TIM5, CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 6),    // S8 
    DEF_TIM(TIM4, CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 0),    // S5
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 4),    // S6
    DEF_TIM(TIM15,CH2, PE6,  TIM_USE_OUTPUT_AUTO, 0, 0),    // S10
    DEF_TIM(TIM15,CH1, PE5,  TIM_USE_OUTPUT_AUTO, 0, 7),    // S9

    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0),    // M1 
    DEF_TIM(TIM1, CH4, PE14, TIM_USE_OUTPUT_AUTO, 0, 3),    // M2 
    DEF_TIM(TIM1, CH2, PE11, TIM_USE_OUTPUT_AUTO, 0, 1),    // M3
    DEF_TIM(TIM1, CH3, PE13, TIM_USE_OUTPUT_AUTO, 0, 2),    // M4   
    
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0),    // PWM
    DEF_TIM(TIM17, CH1, PB9, TIM_USE_BEEPER,      0, 0)     // BEEPER
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
