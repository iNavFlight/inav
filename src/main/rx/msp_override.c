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

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"

#include "flight/failsafe.h"
#include "flight/pid.h"

#include "rx/rx.h"
#include "rx/msp.h"
#include "rx/msp_override.h"


#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)

static bool rxDataProcessingRequired = false;

static bool rxSignalReceived = false;
static bool rxFlightChannelsValid = false;
static bool rxFailsafe = true;

static timeMs_t rxDataFailurePeriod;
static timeMs_t rxDataRecoveryPeriod;
static timeMs_t validRxDataReceivedAt = 0;
static timeMs_t validRxDataFailedAt = 0;

static timeUs_t rxNextUpdateAtUs = 0;
static timeUs_t needRxSignalBefore = 0;

static uint32_t mspOverrideCtrlChannels = 0; // bitmask representing which channels are used to control MSP override
static rcChannel_t mspRcChannels[MAX_SUPPORTED_RC_CHANNEL_COUNT];

static rxRuntimeConfig_t rxRuntimeConfigMSP;

typedef struct mspFlightAxisOverride_s {
    int angleTarget;
    int rateTarget;
    uint8_t angleTargetActive;
    uint8_t rateTargetActive;
} mspFlightAxisOverride_t;

static mspFlightAxisOverride_t mspFlightAxisOverride[XYZ_AXIS_COUNT];

void mspOverrideInit(void)
{
    timeMs_t nowMs = millis();

    for (int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
        mspRcChannels[i].raw = PWM_RANGE_MIDDLE;
        mspRcChannels[i].data = PWM_RANGE_MIDDLE;
        mspRcChannels[i].expiresAt = nowMs + MAX_INVALID_RX_PULSE_TIME;
    }

    mspRcChannels[THROTTLE].raw = (feature(FEATURE_REVERSIBLE_MOTORS)) ? PWM_RANGE_MIDDLE : rxConfig()->rx_min_usec;
    mspRcChannels[THROTTLE].data = mspRcChannels[THROTTLE].raw;

    // Initialize ARM switch to OFF position when arming via switch is defined
    for (int i = 0; i < MAX_MODE_ACTIVATION_CONDITION_COUNT; i++) {
        if (modeActivationConditions(i)->modeId == BOXARM && IS_RANGE_USABLE(&modeActivationConditions(i)->range)) {
            // ARM switch is defined, determine an OFF value
            uint16_t value;
            if (modeActivationConditions(i)->range.startStep > 0) {
                value = MODE_STEP_TO_CHANNEL_VALUE((modeActivationConditions(i)->range.startStep - 1));
            } else {
                value = MODE_STEP_TO_CHANNEL_VALUE((modeActivationConditions(i)->range.endStep + 1));
            }
            // Initialize ARM AUX channel to OFF value
            rcChannel_t *armChannel = &mspRcChannels[modeActivationConditions(i)->auxChannelIndex + NON_AUX_CHANNEL_COUNT];
            armChannel->raw = value;
            armChannel->data = value;
        }

        // Find which channels are used to control MSP override
        if (modeActivationConditions(i)->modeId == BOXMSPRCOVERRIDE && IS_RANGE_USABLE(&modeActivationConditions(i)->range)) {
            mspOverrideCtrlChannels |= 1 << (modeActivationConditions(i)->auxChannelIndex + NON_AUX_CHANNEL_COUNT);
        }
    }

    rxDataFailurePeriod = PERIOD_RXDATA_FAILURE + failsafeConfig()->failsafe_delay * MILLIS_PER_TENTH_SECOND;
    rxDataRecoveryPeriod = PERIOD_RXDATA_RECOVERY + failsafeConfig()->failsafe_recovery_delay * MILLIS_PER_TENTH_SECOND;

    rxMspInit(rxConfig(), &rxRuntimeConfigMSP);

    for (uint8_t axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        mspFlightAxisOverride[axis].angleTargetActive = 0;
        mspFlightAxisOverride[axis].rateTargetActive = 0;
        mspFlightAxisOverride[axis].angleTarget = 0;
        mspFlightAxisOverride[axis].rateTarget = 0;
    }
}

