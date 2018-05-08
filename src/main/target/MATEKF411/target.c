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
    { TIM3, IO_TAG(PB4),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  //S1
    { TIM3, IO_TAG(PB5),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  //S2
    { TIM4, IO_TAG(PB6),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  //S3
    { TIM4, IO_TAG(PB7),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },  //S4
    { TIM2, IO_TAG(PB3),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO },  //S5
    { TIM2, IO_TAG(PB10),   TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO },  //S6
    { TIM2, IO_TAG(PA15),   TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_CHNFW | TIM_USE_FW_SERVO },  //S7

    { TIM1, IO_TAG(PA8),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_LED},   //2812LED
    { TIM5, IO_TAG(PA0),    TIM_Channel_1, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_PPM },  //use rssi pad for PPM/softserial_tx1
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
