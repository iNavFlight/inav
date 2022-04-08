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
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {

    DEF_TIM(TIM8, CH3, PC8,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S1   UP2-1   D(2, 4, 7)
    DEF_TIM(TIM8, CH4, PC9,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S2   UP2-1   D(2, 7, 7)
    DEF_TIM(TIM3, CH4, PB1,     TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),   // S6   UP1-2   D(1, 2, 5)
    DEF_TIM(TIM3, CH3, PB0,     TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),   // S5   UP1-2   D(1, 7, 5)
    DEF_TIM(TIM8, CH2, PC7,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S5 D(2, 3, 7)
    DEF_TIM(TIM8, CH1, PC6,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),   // S6 D(1, 4, 5)

    DEF_TIM(TIM2, CH1, PA15,    TIM_USE_LED, 0, 0),                            // LED 
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
