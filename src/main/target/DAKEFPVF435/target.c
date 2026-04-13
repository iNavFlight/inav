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

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

// # pin A00: TIM2 CH1 (AF1)
// # pin A01: TIM2 CH2 (AF1)
// # pin A02: TIM2 CH3 (AF1)
// # pin A03: TIM2 CH4 (AF1)
// # pin A08: TIM1 CH1 (AF1)
// # pin A09: TIM1 CH2 (AF1)
// # pin A10: TIM1 CH3 (AF1)
// # pin C09: TIM8 CH4 (AF3)
// # pin H03: TIM5 CH2 (AF2)
// # pin H02: TIM5 CH1 (AF2)
// # pin C08: TIM3 CH3 (AF2)
// # pin C05: TIM9 CH2 (AF3)

timerHardware_t timerHardware[] = {

    DEF_TIM(TMR2, CH1, PA0,  TIM_USE_OUTPUT_AUTO, 0,0), // M1
    DEF_TIM(TMR2, CH2, PA1,  TIM_USE_OUTPUT_AUTO, 0,1), // M2
    DEF_TIM(TMR2, CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0,2), // M3
    DEF_TIM(TMR2, CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0,3), // M4
    DEF_TIM(TMR1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0,4), // M5
    DEF_TIM(TMR1, CH2, PA9,  TIM_USE_OUTPUT_AUTO, 0,5), // M6
    DEF_TIM(TMR1, CH3, PA10, TIM_USE_OUTPUT_AUTO, 0,6), // M7
	DEF_TIM(TMR8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0,8), // M8
	DEF_TIM(TMR5, CH2, PH3,  TIM_USE_OUTPUT_AUTO, 0,9), // S1
	DEF_TIM(TMR5, CH1, PH2,  TIM_USE_OUTPUT_AUTO, 0,10),// S2
    
	DEF_TIM(TMR3, CH3, PC8,   TIM_USE_LED | TIM_USE_ANY, 0, 11),  // LED_STRIP
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

