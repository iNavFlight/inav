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
#include <stdlib.h>

#include <string.h>

#include "platform.h"

#if defined(USE_RX_PPM)

#include "build/debug.h"
#include "common/utils.h"

#include "config/feature.h"

#include "drivers/timer.h"
#include "drivers/rx_pwm.h"
#include "drivers/time.h"

#include "fc/config.h"

#include "rx/rx.h"
#include "rx/pwm.h"

#define RC_PWM_50HZ_UPDATE          (20 * 1000)     // 50Hz update rate period

static uint16_t channelData[MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT];

static uint16_t readRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t channel)
{
    UNUSED(rxRuntimeConfig);
    return channelData[channel];
}

static uint8_t ppmFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);

    // PPM receiver counts received frames so we actually know if new data is available
    if (isPPMDataBeingReceived()) {
        for (int channel = 0; channel < MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT; channel++) {
            channelData[channel] = ppmRead(channel);
        }

        resetPPMDataReceivedState();
        return RX_FRAME_COMPLETE;
    }

    return RX_FRAME_PENDING;
}

bool rxPpmInit(rxRuntimeConfig_t *rxRuntimeConfig)
{
    const timerHardware_t * timHw = timerGetByUsageFlag(TIM_USE_PPM);

    if (timHw == NULL) {
        return false;
    }

    if (!ppmInConfig(timHw)) {
        return false;
    }

    rxRuntimeConfig->rxRefreshRate = RC_PWM_50HZ_UPDATE;
    rxRuntimeConfig->requireFiltering = true;
    rxRuntimeConfig->channelCount = MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT;
    rxRuntimeConfig->rcReadRawFn = readRawRC;
    rxRuntimeConfig->rcFrameStatusFn = ppmFrameStatus;

    return true;
}
#endif

