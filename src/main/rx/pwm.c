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

#if defined(USE_RX_PWM) || defined(USE_RX_PPM)

#include "common/utils.h"

#include "config/feature.h"

#include "drivers/rx_pwm.h"
#include "drivers/time.h"

#include "fc/config.h"

#include "rx/rx.h"
#include "rx/pwm.h"

#define RC_PWM_50HZ_UPDATE          (20 * 1000)     // 50Hz update rate period

#if MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT > MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT
    #define PWM_RX_CHANNEL_COUNT    MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT
#else
    #define PWM_RX_CHANNEL_COUNT    MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT
#endif

static uint16_t channelData[PWM_RX_CHANNEL_COUNT];
static timeUs_t lastReceivedFrameUs;

static uint16_t readRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t channel)
{
    UNUSED(rxRuntimeConfig);
    return channelData[channel];
}

static uint8_t pwmFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);

    timeUs_t currentTimeUs = micros();

    // PWM doesn't indicate individual updates, if low level code indicates that we have valid signal
    // we mimic the update at 50Hz rate

    if (isPWMDataBeingReceived()) {
        if ((currentTimeUs - lastReceivedFrameUs) >= RC_PWM_50HZ_UPDATE) {
            lastReceivedFrameUs = currentTimeUs;

            for (int channel = 0; channel < MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT; channel++) {
                channelData[channel] = pwmRead(channel);
            }

            return RX_FRAME_COMPLETE;
        }
    }

    return RX_FRAME_PENDING;
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

void rxPwmInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    rxRuntimeConfig->rxRefreshRate = RC_PWM_50HZ_UPDATE;
    rxRuntimeConfig->requireFiltering = true;

    // configure PWM/CPPM read function and max number of channels. serial rx below will override both of these, if enabled
    if (rxConfig->receiverType == RX_TYPE_PWM) {
        rxRuntimeConfig->channelCount = MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT;
        rxRuntimeConfig->rcReadRawFn = readRawRC;
        rxRuntimeConfig->rcFrameStatusFn = pwmFrameStatus;
    } else if (rxConfig->receiverType == RX_TYPE_PPM) {
        rxRuntimeConfig->channelCount = MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT;
        rxRuntimeConfig->rcReadRawFn = readRawRC;
        rxRuntimeConfig->rcFrameStatusFn = ppmFrameStatus;
    }
}
#endif

