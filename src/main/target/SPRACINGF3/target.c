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
#include "drivers/timer_def.h"
#include "drivers/bus.h"

const timerHardware_t timerHardware[] = {    
    DEF_TIM(TIM16,  CH1,    PA6,    TIM_USE_MC_MOTOR |                    TIM_USE_FW_MOTOR,     0),  // PWM1
    DEF_TIM(TIM17,  CH1,    PA7,    TIM_USE_MC_MOTOR |                    TIM_USE_FW_MOTOR,     0), // PWM2
    DEF_TIM(TIM4,   CH1,    PA11,   TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,     0), // PWM3
    DEF_TIM(TIM4,   CH2,    PA12,   TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,     0), // PWM4
    DEF_TIM(TIM4,   CH3,    PB8,    TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,     0), // PWM5
    DEF_TIM(TIM4,   CH4,    PB9,    TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO,     0), // PWM6
    DEF_TIM(TIM15,  CH1,    PA2,    TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,     0), // PWM7
    DEF_TIM(TIM15,  CH2,    PA3,    TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,     0), // PWM8

    DEF_TIM(TIM1,   CH1,    PA8,    TIM_USE_LED,                                                0), // GPIO_TIMER / LED_STRIP

    DEF_TIM(TIM2,   CH1,    PA0,    TIM_USE_PWM | TIM_USE_PPM,                                  0), // RC_CH1
    DEF_TIM(TIM2,   CH2,    PA1,    TIM_USE_PWM,                                                0), // RC_CH2
    DEF_TIM(TIM2,   CH4,    PB11,   TIM_USE_PWM,                                                0), // RC_CH3
    DEF_TIM(TIM2,   CH3,    PB10,   TIM_USE_PWM,                                                0), // RC_CH4
    DEF_TIM(TIM3,   CH1,    PB4,    TIM_USE_PWM | TIM_USE_MC_SERVO,                             0), // RC_CH5
    DEF_TIM(TIM3,   CH2,    PB5,    TIM_USE_PWM | TIM_USE_MC_SERVO,                             0), // RC_CH6
    DEF_TIM(TIM3,   CH3,    PB0,    TIM_USE_PWM | TIM_USE_MC_SERVO,                             0), // RC_CH7
    DEF_TIM(TIM3,   CH4,    PB1,    TIM_USE_PWM | TIM_USE_MC_SERVO,                             0), // RC_CH8
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
