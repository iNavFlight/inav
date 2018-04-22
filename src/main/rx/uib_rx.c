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
#include <string.h>

#include "platform.h"

#if defined(USE_UAV_INTERCONNECT) && defined(USE_RX_UIB)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/utils.h"
#include "common/maths.h"

#include "rx/rx.h"
#include "rx/uib_rx.h"

#include "uav_interconnect/uav_interconnect.h"

#define UIB_DEVICE_ADDRESS      UIB_DEVICE_ID_RC_RECEIVER

typedef struct __attribute__((packed)) {
    uint8_t  flags;         // UIB_DATA_VALID (0x01) - link ok
    uint8_t  rssi;
    uint8_t  sticks[4];     // Values in range [0;255], center = 127
    uint8_t  aux[8];        // Analog AUX channels - values in range [0;255], center = 127
    uint16_t reserved;      // Reserved for future use
} rcUibReceiverData_t;

static rcUibReceiverData_t uibData;

#define UIB_RX_MAX_CHANNEL_COUNT    16

static uint16_t rxUIBReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfigPtr, uint8_t chan)
{
    UNUSED(rxRuntimeConfigPtr);

    switch (chan) {
        case 0 ... 3:
            return scaleRange(uibData.sticks[chan], 0, 255, 1000, 2000);

        case 4 ... 11:
            return scaleRange(uibData.aux[chan - 4], 0, 255, 1000, 2000);

        case 12:
        case 13:
        case 14:
            return 1500;

        case 15:
            return scaleRange(uibData.rssi, 0, 255, 1000, 2000);

        default:
            return 1500;
    }
}

static uint8_t rxUIBFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);

    if (!uavInterconnectBusIsInitialized()) {
        return RX_FRAME_FAILSAFE;
    }

    // If bus didn't detect the yet - report failure
    if (!uibDetectAndActivateDevice(UIB_DEVICE_ADDRESS)) {
        return RX_FRAME_FAILSAFE;
    }

    if (uibGetUnansweredRequests(UIB_DEVICE_ADDRESS) > 10) {         // Tolerate 200ms loss (10 packet loss)
        return RX_FRAME_FAILSAFE;
    }

    if (uibDataAvailable(UIB_DEVICE_ADDRESS)) {
        rcUibReceiverData_t uibDataTmp;
        uibRead(UIB_DEVICE_ADDRESS, (uint8_t*)&uibDataTmp, sizeof(uibDataTmp));

        if (!(uibDataTmp.flags & UIB_DATA_VALID))
            return RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;

        memcpy(&uibData, &uibDataTmp, sizeof(rcUibReceiverData_t));
        return RX_FRAME_COMPLETE;
    }
    else {
        return RX_FRAME_PENDING;
    }
}

void rxUIBInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxConfig);

    rxRuntimeConfig->channelCount = UIB_RX_MAX_CHANNEL_COUNT;
    rxRuntimeConfig->rxRefreshRate = 20000;
    rxRuntimeConfig->rxSignalTimeout = DELAY_5_HZ;
    rxRuntimeConfig->rcReadRawFn = rxUIBReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = rxUIBFrameStatus;
}
#endif
