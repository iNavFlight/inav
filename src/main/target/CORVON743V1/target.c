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

#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1,  CH4, PE14,  TIM_USE_OUTPUT_AUTO, 0, 0), // S1
    DEF_TIM(TIM1,  CH3, PE13,  TIM_USE_OUTPUT_AUTO, 0, 1), // S2
    DEF_TIM(TIM1,  CH2, PE11,  TIM_USE_OUTPUT_AUTO, 0, 2), // S3
    DEF_TIM(TIM1,  CH1, PE9,   TIM_USE_OUTPUT_AUTO, 0, 3), // S4
    DEF_TIM(TIM3,  CH4, PB1,   TIM_USE_OUTPUT_AUTO, 0, 4), // S5
    DEF_TIM(TIM3,  CH3, PB0,   TIM_USE_OUTPUT_AUTO, 0, 5), // S6
    DEF_TIM(TIM4,  CH1, PD12,  TIM_USE_OUTPUT_AUTO, 0, 6), // S7
    DEF_TIM(TIM4,  CH2, PD13,  TIM_USE_OUTPUT_AUTO, 0, 7), // S8
    DEF_TIM(TIM4,  CH3, PD14,  TIM_USE_OUTPUT_AUTO, 0, 0), // S9
    DEF_TIM(TIM4,  CH4, PD15,  TIM_USE_OUTPUT_AUTO, 0, 0), // S10 DMA_NONE
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
