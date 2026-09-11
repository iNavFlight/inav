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

#include <array>
#include <deque>
#include <vector>
#include <cstring>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "common/crc.h"
#include "drivers/time.h"
#include "drivers/vtx_common.h"
#include "io/serial.h"
#include "io/vtx.h"
#include "io/vtx_control.h"
#include "io/vtx_tramp.h"
vtxSettingsConfig_t vtxSettingsConfig_System;
vtxConfig_t vtxConfig_System;
}

static vtxDevice_t *device;
static serialPort_t port;
static serialPortConfig_t portConfig;
static std::deque<uint8_t> received;
static std::vector<std::array<uint8_t, 16>> sent;
static timeMs_t now;
static uint16_t reportedMax, actualPower, actualFrequency;
static bool actualPit, ignoreNextPit, corruptNextStatus;

static void respond(char command, uint16_t a, uint16_t b, uint16_t c)
{
    std::array<uint8_t, 16> frame{};
    frame[0] = 15; frame[1] = command;
    frame[2] = a; frame[3] = a >> 8;
    frame[4] = b; frame[5] = b >> 8;
    frame[6] = c; frame[7] = c >> 8;
    if (command == 'v') frame[7] = actualPit;
    frame[14] = crc8_sum_update(0, frame.data() + 1, 13);
    if (command == 'v' && corruptNextStatus) {
        frame[14] ^= 1;
        corruptNextStatus = false;
    }
    received.insert(received.end(), frame.begin(), frame.end());
}

extern "C" {
timeMs_t millis(void) { return now; }
void vtxCommonSetDevice(vtxDevice_t *d) { device = d; }
serialPortConfig_t *findSerialPortConfig(serialPortFunction_e) { return &portConfig; }
serialPort_t *openSerialPort(serialPortIdentifier_e, serialPortFunction_e,
    serialReceiveCallbackPtr, void *, uint32_t, portMode_t, portOptions_t) { return &port; }
uint32_t serialRxBytesWaiting(const serialPort_t *) { return received.size(); }
uint8_t serialRead(serialPort_t *) { uint8_t b = received.front(); received.pop_front(); return b; }
void serialWriteBuf(serialPort_t *, const uint8_t *data, int size)
{
    EXPECT_EQ(16, size);
    if (size != 16) return;
    std::array<uint8_t, 16> frame;
    std::memcpy(frame.data(), data, size);
    EXPECT_EQ(crc8_sum_update(0, data + 1, 13), data[14]);
    EXPECT_EQ(0, data[15]);
    sent.push_back(frame);
    uint16_t value = data[2] | (data[3] << 8);
    switch (data[1]) {
    case 'r': respond('r', 5000, 5999, reportedMax); break;
    case 'v': respond('v', actualFrequency, actualPower, 0); break;
    case 'F': actualFrequency = value; break;
    case 'P': actualPower = value; break;
    case 'I':
        if (ignoreNextPit) ignoreNextPit = false;
        else actualPit = value == 0;
        break;
    }
}
}

class TrampTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::memset(&vtxSettingsConfig_System, 0, sizeof(vtxSettingsConfig_System));
        std::memset(&vtxConfig_System, 0, sizeof(vtxConfig_System));
        now = 0; reportedMax = 2500; actualPower = 25; actualFrequency = 5732;
        actualPit = false; ignoreNextPit = false; corruptNextStatus = false;
        received.clear(); sent.clear(); device = nullptr;
    }
    void tick(int n = 1) {
        for (int i = 0; i < n; ++i) { now += 250; device->vTable->process(device, now * 1000); }
    }
    void start() {
        ASSERT_TRUE(vtxTrampInit());
        tick(3); // reset, capability request, capability reply
        ASSERT_TRUE(device->vTable->isReady(device));
        device->vTable->setBandAndChannel(device, 5, 3);
        device->vTable->setPowerByIndex(device, 1);
        tick(24);
        sent.clear();
    }
    void custom() {
        const uint16_t powers[] = {25, 400, 1000, 2500, 0};
        std::memcpy(vtxSettingsConfig_System.trampPowerLevels, powers, sizeof(powers));
    }
    int commands(char command, int value = -1) {
        int n = 0;
        for (auto &f : sent) if (f[1] == command && (value < 0 || (f[2] | (f[3] << 8)) == value)) ++n;
        return n;
    }
};

