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

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

timerHardware_t timerHardware[] = {
    // DEF_TIM(TIM4, CH2, PB7,  TIM_USE_PPM,   0, 0),  // PPM

    DEF_TIM(TIM1, CH1, PA8,     TIM_USE_OUTPUT_AUTO, 0, 1),  // S1
    DEF_TIM(TIM1, CH2, PA9,     TIM_USE_OUTPUT_AUTO, 0, 1),  // S2
    DEF_TIM(TIM1, CH3, PA10,    TIM_USE_OUTPUT_AUTO, 0, 1),  // S3
    DEF_TIM(TIM3, CH4, PC9,     TIM_USE_OUTPUT_AUTO, 0, 0),  // S4 // used to be fw motor

    DEF_TIM(TIM8, CH2N, PB0,  TIM_USE_LED, 0, 0),    // LED

    DEF_TIM(TIM5, CH1, PA0,  TIM_USE_ANY, 0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
