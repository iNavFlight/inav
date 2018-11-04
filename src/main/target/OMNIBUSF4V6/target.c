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
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

#define DEF_TIM_CHNL_CH1    TIM_Channel_1
#define DEF_TIM_CHNL_CH2    TIM_Channel_2
#define DEF_TIM_CHNL_CH3    TIM_Channel_3
#define DEF_TIM_CHNL_CH4    TIM_Channel_4

#define DEF_TIM(_tim, _ch, _pin, _usage, _flags) \
    { _tim, IO_TAG(_pin), DEF_TIM_CHNL_##_ch, _flags, IOCFG_AF_PP, GPIO_AF_##_tim, _usage }


const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM10, CH1, PB8, TIM_USE_PPM,       0), // PPM

    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,   1), // S1_OUT
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,   1), // S2_OUT
    DEF_TIM(TIM9, CH2, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,   1), // S3_OUT
    DEF_TIM(TIM3, CH2, PB5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,   1), // S4_OUT

    DEF_TIM(TIM4, CH1, PB6, TIM_USE_LED,        0), // LED strip

    DEF_TIM(TIM1, CH2, PA9, TIM_USE_ANY,        0), // SS1
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
