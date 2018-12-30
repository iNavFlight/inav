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

const timerHardware_t timerHardware[] = {
    // DEF_TIM(TIM1,  CH1, PA8, TIM_USE_PPM,        0, 0), // PPM  - timer clash with SS1_TX

    DEF_TIM(TIM8,  CH3, PC8, TIM_USE_MC_MOTOR,   1, 0), // M1 - D(2, 4, 7)
    DEF_TIM(TIM3,  CH3, PB0, TIM_USE_MC_MOTOR,   1, 0), // M2 - D(1, 7, 5)
    DEF_TIM(TIM3,  CH4, PB1, TIM_USE_MC_MOTOR,   1, 0), // M3 - D(1, 2, 5)
    DEF_TIM(TIM4,  CH2, PB7, TIM_USE_MC_MOTOR,   1, 0), // M4 - D(1, 6, 3)
    DEF_TIM(TIM5,  CH3, PA2, TIM_USE_LED,        0, 0), // TX2; SA port ---> LED - D(1, 0, 6)
    DEF_TIM(TIM1,  CH2, PA9, TIM_USE_ANY,        0, 0), // SS1
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
