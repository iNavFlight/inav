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

BUSDEV_REGISTER_SPI_TAG(busdev_icm42688_1, DEVHW_ICM42605, ICM42688_SPI_BUS_1,  ICM42688_CS_PIN_1,  NONE,  0,  DEVFLAGS_NONE,  IMU_ICM42688_ALIGN_1);
BUSDEV_REGISTER_SPI_TAG(busdev_icm42688_2, DEVHW_ICM42605, ICM42688_SPI_BUS_2,  ICM42688_CS_PIN_2,  NONE,  0,  DEVFLAGS_NONE,  IMU_ICM42688_ALIGN_2);


timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5, CH1, PA0, TIM_USE_OUTPUT_AUTO, 0, 0),   // S1
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_OUTPUT_AUTO, 0, 1),   // S2
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_OUTPUT_AUTO, 0, 2),   // S3  
    DEF_TIM(TIM5, CH4, PA3, TIM_USE_OUTPUT_AUTO, 0, 3),   // S4

    DEF_TIM(TIM3, CH3, PB0, TIM_USE_OUTPUT_AUTO, 0, 4),   // S5
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_OUTPUT_AUTO, 0, 5),   // S6
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 6),   // S7
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 7),   // S8
    
    DEF_TIM(TIM2, CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0),   // S9
    DEF_TIM(TIM2, CH2, PB3, TIM_USE_OUTPUT_AUTO, 0, 0),   // S10 DMA_NONE
    DEF_TIM(TIM4, CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 0),   // S11
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 0),   // S12 DMA_NONE

    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED, 0, 14),    // LED_2812
    DEF_TIM(TIM12,  CH2, PB15, TIM_USE_PPM, 0, 0),  // BEEPER PWM
  
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
