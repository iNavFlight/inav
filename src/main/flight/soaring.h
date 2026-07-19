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

#include "common/vector.h"

#include "config/parameter_group.h"

// Thermal soaring: motor off, ride the thermal. A net (total-energy) vario
// built from the pitot airspeed and the vertical estimate finds the rising
// air; the loiter circle is centred on the thermal by a sin/cos gradient
// over each turn PLUS a wind-drift shift (the thermal is locked to the air
// mass, so the circle slides with the wind - the piece ArduSoar scales by
// climb/strength without physical basis, done here as a plain wind * dt).
// A pilot-selected SOARING mode (BOXSOARING) activates it; it cuts the
// motor while circling and hands the loiter target back to the nav layer.

typedef struct soaringConfig_s {
    uint16_t varioTriggerCms;   // net vario [cm/s] over which cruise -> thermal
    uint16_t varioExitCms;      // net vario [cm/s] under which thermal -> cruise
    uint16_t altMinM;           // do not thermal below this altitude [m]
    uint16_t altMaxM;           // leave the thermal when this is reached [m]
    uint8_t  bankDeg;           // thermalling bank angle [deg]
    uint16_t sinkLevelCms;      // level-flight sink at the tuning airspeed [cm/s]
    uint8_t  centreGainPct;     // gradient-shift gain [% per turn]
} soaringConfig_t;

PG_DECLARE(soaringConfig_t, soaringConfig);

// Call once per main PID loop iteration (after the nav update)
void soaringUpdate(float dT);

// True while the pilot's SOARING mode is engaged and armed in the air
bool soaringActive(void);

// True while circling a thermal (motor off, loiter centred on the estimate)
bool soaringThermalling(void);

// The estimated thermal centre to loiter (earth frame, cm from home). Only
// meaningful while soaringThermalling(); the nav layer drives the forced
// poshold onto it (the wandering-anchor mechanism).
void soaringThermalCentre(fpVector3_t *centre);

// The net (total-energy compensated) variometer [cm/s], for telemetry/OSD
float soaringNetVarioCms(void);
