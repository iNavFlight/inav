/*
 * This file is part of INAV Project.
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RPM_SOURCE_DATA_INVALID 255

bool rpmSourceIsConfigured(void);
bool rpmSourceIsDshotBidirConfigured(void);
bool rpmSourceIsDshotBidirActive(void);
uint8_t rpmSourceGetDshotBidirValidCount(void);
bool rpmSourceGetAverageRpm(uint32_t *rpm);
bool rpmSourceGetMotorRpm(uint8_t motor, uint32_t *rpm);
void rpmSourceResetDshotBidir(void);
void rpmSourceSetDshotBidirRpm(uint8_t motor, uint32_t rpm);
void rpmSourceInvalidateDshotBidir(uint8_t motor);
