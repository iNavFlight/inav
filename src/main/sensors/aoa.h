/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, contents of this file may be used under terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "config/parameter_group.h"
#include "drivers/aoa/aoa_virtual.h"

typedef enum {
    AOA_NONE = 0,
    AOA_MSP = 1,
    AOA_FAKE = 2,
} aoaType_e;

typedef struct aoaConfig_s {
    uint8_t aoa_hardware;
    int16_t aoa_offset;
    int16_t aoa_max_angle;
    int16_t aoa_min_angle;
} aoaConfig_t;

PG_DECLARE(aoaConfig_t, aoaConfig);

typedef struct aoaControlConfig_s {
    int8_t fw_aoa_control_channel;
    int8_t fw_aoa_gvar_index;
    uint8_t fw_aoa_deg2pwm;
    int8_t fw_aoa_trim_angle;
    int8_t fw_aoa_upper_limit_angle;
    int8_t fw_aoa_lower_limit_angle;
    uint8_t fw_aoa_aircraft_type;
    uint8_t fw_aoa_kp;
} aoaControlConfig_t;

PG_DECLARE(aoaControlConfig_t, aoaControlConfig);

extern bool isAoaControlEnabled;
extern int16_t aoaPidOutput;
extern int16_t aoaServoOffset;

typedef struct aoa_s {
    aoaDev_t dev;
    int16_t aoa;
    int16_t sideslip;
    timeMs_t lastValidResponseTimeMs;
} aoa_t;

extern aoa_t aoaSensor;

bool aoaInit(void);
void aoaGetLatestData(int16_t *aoa, int16_t *sideslip);
timeDelta_t aoaUpdate(void);
bool aoaProcess(void);
bool aoaIsHealthy(void);
bool aoaControlEnable(int8_t input_rc_channel);
void aoaControlUpdate(float pidPitchOutput, float rateError, float newPTerm, float newDTerm, float newFFTerm, float errorGyroIf, float limit);
