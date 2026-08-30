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
#include "drivers/sensor.h"

timerHardware_t timerHardware[] = {

    DEF_TIM(TMR4, CH1, PB6, TIM_USE_MOTOR, 0, 0),  // TMR3_CH3 motor 1
    DEF_TIM(TMR4, CH2, PB7, TIM_USE_MOTOR, 0,10),  // TMR3_CH4 motor 2
    DEF_TIM(TMR2, CH4, PA3, TIM_USE_MOTOR, 0, 7),  // TMR2_CH4 motor 3
    DEF_TIM(TMR2, CH3, PA2, TIM_USE_MOTOR, 0, 6),  // TMR2_CH3 motor 4

    DEF_TIM(TMR3, CH4, PB1, TIM_USE_LED, 0, 2),  // TMR4_CH1 LED_STRIP

};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

