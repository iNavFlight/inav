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
    // DEF_TIM(TIM2,  CH2,  PB3,  TIM_USE_CAMERA_CONTROL,      0, 0), // There is not camera control in INAV
    DEF_TIM(TIM8,  CH1,  PC6,  TIM_USE_OUTPUT_AUTO, 0, 1), // S1
    DEF_TIM(TIM8,  CH2,  PC7,  TIM_USE_OUTPUT_AUTO, 0, 1), // S2
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_OUTPUT_AUTO, 0, 1), // S3
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_OUTPUT_AUTO, 0, 0), // S4
    DEF_TIM(TIM4,  CH1,  PB6,  TIM_USE_LED, 0, 0), // LED
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
