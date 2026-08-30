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

#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0), // S1  D(1,7)
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0), // S2  D(1,2)
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0), // S3  -D(1,5)
    DEF_TIM(TIM2,  CH2, PB3,  TIM_USE_OUTPUT_AUTO, 0, 0), // S4  D(1,6)
    DEF_TIM(TIM3,  CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0), // S5  D(1,4)
    DEF_TIM(TIM3,  CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0), // S6  -D(1,5)
    DEF_TIM(TIM4,  CH3, PB8,  TIM_USE_OUTPUT_AUTO, 0, 0), // S7
    DEF_TIM(TIM4,  CH4, PB9,  TIM_USE_OUTPUT_AUTO, 0, 0), // S8
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0), // S9
    DEF_TIM(TIM12, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0), // S10
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
