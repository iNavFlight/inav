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
#include <string.h>

#include "unittest_macros.h"
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"

#include "fc/rc_modes.h"
#include "fc/rc_controls.h"
#include "fc/settings.h"
#include "programming/logic_condition.h"
#include "rx/rx.h"
}

static uint32_t fakeTimeUs;
static uint32_t failsafeValidCount;
static uint32_t failsafeFailedCount;
static uint8_t frameStatusForLink[RX_LINK_COUNT];
static uint16_t channelValueForLink[RX_LINK_COUNT] = { PWM_RANGE_MIDDLE, PWM_RANGE_MIDDLE };

extern "C" {
    uint32_t micros(void)
    {
        return fakeTimeUs;
    }

    uint32_t millis(void)
    {
        return fakeTimeUs / 1000;
    }

    bool feature(uint32_t mask)
    {
        UNUSED(mask);
        return false;
    }

    int16_t getRcChannelOverride(uint8_t channelNumber, int16_t defaultValue)
    {
        UNUSED(channelNumber);
        return defaultValue;
    }

    uint64_t logicConditionsGlobalFlags;

    void failsafeOnValidDataReceived(void)
    {
        failsafeValidCount++;
    }

    void failsafeOnValidDataFailed(void)
    {
        failsafeFailedCount++;
    }

    void failsafeOnRxSuspend(void)
    {
    }

    void failsafeOnRxResume(void)
    {
    }

    void mspOverrideInit(void)
    {
    }

    bool mspOverrideUpdateCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime)
    {
        UNUSED(currentTimeUs);
        UNUSED(currentDeltaTime);
        return false;
    }

    void mspOverrideCalculateChannels(timeUs_t currentTimeUs)
    {
        UNUSED(currentTimeUs);
    }

    bool mspOverrideIsInFailsafe(void)
    {
        return false;
    }

    void mspOverrideChannels(rcChannel_t *channels)
    {
        UNUSED(channels);
    }

    uint32_t stateFlags;
    uint32_t armingFlags;
    int16_t rcCommand[4];

    rcControlsConfig_t rcControlsConfig_System;
    rcControlsConfig_t rcControlsConfig_Copy;
    const pgRegistry_t rcControlsConfig_Registry = {};

    // Programming framework state referenced by rxSelectActiveLink(). The override
    // flag is never set in these tests, so the values are only needed to link.
    int logicConditionValuesByType[LOGIC_CONDITION_LAST];

    // Physical RX drivers are not exercised by the dual-link tests (channels are
    // driven through the fake runtime configs), so the protocol init entry points
    // only need to satisfy the linker.
    bool srxl2RxInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool sbusInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool sbusInitFast(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool ibusInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool jetiExBusInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool crsfRxInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool fportRxInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    bool fport2RxInit(const rxConfig_t *, rxRuntimeConfig_t *, bool, serialPortFunction_e) { return false; }
    bool ghstRxInit(const rxConfig_t *, rxRuntimeConfig_t *, serialPortFunction_e) { return false; }
    void rxMspInit(const rxConfig_t *, rxRuntimeConfig_t *) {}
    uint8_t rxMspGetLastChannelCount(void) { return 0; }
    void rxSimInit(const rxConfig_t *, rxRuntimeConfig_t *) {}
}

static uint8_t primaryFrameStatusFn(rxRuntimeConfig_t *)
{
    return frameStatusForLink[RX_LINK_PRIMARY];
}

static uint8_t secondaryFrameStatusFn(rxRuntimeConfig_t *)
{
    return frameStatusForLink[RX_LINK_SECONDARY];
}

static uint16_t primaryReadFn(const rxRuntimeConfig_t *, uint8_t)
{
    return channelValueForLink[RX_LINK_PRIMARY];
}

static uint16_t secondaryReadFn(const rxRuntimeConfig_t *, uint8_t)
{
    return channelValueForLink[RX_LINK_SECONDARY];
}

class RxDualLinkTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fakeTimeUs = 0;
        failsafeValidCount = 0;
        failsafeFailedCount = 0;

        for (int i = 0; i < MAX_MODE_ACTIVATION_CONDITION_COUNT; i++) {
            modeActivationCondition_t *cond = modeActivationConditionsMutable(i);
            memset(cond, 0, sizeof(*cond));
        }

        rxConfig_t *cfg = rxConfigMutable();
        memset(cfg, 0, sizeof(*cfg));
        cfg->receiverType = RX_TYPE_NONE;
        cfg->rcmap[0] = 0;
        cfg->rcmap[1] = 1;
        cfg->rcmap[2] = 3;
        cfg->rcmap[3] = 2;
        cfg->serialrx_provider = 0;
        cfg->serialrx_inverted = 0;
        cfg->halfDuplex = SETTING_SERIALRX_HALFDUPLEX_DEFAULT;
        cfg->dualRxEnabled = true;
        cfg->mincheck = SETTING_MIN_CHECK_DEFAULT;
        cfg->maxcheck = SETTING_MAX_CHECK_DEFAULT;
        cfg->rx_min_usec = SETTING_RX_MIN_USEC_DEFAULT;
        cfg->rx_max_usec = SETTING_RX_MAX_USEC_DEFAULT;
        cfg->rssi_channel = SETTING_RSSI_CHANNEL_DEFAULT;
        cfg->rssiMin = SETTING_RSSI_MIN_DEFAULT;
        cfg->rssiMax = SETTING_RSSI_MAX_DEFAULT;
        cfg->sbusSyncInterval = SETTING_SBUS_SYNC_INTERVAL_DEFAULT;
        cfg->rcFilterFrequency = SETTING_RC_FILTER_LPF_HZ_DEFAULT;
        cfg->autoSmooth = SETTING_RC_FILTER_AUTO_DEFAULT;
        cfg->autoSmoothFactor = SETTING_RC_FILTER_SMOOTHING_FACTOR_DEFAULT;
        cfg->mspOverrideChannels = 0;
        cfg->rssi_source = SETTING_RSSI_SOURCE_DEFAULT;

        resetAllRxChannelRangeConfigurations();

        rxConfigMutable()->receiverType = RX_TYPE_NONE;
        rxInit();

        configureLink(RX_LINK_PRIMARY, primaryFrameStatusFn, primaryReadFn);
        configureLink(RX_LINK_SECONDARY, secondaryFrameStatusFn, secondaryReadFn);

        frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_PENDING;
        frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_PENDING;
    }

    void configureLink(rxLink_e link, rcFrameStatusFnPtr frameFn, rcReadRawDataFnPtr readFn)
    {
        rxRuntimeConfig_t *runtime = rxTestRuntimeConfig(link);
        runtime->channelCount = STICK_CHANNEL_COUNT;
        runtime->rcFrameStatusFn = frameFn;
        runtime->rcReadRawFn = readFn;
        runtime->receiverType = RX_TYPE_SERIAL;
    }

    void stepRx(void)
    {
        fakeTimeUs += 100;
        rxUpdateCheck(fakeTimeUs, 0);
        calculateRxChannelsAndUpdateFailsafe(fakeTimeUs);
    }
};

TEST_F(RxDualLinkTest, PrimarySwitchesToSecondaryOnFailsafe)
{
    frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_COMPLETE;
    frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_COMPLETE;

    stepRx();

    EXPECT_EQ(RX_LINK_PRIMARY, rxGetActiveLink());
    EXPECT_TRUE(rxIsLinkReceivingSignal(RX_LINK_PRIMARY));
    EXPECT_FALSE(rxIsPrimaryFailsafe());

    frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;
    frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_COMPLETE;

    stepRx();

    EXPECT_EQ(RX_LINK_SECONDARY, rxGetActiveLink());
    EXPECT_TRUE(rxIsLinkReceivingSignal(RX_LINK_SECONDARY));
    EXPECT_TRUE(rxIsPrimaryFailsafe());
    EXPECT_EQ(0U, failsafeFailedCount);
}

TEST_F(RxDualLinkTest, SecondaryTakesControlWhenPrimaryStartsInFailsafe)
{
    frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;
    frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_COMPLETE;

    stepRx();

    EXPECT_EQ(RX_LINK_SECONDARY, rxGetActiveLink());
    EXPECT_TRUE(rxIsLinkReceivingSignal(RX_LINK_SECONDARY));
    EXPECT_TRUE(rxIsPrimaryFailsafe());
    EXPECT_EQ(1U, failsafeValidCount);
}

TEST_F(RxDualLinkTest, MainFailsafeRequiresBothLinksDown)
{
    frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_COMPLETE;
    frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;

    stepRx();

    EXPECT_EQ(1U, failsafeValidCount);
    EXPECT_EQ(0U, failsafeFailedCount);
    EXPECT_TRUE(rxIsLinkReceivingSignal(RX_LINK_PRIMARY));

    frameStatusForLink[RX_LINK_PRIMARY] = RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;
    frameStatusForLink[RX_LINK_SECONDARY] = RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;

    stepRx();

    EXPECT_FALSE(rxIsLinkReceivingSignal(RX_LINK_PRIMARY));
    EXPECT_FALSE(rxIsLinkReceivingSignal(RX_LINK_SECONDARY));
    EXPECT_EQ(1U, failsafeFailedCount);
}
