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

const timerHardware_t timerHardware[] = {
#if defined(FF_PIKOF4OSD)
        { TIM2, IO_TAG(PA3),  TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // S1_OUT // TIM2_CH4 | TIM5_CH4 | TIM9_CH2
        { TIM3, IO_TAG(PB1),  TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // S2_OUT // TIM1_CH3N | TIM3_CH4 | TIM8_CH3N
        { TIM5, IO_TAG(PA2),  TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S3_OUT // TIM2_CH3 | TIM5_CH3 | TIM9_CH1
        { TIM3, IO_TAG(PB0),  TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S4_OUT // TIM1_CH2N | TIM3_CH3 | TIM8_CH2N
        { TIM12, IO_TAG(PB14), TIM_Channel_1, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM12, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // RC4  // TIM1_CH2N | TIM8_CH2N | TIM12_CH1
        { TIM12, IO_TAG(PB15), TIM_Channel_2, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM12, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // RC5  // TIM1_CH3N | TIM8_CH3N | TIM12_CH2
//    DEF_TIM(TIM5, CH4, PA3, TIM_USE_MOTOR, 0, 0 ), // S1_OUT - DMA1_ST7
//    DEF_TIM(TIM3, CH3, PB1, TIM_USE_MOTOR, 0, 0 ), // S2_OUT - DMA1_ST1
//    DEF_TIM(TIM5, CH3, PA2, TIM_USE_MOTOR, 0, 0 ), // S3_OUT - DMA1_ST6
//    DEF_TIM(TIM3, CH4, PB0, TIM_USE_MOTOR, 0, 0 ), // S4_OUT - DMA1_ST2
//    DEF_TIM(TIM8, CH2N, PB14, TIM_USE_MOTOR, 0, 0 ), // PA14 RC4  - DMA2_ST6, *DMA2_ST2
//    DEF_TIM(TIM1, CH3N, PB15, TIM_USE_MOTOR, 0, 0 ), // PA15 RC5  - DMA2_ST6, DMA2_ST6
#else
        { TIM2, IO_TAG(PA3), TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // S1_OUT // TIM2_CH4 | TIM5_CH4 | TIM9_CH2
        { TIM3, IO_TAG(PB0), TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S2_OUT // TIM1_CH2N | TIM3_CH3 | TIM8_CH2N
        { TIM5, IO_TAG(PA2), TIM_Channel_3, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S3_OUT // TIM2_CH3 | TIM5_CH3 | TIM9_CH1
        { TIM3, IO_TAG(PB1), TIM_Channel_4, TIM_EN, IOCFG_AF_PP, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // S4_OUT // TIM1_CH3N | TIM3_CH4 | TIM8_CH3N
//    DEF_TIM(TIM2, CH4, PA3, TIM_USE_MOTOR, 0, 1 ), // S1_OUT - DMA1_ST6
//    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MOTOR, 0, 0 ), // S2_OUT - DMA1_ST7
//    DEF_TIM(TIM2, CH3, PA2, TIM_USE_MOTOR, 0, 0 ), // S3_OUT - DMA1_ST1
//    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MOTOR, 0, 0 ), // S4_OUT - DMA1_ST2
#endif
        { TIM4, IO_TAG(PB7), TIM_Channel_2, 0,      IOCFG_AF_PP, GPIO_AF_TIM4, TIM_USE_LED | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO }, // LED
//    DEF_TIM(TIM4, CH2, PB7, TIM_USE_LED,   0, 0 ), // LED    - DMA1_ST3
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
