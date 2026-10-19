/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>
#include <vector>

#include "gtest/gtest.h"
#include "unittest_macros.h"

extern "C" {
#include "platform.h"
#include "common/utils.h"
#include "config/mztc_camera.h"
#include "io/mztc_camera.h"
// The driver reads the THERMAL CALIBRATE switch, so the test provides
// IS_RC_MODE_ACTIVE and needs the box id enum.
#include "fc/rc_modes.h"

// Driver internals the receive-path tests drive directly. These are static in
// production builds and visible here through STATIC_UNIT_TESTED.
void mztcSerialReceiveCallback(uint16_t c, void *rxCallbackData);
void mztcSendConfiguration(void);

// Test hooks defined in the driver under UNIT_TEST.
void mztcTestReset(void);
void mztcTestForceReinit(void);
void mztcTestSetStatus(uint8_t status);
void mztcTestSetLastCalibration(uint16_t minutes);
}

/*
 * These tests exercise what can be checked without a camera attached: the
 * serial wire format and the configuration validator that the MSP set handler
 * relies on. The wire format is pinned against a packet captured from real
 * hardware. A regression in the framing math fails here, before it reaches a
 * bench.
 */

// Captured from a MassZero camera: read device model, no payload.
// begin, size, address, class, subclass, flags, checksum, end.
static const uint8_t READ_MODEL_PACKET[] = { 0xF0, 0x04, 0x36, 0x74, 0x02, 0x01, 0xAD, 0xFF };

// Whether the Ports tab has MZTC_CAMERA assigned to a UART.
static bool mztcPortAssigned;

// Whether the THERMAL CALIBRATE switch is held. The driver triggers one flat
// field correction on the rising edge.
static bool mztcCalibrateBoxActive;

class MztcFramingTest : public ::testing::Test {
protected:
    uint8_t packet[MZTC_MAX_PACKET_LEN];
};

TEST_F(MztcFramingTest, ZeroPayloadCommandMatchesHardwareCapture)
{
    const uint8_t len = mztcBuildPacket(packet, 0x74, 0x02, 0x01, NULL, 0);

    ASSERT_EQ(sizeof(READ_MODEL_PACKET), len);
    EXPECT_EQ(0, memcmp(READ_MODEL_PACKET, packet, len));
}

TEST_F(MztcFramingTest, TotalLengthIsPayloadPlusOverhead)
{
    const uint8_t payload[MZTC_MAX_DATA_LEN] = { 0 };

    for (uint8_t dataLen = 0; dataLen <= MZTC_MAX_DATA_LEN; dataLen++) {
        const uint8_t len = mztcBuildPacket(packet, 0x78, 0x02, 0x00, payload, dataLen);
        EXPECT_EQ(MZTC_PACKET_OVERHEAD + dataLen, len) << "data_len " << (int)dataLen;
    }
}

TEST_F(MztcFramingTest, SizeFieldIsPayloadPlusFour)
{
    const uint8_t payload[3] = { 0x11, 0x22, 0x33 };
    const uint8_t len = mztcBuildPacket(packet, 0x78, 0x02, 0x00, payload, sizeof(payload));

    ASSERT_NE(0, len);
    EXPECT_EQ(sizeof(payload) + MZTC_SIZE_FIELD_OFFSET, packet[1]);
    // The size field describes address through checksum. The wire length is the
    // size field plus the begin and end markers plus the size byte itself.
    EXPECT_EQ(packet[1] + 4, len);
}

TEST_F(MztcFramingTest, ChecksumAndTerminatorAreAlwaysPresent)
{
    const uint8_t payload[5] = { 0xF0, 0xFF, 0x00, 0xFF, 0xF0 };
    const uint8_t len = mztcBuildPacket(packet, 0x7C, 0x04, 0x00, payload, sizeof(payload));

    ASSERT_NE(0, len);
    EXPECT_EQ(MZTC_PACKET_END, packet[len - 1]);

    uint8_t expected = 0;
    for (uint8_t i = 2; i < (uint8_t)(len - 2); i++) {
        expected += packet[i];
    }
    EXPECT_EQ(expected, packet[len - 2]);
}

