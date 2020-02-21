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

      DEF_TIM(TIM8, CH4,  PC9, TIM_USE_PPM,   0, 0 ), // PPM IN
      DEF_TIM(TIM2, CH4,  PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 1 ), // S1_OUT - D(1, 6, 3)
      DEF_TIM(TIM3, CH3,  PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0 ), // S2_OUT - D(1, 7, 5)
      DEF_TIM(TIM3, CH4,  PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0 ), // S3_OUT - D(1, 2, 5)
      DEF_TIM(TIM2, CH3,  PA2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0 ), // S4_OUT - D(1, 1, 3)
      DEF_TIM(TIM2, CH3, PB10, TIM_USE_ANY, 0, 0),
      DEF_TIM(TIM2, CH4, PB11, TIM_USE_ANY, 0, 0), 
      DEF_TIM(TIM5, CH1, PA0, TIM_USE_LED, 0, 0 ), // LED_STRIP - DMA1_ST2_CH6
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
