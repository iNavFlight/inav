/*
 * This file is part of INAV.
 *
 * INAV are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV are distributed in the hope that they
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
    DEF_TIM(TMR3, CH3, PB0, TIM_USE_MOTOR, 0, 1),  // TMR3_CH3 motor 1
    DEF_TIM(TMR3, CH4, PB1, TIM_USE_MOTOR, 0, 2),  // TMR3_CH4 motor 2
    DEF_TIM(TMR2, CH4, PA3, TIM_USE_MOTOR, 0, 7),  // TMR2_CH4 motor 3
    DEF_TIM(TMR2, CH3, PA2, TIM_USE_MOTOR, 0, 6),  // TMR2_CH3 motor 4

    DEF_TIM(TMR8, CH3, PC8, TIM_USE_OUTPUT_AUTO | TIM_USE_ANY, 0, 4),  // TMR8_CH3 motor 5
    DEF_TIM(TMR1, CH1, PA8, TIM_USE_OUTPUT_AUTO | TIM_USE_ANY, 0, 5),  // TMR1_CH3 motor 6

    DEF_TIM(TMR4, CH1, PB6, TIM_USE_LED, 0, 0),  // TMR4_CH1 LED_STRIP
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