TEST_F(MztcFramingTest, PayloadBytesAreCopiedVerbatim)
{
    const uint8_t payload[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    const uint8_t len = mztcBuildPacket(packet, 0x78, 0x10, 0x00, payload, sizeof(payload));

    ASSERT_NE(0, len);
    EXPECT_EQ(0, memcmp(payload, &packet[6], sizeof(payload)));
}

TEST_F(MztcFramingTest, OversizedPayloadIsRejected)
{
    const uint8_t payload[MZTC_MAX_DATA_LEN + 1] = { 0 };

    EXPECT_EQ(0, mztcBuildPacket(packet, 0x78, 0x02, 0x00, payload, MZTC_MAX_DATA_LEN + 1));
}

TEST_F(MztcFramingTest, NullPayloadWithNonZeroLengthIsRejected)
{
    EXPECT_EQ(0, mztcBuildPacket(packet, 0x78, 0x02, 0x00, NULL, 4));
}

TEST_F(MztcFramingTest, BuiltPacketsValidate)
{
    const uint8_t payload[MZTC_MAX_DATA_LEN] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

    for (uint8_t dataLen = 0; dataLen <= MZTC_MAX_DATA_LEN; dataLen++) {
        const uint8_t len = mztcBuildPacket(packet, 0x74, 0x02, 0x03, payload, dataLen);
        ASSERT_NE(0, len);
        EXPECT_TRUE(mztcPacketIsValid(packet, len)) << "data_len " << (int)dataLen;
    }
}

TEST_F(MztcFramingTest, HardwareCaptureValidates)
{
    EXPECT_TRUE(mztcPacketIsValid(READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET)));
}

TEST_F(MztcFramingTest, CorruptChecksumIsRejected)
{
    memcpy(packet, READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET));
    packet[6] ^= 0xFF;

    EXPECT_FALSE(mztcPacketIsValid(packet, sizeof(READ_MODEL_PACKET)));
}

TEST_F(MztcFramingTest, WrongDeviceAddressIsRejected)
{
    memcpy(packet, READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET));
    packet[2] = 0x37;
    packet[6] += 1; // keep the checksum consistent so only the address is wrong

    EXPECT_FALSE(mztcPacketIsValid(packet, sizeof(READ_MODEL_PACKET)));
}

TEST_F(MztcFramingTest, DeclaredLengthMustMatchReceivedLength)
{
    memcpy(packet, READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET));
    packet[1] = 0x05;

    EXPECT_FALSE(mztcPacketIsValid(packet, sizeof(READ_MODEL_PACKET)));
}

TEST_F(MztcFramingTest, MissingMarkersAreRejected)
{
    memcpy(packet, READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET));
    packet[0] = 0x00;
    EXPECT_FALSE(mztcPacketIsValid(packet, sizeof(READ_MODEL_PACKET)));

    memcpy(packet, READ_MODEL_PACKET, sizeof(READ_MODEL_PACKET));
    packet[sizeof(READ_MODEL_PACKET) - 1] = 0x00;
    EXPECT_FALSE(mztcPacketIsValid(packet, sizeof(READ_MODEL_PACKET)));
}

TEST_F(MztcFramingTest, RuntPacketIsRejected)
{
    EXPECT_FALSE(mztcPacketIsValid(READ_MODEL_PACKET, MZTC_MIN_PACKET_LEN - 1));
    EXPECT_FALSE(mztcPacketIsValid(NULL, MZTC_MIN_PACKET_LEN));
}

class MztcConfigValidationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memset(&config, 0, sizeof(config));
        config.preset = MZTC_PRESET_CUSTOM;
        config.palette_mode = MZTC_PALETTE_WHITE_HOT;
        config.auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
        config.digital_enhancement = 50;
        config.spatial_denoise = 50;
        config.temporal_denoise = 50;
        config.brightness = 50;
        config.contrast = 50;
        config.zoom_level = MZTC_ZOOM_1X;
        config.mirror_mode = MZTC_MIRROR_NONE;
        config.ffc_interval = 5;
    }

    mztcConfig_t config;
};