bool mspOverrideIsReceivingSignal(void)
{
    return rxSignalReceived;
}

bool mspOverrideAreFlightChannelsValid(void)
{
    return rxFlightChannelsValid;
}

bool mspOverrideIsInFailsafe(void)
{
    return rxFailsafe;
}

static bool mspFlightAxisOverridesEnabled(void)
{
    return IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE) && !mspOverrideIsInFailsafe();
}

bool mspOverrideUpdateCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime)
{
    UNUSED(currentDeltaTime);

    if (rxSignalReceived) {
        if (currentTimeUs >= needRxSignalBefore) {
            rxSignalReceived = false;
        }
    }

    // Changing receiver_type from MSP to anything can cause a race with
    // the function pointer as NULL. This will not end well, so bail out early.
    if(rxRuntimeConfigMSP.rcFrameStatusFn == NULL) {
	return false;
    }

    const uint8_t frameStatus = rxRuntimeConfigMSP.rcFrameStatusFn(&rxRuntimeConfigMSP);
    if (frameStatus & RX_FRAME_COMPLETE) {
        rxDataProcessingRequired = true;
        rxSignalReceived = true;
        needRxSignalBefore = currentTimeUs + rxRuntimeConfigMSP.rxSignalTimeout;
    }

    if (cmpTimeUs(currentTimeUs, rxNextUpdateAtUs) > 0) {
        rxDataProcessingRequired = true;
    }

    return rxDataProcessingRequired; // data driven or 50Hz
}

