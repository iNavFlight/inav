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
#include "drivers/pwm_mapping.h"

#define DEF_TIM_CHNL_CH1    TIM_Channel_1
#define DEF_TIM_CHNL_CH2    TIM_Channel_2
#define DEF_TIM_CHNL_CH3    TIM_Channel_3
#define DEF_TIM_CHNL_CH4    TIM_Channel_4

#define DEF_TIM(_tim, _ch, _pin, _usage, _flags) \
    { _tim, IO_TAG(_pin), DEF_TIM_CHNL_##_ch, _flags, IOCFG_AF_PP, GPIO_AF_##_tim, _usage }

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    // DEF_TIM(TIM4, CH3, PB8, TIM_USE_PWM | TIM_USE_PPM, 0), // PPM
    // DEF_TIM(TIM4, CH4, PB9, TIM_USE_PWM,               0), // S2_IN
    // DEF_TIM(TIM8, CH1, PC6, TIM_USE_PWM,               0), // S3_IN, UART6_TX
    // DEF_TIM(TIM8, CH2, PC7, TIM_USE_PWM,               0), // S4_IN, UART6_RX
    // DEF_TIM(TIM8, CH3, PC8, TIM_USE_PWM,               0), // S5_IN
    // DEF_TIM(TIM8, CH4, PC9, TIM_USE_PWM,               0), // S6_IN

    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,    1), // S1_OUT
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,    1), // S2_OUT
    DEF_TIM(TIM2, CH4, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1), // S3_OUT
    DEF_TIM(TIM2, CH3, PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1), // S4_OUT
    DEF_TIM(TIM5, CH2, PA1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1), // S5_OUT
    DEF_TIM(TIM1, CH1, PA8, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1), // S6_OUT

    DEF_TIM(TIM4, CH1, PB6, TIM_USE_LED,                            0), // LED strip
};
