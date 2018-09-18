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

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

/* TIMERS */
const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_PPM,      0, 0), // PPM In
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_MC_MOTOR, 0, 0), // S1
    DEF_TIM(TIM5,  CH4, PA3,  TIM_USE_MC_MOTOR, 0, 0), // S2
    DEF_TIM(TIM1,  CH3, PA10, TIM_USE_MC_MOTOR, 0, 0), // S3
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_MC_MOTOR, 0, 0), // S4
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MC_MOTOR, 0, 0), // S5
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_MC_MOTOR, 0, 0), // S6
    DEF_TIM(TIM11, CH1, PB9,  TIM_USE_ANY,      0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