TEST_F(MztcConfigValidationTest, DefaultsAreAccepted)
{
    EXPECT_TRUE(mztcConfigIsValid(&config));
}

// Every preset value must round-trip the validator, since selecting one writes
// these fields directly into the running configuration.
TEST_F(MztcConfigValidationTest, EveryPresetProducesAValidConfig)
{
    for (int p = MZTC_PRESET_CUSTOM; p <= MZTC_PRESET_MARITIME; p++) {
        mztcTestReset();
        mztcPortAssigned = true;
        // Start from the valid baseline the parameter group resets to. Custom
        // writes nothing, so it cannot repair a configuration that was already
        // out of range.
        *mztcConfigMutable() = config;

        ASSERT_TRUE(mztcSetPreset((mztcPreset_e)p)) << "preset " << p;
        EXPECT_TRUE(mztcConfigIsValid(mztcConfig())) << "preset " << p;
        EXPECT_EQ(p, mztcConfig()->preset) << "preset " << p;
    }
}

// Custom deliberately writes nothing, so it does not rescue an invalid
// configuration. This pins that behaviour rather than leaving it implied.
TEST_F(MztcConfigValidationTest, CustomDoesNotRepairAnInvalidConfig)
{
    mztcTestReset();
    mztcPortAssigned = true;
    memset(mztcConfigMutable(), 0, sizeof(mztcConfig_t));

    ASSERT_TRUE(mztcSetPreset(MZTC_PRESET_CUSTOM));
    EXPECT_FALSE(mztcConfigIsValid(mztcConfig()));
}

// CUSTOM is the escape hatch. It must not overwrite a hand-tuned value.
TEST_F(MztcConfigValidationTest, CustomPresetWritesNothing)
{
    mztcTestReset();
    mztcPortAssigned = true;
    *mztcConfigMutable() = config;
    mztcConfigMutable()->brightness = 17;
    mztcConfigMutable()->palette_mode = MZTC_PALETTE_SEPIA;

    ASSERT_TRUE(mztcSetPreset(MZTC_PRESET_CUSTOM));

    EXPECT_EQ(17, mztcConfig()->brightness);
    EXPECT_EQ(MZTC_PALETTE_SEPIA, mztcConfig()->palette_mode);
}

// A named preset must actually change the owned settings, and must leave the
// two it does not own alone.
TEST_F(MztcConfigValidationTest, PresetWritesOwnedFieldsAndSparesTheRest)
{
    mztcTestReset();
    mztcPortAssigned = true;
    *mztcConfigMutable() = config;
    mztcConfigMutable()->zoom_level = MZTC_ZOOM_4X;
    mztcConfigMutable()->mirror_mode = MZTC_MIRROR_VERTICAL;

    ASSERT_TRUE(mztcSetPreset(MZTC_PRESET_FIRE));

    EXPECT_EQ(MZTC_PALETTE_IRON_RED_1, mztcConfig()->palette_mode);
    EXPECT_EQ(75, mztcConfig()->contrast);
    EXPECT_EQ(25, mztcConfig()->digital_enhancement);
    EXPECT_EQ(10, mztcConfig()->ffc_interval);

    // Zoom belongs to the pilot, mirror to the airframe.
    EXPECT_EQ(MZTC_ZOOM_4X, mztcConfig()->zoom_level);
    EXPECT_EQ(MZTC_MIRROR_VERTICAL, mztcConfig()->mirror_mode);
}

TEST_F(MztcConfigValidationTest, OutOfRangePresetIsRejected)
{
    mztcTestReset();
    mztcPortAssigned = true;
    EXPECT_FALSE(mztcSetPreset((mztcPreset_e)(MZTC_PRESET_MARITIME + 1)));
}

TEST_F(MztcConfigValidationTest, EnumsAreBounded)
{
    config.preset = MZTC_PRESET_MARITIME + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.preset = MZTC_PRESET_MARITIME;

    config.palette_mode = MZTC_PALETTE_RED_HOT + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.palette_mode = MZTC_PALETTE_RED_HOT;

    config.zoom_level = MZTC_ZOOM_8X + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.zoom_level = MZTC_ZOOM_8X;

    config.mirror_mode = MZTC_MIRROR_CENTRAL + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.mirror_mode = MZTC_MIRROR_CENTRAL;

    config.auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;

    EXPECT_TRUE(mztcConfigIsValid(&config));
}

