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
#include "drivers/timer.h"
#include "drivers/dma.h"

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    { TIM8, IO_TAG(PB9),  TIM_Channel_3, 1, IOCFG_AF_PP, GPIO_AF_10, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  // M1 / PWM1
#ifdef FRSKYF3_M2_ON_PWM5
    { TIM2, IO_TAG(PA2),  TIM_Channel_3, 1, IOCFG_AF_PP,  GPIO_AF_1, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  // PWM5 / USART2_TX (AF7)
#else
    { TIM2, IO_TAG(PA2),  TIM_Channel_3, 1, IOCFG_AF_PP,  GPIO_AF_1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // PWM5 / USART2_TX (AF7)
#endif
    { TIM8, IO_TAG(PB8),  TIM_Channel_2, 1, IOCFG_AF_PP, GPIO_AF_10, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },  // M2 / PWM2
    { TIM2, IO_TAG(PA3),  TIM_Channel_4, 1, IOCFG_AF_PP,  GPIO_AF_1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // PWM6 / USART2_RX (AF7)
    { TIM2, IO_TAG(PA0),  TIM_Channel_1, 1, IOCFG_AF_PP,  GPIO_AF_1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // PWM7
    { TIM3, IO_TAG(PB0),  TIM_Channel_3, 1, IOCFG_AF_PP,  GPIO_AF_2, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // PWM8
    { TIM3, IO_TAG(PB1),  TIM_Channel_4, 1, IOCFG_AF_PP,  GPIO_AF_2, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // M3 / PWM3
    { TIM2, IO_TAG(PA1),  TIM_Channel_2, 1, IOCFG_AF_PP,  GPIO_AF_1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW },  // M4 / PWM4

    { TIM1, IO_TAG(PA8),  TIM_Channel_1, 1, IOCFG_AF_PP, GPIO_AF_6, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO | TIM_USE_MC_CHNFW | TIM_USE_LED },
};
