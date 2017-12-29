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

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    { TIM9, IO_TAG(PA3),    TIM_Channel_2, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM9, TIM_USE_PPM },

    { TIM3, IO_TAG(PC6),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR },  //S1 DMA1_ST4 MT1
    { TIM8, IO_TAG(PC7),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO },  //S2 DMA2_ST3 SV3
    { TIM8, IO_TAG(PC8),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO },  //S3 DMA2_ST4 SV4
    { TIM8, IO_TAG(PC9),    TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO },  //S4 DMA3_ST7 SV5

    { TIM2, IO_TAG(PA15),   TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_LED },  //S5 DMA1_ST5 2812LED

    { TIM1, IO_TAG(PA8),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1, TIM_USE_MC_MOTOR |                    TIM_USE_FW_SERVO },  //S6 DMA2_ST6 SV6
    { TIM4, IO_TAG(PB8),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_CHNFW |                    TIM_USE_FW_MOTOR },  //S7 DMA1_ST7 MT2

    { TIM5, IO_TAG(PA2),    TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_PWM },  //TX2

    //{ TIM5, IO_TAG(PA0),    TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_ANY },  //TX4 softserial1_tx
    //{ TIM5, IO_TAG(PA1),    TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_ANY },  //RX4 softserial1_rx

};
