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

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

BUSDEV_REGISTER_SPI_TAG(busdev_icm20689,     DEVHW_ICM20689,      ICM20689_SPI_BUS,    ICM20689_CS_PIN,     ICM20689_EXTI_PIN,       0,  DEVFLAGS_NONE);

const timerHardware_t timerHardware[] = {
    { TIM1,  IO_TAG(PE13), TIM_CHANNEL_3, 0, IOCFG_AF_PP_PD, GPIO_AF1_TIM1, TIM_USE_PPM }, // PPM

    { TIM3,  IO_TAG(PB0),  TIM_CHANNEL_3, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM3, TIM_USE_MC_MOTOR                    | TIM_USE_FW_MOTOR }, // MOTOR_1
    { TIM3,  IO_TAG(PB1),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM3, TIM_USE_MC_MOTOR                    | TIM_USE_FW_MOTOR }, // MOTOR_2
    { TIM1,  IO_TAG(PE9),  TIM_CHANNEL_1, 1, IOCFG_AF_PP_PD, GPIO_AF1_TIM1, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_3
    { TIM1,  IO_TAG(PE11), TIM_CHANNEL_2, 1, IOCFG_AF_PP_PD, GPIO_AF1_TIM1, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_4
    { TIM8,  IO_TAG(PC9),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF3_TIM8, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_5
    { TIM5,  IO_TAG(PA3),  TIM_CHANNEL_4, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM5, TIM_USE_MC_MOTOR                    | TIM_USE_FW_SERVO }, // MOTOR_6

    { TIM4,  IO_TAG(PD12), TIM_CHANNEL_1, 1, IOCFG_AF_PP_PD, GPIO_AF2_TIM4, TIM_USE_LED }
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
