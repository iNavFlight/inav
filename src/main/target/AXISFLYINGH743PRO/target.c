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

// Gyro 1 - ICM42688P on SPI1, tag 0
BUSDEV_REGISTER_SPI_TAG(busdev_gyro1_icm42605, DEVHW_ICM42605, GYRO_1_SPI_BUS, GYRO_1_CS_PIN, GYRO_1_EXTI_PIN, 0, DEVFLAGS_NONE, GYRO_1_ALIGN);

// Gyro 2 - ICM42688P on SPI4, tag 1
BUSDEV_REGISTER_SPI_TAG(busdev_gyro2_icm42605, DEVHW_ICM42605, GYRO_2_SPI_BUS, GYRO_2_CS_PIN, GYRO_2_EXTI_PIN, 1, DEVFLAGS_NONE, GYRO_2_ALIGN);

timerHardware_t timerHardware[] = {
    // Motors - DMA1 Stream0-7 (dmaopt 0-7), kept unique per channel so a
    // fallback to per-channel DMA (USE_DSHOT_DMAR disabled) still works.
    DEF_TIM(TIM8, CH1, PC6,  TIM_USE_OUTPUT_AUTO, 0, 0),   // M1
    DEF_TIM(TIM8, CH2, PC7,  TIM_USE_OUTPUT_AUTO, 0, 1),   // M2
    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 2),   // M3
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 3),   // M4
    DEF_TIM(TIM2, CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0, 4),   // M5
    DEF_TIM(TIM2, CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0, 5),   // M6
    DEF_TIM(TIM2, CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0, 6),   // M7
    DEF_TIM(TIM2, CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0, 7),   // M8

    // dmaopt 8 (DMA2 Stream0) is reserved for ADC
    DEF_TIM(TIM4, CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 9),   // S1
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 10),  // S2
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 11),  // S3
    DEF_TIM(TIM4, CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 12),  // S4 - needs USE_DSHOT_DMAR, see target.h

    // LED strip - DMA2 Stream5 (dmaopt 13)
    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_LED, 0, 13),         // LED_STRIP_PIN
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
