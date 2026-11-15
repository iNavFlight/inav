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
 * Motor pads, silk M1..M4 = GPIO31..28. The order of this table is the INAV
 * output order, so M1 comes first.
 * GPIO28/29 share PWM slice 6 and GPIO30/31 slice 7 (even GPIO = CH1).
 */
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM7, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0),    // M1, GPIO31
    DEF_TIM(TIM7, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0),    // M2, GPIO30
    DEF_TIM(TIM6, CH2, PB13, TIM_USE_OUTPUT_AUTO, 0, 0),    // M3, GPIO29
    DEF_TIM(TIM6, CH1, PB12, TIM_USE_OUTPUT_AUTO, 0, 0),    // M4, GPIO28
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
