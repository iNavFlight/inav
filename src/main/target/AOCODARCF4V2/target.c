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


#include <stdint.h>

#include "platform.h"
#include "drivers/io.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/bus.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5,  CH4,  PA3,     TIM_USE_PPM,       0, 0 ),       // PPM IN

    DEF_TIM(TIM8,   CH1,  PC6,    TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 1 ),     // S1 
    DEF_TIM(TIM8,   CH2,  PC7,    TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 1 ),     // S2
    DEF_TIM(TIM8,   CH3,  PC8,    TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 1 ),     // S3
    DEF_TIM(TIM8,   CH4,  PC9,    TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,  0, 0 ),     // S4

    DEF_TIM(TIM2,   CH1,  PA15,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0, 0 ),     // S5
    DEF_TIM(TIM1,   CH1,  PA8,    TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0, 1 ),     // S6

    DEF_TIM(TIM2,   CH3,  PB10,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0, 0 ),     // S7
    DEF_TIM(TIM2,   CH4,  PB11,   TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,  0, 0 ),     // S8

    DEF_TIM(TIM3,   CH4,  PB1,   TIM_USE_LED,  0, 0 ),     // LED_STRIP

    
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
