/*
 * This file is part of INAV Project.
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

/*
 * Motor pads, silk M1..M4 = GPIO25..22. The order of this table is the INAV
 * output order, so M1 comes first.
 * The TIMn tokens only group the pins of one PWM slice. GPIO24/25 are slice 4.
 * GPIO22/23 are slice 3, which has no token of its own; TIM5 stands in for
 * it, its own pins (GPIO10/11, GPIO26/27) are not outputs on this board.
 */
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4, CH2, PB9, TIM_USE_OUTPUT_AUTO, 0, 0),     // M1, GPIO25
    DEF_TIM(TIM4, CH1, PB8, TIM_USE_OUTPUT_AUTO, 0, 0),     // M2, GPIO24
    DEF_TIM(TIM5, CH2, PB7, TIM_USE_OUTPUT_AUTO, 0, 0),     // M3, GPIO23
    DEF_TIM(TIM5, CH1, PB6, TIM_USE_OUTPUT_AUTO, 0, 0),     // M4, GPIO22
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
