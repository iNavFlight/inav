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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_RX_SIM

#include "build/build_config.h"

#include "common/utils.h"

#include "rx/rx.h"
#include "rx/sim.h"

static uint16_t channels[MAX_SUPPORTED_RC_CHANNEL_COUNT];
static bool hasNewData = false;

static uint16_t rxSimReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfigPtr, uint8_t chan)
{
    UNUSED(rxRuntimeConfigPtr);
    return channels[chan];
}

void rxSimSetChannelValue(uint16_t* values, uint8_t count)
{
    for (size_t i = 0; i < count; i++) {    
        channels[i] = values[i];
    }

    hasNewData = true;
}

static uint8_t rxSimFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);
    
    if (!hasNewData) {    
        return RX_FRAME_PENDING;
    }

    hasNewData = false;
    return RX_FRAME_COMPLETE;
}

void rxSimInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxConfig);

    rxRuntimeConfig->channelCount = MAX_SUPPORTED_RC_CHANNEL_COUNT;
    rxRuntimeConfig->rxSignalTimeout = DELAY_5_HZ;
    rxRuntimeConfig->rcReadRawFn = rxSimReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = rxSimFrameStatus;
}
#endif
