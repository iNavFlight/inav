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
#include "drivers/pinio.h"
#include "drivers/sensor.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4, CH1, PB6, TIM_USE_OUTPUT_AUTO,   1, 0), // S1
    DEF_TIM(TIM4, CH2, PB7, TIM_USE_OUTPUT_AUTO,   1, 0), // S2
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_OUTPUT_AUTO,   1, 0), // S3
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_OUTPUT_AUTO,   1, 0), // S4

    DEF_TIM(TIM12,  CH1,    PB14,   TIM_USE_ANY,   1, 0), // CAM_CTRL
    DEF_TIM(TIM1,   CH1,    PA8,    TIM_USE_LED,   0, 0), // LED

    DEF_TIM(TIM5, CH3, PA2, TIM_USE_ANY,           0, 0), //TX2  softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
