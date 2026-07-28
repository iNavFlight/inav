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

#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/utils.h"

#include "programming/logic_condition.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"


#include "drivers/adc.h"
#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/settings.h"

#include "flight/failsafe.h"

#include "io/serial.h"

#include "rx/rx.h"
#include "rx/crsf.h"
#include "rx/ibus.h"
#include "rx/jetiexbus.h"
#include "rx/fport.h"
#include "rx/fport2.h"
#include "rx/msp.h"
#include "rx/msp_override.h"
#include "rx/sbus.h"
#include "rx/spektrum.h"
#include "rx/srxl2.h"
#include "rx/sumd.h"
#include "rx/ghst.h"
#include "rx/mavlink.h"
#include "rx/sim.h"

const char rcChannelLetters[] = "AERT";

static uint16_t rssi = 0;                  // range: [0;1023]
static timeUs_t lastMspRssiUpdateUs = 0;

#define MSP_RSSI_TIMEOUT_US     1500000   // 1.5 sec
#define RX_LQ_INTERVAL_MS       200
#define RX_LQ_TIMEOUT_MS        1000

static rssiSource_e activeRssiSource;

static bool rxSignalReceived = false;
static bool rxFlightChannelsValid = false;

static bool isRxSuspended = false;
static bool dualRxEnabled = false;

typedef struct rxLinkState_s {
    rxRuntimeConfig_t runtimeConfig;
    rxLinkStatistics_t statistics;
    rxLinkQualityTracker_e linkQuality;
    rcChannel_t channels[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    uint16_t channelData[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    bool dataProcessingRequired;
    bool auxiliaryProcessingRequired;
    bool signalReceived;
    bool flightChannelsValid;
    uint8_t frameStatus;
    timeUs_t nextUpdateAtUs;
    timeUs_t needRxSignalBefore;
} rxLinkState_t;

static rxLinkState_t rxLinks[RX_LINK_COUNT];
static rxLink_e activeLink = RX_LINK_PRIMARY;

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
static bool mspOverrideDataProcessingRequired = false;
#endif

static rcChannel_t rcChannels[MAX_SUPPORTED_RC_CHANNEL_COUNT];

// MSP aux channel overlay: non-zero values override rcChannels[].data for CH9-CH32
static uint16_t mspAuxOverlay[MAX_SUPPORTED_RC_CHANNEL_COUNT];

rxLinkStatistics_t rxLinkStatistics;
rxRuntimeConfig_t rxRuntimeConfig;
static uint8_t rcSampleIndex = 0;

PG_REGISTER_WITH_RESET_TEMPLATE(rxConfig_t, rxConfig, PG_RX_CONFIG, 14);

#ifndef SERIALRX_PROVIDER
#define SERIALRX_PROVIDER 0
#endif

#ifndef DEFAULT_RX_TYPE
#define DEFAULT_RX_TYPE   RX_TYPE_NONE
#endif

#define RX_MIN_USEX 885
PG_RESET_TEMPLATE(rxConfig_t, rxConfig,
    .receiverType = DEFAULT_RX_TYPE,
    .dualRxEnabled = 0,
    .rcmap = {0, 1, 3, 2},      // Default to AETR map
    .halfDuplex = SETTING_SERIALRX_HALFDUPLEX_DEFAULT,
    .receiverTypeSecondary = RX_TYPE_NONE,
#ifdef USE_SERIAL_RX
    .serialrx_provider = SERIALRX_PROVIDER,
    .serialrx_inverted = SETTING_SERIALRX_INVERTED_DEFAULT,
    .serialrx_provider_secondary = SERIALRX_PROVIDER,
    .serialrx_inverted_secondary = SETTING_SERIALRX_INVERTED_DEFAULT,
    .halfDuplexSecondary = SETTING_SERIALRX_HALFDUPLEX_DEFAULT,
    .sbusSyncIntervalSecondary = SETTING_SBUS_SYNC_INTERVAL_DEFAULT,
#else
    .serialrx_provider = SERIALRX_PROVIDER,
    .serialrx_inverted = 0,
    .serialrx_provider_secondary = SERIALRX_PROVIDER,
    .serialrx_inverted_secondary = 0,
    .halfDuplexSecondary = 0,
    .sbusSyncIntervalSecondary = SETTING_SBUS_SYNC_INTERVAL_DEFAULT,
#endif
#ifdef USE_SPEKTRUM_BIND
    .spektrum_sat_bind = SETTING_SPEKTRUM_SAT_BIND_DEFAULT,
#endif
    .mincheck = SETTING_MIN_CHECK_DEFAULT,
    .maxcheck = SETTING_MAX_CHECK_DEFAULT,
    .rx_min_usec = SETTING_RX_MIN_USEC_DEFAULT,          // any of first 4 channels below this value will trigger rx loss detection
    .rx_max_usec = SETTING_RX_MAX_USEC_DEFAULT,          // any of first 4 channels above this value will trigger rx loss detection
    .rssi_channel = SETTING_RSSI_CHANNEL_DEFAULT,
    .rssiMin = SETTING_RSSI_MIN_DEFAULT,
    .rssiMax = SETTING_RSSI_MAX_DEFAULT,
    .sbusSyncInterval = SETTING_SBUS_SYNC_INTERVAL_DEFAULT,
    .rcFilterFrequency = SETTING_RC_FILTER_LPF_HZ_DEFAULT,
    .autoSmooth = SETTING_RC_FILTER_AUTO_DEFAULT,
    .autoSmoothFactor = SETTING_RC_FILTER_SMOOTHING_FACTOR_DEFAULT,
#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    .mspOverrideChannels = SETTING_MSP_OVERRIDE_CHANNELS_DEFAULT,
#endif
    .rssi_source = SETTING_RSSI_SOURCE_DEFAULT,
#ifdef USE_SERIALRX_SRXL2
    .srxl2_unit_id = SETTING_SRXL2_UNIT_ID_DEFAULT,
    .srxl2_baud_fast = SETTING_SRXL2_BAUD_FAST_DEFAULT,
#endif
);

void resetAllRxChannelRangeConfigurations(void)
{
    // set default calibration to full range and 1:1 mapping
    for (int i = 0; i < NON_AUX_CHANNEL_COUNT; i++) {
        rxChannelRangeConfigsMutable(i)->min = PWM_RANGE_MIN;
        rxChannelRangeConfigsMutable(i)->max = PWM_RANGE_MAX;
    }
}

PG_REGISTER_ARRAY_WITH_RESET_FN(rxChannelRangeConfig_t, NON_AUX_CHANNEL_COUNT, rxChannelRangeConfigs, PG_RX_CHANNEL_RANGE_CONFIG, 0);

void pgResetFn_rxChannelRangeConfigs(rxChannelRangeConfig_t *rxChannelRangeConfigs)
{
    // set default calibration to full range and 1:1 mapping
    for (int i = 0; i < NON_AUX_CHANNEL_COUNT; i++) {
        rxChannelRangeConfigs[i].min = PWM_RANGE_MIN;
        rxChannelRangeConfigs[i].max = PWM_RANGE_MAX;
    }
}

static uint16_t nullReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t channel)
{
    UNUSED(rxRuntimeConfig);
    UNUSED(channel);

    return 0;
}

static uint8_t nullFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);
    return RX_FRAME_PENDING;
}

