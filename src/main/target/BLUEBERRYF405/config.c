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
#include "fc/fc_msp_box.h"
#include "fc/config.h"
#include "drivers/pwm_mapping.h"
#include "flight/mixer_profile.h"
#include "io/piniobox.h"

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;

	// S1 and S2 are not as readily accessible, set default mixer to skip them
	timerOverridesMutable(timer2id(TIM8))->outputMode = OUTPUT_MODE_SERVOS;
	timerOverridesMutable(timer2id(TIM2))->outputMode = OUTPUT_MODE_SERVOS;
    timerOverridesMutable(timer2id(TIM1))->outputMode = OUTPUT_MODE_MOTORS;

    *primaryMotorMixerMutable(0) = (motorMixer_t){ 1.0f, 0.0f, 0.0f, 0.0f };

    *customServoMixersMutable(0) = (servoMixer_t){ .targetChannel = 1, .inputSource = 29, .rate = 100, .speed = 0 };
    *customServoMixersMutable(1) = (servoMixer_t){ .targetChannel = 2, .inputSource = 29, .rate = 100, .speed = 0 };
    *customServoMixersMutable(2) = (servoMixer_t){ .targetChannel = 3, .inputSource =  1, .rate = 100, .speed = 0 };
    *customServoMixersMutable(3) = (servoMixer_t){ .targetChannel = 4, .inputSource =  0, .rate = 100, .speed = 0 };
    *customServoMixersMutable(4) = (servoMixer_t){ .targetChannel = 5, .inputSource =  0, .rate = 100, .speed = 0 };
    *customServoMixersMutable(5) = (servoMixer_t){ .targetChannel = 6, .inputSource =  2, .rate = 100, .speed = 0 };
}
