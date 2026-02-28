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

#include "fc/control_profile_config_struct.h"
#include "fc/settings.h"

#define MAX_CONTROL_PROFILE_COUNT SETTING_CONSTANT_MAX_CONTROL_PROFILE_COUNT

PG_DECLARE_ARRAY(controlConfig_t, MAX_CONTROL_PROFILE_COUNT, controlProfiles);

extern const controlConfig_t *currentControlProfile;

void setControlProfile(uint8_t profileIndex);
void changeControlProfile(uint8_t profileIndex);
void activateControlConfig(void);
uint8_t getCurrentControlProfile(void);
