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
 
BUSDEV_REGISTER_SPI_TAG(busdev_1_ICM42605,   DEVHW_ICM42605,  IMU_1_SPI_BUS,  IMU_1_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_1_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_2_ICM42605,   DEVHW_ICM42605,  IMU_2_SPI_BUS,  IMU_2_CS_PIN,   NONE,   1,  DEVFLAGS_NONE,  IMU_2_ALIGN);


timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5,  CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 0),   // M1
    DEF_TIM(TIM5,  CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 1),   // M2
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 2),   // M3
    DEF_TIM(TIM5,  CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 3),   // M4

    DEF_TIM(TIM4,  CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 4),   // M5
    DEF_TIM(TIM4,  CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 5),   // M6
    DEF_TIM(TIM4,  CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 6),   // M7
    DEF_TIM(TIM4,  CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 7),   // M8

    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_OUTPUT_AUTO, 0, 7),  // S1
    DEF_TIM(TIM15, CH2, PE6,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S2 DMA None
    DEF_TIM(TIM3,  CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 10),  // S3
    DEF_TIM(TIM3,  CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 11),  // S4

	DEF_TIM(TIM1, CH1, PE9, TIM_USE_LED, 0, 9), // LED
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
