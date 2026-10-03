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
#include "drivers/pinio.h"

timerHardware_t timerHardware[] = {

    /*DEF_TIM(TIM1, CH2, PA9,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR1 = correct
    DEF_TIM(TIM1, CH1, PA8,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR2 = correct
    DEF_TIM(TIM8, CH4, PC9,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR3 = or CH3/CH4
    DEF_TIM(TIM8, CH3, PC8,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR4 = or TIM3/TIM8
    DEF_TIM(TIM3, CH2, PB5,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR5 = correct
    DEF_TIM(TIM3, CH1, PB4,   TIM_USE_OUTPUT_AUTO,  0, 0 ),   // MOTOR6 = correct*/

    //DEF_TIM(TIM1, CH3, PA10,  TIM_USE_CAMERA_CONTROL, 0, 0),

    DEF_TIM(TIM8,   CH3,  PC8,   TIM_USE_OUTPUT_AUTO,  0, 0 ),     // S1_OUT – D(2, 4, 7)
    DEF_TIM(TIM8,   CH4,  PC9,   TIM_USE_OUTPUT_AUTO,  0, 0 ),     // S2_OUT – D(2, 7, 7)
    DEF_TIM(TIM1,   CH1,  PA8,   TIM_USE_OUTPUT_AUTO,  0, 1 ),     // S3_OUT – D(2, 1, 6)
    DEF_TIM(TIM1,   CH2,  PA9,   TIM_USE_OUTPUT_AUTO,  0, 1 ),     // S4_OUT – D(2, 2, 6)

};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

