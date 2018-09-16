/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

const timerHardware_t timerHardware[] = {
    { TIM1,     IO_TAG(PA8),    TIM_Channel_1, 1 | TIMER_OUTPUT_INVERTED,                           IOCFG_AF_PP, GPIO_AF_6,     TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // PWM1
    { TIM1,     IO_TAG(PB0),    TIM_Channel_2, 1 | TIMER_OUTPUT_INVERTED | TIMER_OUTPUT_N_CHANNEL,  IOCFG_AF_PP, GPIO_AF_6,     TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // PWM2
    { TIM15,    IO_TAG(PB14),   TIM_Channel_1, 1 | TIMER_OUTPUT_INVERTED,                           IOCFG_AF_PP, GPIO_AF_1,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM3
    { TIM15,    IO_TAG(PB15),   TIM_Channel_2, 1 | TIMER_OUTPUT_INVERTED,                           IOCFG_AF_PP, GPIO_AF_1,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM4
    { TIM3,     IO_TAG(PA6),    TIM_Channel_1, 1 | TIMER_OUTPUT_INVERTED,                           IOCFG_AF_PP, GPIO_AF_2,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM5
    { TIM3,     IO_TAG(PA7),    TIM_Channel_2, 1 | TIMER_OUTPUT_INVERTED,                           IOCFG_AF_PP, GPIO_AF_2,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM6
    { TIM4,     IO_TAG(PA13),   TIM_Channel_3, 0,                                               IOCFG_AF_PP, GPIO_AF_10,    TIM_USE_PWM }, // AUX1
    { TIM2,     IO_TAG(PB3),    TIM_Channel_2, 0,                                               IOCFG_AF_PP, GPIO_AF_1,     TIM_USE_PWM }, // TX2
    { TIM8,     IO_TAG(PA15),   TIM_Channel_1, 0,                                               IOCFG_AF_PP, GPIO_AF_2,     TIM_USE_PWM }, // ROLL
    { TIM2,     IO_TAG(PA2),    TIM_Channel_3, 0,                                               IOCFG_AF_PP, GPIO_AF_1,     TIM_USE_PWM }, // PITCH
    { TIM2,     IO_TAG(PB11),   TIM_Channel_4, 0,                                               IOCFG_AF_PP, GPIO_AF_1,     TIM_USE_PWM }, // YAW 
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
