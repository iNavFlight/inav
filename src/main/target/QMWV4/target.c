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

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1, CH3, PA10, TIM_USE_FW_MOTOR, 0, 0),
    DEF_TIM(TIM3, CH3, PC8,  TIM_USE_FW_SERVO, 0, 0),
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_FW_SERVO, 0, 0),
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_FW_SERVO, 0, 0),

    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_ANY      , 0, 0) // SS1
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
