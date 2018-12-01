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
#include <stdbool.h>
 #include <platform.h>
 #include "drivers/io.h"
 #include "drivers/pwm_mapping.h"
 #include "drivers/timer.h"

 const timerHardware_t timerHardware[] = {
    { TIM11, IO_TAG(PB9),      TIM_Channel_1, 0,  IOCFG_AF_PP, GPIO_AF_TIM11,    TIM_USE_PPM | TIM_USE_PWM },            // PPM
    { TIM4,  IO_TAG(PB8),      TIM_Channel_3, 1,  IOCFG_AF_PP, GPIO_AF_TIM4,     TIM_USE_LED },                          // LED_STRIP

#if defined(CLRACINGF4AIRV2) || defined(CLRACINGF4AIRV3)
    { TIM3,  IO_TAG(PB0),    TIM_Channel_3, 1,  IOCFG_AF_PP, GPIO_AF_TIM3,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },        // MOTOR 3 ELE
    { TIM3,  IO_TAG(PB1),    TIM_Channel_4, 1,  IOCFG_AF_PP, GPIO_AF_TIM3,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },        // MOTOR 4 ROLL 1
    { TIM12, IO_TAG(PB15),   TIM_Channel_2, 1,  IOCFG_AF_PP, GPIO_AF_TIM12,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },     // MOTOR 5 ROLL 2
    { TIM8,  IO_TAG(PC8),    TIM_Channel_3, 1,  IOCFG_AF_PP, GPIO_AF_TIM8,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },      // MOTOR 1 ESC
    { TIM8,  IO_TAG(PC9),    TIM_Channel_4, 1,  IOCFG_AF_PP, GPIO_AF_TIM8,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },      // MOTOR 2 ESC
    { TIM1,  IO_TAG(PA8),    TIM_Channel_1, 1,  IOCFG_AF_PP, GPIO_AF_TIM1,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },       // MOTOR  6 YAW
#else
    { TIM3,  IO_TAG(PB0),    TIM_Channel_3, 1,  IOCFG_AF_PP, GPIO_AF_TIM3,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },      // MOTOR_1
    { TIM3,  IO_TAG(PB1),    TIM_Channel_4, 1,  IOCFG_AF_PP, GPIO_AF_TIM3,   TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },      // MOTOR_2
    { TIM9,  IO_TAG(PA3),    TIM_Channel_2, 1,  IOCFG_AF_PP, GPIO_AF_TIM9,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },       // SERVO_1
    { TIM2,  IO_TAG(PA2),    TIM_Channel_3, 1,  IOCFG_AF_PP, GPIO_AF_TIM2,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },        // SERVO_2
    { TIM12, IO_TAG(PB15),   TIM_Channel_2, 1,  IOCFG_AF_PP, GPIO_AF_TIM12,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },   // SERVO_3
    { TIM1,  IO_TAG(PA8),    TIM_Channel_1, 1,  IOCFG_AF_PP, GPIO_AF_TIM1,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },        // SERVO_4
#endif
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
