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

 #include <platform.h>
 #include "drivers/io.h"
 #include "drivers/pwm_mapping.h"
 #include "drivers/timer.h"
 #include "drivers/bus.h"
 #include "drivers/sensor.h"
 
 #include "drivers/pwm_output.h"
 #include "common/maths.h"
 #include "fc/config.h"
 // Board hardware definitions - IMU1 slot
 
BUSDEV_REGISTER_SPI_TAG(busdev_1_mpu6000,   DEVHW_MPU6000,  IMU_1_SPI_BUS,  IMU_1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_1_mpu6500,   DEVHW_MPU6500,  IMU_1_SPI_BUS,  IMU_1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_1_ICM42605,   DEVHW_ICM42605,  IMU_1_SPI_BUS,  IMU_1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_1_BMI270,   DEVHW_BMI270,  IMU_1_SPI_BUS,  IMU_1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ALIGN);

// // Board hardware definitions - IMU2 slot
BUSDEV_REGISTER_SPI_TAG(busdev_2_mpu6000,   DEVHW_MPU6000,  IMU_2_SPI_BUS,  IMU_2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_2_mpu6500,   DEVHW_MPU6500,  IMU_2_SPI_BUS,  IMU_2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_2_ICM42605,   DEVHW_ICM42605,  IMU_2_SPI_BUS,  IMU_2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_2_BMI270,   DEVHW_BMI270,  IMU_2_SPI_BUS,  IMU_2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ALIGN);

/*
From FLYWOOH743PRO
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),   // S1
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 1),   // S2
    DEF_TIM(TIM5, CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 2),   // S3  
    DEF_TIM(TIM5, CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 3),   // S4
    DEF_TIM(TIM5, CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 4),   // S5
    DEF_TIM(TIM5, CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 5),   // S6
    DEF_TIM(TIM4, CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 6),   // S7
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 7),   // S8
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 0),   // S9
    DEF_TIM(TIM4, CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 0),   // S10
    DEF_TIM(TIM15, CH1, PE5, TIM_USE_OUTPUT_AUTO, 0, 0),   // S11
    DEF_TIM(TIM15, CH2, PE6, TIM_USE_OUTPUT_AUTO, 0, 0),   // S12
    DEF_TIM(TIM1,  CH1, PA8, TIM_USE_LED, 0, 9),    // LED_2812
*/

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM2,  CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 0),   // M1 y  TIM5, CH1
    DEF_TIM(TIM2,  CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 1),   // M2 y  TIM5, CH2
    DEF_TIM(TIM2,  CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 2),   // M3 y  TIM5, CH3
    DEF_TIM(TIM2,  CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 3),   // M4 y  TIM5, CH4
	/*
    DEF_TIM(TIM5,  CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 0),   // M1 y  TIM5, CH1
    DEF_TIM(TIM5,  CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 1),   // M2 y  TIM5, CH2
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 2),   // M3 y  TIM5, CH3
    DEF_TIM(TIM5,  CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 3),   // M4 y  TIM5, CH4
    */

    DEF_TIM(TIM4,  CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 4),   // M5 y  same TIM4
    DEF_TIM(TIM4,  CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 5),   // M6 y  same TIM4
    DEF_TIM(TIM4,  CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 6),   // M7 y  same TIM4
    DEF_TIM(TIM4,  CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 0),   // M8 DMA None y Same TIM4
	/*
    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_OUTPUT_AUTO, 0, 7),  // S1 y  Same TIM15
    DEF_TIM(TIM15, CH2, PE6,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S2 DMA None y Same TIM15
    DEF_TIM(TIM3,  CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 10),  // S3 n
    DEF_TIM(TIM3,  CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 11),  // S4 n
    DEF_TIM(TIM16, CH1, PB8,  TIM_USE_ANY, 0, 0),  // CAMERA_CONTROL_PIN // n Does this need a timer ?
    DEF_TIM(TIM17, CH1, PB9,  TIM_USE_ANY, 0, 0),  // GYRO_1_CLKIN_PIN // n Does this need a timer?
	*/
    DEF_TIM(TIM1,  CH1, PE9,  TIM_USE_LED, 0, 8),    // LED_2812 // n
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