TEST_F(MztcConfigValidationTest, PercentagesAreBounded)
{
    config.brightness = MZTC_MAX_PERCENT + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.brightness = MZTC_MAX_PERCENT;

    config.contrast = MZTC_MAX_PERCENT + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.contrast = MZTC_MAX_PERCENT;

    config.spatial_denoise = MZTC_MAX_PERCENT + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));
    config.spatial_denoise = MZTC_MAX_PERCENT;

    EXPECT_TRUE(mztcConfigIsValid(&config));
}

TEST_F(MztcConfigValidationTest, FfcIntervalIsBounded)
{
    config.ffc_interval = 0;
    EXPECT_FALSE(mztcConfigIsValid(&config));

    config.ffc_interval = MZTC_MAX_FFC_INTERVAL + 1;
    EXPECT_FALSE(mztcConfigIsValid(&config));

    config.ffc_interval = MZTC_MIN_FFC_INTERVAL;
    EXPECT_TRUE(mztcConfigIsValid(&config));

    config.ffc_interval = MZTC_MAX_FFC_INTERVAL;
    EXPECT_TRUE(mztcConfigIsValid(&config));
}

// The camera accepts 0x01 to 0x03 for the shutter mode and answers 0x00 with a
// threshold error, so every setting value has to land inside that window once
// the wire offset is applied.
TEST_F(MztcConfigValidationTest, ShutterModeMapsIntoTheCameraWireRange)
{
    for (uint8_t mode = MZTC_SHUTTER_TEMP_ONLY; mode <= MZTC_SHUTTER_TIME_AND_TEMP; mode++) {
        const uint8_t wire = (uint8_t)(mode + MZTC_SHUTTER_WIRE_OFFSET);
        EXPECT_GE(wire, 0x01) << "mode " << (int)mode;
        EXPECT_LE(wire, 0x03) << "mode " << (int)mode;
    }
}

TEST_F(MztcConfigValidationTest, NullConfigIsRejected)
{
    EXPECT_FALSE(mztcConfigIsValid(NULL));
}

/*
 * Receive path, response dispatch and transmit path.
 *
 * These drive the driver through its serial callback rather than calling the
 * decoders directly, so the framing, the dispatch and the connection state
 * machine are all exercised on the path the camera actually uses.
 */

// Bytes the driver has handed to the serial port since the last reset.
static std::vector<uint8_t> txBytes;
static timeMs_t fakeNow;


class MztcLinkTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        txBytes.clear();
        fakeNow = 1000;
        mztcPortAssigned = true;
        mztcCalibrateBoxActive = false;

        mztcConfig_t *cfg = mztcConfigMutable();
        memset(cfg, 0, sizeof(*cfg));
        cfg->preset = MZTC_PRESET_CUSTOM;
        cfg->palette_mode = MZTC_PALETTE_WHITE_HOT;
        cfg->auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
        cfg->digital_enhancement = 50;
        cfg->spatial_denoise = 50;
        cfg->temporal_denoise = 50;
        cfg->brightness = 50;
        cfg->contrast = 50;
        cfg->zoom_level = MZTC_ZOOM_1X;
        cfg->mirror_mode = MZTC_MIRROR_NONE;
        cfg->ffc_interval = 5;

        mztcTestReset();
    }

    // Push a whole packet through the receive callback one byte at a time.
    void feed(const uint8_t *bytes, size_t len)
    {
        for (size_t i = 0; i < len; i++) {
            mztcSerialReceiveCallback(bytes[i], NULL);
        }
    }

    void feedReply(uint8_t classCmd, uint8_t subCmd, uint8_t flags,
                   const uint8_t *payload = NULL, uint8_t payloadLen = 0)
    {
        uint8_t packet[MZTC_MAX_PACKET_LEN];
        const uint8_t len = mztcBuildPacket(packet, classCmd, subCmd, flags, payload, payloadLen);
        ASSERT_NE(0, len);
        feed(packet, len);
    }

    // Find a transmitted packet with the given class and subclass.
    const uint8_t *findTx(uint8_t classCmd, uint8_t subCmd, uint8_t *lenOut) const
    {
        size_t i = 0;
        while (i + MZTC_MIN_PACKET_LEN <= txBytes.size()) {
            const uint8_t len = (uint8_t)(txBytes[i + 1] + 4);
            if (i + len > txBytes.size()) {
                return NULL;
            }
            if (txBytes[i + 3] == classCmd && txBytes[i + 4] == subCmd) {
                if (lenOut) {
                    *lenOut = len;
                }
                return &txBytes[i];
            }
            i += len;
        }
        return NULL;
    }

    // How many transmitted packets carry the given class and subclass. Used to
    // prove an action fires once rather than on every task tick.
    size_t countTx(uint8_t classCmd, uint8_t subCmd) const
    {
        size_t i = 0, found = 0;
        while (i + MZTC_MIN_PACKET_LEN <= txBytes.size()) {
            const uint8_t len = (uint8_t)(txBytes[i + 1] + 4);
            if (i + len > txBytes.size()) {
                break;
            }
            if (txBytes[i + 3] == classCmd && txBytes[i + 4] == subCmd) {
                found++;
            }
            i += len;
        }
        return found;
    }
};

