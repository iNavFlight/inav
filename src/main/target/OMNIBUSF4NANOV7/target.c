/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

#include "drivers/pwm_output.h"
#include "common/maths.h"
#include "fc/config.h"


const timerHardware_t timerHardware[] = {

    DEF_TIM(TIM8, CH3, PC8, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,    0, 0), // S1_OUT    
    DEF_TIM(TIM4, CH1, PB6, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    0, 0), // S2_OUT    
    DEF_TIM(TIM8, CH4, PC9, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    0, 0), // S3_OUT    
    DEF_TIM(TIM4, CH2, PB7, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    0, 0), // S4_OUT    

    DEF_TIM(TIM2, CH1, PA15, TIM_USE_LED,                           0, 0), // onboard LED 
    DEF_TIM(TIM1, CH1, PA8, TIM_USE_ANY,                            0, 0), // cam control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

#ifdef USE_DSHOT
void validateAndFixTargetConfig(void)
{
    // On airplanes DSHOT is not supported on this target
    if (mixerConfig()->platformType != PLATFORM_MULTIROTOR && mixerConfig()->platformType != PLATFORM_TRICOPTER) {
        if (motorConfig()->motorPwmProtocol >= PWM_TYPE_DSHOT150) {
            motorConfigMutable()->motorPwmProtocol = PWM_TYPE_STANDARD;
            motorConfigMutable()->motorPwmRate = MIN(motorConfig()->motorPwmRate, 490);
        }
    }
}
#endif
