/*
 * This file is part of INAV.
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
 *
 * @author Konstantin Sharlaimov <konstantin.sharlaimov@gmail.com>
 */

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "platform.h"
#include "build/build_config.h"
#include "drivers/rangefinder/rangefinder.h"

#ifdef USE_UAV_INTERCONNECT

#define UIB_MAX_PACKET_SIZE 32

#define UIB_DEVICE_ID_RANGEFINDER       0x12
#define UIB_DEVICE_ID_GPS               0x13
#define UIB_DEVICE_ID_COMPASS           0x14
#define UIB_DEVICE_ID_RC_RECEIVER       0x80

typedef enum {
    UIB_DATA_NONE           = 0,
    UIB_DATA_VALID          = (1 << 0),     // Data is valid
} uibDataFlags_t;

/* Bus task */
bool uavInterconnectBusCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime);
void uavInterconnectBusTask(timeUs_t currentTimeUs);
void uavInterconnectBusInit(void);
bool uavInterconnectBusIsInitialized(void);

/* Bus device API */
bool uibRegisterDevice(uint8_t devId);
bool uibDetectAndActivateDevice(uint8_t devId);
bool uibGetDeviceParams(uint8_t devId, uint8_t * params);
timeUs_t uibGetPollRateUs(uint8_t devId);
uint32_t uibGetUnansweredRequests(uint8_t devId);
uint8_t uibDataAvailable(uint8_t devId);
uint8_t uibRead(uint8_t devId, uint8_t * buffer, uint8_t bufSize);
bool uibCanWrite(uint8_t devId);
bool uibWrite(uint8_t devId, const uint8_t * buffer, uint8_t bufSize);

#define RANGEFINDER_UIB_TASK_PERIOD_MS  100
bool uibRangefinderDetect(rangefinderDev_t *dev);

#endif
