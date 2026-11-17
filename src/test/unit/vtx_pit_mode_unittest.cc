/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "gtest/gtest.h"
extern "C" {
#include "platform.h"
#include "drivers/vtx_common.h"
#include "fc/runtime_config.h"
#include "fc/rc_modes.h"
#include "io/vtx.h"
uint32_t armingFlags;
uint8_t cliMode;
}
static bool assigned, receiving, modeActive, pit;
static int requests, powerRequests;
static uint8_t power;
static vtxDevType_e deviceType;
static vtxDevice_t device{};
extern "C" {
bool isModeActivationConditionPresent(boxId_e id) { return id == BOXVTXPITMODE && assigned; }
bool IS_RC_MODE_ACTIVE(boxId_e id) { return id == BOXVTXPITMODE && modeActive; }
bool rxIsReceivingSignal(void) { return receiving; }
bool failsafeIsActive(void) { return false; }
void vtxControlInputPoll(void) {}
vtxDevice_t *vtxCommonDevice(void) { return &device; }
bool vtxCommonGetPitMode(vtxDevice_t *, uint8_t *value) { *value = pit; return true; }
void vtxCommonSetPitMode(vtxDevice_t *, uint8_t value) { pit = value; ++requests; }
vtxDevType_e vtxCommonGetDeviceType(vtxDevice_t *) { return deviceType; }
bool vtxCommonGetPowerIndex(vtxDevice_t *, uint8_t *value) { *value = power; return true; }
void vtxCommonSetPowerByIndex(vtxDevice_t *d, uint8_t value) {
 if (value > d->capability.powerCount) return;
 power = value; ++powerRequests;
}
bool vtxCommonGetBandAndChannel(vtxDevice_t *, uint8_t *, uint8_t *) { return false; }
void vtxCommonSetBandAndChannel(vtxDevice_t *, uint8_t, uint8_t) {}
void vtxCommonProcess(vtxDevice_t *, timeUs_t) {}
}
class PitModeTest : public ::testing::Test {
protected:
 void SetUp() override {
  assigned = true; receiving = true; modeActive = false; pit = false;
  requests = 0; armingFlags = 0; cliMode = 0;
  device.capability.supportsPitMode = true;
  device.capability.powerCount = 3;
  deviceType = VTXDEV_TRAMP; power = 1; powerRequests = 0;
  vtxSettingsConfigMutable()->power = 1;
 }
 void update() { for (int i=0; i<3; ++i) vtxUpdate(0); }
};
TEST_F(PitModeTest, UnassignedModePreservesHardwarePitMode) {
 assigned=false; pit=true; update(); EXPECT_TRUE(pit); EXPECT_EQ(0,requests);
}
TEST_F(PitModeTest, AssignedSwitchControlsBothDirections) {
 modeActive=true; update(); EXPECT_TRUE(pit); EXPECT_EQ(1,requests);
 modeActive=false; update(); EXPECT_FALSE(pit); EXPECT_EQ(2,requests);
}
TEST_F(PitModeTest, ArmingExitsPitAndPreventsReentry) {
 modeActive=true; update(); ASSERT_TRUE(pit);
 armingFlags=ARMED; update(); EXPECT_FALSE(pit);
 update(); EXPECT_FALSE(pit); EXPECT_EQ(2,requests);
}
TEST_F(PitModeTest, ReceiverLossDoesNotActOnStaleSwitch) {
 receiving=false; modeActive=true; update(); EXPECT_FALSE(pit); EXPECT_EQ(0,requests);
}
TEST_F(PitModeTest, NoCommandWhenReportedStateMatchesSwitch) {
 update(); EXPECT_EQ(0,requests);
 modeActive=true; pit=true; update(); EXPECT_EQ(0,requests);
}
TEST_F(PitModeTest, CliDoesNotSendPitCommands) {
 cliMode=1; modeActive=true; update(); EXPECT_FALSE(pit); EXPECT_EQ(0,requests);
}

TEST_F(PitModeTest, UnsupportedDeviceDoesNotReceivePitRequests) {
 device.capability.supportsPitMode=false; modeActive=true; update();
 EXPECT_FALSE(pit); EXPECT_EQ(0,requests);
}
TEST_F(PitModeTest, SavedPowerAboveShorterTrampTableIsClampedOnce) {
 vtxSettingsConfigMutable()->power=5; update();
 EXPECT_EQ(3,power); EXPECT_EQ(1,powerRequests);
 update(); EXPECT_EQ(1,powerRequests);
 EXPECT_EQ(5,vtxSettingsConfig()->power);
}
TEST_F(PitModeTest, OtherDevicePowerSemanticsAreUnchanged) {
 deviceType=VTXDEV_SMARTAUDIO; vtxSettingsConfigMutable()->power=5; update();
 EXPECT_EQ(1,power); EXPECT_EQ(0,powerRequests);
}