static bool nullProcessFrame(const rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);
    return true;
}

bool isRxPulseValid(uint16_t pulseDuration)
{
    return  pulseDuration >= rxConfig()->rx_min_usec &&
            pulseDuration <= rxConfig()->rx_max_usec;
}

static void rxInitChannelState(rcChannel_t *channels, timeMs_t nowMs)
{
    for (int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
        channels[i].raw = PWM_RANGE_MIDDLE;
        channels[i].data = PWM_RANGE_MIDDLE;
        channels[i].expiresAt = nowMs + MAX_INVALID_RX_PULSE_TIME;
    }

    channels[THROTTLE].raw = feature(FEATURE_REVERSIBLE_MOTORS) ? PWM_RANGE_MIDDLE : rxConfig()->rx_min_usec;
    channels[THROTTLE].data = channels[THROTTLE].raw;

    for (int i = 0; i < MAX_MODE_ACTIVATION_CONDITION_COUNT; i++) {
        if (modeActivationConditions(i)->modeId == BOXARM && IS_RANGE_USABLE(&modeActivationConditions(i)->range)) {
            uint16_t value;
            if (modeActivationConditions(i)->range.startStep > 0) {
                value = MODE_STEP_TO_CHANNEL_VALUE((modeActivationConditions(i)->range.startStep - 1));
            } else {
                value = MODE_STEP_TO_CHANNEL_VALUE((modeActivationConditions(i)->range.endStep + 1));
            }
            rcChannel_t *armChannel = &channels[modeActivationConditions(i)->auxChannelIndex + NON_AUX_CHANNEL_COUNT];
            armChannel->raw = value;
            armChannel->data = value;
        }
    }
}