TEST_F(MztcLinkTest, ValidReplyIsAccepted)
{
    const uint8_t model[3] = { 'M', 'Z', '1' };
    feedReply(0x74, 0x02, 0x03, model, sizeof(model));

    uint8_t idLen = 0;
    const uint8_t *id = mztcGetDeviceId(&idLen);
    ASSERT_NE(nullptr, id);
    EXPECT_EQ(sizeof(model), idLen);
    EXPECT_EQ(0, memcmp(model, id, idLen));
}

// The parser is length driven precisely so a payload byte that happens to equal
// a framing marker cannot split or truncate the packet.
TEST_F(MztcLinkTest, PayloadContainingFramingMarkersIsParsed)
{
    const uint8_t payload[4] = { 0xF0, 0xFF, 0xFF, 0xF0 };
    feedReply(0x74, 0x02, 0x03, payload, sizeof(payload));

    uint8_t idLen = 0;
    const uint8_t *id = mztcGetDeviceId(&idLen);
    ASSERT_NE(nullptr, id);
    EXPECT_EQ(sizeof(payload), idLen);
    EXPECT_EQ(0, memcmp(payload, id, idLen));
}

TEST_F(MztcLinkTest, GarbageBeforeAFrameIsSkipped)
{
    const uint8_t noise[4] = { 0x11, 0x22, 0x33, 0x44 };
    feed(noise, sizeof(noise));

    const uint8_t model[1] = { 0x5A };
    feedReply(0x74, 0x02, 0x03, model, sizeof(model));

    uint8_t idLen = 0;
    ASSERT_NE(nullptr, mztcGetDeviceId(&idLen));
    EXPECT_EQ(1, idLen);
}

TEST_F(MztcLinkTest, BadChecksumIsRejected)
{
    uint8_t packet[MZTC_MAX_PACKET_LEN];
    const uint8_t model[1] = { 0x5A };
    const uint8_t len = mztcBuildPacket(packet, 0x74, 0x02, 0x03, model, sizeof(model));
    packet[len - 2] ^= 0xFF;
    feed(packet, len);

    uint8_t idLen = 0;
    EXPECT_EQ(nullptr, mztcGetDeviceId(&idLen));
    EXPECT_FALSE(mztcGetStatus()->connected);
}

// A size byte outside the legal range must not wedge the parser. The next
// well-formed packet has to be decoded.
TEST_F(MztcLinkTest, BogusLengthResynchronises)
{
    const uint8_t bogus[2] = { MZTC_PACKET_BEGIN, 0xFE };
    feed(bogus, sizeof(bogus));

    const uint8_t model[1] = { 0x5A };
    feedReply(0x74, 0x02, 0x03, model, sizeof(model));

    uint8_t idLen = 0;
    ASSERT_NE(nullptr, mztcGetDeviceId(&idLen));
    EXPECT_EQ(1, idLen);
}

