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
#include "drivers/sensor.h"
#include "drivers/timer.h"

// BMI088 accelerometer and gyro must use the same selection tag.
BUSDEV_REGISTER_SPI_TAG(busdev_bmi088_gyro, DEVHW_BMI088_GYRO, BMI088_SPI_BUS, BMI088_GYRO_CS_PIN, BMI088_GYRO_EXTI_PIN, 0, DEVFLAGS_NONE, IMU_BMI088_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_bmi088_acc, DEVHW_BMI088_ACC, BMI088_SPI_BUS, BMI088_ACC_CS_PIN, BMI088_ACC_EXTI_PIN, 0, DEVFLAGS_NONE, IMU_BMI088_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_icm45686, DEVHW_ICM45686, ICM45686_SPI_BUS, ICM45686_CS_PIN, ICM45686_EXTI_PIN, 1, DEVFLAGS_NONE, IMU_ICM45686_ALIGN);

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1,  CH4, PE14, TIM_USE_OUTPUT_AUTO, 0, 0), // M1
    DEF_TIM(TIM1,  CH3, PE13, TIM_USE_OUTPUT_AUTO, 0, 1), // M2
    DEF_TIM(TIM1,  CH2, PE11, TIM_USE_OUTPUT_AUTO, 0, 2), // M3
    DEF_TIM(TIM1,  CH1, PE9,  TIM_USE_OUTPUT_AUTO, 0, 3), // M4
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 4), // M5
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 5), // M6
    DEF_TIM(TIM4,  CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 6), // M7
    DEF_TIM(TIM4,  CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 7), // M8
    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_OUTPUT_AUTO, 0, 13), // M9: DMA2 Stream5, independent of M1
    DEF_TIM(TIM15, CH2, PE6,  TIM_USE_OUTPUT_AUTO, 0, 0), // M10: PWM only (no channel DMA)

    DEF_TIM(TIM4,  CH3, PD14, TIM_USE_LED,         0, 9), // LED strip, or an extra output when LED_STRIP is disabled
    // PD15 remains a GPIO beeper and must not be claimed as a TIM4 output.
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