static void rxResetLinkState(rxLinkState_t *link, timeMs_t nowMs)
{
    lqTrackerReset(&link->linkQuality);
    memset(&link->statistics, 0, sizeof(link->statistics));
    memset(link->channelData, 0, sizeof(link->channelData));

    link->runtimeConfig.lqTracker = &link->linkQuality;
    link->runtimeConfig.linkStatistics = &link->statistics;
    link->runtimeConfig.rcReadRawFn = nullReadRawRC;
    link->runtimeConfig.rcFrameStatusFn = nullFrameStatus;
    link->runtimeConfig.rcProcessFrameFn = nullProcessFrame;
    link->runtimeConfig.rxSignalTimeout = DELAY_10_HZ;
    link->runtimeConfig.receiverType = RX_TYPE_NONE;
    link->runtimeConfig.channelData = link->channelData;
    link->runtimeConfig.channelCount = 0;
    link->runtimeConfig.frameData = NULL;

    link->dataProcessingRequired = false;
    link->auxiliaryProcessingRequired = false;
    link->signalReceived = false;
    link->flightChannelsValid = false;
    link->frameStatus = RX_FRAME_PENDING;
    link->nextUpdateAtUs = 0;
    link->needRxSignalBefore = 0;

    rxInitChannelState(link->channels, nowMs);
}

static void populateLinkConfig(rxConfig_t *dst, const rxConfig_t *src, rxLink_e link)
{
    *dst = *src;

#ifdef USE_SERIAL_RX
    if (link == RX_LINK_SECONDARY) {
        dst->receiverType = src->receiverTypeSecondary;
        dst->serialrx_provider = src->serialrx_provider_secondary;
        dst->serialrx_inverted = src->serialrx_inverted_secondary;
        dst->halfDuplex = src->halfDuplexSecondary;
        dst->sbusSyncInterval = src->sbusSyncIntervalSecondary;
    }
#else
    if (link == RX_LINK_SECONDARY) {
        dst->receiverType = RX_TYPE_NONE;
    }
    UNUSED(src);
#endif
}

