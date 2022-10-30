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
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

timerHardware_t timerHardware[] = {

    DEF_TIM(TIM3, CH3, PB0,     TIM_USE_MC_MOTOR, 0, 0),  // S1
    DEF_TIM(TIM3, CH4, PB1,     TIM_USE_MC_MOTOR, 0, 0),  // S2
    DEF_TIM(TIM1, CH1, PE9,     TIM_USE_MC_MOTOR, 0, 2),  // S3
    DEF_TIM(TIM1, CH2, PE11,    TIM_USE_MC_MOTOR, 0, 1),  // S4

    DEF_TIM(TIM4, CH1, PD12,    TIM_USE_LED, 0, 0),    // LED
    DEF_TIM(TIM2, CH2, PB3,     TIM_USE_ANY, 0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);