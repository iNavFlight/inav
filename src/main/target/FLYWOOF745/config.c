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
#include "platform.h"

#include "fc/fc_msp_box.h"

#include "io/piniobox.h"

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;

    // Make sure S1-S4 default to Motors

    timerOverridesMutable(timer2id(TIM3))->outputMode = OUTPUT_MODE_MOTORS;
    timerOverridesMutable(timer2id(TIM8))->outputMode = OUTPUT_MODE_MOTORS;
    timerOverridesMutable(timer2id(TIM1))->outputMode = OUTPUT_MODE_MOTORS;
}
