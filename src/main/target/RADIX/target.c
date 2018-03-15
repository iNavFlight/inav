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



const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {

    //{ TIM12, IO_TAG(PB14), TIM_Channel_1, TIM_USE_PPM,   0, GPIO_AF_TIM12, NULL,  0, 0, },    // PPM_IN
    //{ TIM5,  IO_TAG(PA2),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM5,  DMA1_Stream0, DMA_Channel_6, DMA1_ST0_HANDLER}, // S1_OUT
    //{ TIM5,  IO_TAG(PA3),  TIM_Channel_4, TIM_USE_MOTOR, 1, GPIO_AF_TIM5,  DMA1_Stream1, DMA_Channel_6, DMA1_ST1_HANDLER}, // S2_OUT
    //{ TIM1,  IO_TAG(PA10), TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM1,  DMA2_Stream6, DMA_Channel_0, DMA2_ST6_HANDLER}, // S3_OUT
    //{ TIM2,  IO_TAG(PA15), TIM_Channel_1, TIM_USE_MOTOR, 1, GPIO_AF_TIM2,  DMA1_Stream5, DMA_Channel_3, DMA1_ST5_HANDLER}, // S4_OUT
    //{ TIM8,  IO_TAG(PC8),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM8,  DMA2_Stream2, DMA_Channel_0, DMA2_ST2_HANDLER}, // S5_OUT
    //{ TIM3,  IO_TAG(PB0),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM3,  DMA1_Stream7, DMA_Channel_5, DMA1_ST7_HANDLER}, // S6_OUT
    //{ TIM11, IO_TAG(PB9),  TIM_Channel_1, TIM_USE_ANY, 1,   GPIO_AF_TIM11, NULL,         NULL,          NULL},             // Camera Control
    { TIM12, IO_TAG(PB14), TIM_Channel_1, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM12, TIM_USE_PPM }, // PPM
    { TIM5,  IO_TAG(PA2),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S1_OUT
    { TIM5,  IO_TAG(PA3),  TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S2_OUT
    { TIM1,  IO_TAG(PA10), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM1,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S3_OUT
    { TIM2,  IO_TAG(PA15), TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S4_OUT
    { TIM8,  IO_TAG(PC8),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S5_OUT
    { TIM3,  IO_TAG(PB0),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3,  TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // S6_OUT
//    { TIM11, IO_TAG(PB9),  TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM11, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR | TIM_USE_FW_MOTOR }, // CC
};

