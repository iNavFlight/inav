/*
 * This file is part of iNav
 *
 * iNav free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <common/time.h>

#include "config/parameter_group.h"

#include <telemetry/smartport.h>

#if defined(USE_SMARTPORT_MASTER)

typedef struct {
    bool halfDuplex;
    bool inverted;
} smartportMasterConfig_t;

PG_DECLARE(smartportMasterConfig_t, smartportMasterConfig);

typedef struct {
    int8_t count;
    int16_t voltage[6];
} cellsData_t;

typedef enum {
    VS600_BAND_A,
    VS600_BAND_B,
    VS600_BAND_C,
    VS600_BAND_D,
    VS600_BAND_E,
    VS600_BAND_F,
} vs600Band_e;

typedef enum {
    VS600_POWER_PIT,
    VS600_POWER_25MW,
    VS600_POWER_200MW,
    VS600_POWER_600MW,
} vs600Power_e;

typedef struct {
    vs600Band_e band;
    uint8_t channel;
    vs600Power_e power;
} vs600Data_t;


bool smartportMasterInit(void);
void smartportMasterHandle(timeUs_t currentTimeUs);

bool smartportMasterPhyIDIsActive(uint8_t phyID);
int8_t smartportMasterStripPhyIDCheckBits(uint8_t phyID);

// Returns latest received SmartPort payload for phyID
bool smartportMasterGetSensorPayload(uint8_t phyID, smartPortPayload_t *payload);

// Forwarding
bool smartportMasterForward(uint8_t phyID, smartPortPayload_t *payload);
bool smartportMasterHasForwardResponse(uint8_t phyID);
bool smartportMasterNextForwardResponse(uint8_t phyID, smartPortPayload_t *payload);

// Returns latest Cells data or NULL if the data is too old
cellsData_t *smartportMasterGetCellsData(void);
vs600Data_t *smartportMasterGetVS600Data(void);

#endif /* USE_SMARTPORT_MASTER */