#ifdef USE_SERIAL_RX
bool serialRxInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, serialPortFunction_e portFunction)
{
    bool enabled = false;
    switch (rxConfig->serialrx_provider) {
#ifdef USE_SERIALRX_SRXL2
    case SERIALRX_SRXL2:
        enabled = srxl2RxInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_SPEKTRUM
    case SERIALRX_SPEKTRUM1024:
    case SERIALRX_SPEKTRUM2048:
        enabled = spektrumInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_SBUS
    case SERIALRX_SBUS2:
    case SERIALRX_SBUS:
        enabled = sbusInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
    case SERIALRX_SBUS_FAST:
        enabled = sbusInitFast(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_SUMD
    case SERIALRX_SUMD:
        enabled = sumdInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_IBUS
    case SERIALRX_IBUS:
        enabled = ibusInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_JETIEXBUS
    case SERIALRX_JETIEXBUS:
        enabled = jetiExBusInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_CRSF
    case SERIALRX_CRSF:
        enabled = crsfRxInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_FPORT
    case SERIALRX_FPORT:
        enabled = fportRxInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_FPORT2
    case SERIALRX_FPORT2:
        enabled = fport2RxInit(rxConfig, rxRuntimeConfig, false, portFunction);
        break;
    case SERIALRX_FBUS:
        enabled = fport2RxInit(rxConfig, rxRuntimeConfig, true, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_GHST
    case SERIALRX_GHST:
        enabled = ghstRxInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
#ifdef USE_SERIALRX_MAVLINK
    case SERIALRX_MAVLINK:
        enabled = mavlinkRxInit(rxConfig, rxRuntimeConfig, portFunction);
        break;
#endif
    default:
        enabled = false;
        break;
    }
    return enabled;
}
#endif

// Identifies the driver backing a serial RX provider. Providers that map to the
// same group share that driver's module-static parser state, so two links in the
// same group alias each other unless the driver has been made instance-safe.
static uint8_t serialRxDriverGroup(uint8_t provider)
{
    switch (provider) {
    case SERIALRX_SBUS:
    case SERIALRX_SBUS_FAST:
    case SERIALRX_SBUS2:
        return SERIALRX_SBUS;
    case SERIALRX_FPORT2:
    case SERIALRX_FBUS:
        return SERIALRX_FPORT2;
    default:
        return provider;
    }
}

// Driver groups that carry per-link state and so can run on both links at once.
static bool serialRxDriverGroupIsDualLinkSafe(uint8_t group)
{
    return group == SERIALRX_SBUS || group == SERIALRX_CRSF;
}

void rxInit(void)
{
    rcSampleIndex = 0;
    dualRxEnabled = rxConfig()->dualRxEnabled;
    if (rxConfig()->receiverType == RX_TYPE_MSP || rxConfig()->receiverTypeSecondary == RX_TYPE_MSP) {
        dualRxEnabled = false;
    }
    if ((rxConfig()->receiverType == RX_TYPE_SERIAL && rxConfig()->serialrx_provider == SERIALRX_MAVLINK) ||
        (rxConfig()->receiverTypeSecondary == RX_TYPE_SERIAL && rxConfig()->serialrx_provider_secondary == SERIALRX_MAVLINK)) {
        dualRxEnabled = false;
    }
    // Two links sharing a not-yet-instance-safe serial driver would alias its module
    // parser buffers and corrupt both. Refuse dual RX for that combination; links on
    // different drivers are unaffected.
    if (rxConfig()->receiverType == RX_TYPE_SERIAL && rxConfig()->receiverTypeSecondary == RX_TYPE_SERIAL) {
        const uint8_t primaryGroup = serialRxDriverGroup(rxConfig()->serialrx_provider);
        const uint8_t secondaryGroup = serialRxDriverGroup(rxConfig()->serialrx_provider_secondary);
        if (primaryGroup == secondaryGroup && !serialRxDriverGroupIsDualLinkSafe(primaryGroup)) {
            dualRxEnabled = false;
        }
    }

    const timeMs_t nowMs = millis();

    for (int linkIndex = 0; linkIndex < RX_LINK_COUNT; linkIndex++) {
        rxResetLinkState(&rxLinks[linkIndex], nowMs);
    }

    rxConfig_t linkConfigs[RX_LINK_COUNT];
    populateLinkConfig(&linkConfigs[RX_LINK_PRIMARY], rxConfig(), RX_LINK_PRIMARY);
    populateLinkConfig(&linkConfigs[RX_LINK_SECONDARY], rxConfig(), RX_LINK_SECONDARY);

    if (!dualRxEnabled) {
        linkConfigs[RX_LINK_SECONDARY].receiverType = RX_TYPE_NONE;
    }

    for (int linkIndex = 0; linkIndex < RX_LINK_COUNT; linkIndex++) {
        rxLinkState_t *link = &rxLinks[linkIndex];
        rxConfig_t *linkConfig = &linkConfigs[linkIndex];
        link->runtimeConfig.receiverType = linkConfig->receiverType;

        switch (linkConfig->receiverType) {

#ifdef USE_SERIAL_RX
        case RX_TYPE_SERIAL:
        {
            const serialPortFunction_e linkPortFunction = linkIndex == RX_LINK_PRIMARY ? FUNCTION_RX_SERIAL : FUNCTION_RX_SERIAL_SECONDARY;
            if (!serialRxInit(linkConfig, &link->runtimeConfig, linkPortFunction)) {
                if (linkIndex == RX_LINK_PRIMARY) {
                    rxConfigMutable()->receiverType = RX_TYPE_NONE;
                } else {
                    rxConfigMutable()->receiverTypeSecondary = RX_TYPE_NONE;
                }
                link->runtimeConfig.receiverType = RX_TYPE_NONE;
                link->runtimeConfig.rcReadRawFn = nullReadRawRC;
                link->runtimeConfig.rcFrameStatusFn = nullFrameStatus;
                link->runtimeConfig.rcProcessFrameFn = nullProcessFrame;
            }
            break;
        }
#endif

#ifdef USE_RX_MSP
        case RX_TYPE_MSP:
            rxMspInit(linkConfig, &link->runtimeConfig);
            break;
#endif

#ifdef USE_RX_SIM
        case RX_TYPE_SIM:
            rxSimInit(linkConfig, &link->runtimeConfig);
            break;
#endif

        default:
        case RX_TYPE_NONE:
            link->runtimeConfig.receiverType = RX_TYPE_NONE;
            link->runtimeConfig.rcReadRawFn = nullReadRawRC;
            link->runtimeConfig.rcFrameStatusFn = nullFrameStatus;
            link->runtimeConfig.rcProcessFrameFn = nullProcessFrame;
            break;
        }

        link->runtimeConfig.channelCount = MIN(MAX_SUPPORTED_RC_CHANNEL_COUNT, link->runtimeConfig.channelCount);
    }

    rxUpdateRSSISource();

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    if (rxConfig()->receiverType != RX_TYPE_MSP) {
        mspOverrideInit();
    }
#endif

    rxSetActiveLink(RX_LINK_PRIMARY);
}

void rxUpdateRSSISource(void)
{
    activeRssiSource = RSSI_SOURCE_NONE;

    if (rxConfig()->rssi_source == RSSI_SOURCE_NONE) {
        return;
    }

#if defined(USE_ADC)
    if (rxConfig()->rssi_source == RSSI_SOURCE_ADC || rxConfig()->rssi_source == RSSI_SOURCE_AUTO) {
        if (feature(FEATURE_RSSI_ADC)) {
            activeRssiSource = RSSI_SOURCE_ADC;
            return;
        }
    }
#endif

    if (rxConfig()->rssi_source == RSSI_SOURCE_RX_CHANNEL || rxConfig()->rssi_source == RSSI_SOURCE_AUTO) {
        if (rxConfig()->rssi_channel > 0) {
            activeRssiSource = RSSI_SOURCE_RX_CHANNEL;
            return;
        }
    }

    if (rxConfig()->rssi_source == RSSI_SOURCE_RX_PROTOCOL || rxConfig()->rssi_source == RSSI_SOURCE_AUTO) {
        activeRssiSource = RSSI_SOURCE_RX_PROTOCOL;
        return;
    }
}

uint8_t calculateChannelRemapping(const uint8_t *channelMap, uint8_t channelMapEntryCount, uint8_t channelToRemap)
{
    if (channelToRemap < channelMapEntryCount) {
        return channelMap[channelToRemap];
    }
    return channelToRemap;
}

bool rxIsReceivingSignal(void)
{
    return rxSignalReceived && rxFlightChannelsValid;
}

bool rxAreFlightChannelsValid(void)
{
    return rxFlightChannelsValid;
}

rxLink_e rxGetActiveLink(void)
{
    return activeLink;
}

static bool rxLinkHasValidSignal(const rxLinkState_t *link)
{
    return link->signalReceived && link->flightChannelsValid;
}

bool rxIsLinkReceivingSignal(rxLink_e link)
{
    if ((unsigned)link >= RX_LINK_COUNT) {
        return false;
    }

    return rxLinkHasValidSignal(&rxLinks[link]);
}

bool rxIsPrimaryFailsafe(void)
{
    return !rxIsLinkReceivingSignal(RX_LINK_PRIMARY);
}

rxLinkStatistics_t *rxGetLinkStatisticsMutable(rxLink_e link)
{
    if ((unsigned)link >= RX_LINK_COUNT) {
        return NULL;
    }

    return &rxLinks[link].statistics;
}

void rxLinkStatisticsUpdated(rxLink_e link)
{
    if (link == activeLink) {
        rxLinkStatistics = rxLinks[link].statistics;
    }
}

static void rxApplyActiveLink(rxLink_e link)
{
    activeLink = link;

    rxRuntimeConfig = rxLinks[link].runtimeConfig;
    rxLinkStatistics = rxLinks[link].statistics;

    rxSignalReceived = rxLinks[link].signalReceived;
    rxFlightChannelsValid = rxLinks[link].flightChannelsValid;
    memcpy(rcChannels, rxLinks[link].channels, sizeof(rcChannels));
}

void rxSetActiveLink(rxLink_e link)
{
    if ((unsigned)link >= RX_LINK_COUNT) {
        return;
    }

    if (!dualRxEnabled && link != RX_LINK_PRIMARY) {
        return;
    }

    rxApplyActiveLink(link);
}

static void rxSelectActiveLink(void)
{
    if (!dualRxEnabled) {
        rxApplyActiveLink(RX_LINK_PRIMARY);
        return;
    }

    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RX_LINK)) {
        const rxLink_e forcedLink = logicConditionValuesByType[LOGIC_CONDITION_RX_SELECT_OVERRIDE] == RX_LINK_SECONDARY ? RX_LINK_SECONDARY : RX_LINK_PRIMARY;
        // Only honour the forced link while it actually has signal; otherwise fall
        // through to automatic selection so a dead forced link can't strand control
        // on stale channel data with failsafe suppressed.
        if (rxLinkHasValidSignal(&rxLinks[forcedLink])) {
            rxApplyActiveLink(forcedLink);
            return;
        }
    }

    const bool primaryValid = rxLinkHasValidSignal(&rxLinks[RX_LINK_PRIMARY]);
    const bool secondaryValid = rxLinkHasValidSignal(&rxLinks[RX_LINK_SECONDARY]);

    if (primaryValid) {
        rxApplyActiveLink(RX_LINK_PRIMARY);
    } else if (secondaryValid) {
        rxApplyActiveLink(RX_LINK_SECONDARY);
    } else {
        rxApplyActiveLink(RX_LINK_PRIMARY);
    }
}