TEST_F(MztcLinkTest, TruncatedPacketIsNotDecoded)
{
    uint8_t packet[MZTC_MAX_PACKET_LEN];
    const uint8_t model[2] = { 0x5A, 0x5B };
    const uint8_t len = mztcBuildPacket(packet, 0x74, 0x02, 0x03, model, sizeof(model));
    feed(packet, len - 1);

    uint8_t idLen = 0;
    EXPECT_EQ(nullptr, mztcGetDeviceId(&idLen));
}

// The camera answers the 0x7C/0x14 initialization request on class 0x7D
// subclass 0x06. Decoding it on the request address instead never matches.
TEST_F(MztcLinkTest, InitStatusReplyUsesItsOwnAddress)
{
    mztcTestSetStatus(MZTC_STATUS_INITIALIZING);

    const uint8_t outputStage[1] = { 0x01 };
    feedReply(0x7D, 0x06, 0x03, outputStage, sizeof(outputStage));
    EXPECT_EQ(MZTC_STATUS_READY, mztcGetStatus()->status);

    const uint8_t logoStage[1] = { 0x00 };
    feedReply(0x7D, 0x06, 0x03, logoStage, sizeof(logoStage));
    EXPECT_EQ(MZTC_STATUS_INITIALIZING, mztcGetStatus()->status);
}

TEST_F(MztcLinkTest, ShutterReplyRestartsTheCalibrationClock)
{
    mztcTestSetStatus(MZTC_STATUS_CALIBRATING);
    mztcTestSetLastCalibration(42);

    feedReply(0x7C, 0x02, 0x03);

    EXPECT_EQ(0, mztcGetStatus()->last_calibration);
    EXPECT_EQ(MZTC_STATUS_READY, mztcGetStatus()->status);
}

TEST_F(MztcLinkTest, ErrorReplyRaisesTheCommunicationFlag)
{
    const uint8_t thresholdExceeded[1] = { 0x01 };
    feedReply(0x78, 0x02, 0x04, thresholdExceeded, sizeof(thresholdExceeded));

    EXPECT_TRUE((mztcGetStatus()->error_flags & MZTC_ERROR_COMMUNICATION) != 0);
}

TEST_F(MztcLinkTest, SuccessReplyClearsTheCommunicationFlag)
{
    const uint8_t thresholdExceeded[1] = { 0x01 };
    feedReply(0x78, 0x02, 0x04, thresholdExceeded, sizeof(thresholdExceeded));
    ASSERT_TRUE((mztcGetStatus()->error_flags & MZTC_ERROR_COMMUNICATION) != 0);

    feedReply(0x78, 0x02, 0x03);
    EXPECT_FALSE((mztcGetStatus()->error_flags & MZTC_ERROR_COMMUNICATION) != 0);
}

// Opening the port is not enough. The camera has to answer before the link
// counts as up.
TEST_F(MztcLinkTest, AnAnswerPromotesTheLinkToConnected)
{
    EXPECT_FALSE(mztcGetStatus()->connected);

    feedReply(0x74, 0x02, 0x03);

    EXPECT_TRUE(mztcGetStatus()->connected);
}

// Assigning the function in the Ports tab is what enables the camera. There is
// no separate enable setting to fall out of step with it.
// "set mztc_preset = SEARCH" writes the byte and nothing else. The task has to
// notice and apply it, otherwise the setting records a label that does not
// match the values in effect. That is the defect the old mode enum had.
// The calibrate switch fires once per flip. Holding it must not stream shutter
// commands at the camera on every task tick.
TEST_F(MztcLinkTest, TheCalibrateSwitchFiresOnceOnTheRisingEdge)
{
    mztcUpdate(0);
    feedReply(0x74, 0x02, 0x03);
    mztcUpdate(0);
    txBytes.clear();

    mztcCalibrateBoxActive = true;
    mztcUpdate(0);
    const size_t afterFirst = countTx(0x7C, 0x02);
    EXPECT_EQ(1u, afterFirst);

    // Still held. No further corrections.
    mztcUpdate(0);
    mztcUpdate(0);
    EXPECT_EQ(afterFirst, countTx(0x7C, 0x02));

    // Released and flipped again. One more.
    mztcCalibrateBoxActive = false;
    mztcUpdate(0);
    mztcCalibrateBoxActive = true;
    mztcUpdate(0);
    EXPECT_EQ(afterFirst + 1, countTx(0x7C, 0x02));
}

