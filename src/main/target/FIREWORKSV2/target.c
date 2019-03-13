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
    DEF_TIM(TIM10, CH1, PB8, TIM_USE_PPM,                           0, 0), // PPM

    // Motor output 1: use different set of timers for MC and FW
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR,                       1, 0), // S1_OUT    D(1,7)
    DEF_TIM(TIM8, CH2N, PB0,                    TIM_USE_FW_MOTOR,   1, 1), // S1_OUT    D(2,2,0),D(2,3,7)

    // Motor output 2: use different set of timers for MC and FW
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_MOTOR,                       1, 0), // S2_OUT    D(1,2)
    DEF_TIM(TIM8, CH3N, PB1,                    TIM_USE_FW_MOTOR,   1, 1), // S2_OUT    D(2,2,0),D(2,4,7)

    DEF_TIM(TIM2, CH4, PA3, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 1), // S3_OUT    D(1,6)
    DEF_TIM(TIM3, CH2, PB5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0), // S4_OUT    D(1,5)
    DEF_TIM(TIM3, CH3, PC8, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    0, 0), // S5_OUT    D(1,7)
    DEF_TIM(TIM3, CH4, PC9, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    0, 0), // S6_OUT    D(1,8)
	
    DEF_TIM(TIM4, CH1, PB6, TIM_USE_LED,                            0, 0), // LED strip D(1,0)

    DEF_TIM(TIM1, CH2, PA9, TIM_USE_ANY,                            0, 0), // SS1
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