void suspendRxSignal(void)
{
    failsafeOnRxSuspend();
    isRxSuspended = true;
}

void resumeRxSignal(void)
{
    isRxSuspended = false;
    failsafeOnRxResume();
}

bool rxUpdateCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime)
{
    UNUSED(currentDeltaTime);

    bool result = false;

    const int linkCount = dualRxEnabled ? RX_LINK_COUNT : 1;
    for (int linkIndex = 0; linkIndex < linkCount; linkIndex++) {
        rxLinkState_t *link = &rxLinks[linkIndex];

        if (link->signalReceived && currentTimeUs >= link->needRxSignalBefore) {
            link->signalReceived = false;
        }

        const uint8_t frameStatus = link->runtimeConfig.rcFrameStatusFn(&link->runtimeConfig);
        link->frameStatus = frameStatus;

        if (frameStatus & RX_FRAME_COMPLETE) {
            link->signalReceived = (frameStatus & RX_FRAME_FAILSAFE) == 0;
            link->needRxSignalBefore = currentTimeUs + link->runtimeConfig.rxSignalTimeout;
            link->dataProcessingRequired = true;
        } else if ((frameStatus & RX_FRAME_FAILSAFE) && link->signalReceived) {
            link->signalReceived = false;
        }

        if (frameStatus & RX_FRAME_PROCESSING_REQUIRED) {
            link->auxiliaryProcessingRequired = true;
        }

        if (cmpTimeUs(currentTimeUs, link->nextUpdateAtUs) > 0) {
            link->dataProcessingRequired = true;
        }

        result = result || link->dataProcessingRequired || link->auxiliaryProcessingRequired;
    }

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    if (rxRuntimeConfig.receiverType != RX_TYPE_MSP) {
        mspOverrideDataProcessingRequired = mspOverrideUpdateCheck(currentTimeUs, currentDeltaTime);
        result = result || mspOverrideDataProcessingRequired;
    }
#endif

    if (!result && !isRxSuspended) {
        // Re-select even when no frame needs processing so a link that has just
        // timed out hands control over without waiting for the next (absent) frame.
        rxSelectActiveLink();
    }

    return result;
}

