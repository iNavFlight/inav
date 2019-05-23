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
#include <platform.h>
#include "config/config_master.h"
#include "config/feature.h"
#include "flight/mixer.h"
#include "io/serial.h"
#include "telemetry/telemetry.h"

// alternative defaults settings for MATEKF405SE targets
void targetConfiguration(void)
{
    
    serialConfigMutable()->portConfigs[1].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[1].msp_baudrateIndex = BAUD_57600;

    //featureSet(FEATURE_PWM_OUTPUT_ENABLE); // enable PWM outputs by default
    //mixerConfigMutable()->mixerMode = MIXER_FLYING_WING; // default mixer to flying wing
    mixerConfigMutable()->platformType = PLATFORM_AIRPLANE;   // default mixer to Airplane

    serialConfigMutable()->portConfigs[7].functionMask = FUNCTION_TELEMETRY_SMARTPORT;
}