TEST_F(TrampTest, DefaultTableRemainsAutomatic) {
    start(); EXPECT_EQ(5, device->capability.powerCount);
    EXPECT_STREQ("800", device->capability.powerNames[5]);
}
TEST_F(TrampTest, CustomTableUsesAllFourBlitzPowers) {
    custom(); start(); ASSERT_EQ(4, device->capability.powerCount);
    const char *labels[] = {"25", "400", "1000", "2500"};
    const int powers[] = {25, 400, 1000, 2500};
    for (int i = 0; i < 4; i++) {
        EXPECT_STREQ(labels[i], device->capability.powerNames[i + 1]);
        device->vTable->setPowerByIndex(device, i + 1); tick(20);
        EXPECT_EQ(powers[i], actualPower);
    }
}
TEST_F(TrampTest, CustomPowerStillRespectsReportedLimit) {
    custom(); reportedMax = 400; start();
    device->vTable->setPowerByIndex(device, 4); tick(20); EXPECT_EQ(400, actualPower);
}
TEST_F(TrampTest, ExplicitOverrideAllowsConfiguredMaximum) {
    custom(); reportedMax = 400; vtxSettingsConfig_System.maxPowerOverride = 2500; start();
    device->vTable->setPowerByIndex(device, 4); tick(20); EXPECT_EQ(2500, actualPower);
}
TEST_F(TrampTest, InvalidIndexDoesNotTransmitPower) {
    custom(); start(); device->vTable->setPowerByIndex(device, 0);
    device->vTable->setPowerByIndex(device, 5); tick(20); EXPECT_EQ(0, commands('P'));
}
TEST_F(TrampTest, HoleInCustomTableFallsBackToAutomatic) {
    custom(); vtxSettingsConfig_System.trampPowerLevels[1] = 0; start(); EXPECT_EQ(5, device->capability.powerCount);
}
TEST_F(TrampTest, DecreasingCustomTableFallsBackToAutomatic) {
    custom(); vtxSettingsConfig_System.trampPowerLevels[2] = 200; start(); EXPECT_EQ(5, device->capability.powerCount);
}
TEST_F(TrampTest, DuplicateCustomTableFallsBackToAutomatic) {
    custom(); vtxSettingsConfig_System.trampPowerLevels[2] = 400; start(); EXPECT_EQ(5, device->capability.powerCount);
}
TEST_F(TrampTest, OutOfRangeCustomTableFallsBackToAutomatic) {
    custom(); vtxSettingsConfig_System.trampPowerLevels[3] = 10001; start(); EXPECT_EQ(5, device->capability.powerCount);
}
TEST_F(TrampTest, FiveDigitLabelFits) {
    custom(); vtxSettingsConfig_System.trampPowerLevels[4] = 10000; start();
    EXPECT_EQ(5, device->capability.powerCount); EXPECT_STREQ("10000", device->capability.powerNames[5]);
}
TEST_F(TrampTest, PitModeUsesCorrectCommandAndPolarity) {
    start(); device->vTable->setPitMode(device, 1); tick(20);
    EXPECT_TRUE(actualPit); EXPECT_GT(commands('I', 0), 0);
    device->vTable->setPitMode(device, 0); tick(20);
    EXPECT_FALSE(actualPit); EXPECT_GT(commands('I', 1), 0);
    EXPECT_EQ(0, commands('s'));
}
TEST_F(TrampTest, LostPitCommandIsRetriedAfterStatusMismatch) {
    start(); ignoreNextPit = true; device->vTable->setPitMode(device, 1); tick(40);
    EXPECT_TRUE(actualPit); EXPECT_GE(commands('I', 0), 2);
}
TEST_F(TrampTest, LastPitRequestWinsBeforeTransmission) {
    start(); device->vTable->setPitMode(device, 1); device->vTable->setPitMode(device, 0); tick(20);
    EXPECT_FALSE(actualPit); EXPECT_EQ(0, commands('I', 0));
}
TEST_F(TrampTest, HardwarePitModeIsPreservedWithoutExplicitRequest) {
    actualPit = true; start(); tick(20); EXPECT_TRUE(actualPit); EXPECT_EQ(0, commands('I'));
}
TEST_F(TrampTest, CorruptStatusCannotChangeReportedPitMode) {
    start(); uint8_t pit = 1;
    actualPit = true; corruptNextStatus = true;
    for (int i = 0; i < 30 && corruptNextStatus; ++i) tick();
    ASSERT_FALSE(corruptNextStatus);
    tick(); // consume and reject the corrupt response
    EXPECT_TRUE(device->vTable->getPitMode(device, &pit)); EXPECT_EQ(0, pit);
    tick(20); // subsequent valid polls recover normally
    EXPECT_TRUE(device->vTable->getPitMode(device, &pit)); EXPECT_EQ(1, pit);
}