bool calculateRxChannelsAndUpdateFailsafe(timeUs_t currentTimeUs)
{
    int16_t rcStaging[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    const timeMs_t currentTimeMs = millis();

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    if ((rxRuntimeConfig.receiverType != RX_TYPE_MSP) && mspOverrideDataProcessingRequired) {
        mspOverrideCalculateChannels(currentTimeUs);
    }
#endif

    bool processed = false;

    const int linkCount = dualRxEnabled ? RX_LINK_COUNT : 1;
    for (int linkIndex = 0; linkIndex < linkCount; linkIndex++) {
        rxLinkState_t *link = &rxLinks[linkIndex];

        if (link->auxiliaryProcessingRequired) {
            link->auxiliaryProcessingRequired = !link->runtimeConfig.rcProcessFrameFn(&link->runtimeConfig);
        }

        if (!link->dataProcessingRequired) {
            continue;
        }

        link->dataProcessingRequired = false;
        link->nextUpdateAtUs = currentTimeUs + DELAY_10_HZ;
        processed = true;

        if (isRxSuspended) {
            continue;
        }

        link->flightChannelsValid = true;

        const uint8_t channelCount = MIN(MAX_SUPPORTED_RC_CHANNEL_COUNT, link->runtimeConfig.channelCount);
        link->runtimeConfig.channelCount = channelCount;

        for (int channel = 0; channel < channelCount; channel++) {
            const uint8_t rawChannel = calculateChannelRemapping(rxConfig()->rcmap, REMAPPABLE_CHANNEL_COUNT, channel);

            uint16_t sample = (*link->runtimeConfig.rcReadRawFn)(&link->runtimeConfig, rawChannel);

            if (channel < NON_AUX_CHANNEL_COUNT && sample != 0) {
                sample = scaleRange(sample, rxChannelRangeConfigs(channel)->min, rxChannelRangeConfigs(channel)->max, PWM_RANGE_MIN, PWM_RANGE_MAX);
                sample = MIN(MAX(PWM_PULSE_MIN, sample), PWM_PULSE_MAX);
            }

            link->channels[channel].raw = sample;

            if (!isRxPulseValid(sample)) {
                sample = link->channels[channel].data;
                if ((currentTimeMs > link->channels[channel].expiresAt) && (channel < NON_AUX_CHANNEL_COUNT)) {
                    link->flightChannelsValid = false;
                }
            } else {
                link->channels[channel].expiresAt = currentTimeMs + MAX_INVALID_RX_PULSE_TIME;
            }

            rcStaging[channel] = sample;
        }

        if (link->flightChannelsValid && link->signalReceived) {
            for (int channel = 0; channel < channelCount; channel++) {
                link->channels[channel].data = rcStaging[channel];
            }
        }
    }

    if (isRxSuspended) {
        return processed;
    }

    rxSelectActiveLink();

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    if (IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE) && !mspOverrideIsInFailsafe()) {
        mspOverrideChannels(rcChannels);
    }
#endif

    // Apply MSP aux channel overlay (CH13-CH32)
    {
        int overlayStart = 12;
#ifdef USE_RX_MSP
        // When MSP is the primary RX, skip channels covered by MSP_SET_RAW_RC
        if (rxConfig()->receiverType == RX_TYPE_MSP) {
            const uint8_t mspChannels = rxMspGetLastChannelCount();
            if (mspChannels > overlayStart) {
                overlayStart = mspChannels;
            }
        }
#endif
        for (int i = overlayStart; i < 32; i++) {
            if (mspAuxOverlay[i] > 0) {
#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
                // Skip channels controlled by MSP RC Override when active
                if (IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE) && (rxConfig()->mspOverrideChannels & (1U << i))) {
                    continue;
                }
#endif
                rcChannels[i].data = mspAuxOverlay[i];
            }
        }
    }

    // Update failsafe — main failsafe only fires when both RX links are down
    if (rxIsLinkReceivingSignal(RX_LINK_PRIMARY) || (dualRxEnabled && rxIsLinkReceivingSignal(RX_LINK_SECONDARY))) {
        failsafeOnValidDataReceived();
    } else {
        failsafeOnValidDataFailed();
    }

    if (processed) {
        rcSampleIndex++;
    }

    return processed;
}

