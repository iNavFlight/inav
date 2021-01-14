/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

//this is Part of INAV Sourcecode, more exactly target.h for MAMBAF722, modified by LTwin8 to fit HIFIONRCF7 AIO and HIFIONRCF7 PRO

//Used resources are the INAV-Code and the Betaflight unified target file https://raw.githubusercontent.com/betaflight/unified-targets/master/configs/default/JHEF-JHEF7DUAL.config


#include <stdint.h>

#include "platform.h"
#include "drivers/io.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/bus.h"
const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH3, PB0,   TIM_USE_MC_MOTOR, 0, 0),   // S1   UP1-2
    DEF_TIM(TIM3, CH4, PB1,   TIM_USE_MC_MOTOR, 0, 0),   // S2   UP1-2
    DEF_TIM(TIM3, CH1, PB4,   TIM_USE_MC_MOTOR, 0, 0),   // S3   UP1-2
    DEF_TIM(TIM2, CH2, PB3,   TIM_USE_MC_MOTOR, 0, 0),   // S4   UP1-7
    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_LED, 0, 2),   // LED DMA2-3
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
