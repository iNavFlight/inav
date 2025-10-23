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
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

BUSDEV_REGISTER_SPI_TAG(busdev_icm42688,  DEVHW_ICM42605, ICM42605_SPI_BUS,  ICM42605_CS_PIN,  NONE,  0,  DEVFLAGS_NONE,  IMU_ICM42605_ALIGN);


timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4,   CH2, PB7,  TIM_USE_OUTPUT_AUTO,   1, 0), // S1 D(1,3,2)
    DEF_TIM(TIM4,   CH1, PB6,  TIM_USE_OUTPUT_AUTO,   1, 0), // S2 D(1,0,2)

    DEF_TIM(TIM3,   CH3, PB0,  TIM_USE_OUTPUT_AUTO,   1, 0), // S3 D(1,7,5)
    DEF_TIM(TIM3,   CH4, PB1,  TIM_USE_OUTPUT_AUTO,   1, 0), // S4 D(1,2,5)
    DEF_TIM(TIM8,   CH3, PC8,  TIM_USE_OUTPUT_AUTO,   1, 0), // S5 D(2,4,7)
    DEF_TIM(TIM8,   CH4, PC9,  TIM_USE_OUTPUT_AUTO,   1, 0), // S6 D(2,7,7)
    
    DEF_TIM(TIM8,  CH2N, PB14, TIM_USE_OUTPUT_AUTO,   1, 0), // S7
    DEF_TIM(TIM2,   CH1, PA15, TIM_USE_OUTPUT_AUTO,   1, 0), // S8
    DEF_TIM(TIM2,   CH3, PB10, TIM_USE_OUTPUT_AUTO,   1, 0), // S9
    DEF_TIM(TIM2,   CH4, PB11, TIM_USE_OUTPUT_AUTO,   1, 0), // S10
    DEF_TIM(TIM12,  CH2, PB15, TIM_USE_OUTPUT_AUTO,   1, 0), // S11 

    DEF_TIM(TIM1,   CH1, PA8,  TIM_USE_LED,   0, 0), //2812LED  D(1,5,3)
    DEF_TIM(TIM5,   CH3, PA2,  TIM_USE_ANY,   0, 0), //TX2  softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
