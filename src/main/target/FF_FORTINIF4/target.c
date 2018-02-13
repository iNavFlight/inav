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

#define TIM_EN      TIMER_OUTPUT_ENABLED

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    { TIM3, IO_TAG(PB0), TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S1_OUT
    { TIM3, IO_TAG(PB1), TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S2_OUT
    { TIM2, IO_TAG(PA2), TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S3_OUT
    { TIM2, IO_TAG(PA3), TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S4_OUT
    { TIM4, IO_TAG(PB9), TIM_Channel_4, 0,      IOCFG_AF_PP, GPIO_AF_TIM4, TIM_USE_PPM | TIM_USE_MC_CHNFW      }, // PPM IN
    { TIM4, IO_TAG(PB7), TIM_Channel_2, 0,      IOCFG_AF_PP, GPIO_AF_TIM4, TIM_USE_LED | TIM_USE_MC_CHNFW      }, // LED
//    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD, 0 ), // S1_OUT - DMA1_ST7
//    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD, 0 ), // S2_OUT - DMA1_ST2
//    DEF_TIM(TIM2, CH4, PA3, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD, 1 ), // S3_OUT - DMA1_ST6
//    DEF_TIM(TIM2, CH3, PA2, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD, 0 ), // S4_OUT - DMA1_ST1
//    DEF_TIM(TIM4, CH4, PB9, TIM_USE_PPM,   TIMER_INPUT_ENABLED, 0 ),   // PPM IN
//    DEF_TIM(TIM4, CH2, PB7, TIM_USE_LED,   TIMER_OUTPUT_STANDARD, 0),  // LED    - DMA1_ST3
};
