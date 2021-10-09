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

#pragma once

#include <stdint.h>

#include "config/parameter_group.h"

#include "fc/controlrate_profile_config_struct.h"
#include "fc/settings.h"

#define MAX_CONTROL_RATE_PROFILE_COUNT SETTING_CONSTANT_MAX_CONTROL_RATE_PROFILE_COUNT

PG_DECLARE_ARRAY(controlRateConfig_t, MAX_CONTROL_RATE_PROFILE_COUNT, controlRateProfiles);

extern const controlRateConfig_t *currentControlRateProfile;

void setControlRateProfile(uint8_t profileIndex);
void changeControlRateProfile(uint8_t profileIndex);
void activateControlRateConfig(void);
uint8_t getCurrentControlRateProfile(void);
