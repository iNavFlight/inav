/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"
#include "drivers/io.h"

#include "drivers/dma.h"
#include "drivers/timer.h"
#include "drivers/timer_def.h"

timerHardware_t timerHardware[] = {
    // Motors - Based on your schematic ESC connections
    DEF_TIM(TIM8,  CH1,  PA9,  TIM_USE_MOTOR, 0, 0), // ESC_1 - S1
    DEF_TIM(TIM8,  CH2,  PA8,  TIM_USE_MOTOR, 0, 0), // ESC_2 - S2
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_MOTOR, 0, 0), // ESC_3 - S3
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_MOTOR, 0, 0), // ESC_4 - S4
    
    // Gimbal outputs
    DEF_TIM(TIM3,  CH3,  PB0,  TIM_USE_SERVO, 0, 0), // GIMBAL_X
    DEF_TIM(TIM3,  CH4,  PB1,  TIM_USE_SERVO, 0, 0), // GIMBAL_Y
    DEF_TIM(TIM3,  CH2,  PC5,  TIM_USE_SERVO, 0, 0), // GIMBAL_Z
    
    // LED & PPM - PB6 is used for WS2812
    DEF_TIM(TIM4,  CH1,  PB3,  TIM_USE_LED, 0, 0),   // LED Strip
   
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);