TEST_F(MztcLinkTest, APresetWrittenAsASettingIsAppliedByTheTask)
{
    mztcUpdate(0);
    feedReply(0x74, 0x02, 0x03);
    ASSERT_TRUE(mztcGetStatus()->connected);
    mztcUpdate(0);

    // Write the field directly, the way the settings framework does.
    mztcConfigMutable()->preset = MZTC_PRESET_FIRE;
    mztcUpdate(0);

    EXPECT_EQ(MZTC_PALETTE_IRON_RED_1, mztcConfig()->palette_mode);
    EXPECT_EQ(75, mztcConfig()->contrast);
    EXPECT_EQ(25, mztcConfig()->digital_enhancement);
    EXPECT_EQ(10, mztcConfig()->ffc_interval);
}

// Boot must not reapply. The saved values already reflect the saved preset, so
// reapplying would discard hand tuning done after the preset was chosen.
TEST_F(MztcLinkTest, BootDoesNotReapplyTheStoredPreset)
{
    mztcConfigMutable()->preset = MZTC_PRESET_FIRE;
    mztcConfigMutable()->contrast = 33;      // hand tuned after picking FIRE

    // Run the real boot path. mztcInit() returns early while the driver is
    // already initialised, so without this the test would pass regardless.
    mztcTestForceReinit();
    mztcInit();

    mztcUpdate(0);
    feedReply(0x74, 0x02, 0x03);
    mztcUpdate(0);
    mztcUpdate(0);

    EXPECT_EQ(33, mztcConfig()->contrast) << "boot reapplied and lost the tuning";
    EXPECT_EQ(MZTC_PRESET_FIRE, mztcConfig()->preset);
}

TEST_F(MztcLinkTest, ThePortAssignmentIsWhatEnablesTheCamera)
{
    mztcPortAssigned = true;
    EXPECT_TRUE(mztcIsEnabled());

    mztcPortAssigned = false;
    EXPECT_FALSE(mztcIsEnabled());
}

/*
 * Transmit path
 */

TEST_F(MztcLinkTest, ConfigurationBurstUsesTheCameraShutterWireValues)
{
    for (uint8_t mode = MZTC_SHUTTER_TEMP_ONLY; mode <= MZTC_SHUTTER_TIME_AND_TEMP; mode++) {
        txBytes.clear();
        mztcConfigMutable()->auto_shutter = mode;
        mztcSendConfiguration();

        uint8_t len = 0;
        const uint8_t *packet = findTx(0x7C, 0x04, &len);
        ASSERT_NE(nullptr, packet) << "no auto shutter command for mode " << (int)mode;
        ASSERT_EQ(MZTC_PACKET_OVERHEAD + 1, len);

        // The manual defines 0x01 temperature only, 0x02 time only and 0x03
        // time and temperature. It answers 0x00 with a threshold error.
        EXPECT_EQ(mode + 1, packet[6]) << "mode " << (int)mode;
        EXPECT_GE(packet[6], 0x01);
        EXPECT_LE(packet[6], 0x03);
    }
}

// The camera owns the shutter schedule. The interval has to reach it.
TEST_F(MztcLinkTest, ConfigurationBurstSendsTheShutterInterval)
{
    mztcConfigMutable()->ffc_interval = 7;
    mztcSendConfiguration();

    uint8_t len = 0;
    const uint8_t *packet = findTx(0x7C, 0x05, &len);
    ASSERT_NE(nullptr, packet);
    ASSERT_EQ(MZTC_PACKET_OVERHEAD + 2, len);
    EXPECT_EQ(0, packet[6]);
    EXPECT_EQ(7, packet[7]);
}

