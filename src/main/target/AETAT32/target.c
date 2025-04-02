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

timerHardware_t timerHardware[] = {

DEF_TIM(TMR1, CH4, PE14,  TIM_USE_MOTOR, 0,1), 		// 1
DEF_TIM(TMR1, CH3, PE13,  TIM_USE_MOTOR, 0,2), 		// 2

DEF_TIM(TMR3, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0,3), 		// PWM4 - OUT8  DMA2 CH5
DEF_TIM(TMR3, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0,4), 		// PWM3 - OUT7  DMA2 CH4
DEF_TIM(TMR3, CH2, PC7,  TIM_USE_OUTPUT_AUTO, 0,5), 		// PWM2 - OUT6  DMA2 CH3
DEF_TIM(TMR3, CH1, PC6,  TIM_USE_OUTPUT_AUTO, 0,6), 		// PWM1 - OUT5  DMA2 CH2 DMA2_CHANNEL1->ADC

DEF_TIM(TMR4, CH4, PB9,  TIM_USE_OUTPUT_AUTO, 0,7), // motor4 DMA1 CH4
DEF_TIM(TMR4, CH3, PB8,  TIM_USE_OUTPUT_AUTO, 0,8), // motor3 DMA1 CH3
DEF_TIM(TMR4, CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0,9),  // motor2 DMA1 CH2
DEF_TIM(TMR4, CH1, PB6,  TIM_USE_OUTPUT_AUTO, 0,10),  // motor1 DMA1 CH1

DEF_TIM(TMR2, CH3, PB10, TIM_USE_ANY | TIM_USE_LED, 0,11),           // PWM1 - LED MCO1 DMA1 CH6


};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
  