bool mspOverrideCalculateChannels(timeUs_t currentTimeUs)
{
    int16_t rcStaging[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    const timeMs_t currentTimeMs = millis();

    if (!rxDataProcessingRequired) {
        return false;
    }

    rxDataProcessingRequired = false;
    rxNextUpdateAtUs = currentTimeUs + DELAY_50_HZ;

    rxFlightChannelsValid = true;

    // Read and process channel data
    for (int channel = 0; channel < rxRuntimeConfigMSP.channelCount; channel++) {
        const uint8_t rawChannel = calculateChannelRemapping(rxConfig()->rcmap, REMAPPABLE_CHANNEL_COUNT, channel);

        // sample the channel
        uint16_t sample = (*rxRuntimeConfigMSP.rcReadRawFn)(&rxRuntimeConfigMSP, rawChannel);

        // apply the rx calibration to flight channel
        if (channel < NON_AUX_CHANNEL_COUNT && sample != 0) {
            sample = scaleRange(sample, rxChannelRangeConfigs(channel)->min, rxChannelRangeConfigs(channel)->max, PWM_RANGE_MIN, PWM_RANGE_MAX);
            sample = MIN(MAX(PWM_PULSE_MIN, sample), PWM_PULSE_MAX);
        }

        // Store as rxRaw
        mspRcChannels[channel].raw = sample;

        // Apply invalid pulse value logic
        if (!isRxPulseValid(sample)) {
            sample = mspRcChannels[channel].data;   // hold channel, replace with old value
            if ((currentTimeMs > mspRcChannels[channel].expiresAt) && (channel < NON_AUX_CHANNEL_COUNT)) {
                rxFlightChannelsValid = false;
            }
        } else {
            mspRcChannels[channel].expiresAt = currentTimeMs + MAX_INVALID_RX_PULSE_TIME;
        }

        // Save channel value
        rcStaging[channel] = sample;
    }

    // Update channel input value if receiver is not in failsafe mode
    // If receiver is in failsafe (not receiving signal or sending invalid channel values) - last good input values are retained
    if (rxFlightChannelsValid && rxSignalReceived) {
        for (int channel = 0; channel < rxRuntimeConfigMSP.channelCount; channel++) {
            mspRcChannels[channel].data = rcStaging[channel];
        }
    }

    // Update failsafe
    if (rxFlightChannelsValid && rxSignalReceived) {
        validRxDataReceivedAt = millis();
        if ((validRxDataReceivedAt - validRxDataFailedAt) > rxDataRecoveryPeriod) {
            rxFailsafe = false;
        }
    } else {
        validRxDataFailedAt = millis();
        if ((validRxDataFailedAt - validRxDataReceivedAt) > rxDataFailurePeriod) {
            rxFailsafe = true;
        }
    }

    return true;
}

void mspOverrideChannels(rcChannel_t *rcChannels)
{
    for (uint32_t channel = 0, channelMask = 1; channel < rxRuntimeConfigMSP.channelCount; ++channel, channelMask <<= 1) {
        if (rxConfig()->mspOverrideChannels & ~mspOverrideCtrlChannels & channelMask) {
            rcChannels[channel].raw = rcChannels[channel].data = mspRcChannels[channel].data;
        }
    }
}

int16_t mspOverrideGetChannelValue(unsigned channelNumber)
{
    return mspRcChannels[channelNumber].data;
}

int16_t mspOverrideGetRawChannelValue(unsigned channelNumber)
{
    return mspRcChannels[channelNumber].raw;
}

void mspOverrideSetFlightAxisAngleOverride(uint8_t overrideMask, int16_t roll, int16_t pitch, int16_t yaw)
{
    const int16_t rollTarget = constrain(roll, -pidProfile()->max_angle_inclination[FD_ROLL], pidProfile()->max_angle_inclination[FD_ROLL]);
    const int16_t pitchTarget = constrain(pitch, -pidProfile()->max_angle_inclination[FD_PITCH], pidProfile()->max_angle_inclination[FD_PITCH]);
    const int16_t yawTarget = constrain(yaw, 0, 3600);

    mspFlightAxisOverride[FD_ROLL].angleTarget = rollTarget;
    mspFlightAxisOverride[FD_PITCH].angleTarget = pitchTarget;
    mspFlightAxisOverride[FD_YAW].angleTarget = yawTarget;

    mspFlightAxisOverride[FD_ROLL].angleTargetActive = (overrideMask & 0x01) ? 1 : 0;
    mspFlightAxisOverride[FD_PITCH].angleTargetActive = (overrideMask & 0x02) ? 1 : 0;
    mspFlightAxisOverride[FD_YAW].angleTargetActive = (overrideMask & 0x04) ? 1 : 0;
}

void mspOverrideSetFlightAxisRateOverride(uint8_t overrideMask, int16_t roll, int16_t pitch, int16_t yaw)
{
    const int16_t rollTarget = constrain(roll, -2000, 2000);
    const int16_t pitchTarget = constrain(pitch, -2000, 2000);
    const int16_t yawTarget = constrain(yaw, -2000, 2000);

    mspFlightAxisOverride[FD_ROLL].rateTarget = rollTarget;
    mspFlightAxisOverride[FD_PITCH].rateTarget = pitchTarget;
    mspFlightAxisOverride[FD_YAW].rateTarget = yawTarget;

    mspFlightAxisOverride[FD_ROLL].rateTargetActive = (overrideMask & 0x01) ? 1 : 0;
    mspFlightAxisOverride[FD_PITCH].rateTargetActive = (overrideMask & 0x02) ? 1 : 0;
    mspFlightAxisOverride[FD_YAW].rateTargetActive = (overrideMask & 0x04) ? 1 : 0;
}

bool mspOverrideFlightAxisAngleActive(uint8_t axis, int *target)
{
    if (!mspFlightAxisOverridesEnabled()) {
        return false;
    }

    if (axis >= XYZ_AXIS_COUNT) {
        return false;
    }

    if (!mspFlightAxisOverride[axis].angleTargetActive) {
        return false;
    }

    *target = mspFlightAxisOverride[axis].angleTarget;
    return true;
}

bool mspOverrideFlightAxisRateActive(uint8_t axis, int *target)
{
    if (!mspFlightAxisOverridesEnabled()) {
        return false;
    }

    if (axis >= XYZ_AXIS_COUNT) {
        return false;
    }

    if (!mspFlightAxisOverride[axis].rateTargetActive) {
        return false;
    }

    *target = mspFlightAxisOverride[axis].rateTarget;
    return true;
}

#endif // defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
