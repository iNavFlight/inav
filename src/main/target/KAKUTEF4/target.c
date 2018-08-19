/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/timer.h"

const timerHardware_t timerHardware[] = {
    { TIM8,  IO_TAG(PC7),  TIM_Channel_2, 0, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_PPM }, // PPM

    { TIM3,  IO_TAG(PB0),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR                    | TIM_USE_FW_MOTOR }, // MOTOR_1
    { TIM3,  IO_TAG(PB1),  TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM3, TIM_USE_MC_MOTOR                    | TIM_USE_FW_MOTOR }, // MOTOR_2
    { TIM2,  IO_TAG(PA3),  TIM_Channel_4, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_3
    { TIM2,  IO_TAG(PA2),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM2, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_4

    #if defined(KAKUTEF4V2)
    { TIM8,  IO_TAG(PC8),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_LED }
    #else
    { TIM5,  IO_TAG(PA0),  TIM_Channel_1, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_5
    { TIM8,  IO_TAG(PC8),  TIM_Channel_3, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM8, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_6
    { TIM5,  IO_TAG(PA1),  TIM_Channel_2, 1, IOCFG_AF_PP_PD, GPIO_AF_TIM5, TIM_USE_LED }
    #endif
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
