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

const timerHardware_t timerHardware[] = {
    // up to 10 Motor Outputs
    DEF_TIM(TIM1,  CH3N, PB15, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,               0), // PWM1  - PB15 - NONE
    DEF_TIM(TIM15, CH1,  PB14, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,               0), // PWM2  - PB14 - DMA1_CH5
    DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM3  - PA8  - DMA1_CH2
    DEF_TIM(TIM8,  CH2N, PB0,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM4  - PB0  - DMA2_CH5
    DEF_TIM(TIM16, CH1,  PA6,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM5  - PA6  - DMA1_CH3
    DEF_TIM(TIM2,  CH3,  PA2,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM6  - PA2  - DMA1_CH1
    DEF_TIM(TIM8,  CH3N, PB1,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM7  - PB1  - DMA2_CH1
    DEF_TIM(TIM17, CH1,  PA7,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO | TIM_USE_ANY, 0), // PWM8  - PA7  - DMA1_CH7
    DEF_TIM(TIM3,  CH2,  PA4,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM9  - PA4  - DMA_NONE
    DEF_TIM(TIM2,  CH2,  PA1,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,               0), // PWM10 - PA1  - DMA1_CH7

    DEF_TIM(TIM2,  CH4,  PA3,  TIM_USE_PPM,                                       0), // PPM   - PA3  - DMA1_CH7
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