void parseRcChannels(const char *input)
{
    for (const char *c = input; *c; c++) {
        const char *s = strchr(rcChannelLetters, *c);
        if (s && (s < rcChannelLetters + MAX_MAPPABLE_RX_INPUTS))
            rxConfigMutable()->rcmap[s - rcChannelLetters] = c - input;
    }
}

#define RSSI_SAMPLE_COUNT 16

static void setRSSIValue(uint16_t rssiValue, rssiSource_e source, bool filtered)
{
    if (source != activeRssiSource) {
        return;
    }

    static uint16_t rssiSamples[RSSI_SAMPLE_COUNT];
    static uint8_t rssiSampleIndex = 0;
    static unsigned sum = 0;

    if (filtered) {
        // Value is already filtered
        rssi = rssiValue;

    } else {
        sum = sum + rssiValue;
        sum = sum - rssiSamples[rssiSampleIndex];
        rssiSamples[rssiSampleIndex] = rssiValue;
        rssiSampleIndex = (rssiSampleIndex + 1) % RSSI_SAMPLE_COUNT;

        int16_t rssiMean = sum / RSSI_SAMPLE_COUNT;

        rssi = rssiMean;
    }

    // Apply min/max values
    int rssiMin = rxConfig()->rssiMin * RSSI_VISIBLE_FACTOR;
    int rssiMax = rxConfig()->rssiMax * RSSI_VISIBLE_FACTOR;
    if (rssiMin > rssiMax) {
        int tmp = rssiMax;
        rssiMax = rssiMin;
        rssiMin = tmp;
        int delta = rssi >= rssiMin ? rssi - rssiMin : 0;
        rssi = rssiMax >= delta ? rssiMax - delta : 0;
    }
    rssi = constrain(scaleRange(rssi, rssiMin, rssiMax, 0, RSSI_MAX_VALUE), 0, RSSI_MAX_VALUE);
}

void setRSSIFromMSP_RC(uint8_t newMspRssi)
{
    if (activeRssiSource == RSSI_SOURCE_NONE && (rxConfig()->rssi_source == RSSI_SOURCE_MSP || rxConfig()->rssi_source == RSSI_SOURCE_AUTO)) {
        activeRssiSource = RSSI_SOURCE_MSP;
    }

    if (activeRssiSource == RSSI_SOURCE_MSP) {
        rssi = constrain(scaleRange(constrain(newMspRssi, 0, 100), 0, 100, 0, RSSI_MAX_VALUE), 0, RSSI_MAX_VALUE);
        lastMspRssiUpdateUs = micros();
    }
}

