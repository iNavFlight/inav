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

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0),
    DEF_TIM(TIM1,  CH2N, PB0,  TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0),
    DEF_TIM(TIM15, CH1,  PB14, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0),
    DEF_TIM(TIM15, CH2,  PB15, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0),
    DEF_TIM(TIM3,  CH1,  PA6,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0),
    DEF_TIM(TIM3,  CH2,  PA7,  TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0),

    DEF_TIM(TIM4,  CH3,  PA13, TIM_USE_PWM,                 0),
    DEF_TIM(TIM2,  CH2,  PB3,  TIM_USE_PWM | TIM_USE_PPM,   0),
    DEF_TIM(TIM8,  CH1,  PA15, TIM_USE_PWM,                 0),
    DEF_TIM(TIM2,  CH3,  PA2,  TIM_USE_PWM,                 0),
    DEF_TIM(TIM2,  CH4,  PB11, TIM_USE_PWM,                 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
