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

#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"
#include "drivers/pwm_mapping.h"

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 0),      // S1
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 0),      // S2
    DEF_TIM(TIM3,  CH1, PB4,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 0),      // S3
    DEF_TIM(TIM3,  CH2, PB5,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 0),      // S4
    DEF_TIM(TIM4,  CH1, PB6,  TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0, 0),      // S5
    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,  0, 0),      // S6
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_LED,                          0, 0),      // LED STRIP
    DEF_TIM(TIM9,  CH2, PA3,  TIM_USE_PPM,                          0, 0),      // RX2
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);