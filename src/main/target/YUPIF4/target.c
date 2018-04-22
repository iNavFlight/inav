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
#include "drivers/bus.h"

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    { TIM8, IO_TAG(PC8), TIM_Channel_3, 0, IOCFG_AF_PP, GPIO_AF_TIM8, TIM_USE_PPM },
    { TIM2, IO_TAG(PA0), TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },
    { TIM2, IO_TAG(PA1), TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR },
    { TIM5, IO_TAG(PA2), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },
    { TIM5, IO_TAG(PA3), TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO },
    { TIM3, IO_TAG(PB0), TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO },
    { TIM4, IO_TAG(PB7), TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM4, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO },
    { TIM3, IO_TAG(PB1), TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR | TIM_USE_MC_SERVO | TIM_USE_FW_SERVO | TIM_USE_LED },
    { TIM12, IO_TAG(PC9), TIM_Channel_4, 1, IOCFG_AF_PP, GPIO_AF_TIM4, TIM_USE_BEEPER },
    { TIM12, IO_TAG(PB14), TIM_Channel_1, 1, IOCFG_AF_PP, GPIO_AF_TIM12, TIM_USE_BEEPER },
};

/*
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_PPM,                 0, 0 ), // PPM IN
    DEF_TIM(TIM5,  CH1, PA0,  TIM_USE_MOTOR,               0, 0 ), // S1_OUT - DMA1_ST2
    DEF_TIM(TIM5,  CH2, PA1,  TIM_USE_MOTOR,               0, 0 ), // S2_OUT - DMA1_ST4
    DEF_TIM(TIM2,  CH3, PA2,  TIM_USE_MOTOR,               0, 0 ), // S3_OUT - DMA1_ST1
    DEF_TIM(TIM2,  CH4, PA3,  TIM_USE_MOTOR,               0, 1 ), // S4_OUT - DMA1_ST6
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_MOTOR | TIM_USE_LED, 0, 0 ), // S5_OUT - DMA1_ST7
    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_ANY,                 0, 0 ), // S6_OUT - DMA1_ST3 - Camera control
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_MOTOR,               0, 0 ), // S7_OUT
    DEF_TIM(TIM3,  CH4, PC9,  TIM_USE_BEEPER,              0, 0 ), // BEEPER PWM
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_BEEPER,              0, 0 ), // BEEPER PWM OPT
*/
