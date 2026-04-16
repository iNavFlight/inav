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

typedef enum {
    AIRCRAFT_NORMAL = 0,    // Conventional aircraft layout
    AIRCRAFT_CANARD = 1,    // Canard aircraft layout
} aoa_aircraftType_e;

typedef struct aoaConfig_s {
    uint8_t aoa_hardware;       // AOA sensor hardware type (NONE, MSP, FAKE)
    int16_t aoa_offset;         // AOA sensor offset in decidegrees
    int16_t aoa_max_angle;      // Maximum valid AOA angle in decidegrees
    int16_t aoa_min_angle;      // Minimum valid AOA angle in decidegrees
} aoaConfig_t;

PG_DECLARE(aoaConfig_t, aoaConfig);

typedef struct aoaControlConfig_s {
    int8_t fw_aoa_control_channel;          // RC channel for AOA control enable (-1 to always enable)
    int8_t fw_aoa_gvar_index;               // Global variable index for AOA control
    uint8_t fw_aoa_deg2pwm;                 // Conversion factor from degrees to PWM (default 110)
    int8_t fw_aoa_trim_angle;               // AOA trim angle in degrees
    uint8_t fw_aoa_upper_limit_angle;       // Upper AOA limit in degrees (0-60)
    int8_t fw_aoa_lower_limit_angle;        // Lower AOA limit in degrees
    uint8_t fw_aoa_aircraft_type;           // Aircraft layout type (NORMAL or CANARD)
    uint16_t fw_aoa_kp;                     // P gain percentage (0-1000)
    uint8_t fw_aoa_intervention_threshold;  // Percentage of limit angle to start intervention for NORMAL layout (0-100)
} aoaControlConfig_t;

PG_DECLARE(aoaControlConfig_t, aoaControlConfig);

extern bool isAoaControlEnabled;
extern int16_t aoaPidOutput;

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
void aoaControlUpdate(float *pidPitchOutput, float rateError, float newPTerm, float newDTerm, float newFFTerm, float errorGyroIf, float limit);
