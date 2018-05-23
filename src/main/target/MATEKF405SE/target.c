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

const timerHardware_t timerHardware[] = {
    { TIM4, IO_TAG(PB7),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR },  //S1
    { TIM4, IO_TAG(PB6),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4,  TIM_USE_MC_MOTOR  | TIM_USE_FW_MOTOR },  //S2
    { TIM3, IO_TAG(PB0),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO },  //S3
    { TIM3, IO_TAG(PB1),    TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO },  //S4
    { TIM8, IO_TAG(PC8),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO },  //S5
    { TIM8, IO_TAG(PC9),    TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8,  TIM_USE_MC_MOTOR  | TIM_USE_FW_SERVO },  //S6
    { TIM12, IO_TAG(PB14),  TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM12, TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO },  //S7
    { TIM12, IO_TAG(PB15),  TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM12, TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO },  //S8
    { TIM1, IO_TAG(PA8),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1,  TIM_USE_MC_SERVO  | TIM_USE_FW_SERVO },  //S9

    { TIM2, IO_TAG(PA15),   TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_LED },  //2812LED
    { TIM9, IO_TAG(PA3),    TIM_Channel_2, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM9, TIM_USE_PPM },  //RX2

    { TIM5, IO_TAG(PA2),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_PWM },  //TX2  softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