TEST_F(MztcLinkTest, ConfigurationBurstSendsEveryImageParameter)
{
    mztcConfigMutable()->brightness = 11;
    mztcConfigMutable()->contrast = 22;
    mztcConfigMutable()->digital_enhancement = 33;
    mztcConfigMutable()->spatial_denoise = 44;
    mztcConfigMutable()->temporal_denoise = 55;
    mztcConfigMutable()->palette_mode = MZTC_PALETTE_IRON_RED_1;
    mztcConfigMutable()->zoom_level = MZTC_ZOOM_4X;
    mztcConfigMutable()->mirror_mode = MZTC_MIRROR_VERTICAL;
    mztcSendConfiguration();

    struct { uint8_t cls; uint8_t sub; uint8_t value; const char *name; } expected[] = {
        { 0x78, 0x02, 11, "brightness" },
        { 0x78, 0x03, 22, "contrast" },
        { 0x78, 0x10, 33, "digital enhancement" },
        { 0x78, 0x15, 44, "spatial denoise" },
        { 0x78, 0x16, 55, "temporal denoise" },
        { 0x78, 0x20, MZTC_PALETTE_IRON_RED_1, "palette" },
        { 0x70, 0x12, MZTC_ZOOM_4X, "zoom" },
        { 0x70, 0x11, MZTC_MIRROR_VERTICAL, "mirror" },
    };

    for (size_t i = 0; i < ARRAYLEN(expected); i++) {
        uint8_t len = 0;
        const uint8_t *packet = findTx(expected[i].cls, expected[i].sub, &len);
        ASSERT_NE(nullptr, packet) << expected[i].name << " was not sent";
        ASSERT_EQ(MZTC_PACKET_OVERHEAD + 1, len) << expected[i].name;
        EXPECT_EQ(expected[i].value, packet[6]) << expected[i].name;
    }
}

// Everything the driver puts on the wire has to survive its own validator.
TEST_F(MztcLinkTest, EveryTransmittedPacketIsWellFormed)
{
    mztcSendConfiguration();
    ASSERT_FALSE(txBytes.empty());

    size_t i = 0;
    int packets = 0;
    while (i + MZTC_MIN_PACKET_LEN <= txBytes.size()) {
        const uint8_t len = (uint8_t)(txBytes[i + 1] + 4);
        ASSERT_LE(i + len, txBytes.size()) << "packet " << packets << " runs past the buffer";
        EXPECT_TRUE(mztcPacketIsValid(&txBytes[i], len)) << "packet " << packets;
        i += len;
        packets++;
    }
    EXPECT_EQ(txBytes.size(), i) << "trailing bytes after the last packet";
    EXPECT_GE(packets, 9);
}

// Everything the driver needs from the rest of the firmware. No real serial
// port is ever opened. serialWriteBufShim captures what the driver transmits so
// the transmit tests can assert on the actual wire bytes.
extern "C" {

#include "build/debug.h"
#include "drivers/serial.h"
#include "io/serial.h"

int32_t debug[DEBUG32_VALUE_COUNT];

const uint32_t baudRates[] = { 0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
                               230400, 250000, 460800, 921600, 1000000, 1500000, 2000000, 2470000 };

timeMs_t millis(void)
{
    return fakeNow;
}

// The Ports tab decides whether the camera exists. Returning a config here is
// the test equivalent of assigning MZTC_CAMERA to a UART.
static serialPortConfig_t fakePortConfig;

bool IS_RC_MODE_ACTIVE(boxId_e boxId)
{
    return boxId == BOXMZTCCALIBRATE && mztcCalibrateBoxActive;
}

serialPortConfig_t *findSerialPortConfig(serialPortFunction_e function)
{
    if (function != FUNCTION_MZTC_CAMERA || !mztcPortAssigned) {
        return NULL;
    }
    fakePortConfig.identifier = SERIAL_PORT_USART2;
    fakePortConfig.peripheral_baudrateIndex = 8;
    return &fakePortConfig;
}

serialPort_t *openSerialPort(serialPortIdentifier_e, serialPortFunction_e, serialReceiveCallbackPtr,
                             void *, uint32_t, portMode_t, portOptions_t)
{
    return NULL;
}

void closeSerialPort(serialPort_t *)
{
}

void serialWriteBufShim(void *, const uint8_t *data, int count)
{
    for (int i = 0; i < count; i++) {
        txBytes.push_back(data[i]);
    }
}

}