void setRSSIFromMSP(uint8_t newMspRssi)
{
    if (activeRssiSource == RSSI_SOURCE_NONE && (rxConfig()->rssi_source == RSSI_SOURCE_MSP || rxConfig()->rssi_source == RSSI_SOURCE_AUTO)) {
        activeRssiSource = RSSI_SOURCE_MSP;
    }

    if (activeRssiSource == RSSI_SOURCE_MSP) {
        rssi = ((uint16_t)newMspRssi) << 2;
        lastMspRssiUpdateUs = micros();
    }
}

static void updateRSSIFromChannel(void)
{
    if (rxConfig()->rssi_channel > 0) {
        int pwmRssi = rcChannels[rxConfig()->rssi_channel - 1].raw;
        int rawRSSI = (uint16_t)((constrain(pwmRssi - 1000, 0, 1000) / 1000.0f) * (RSSI_MAX_VALUE * 1.0f));
        setRSSIValue(rawRSSI, RSSI_SOURCE_RX_CHANNEL, false);
    }
}

static void updateRSSIFromADC(void)
{
#ifdef USE_ADC
    uint16_t rawRSSI = adcGetChannel(ADC_RSSI) / 4;    // Reduce to [0;1023]
    setRSSIValue(rawRSSI, RSSI_SOURCE_ADC, false);
#else
    setRSSIValue(0, RSSI_SOURCE_ADC, false);
#endif
}

static void updateRSSIFromProtocol(void)
{
    if (!rxRuntimeConfig.lqTracker) {
        return;
    }

    setRSSIValue(lqTrackerGet(rxRuntimeConfig.lqTracker), RSSI_SOURCE_RX_PROTOCOL, false);
}

void updateRSSI(timeUs_t currentTimeUs)
{
    // Read RSSI
    switch (activeRssiSource) {
    case RSSI_SOURCE_ADC:
        updateRSSIFromADC();
        break;
    case RSSI_SOURCE_RX_CHANNEL:
        updateRSSIFromChannel();
        break;
    case RSSI_SOURCE_RX_PROTOCOL:
        updateRSSIFromProtocol();
        break;
    case RSSI_SOURCE_MSP:
        if (cmpTimeUs(currentTimeUs, lastMspRssiUpdateUs) > MSP_RSSI_TIMEOUT_US) {
            rssi = 0;
        }
        break;
    default:
        rssi = 0;
        break;
    }
}

uint16_t getRSSI(void)
{
    return rssi;
}

rssiSource_e getRSSISource(void)
{
    return activeRssiSource;
}

int16_t rxGetChannelValue(unsigned channelNumber)
{
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL)) {
        return getRcChannelOverride(channelNumber, rcChannels[channelNumber].data);
    } else {
        return rcChannels[channelNumber].data;
    }
}

void rxMspAuxOverlaySet(uint8_t channelIndex, uint16_t value)
{
    if (channelIndex >= 12 && channelIndex < 32) {
        mspAuxOverlay[channelIndex] = value;
    }
}

void lqTrackerReset(rxLinkQualityTracker_e * lqTracker)
{
    lqTracker->lastUpdatedMs = millis();
    lqTracker->lqAccumulator = 0;
    lqTracker->lqCount = 0;
    lqTracker->lqValue = 0;
}

void lqTrackerAccumulate(rxLinkQualityTracker_e * lqTracker, uint16_t rawValue)
{
    const timeMs_t currentTimeMs = millis();

    if (((currentTimeMs - lqTracker->lastUpdatedMs) > RX_LQ_INTERVAL_MS) && lqTracker->lqCount) {
        lqTrackerSet(lqTracker, lqTracker->lqAccumulator / lqTracker->lqCount);
        lqTracker->lqAccumulator = 0;
        lqTracker->lqCount = 0;
    }

    lqTracker->lqAccumulator += rawValue;
    lqTracker->lqCount += 1;
}

void lqTrackerSet(rxLinkQualityTracker_e * lqTracker, uint16_t rawValue)
{
    lqTracker->lqValue = rawValue;
    lqTracker->lastUpdatedMs = millis();
}

uint16_t lqTrackerGet(rxLinkQualityTracker_e * lqTracker)
{
    if ((millis() - lqTracker->lastUpdatedMs) > RX_LQ_TIMEOUT_MS) {
        lqTracker->lqValue = 0;
    }

    return lqTracker->lqValue;
}

#ifdef UNIT_TEST
rxRuntimeConfig_t *rxTestRuntimeConfig(rxLink_e link)
{
    if ((unsigned)link >= RX_LINK_COUNT) {
        return NULL;
    }

    return &rxLinks[link].runtimeConfig;
}
#endif
