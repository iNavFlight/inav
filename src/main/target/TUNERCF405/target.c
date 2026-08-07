/*
 * This file is part of INAV Project.
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

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1,	CH2N,	PB0,	TIM_USE_MOTOR,	0, 0),	//D2s6
    DEF_TIM(TIM1,	CH1,	PA8,	TIM_USE_MOTOR,	0, 0),	//D2s6
    DEF_TIM(TIM4,	CH3,	PB8,	TIM_USE_MOTOR,	0, 0),	//D1s7
    DEF_TIM(TIM2,	CH1,	PA15,	TIM_USE_MOTOR,	0, 0),	//D1s5
	
    DEF_TIM(TIM8,	CH4,	PC9,	TIM_USE_OUTPUT_AUTO,	0, 0), //D2s7
    DEF_TIM(TIM8,	CH3,	PC8,	TIM_USE_OUTPUT_AUTO,	0, 0), //D2s2 / D2s4
    DEF_TIM(TIM8,	CH2,	PC7,	TIM_USE_OUTPUT_AUTO,	0, 0), //D2s2
    DEF_TIM(TIM8,	CH1,	PC6,	TIM_USE_OUTPUT_AUTO,	0, 0), //D2s2
	
    DEF_TIM(TIM3,	CH4,	PB1,	TIM_USE_LED,	0, 0), 
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
