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
 
    DEF_TIM(TMR3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0,0),   // PWM  1    
    DEF_TIM(TMR3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0,1),   // PWM 2    

    DEF_TIM(TMR2, CH4, PA3,  TIM_USE_OUTPUT_AUTO, 0,2),   // PWM  3    
    DEF_TIM(TMR2, CH3, PA2,  TIM_USE_OUTPUT_AUTO, 0,3),  // PWM 4   

    DEF_TIM(TMR2, CH1, PA15,  TIM_USE_OUTPUT_AUTO, 0,4), // PWM 5 

    DEF_TIM(TMR4, CH4, PB9,  TIM_USE_OUTPUT_AUTO, 0,5),  // PWM  6    UART5
    DEF_TIM(TMR4, CH3, PB8,  TIM_USE_OUTPUT_AUTO, 0,6),  // PWM 7    UART5

    DEF_TIM(TMR4, CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0,8),  // PWM 8, DMA 7 may be used by ADC
    DEF_TIM(TMR4, CH1, PB6,  TIM_USE_OUTPUT_AUTO, 0,9),   // PWM  9

    DEF_TIM(TMR1, CH1, PA8,  TIM_USE_LED, 0,10), //WS2812  // PWM1 - LED MCO1 DMA1 CH2


};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

