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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/build_config.h"

#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/settings.h"

#include "io/serial.h"
#include "io/mztc_camera.h"
#include "fc/rc_modes.h"

#ifdef USE_MZTC

// Flag definitions
#define MZTC_FLAG_WRITE      0x00
#define MZTC_FLAG_READ       0x01
#define MZTC_FLAG_SUCCESS    0x03
#define MZTC_FLAG_ERROR      0x04

// Camera commands, as class and subclass pairs
#define MZTC_CLASS_DEVICE            0x74
#define MZTC_CLASS_DISPLAY           0x70
#define MZTC_CLASS_IMAGE             0x78
#define MZTC_CLASS_SHUTTER           0x7C

#define MZTC_SUB_READ_MODEL          0x02
#define MZTC_SUB_READ_FPGA_VER       0x03
#define MZTC_SUB_READ_SW_VER         0x05
#define MZTC_SUB_RESTORE_DEFAULTS    0x0F
#define MZTC_SUB_SAVE_CONFIG         0x10

#define MZTC_SUB_MANUAL_SHUTTER      0x02
#define MZTC_SUB_AUTO_SHUTTER        0x04
#define MZTC_SUB_SHUTTER_INTERVAL    0x05
#define MZTC_SUB_VIGNETTING          0x0C
#define MZTC_SUB_INIT_STATUS         0x14

// The camera answers the 0x7C/0x14 initialization status request on a
// different address than it was asked on. The manual specifies the reply as
// class 0x7D subclass 0x06.
#define MZTC_CLASS_INIT_STATUS_REPLY 0x7D
#define MZTC_SUB_INIT_STATUS_REPLY   0x06

// Vignetting correction takes a fixed data byte
#define MZTC_VIGNETTING_TRIGGER      0x02

#define MZTC_SUB_BRIGHTNESS          0x02
#define MZTC_SUB_CONTRAST            0x03
#define MZTC_SUB_DIGITAL_ENHANCE     0x10
#define MZTC_SUB_SPATIAL_DENOISE     0x15
#define MZTC_SUB_TEMPORAL_DENOISE    0x16
#define MZTC_SUB_PSEUDO_COLOR        0x20

#define MZTC_SUB_IMAGE_MIRROR        0x11
#define MZTC_SUB_ZOOM                0x12

// The MZTC_* limits in config/mztc_camera.h and the constants block in
// settings.yaml describe the same ranges to different consumers. The C code
// validates against the first. The CLI and the generated docs use the second.
// These assertions turn any divergence into a build failure. Without them the
// CLI could reject a value that MSP still accepts.
STATIC_ASSERT(MZTC_MIN_FFC_INTERVAL == SETTING_MZTC_FFC_INTERVAL_MIN, mztc_ffc_interval_min_mismatch);
STATIC_ASSERT(MZTC_MAX_FFC_INTERVAL == SETTING_MZTC_FFC_INTERVAL_MAX, mztc_ffc_interval_max_mismatch);
STATIC_ASSERT(MZTC_MIN_PERCENT == SETTING_MZTC_BRIGHTNESS_MIN, mztc_percent_min_mismatch);
STATIC_ASSERT(MZTC_MAX_PERCENT == SETTING_MZTC_BRIGHTNESS_MAX, mztc_percent_max_mismatch);

// Connection management timings
#define MZTC_PORT_RETRY_MS           1000    // Between attempts to open the port
#define MZTC_PROBE_INTERVAL_MS       500     // Between identity probes
#define MZTC_PROBE_ATTEMPTS          6       // Reopen the port after this many unanswered probes
#define MZTC_RX_TIMEOUT_MS           3000    // No valid reply for this long means the link is down
#define MZTC_CALIBRATION_TIMEOUT_MS  5000    // A shutter cycle finishes well inside this

// Connection quality is the percentage of probes answered over a sliding window
#define MZTC_QUALITY_WINDOW          8

// Internal state
static mztcStatus_t mztcStatus;
static serialPort_t *mztcSerialPort = NULL;
static timeMs_t mztcLastPortRetry = 0;
static timeMs_t mztcLastProbeTime = 0;
static timeMs_t mztcLastCalibrationTime = 0;
static timeMs_t mztcLastValidResponse = 0;
static bool mztcInitialized = false;
static uint8_t mztcProbeAttempts = 0;

// Sliding window of probe outcomes, used for connection_quality. Bit 0 is
// the probe most recently sent. A set bit means that probe was answered.
static uint8_t mztcProbeHistory = 0;
static uint8_t mztcProbeWindowFill = 0;

// What the open port was opened as. The Ports tab can reassign or remove the
// camera at any time, and the update loop compares against these to notice.
static serialPortIdentifier_e mztcPortIdentifier;
static uint8_t mztcPortBaudIndex;

// The configuration as last transmitted. A generic "set mztc_*" only stores a
// byte, so the task compares against this snapshot to know a burst is owed.
static mztcConfig_t mztcLastSentConfig;
static bool mztcLastSentValid = false;

// Set when the camera first answers, consumed by the task so that the
// configuration burst never runs in interrupt context.
static bool mztcConfigurationPending = false;
// Previous state of the THERMAL CALIBRATE switch. The correction runs on the
// rising edge only, so holding the switch does not fire it every task tick.
static bool mztcCalibrateBoxWasActive = false;
// The preset the stored settings currently reflect. Any path that changes
// mztc_preset is noticed by the task and applied, so "set mztc_preset" behaves
// the same as the mztc_preset CLI command and MSP2_SET_MZTC_PRESET.
static uint8_t mztcLastAppliedPreset = MZTC_PRESET_CUSTOM;

// Device identity, filled in from the model and version replies
static uint8_t mztcDeviceModel[MZTC_MAX_DATA_LEN];
static uint8_t mztcDeviceModelLen = 0;

// Receive framing state. The parser is length driven so that a 0xF0 or 0xFF
// byte inside a payload cannot split or truncate a packet.
static uint8_t mztcRxBuffer[MZTC_MAX_PACKET_LEN];
static uint8_t mztcRxLen = 0;
static uint8_t mztcRxExpected = 0;

// Parameter group for MassZero Thermal Camera configuration.
//
// The defaults come from the SETTING_*_DEFAULT macros that the settings
// generator emits from settings.yaml. The CLI defaults and the fresh-EEPROM
// defaults cannot drift apart.
PG_REGISTER_WITH_RESET_TEMPLATE(mztcConfig_t, mztcConfig, PG_MZTC_CAMERA_CONFIG, 0);

PG_RESET_TEMPLATE(mztcConfig_t, mztcConfig,
    .preset = SETTING_MZTC_PRESET_DEFAULT,
    .palette_mode = SETTING_MZTC_PALETTE_MODE_DEFAULT,
    .auto_shutter = SETTING_MZTC_AUTO_SHUTTER_DEFAULT,
    .digital_enhancement = SETTING_MZTC_DIGITAL_ENHANCEMENT_DEFAULT,
    .spatial_denoise = SETTING_MZTC_SPATIAL_DENOISE_DEFAULT,
    .temporal_denoise = SETTING_MZTC_TEMPORAL_DENOISE_DEFAULT,
    .brightness = SETTING_MZTC_BRIGHTNESS_DEFAULT,
    .contrast = SETTING_MZTC_CONTRAST_DEFAULT,
    .zoom_level = SETTING_MZTC_ZOOM_LEVEL_DEFAULT,
    .mirror_mode = SETTING_MZTC_MIRROR_MODE_DEFAULT,
    .ffc_interval = SETTING_MZTC_FFC_INTERVAL_DEFAULT,
);

// Forward declarations
STATIC_UNIT_TESTED void mztcSerialReceiveCallback(uint16_t c, void *rxCallbackData);
static bool mztcSendPacket(uint8_t class_cmd, uint8_t subclass_cmd, uint8_t flags, const uint8_t *data, uint8_t data_len);
STATIC_UNIT_TESTED void mztcHandlePacket(const uint8_t *packet, uint8_t len);
static void mztcCheckCalibration(void);
STATIC_UNIT_TESTED void mztcSendConfiguration(void);
static void mztcClosePort(uint8_t errorFlag);
static void mztcSendProbe(void);

/*
 * Wire framing
 */

// Build a complete packet into out. The caller must supply at least
// MZTC_MAX_PACKET_LEN bytes. Returns the number of bytes written, or 0 if the
// request is invalid.
uint8_t mztcBuildPacket(uint8_t *out, uint8_t class_cmd, uint8_t subclass_cmd,
                        uint8_t flags, const uint8_t *data, uint8_t data_len)
{
    if (!out || data_len > MZTC_MAX_DATA_LEN || (data_len > 0 && !data)) {
        return 0;
    }

    uint8_t i = 0;
    out[i++] = MZTC_PACKET_BEGIN;
    out[i++] = (uint8_t)(data_len + MZTC_SIZE_FIELD_OFFSET);
    out[i++] = MZTC_DEVICE_ADDR;
    out[i++] = class_cmd;
    out[i++] = subclass_cmd;
    out[i++] = flags;

    uint8_t checksum = MZTC_DEVICE_ADDR + class_cmd + subclass_cmd + flags;
    for (uint8_t d = 0; d < data_len; d++) {
        out[i++] = data[d];
        checksum += data[d];
    }

    out[i++] = checksum;
    out[i++] = MZTC_PACKET_END;

    return i;
}

// Validate a fully received packet: markers, declared length and checksum.
bool mztcPacketIsValid(const uint8_t *packet, uint8_t len)
{
    if (!packet || len < MZTC_MIN_PACKET_LEN || len > MZTC_MAX_PACKET_LEN) {
        return false;
    }

    if (packet[0] != MZTC_PACKET_BEGIN || packet[len - 1] != MZTC_PACKET_END) {
        return false;
    }

    const uint8_t declaredSize = packet[1];
    if (declaredSize < MZTC_SIZE_FIELD_OFFSET || (uint16_t)declaredSize + 4u != (uint16_t)len) {
        return false;
    }

    if (packet[2] != MZTC_DEVICE_ADDR) {
        return false;
    }

    // The checksum covers address, class, subclass, flags and payload.
    uint8_t checksum = 0;
    for (uint8_t i = 2; i < (uint8_t)(len - 2); i++) {
        checksum += packet[i];
    }

    return checksum == packet[len - 2];
}

// Serial receive callback. Accumulates one packet at a time using the declared
// length. Payload bytes that happen to equal the begin or end markers cannot
// desynchronise the parser.
STATIC_UNIT_TESTED void mztcSerialReceiveCallback(uint16_t c, void *rxCallbackData)
{
    UNUSED(rxCallbackData);

    const uint8_t byte = (uint8_t)c;

    if (mztcRxLen == 0) {
        // Waiting for a start of frame
        if (byte != MZTC_PACKET_BEGIN) {
            return;
        }
        mztcRxBuffer[mztcRxLen++] = byte;
        mztcRxExpected = 0;
        return;
    }

    if (mztcRxLen == 1) {
        // The size byte determines the length of the rest of the frame.
        const uint16_t total = (uint16_t)byte + 4u;
        if (byte < MZTC_SIZE_FIELD_OFFSET || total > MZTC_MAX_PACKET_LEN) {
            // Bogus length. Resync on the next begin marker.
            mztcRxLen = 0;
            return;
        }
        mztcRxExpected = (uint8_t)total;
        mztcRxBuffer[mztcRxLen++] = byte;
        return;
    }

    mztcRxBuffer[mztcRxLen++] = byte;

    if (mztcRxLen >= mztcRxExpected) {
        if (mztcPacketIsValid(mztcRxBuffer, mztcRxLen)) {
            mztcHandlePacket(mztcRxBuffer, mztcRxLen);
        }
        mztcRxLen = 0;
        mztcRxExpected = 0;
    }
}

// Send a packet to the camera
static bool mztcSendPacket(uint8_t class_cmd, uint8_t subclass_cmd, uint8_t flags, const uint8_t *data, uint8_t data_len)
{
    if (!mztcSerialPort) {
        return false;
    }

    uint8_t packet[MZTC_MAX_PACKET_LEN];
    const uint8_t len = mztcBuildPacket(packet, class_cmd, subclass_cmd, flags, data, data_len);
    if (len == 0) {
        return false;
    }

    serialWriteBufShim(mztcSerialPort, packet, len);
    return true;
}

/*
 * Response handling
 */

// Decode the payload of a successful reply. payloadLen is the payload length,
// which the caller derived from the validated frame.
static void mztcDecodeSuccess(uint8_t class_cmd, uint8_t subclass_cmd, const uint8_t *payload, uint8_t payloadLen)
{
    switch (class_cmd) {
    case MZTC_CLASS_DEVICE:
        switch (subclass_cmd) {
        case MZTC_SUB_READ_MODEL:
        case MZTC_SUB_READ_FPGA_VER:
        case MZTC_SUB_READ_SW_VER:
            if (payloadLen > 0) {
                mztcDeviceModelLen = MIN(payloadLen, (uint8_t)sizeof(mztcDeviceModel));
                memcpy(mztcDeviceModel, payload, mztcDeviceModelLen);
            }
            break;
        default:
            break;
        }
        break;

    case MZTC_CLASS_INIT_STATUS_REPLY:
        if (subclass_cmd == MZTC_SUB_INIT_STATUS_REPLY && payloadLen >= 1) {
            // 0x00 is the logo loading stage. 0x01 is the image output stage.
            if (payload[0] != 0) {
                if (mztcStatus.status == MZTC_STATUS_INITIALIZING) {
                    mztcStatus.status = MZTC_STATUS_READY;
                }
            } else {
                mztcStatus.status = MZTC_STATUS_INITIALIZING;
            }
        }
        break;

    case MZTC_CLASS_SHUTTER:
        switch (subclass_cmd) {
        case MZTC_SUB_MANUAL_SHUTTER:
            // The shutter cycle finished. The calibration clock restarts.
            mztcLastCalibrationTime = millis();
            mztcStatus.last_calibration = 0;
            mztcStatus.error_flags &= (uint8_t)~MZTC_ERROR_CALIBRATION;
            if (mztcStatus.status == MZTC_STATUS_CALIBRATING) {
                mztcStatus.status = MZTC_STATUS_READY;
            }
            break;

        default:
            break;
        }
        break;

    default:
        // Image and display commands acknowledge without a payload we consume.
        break;
    }
}

// Handle one validated packet
STATIC_UNIT_TESTED void mztcHandlePacket(const uint8_t *packet, uint8_t len)
{
    const uint8_t class_cmd = packet[3];
    const uint8_t subclass_cmd = packet[4];
    const uint8_t flags = packet[5];
    const uint8_t *payload = &packet[6];
    const uint8_t payloadLen = (uint8_t)(len - MZTC_PACKET_OVERHEAD);

    mztcLastValidResponse = millis();

    if (class_cmd == MZTC_CLASS_DEVICE && subclass_cmd == MZTC_SUB_READ_MODEL &&
        mztcProbeWindowFill > 0) {
        mztcProbeHistory |= 1;
    }

    switch (flags) {
    case MZTC_FLAG_SUCCESS:
        mztcStatus.error_flags &= (uint8_t)~(MZTC_ERROR_COMMUNICATION | MZTC_ERROR_TIMEOUT);
        mztcDecodeSuccess(class_cmd, subclass_cmd, payload, payloadLen);
        break;

    case MZTC_FLAG_ERROR:
        mztcStatus.error_flags |= MZTC_ERROR_COMMUNICATION;
        if (class_cmd == MZTC_CLASS_SHUTTER && subclass_cmd == MZTC_SUB_MANUAL_SHUTTER) {
            // The camera rejected the shutter cycle. Without this the status
            // would show calibrating until the link itself failed.
            mztcStatus.error_flags |= MZTC_ERROR_CALIBRATION;
            if (mztcStatus.status == MZTC_STATUS_CALIBRATING) {
                mztcStatus.status = MZTC_STATUS_READY;
            }
        }
        break;

    default:
        // Not a reply we decode, but the link is clearly alive.
        break;
    }

    // The camera answered. The link is up regardless of which command it was.
    // This runs in the serial receive interrupt. The configuration burst is
    // deferred to the task.
    if (!mztcStatus.connected) {
        mztcStatus.connected = true;
        mztcProbeAttempts = 0;
        mztcConfigurationPending = true;
        if (mztcStatus.status == MZTC_STATUS_INITIALIZING || mztcStatus.status == MZTC_STATUS_ERROR) {
            mztcStatus.status = MZTC_STATUS_READY;
        }
    }
}

/*
 * Connection management
 */

static void mztcResetLinkState(void)
{
    mztcRxLen = 0;
    mztcRxExpected = 0;
    mztcProbeAttempts = 0;
    mztcProbeHistory = 0;
    mztcProbeWindowFill = 0;
    mztcConfigurationPending = false;
    mztcCalibrateBoxWasActive = false;
    // mztcLastAppliedPreset deliberately survives. A preset stored while the
    // link was down still has to be applied once the camera is back.
    mztcLastSentValid = false;
    mztcDeviceModelLen = 0;
    mztcStatus.connected = false;
    mztcStatus.connection_quality = 0;
}

static void mztcClosePort(uint8_t errorFlag)
{
    if (mztcSerialPort) {
        closeSerialPort(mztcSerialPort);
        mztcSerialPort = NULL;
    }
    mztcResetLinkState();
    mztcStatus.status = MZTC_STATUS_ERROR;
    mztcStatus.error_flags |= errorFlag;
}

// Ask the camera to identify itself. A reply is what promotes the link from
// "the port is open" to "a camera is present".
static void mztcSendProbe(void)
{
    mztcLastProbeTime = millis();
    if (mztcProbeAttempts < UINT8_MAX) {
        mztcProbeAttempts++;
    }
    // Shift an unanswered slot in at bit 0. The reply, if one comes, sets
    // that bit. The oldest outcome falls off the top. Quality therefore
    // follows recent traffic. An average over the whole session would hide a
    // link that just died.
    mztcProbeHistory <<= 1;
    if (mztcProbeWindowFill < MZTC_QUALITY_WINDOW) {
        mztcProbeWindowFill++;
    }
    mztcSendPacket(MZTC_CLASS_DEVICE, MZTC_SUB_READ_MODEL, MZTC_FLAG_READ, NULL, 0);
}

static void mztcUpdateConnectionQuality(void)
{
    if (mztcProbeWindowFill == 0) {
        mztcStatus.connection_quality = 0;
        return;
    }
    uint8_t answered = 0;
    for (uint8_t i = 0; i < mztcProbeWindowFill; i++) {
        if (mztcProbeHistory & (1u << i)) {
            answered++;
        }
    }
    mztcStatus.connection_quality = (uint8_t)((answered * 100u) / mztcProbeWindowFill);
}

// Try to open the configured port. Opening it only means the UART is ours; it
// says nothing about whether a camera is attached.
static void mztcTryOpenPort(timeMs_t now)
{
    if ((now - mztcLastPortRetry) < MZTC_PORT_RETRY_MS) {
        return;
    }
    mztcLastPortRetry = now;

    // The Ports tab owns both the port and its baud rate. No assignment means
    // no camera, which is how the feature is turned on and off.
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MZTC_CAMERA);
    if (portConfig == NULL) {
        mztcStatus.status = MZTC_STATUS_OFFLINE;
        return;
    }

    mztcSerialPort = openSerialPort(portConfig->identifier,
                                    FUNCTION_MZTC_CAMERA,
                                    mztcSerialReceiveCallback,
                                    NULL,
                                    baudRates[portConfig->peripheral_baudrateIndex],
                                    MODE_RXTX,
                                    SERIAL_NOT_INVERTED);

    if (mztcSerialPort == NULL) {
        mztcStatus.status = MZTC_STATUS_ERROR;
        mztcStatus.error_flags |= MZTC_ERROR_COMMUNICATION;
        return;
    }

    mztcPortIdentifier = portConfig->identifier;
    mztcPortBaudIndex = portConfig->peripheral_baudrateIndex;
    mztcResetLinkState();
    mztcStatus.status = MZTC_STATUS_INITIALIZING;
    mztcStatus.error_flags = 0;
    mztcLastValidResponse = now;
    mztcSendProbe();
}

// The fields a preset owns, compared field by field. Zoom and mirror are
// absent on purpose: they belong to the pilot and the airframe, so changing
// them does not leave the preset.
static bool mztcOwnedFieldsDiffer(const mztcConfig_t *a, const mztcConfig_t *b)
{
    return a->palette_mode != b->palette_mode
        || a->brightness != b->brightness
        || a->contrast != b->contrast
        || a->digital_enhancement != b->digital_enhancement
        || a->spatial_denoise != b->spatial_denoise
        || a->temporal_denoise != b->temporal_denoise
        || a->auto_shutter != b->auto_shutter
        || a->ffc_interval != b->ffc_interval;
}

// Every field the configuration burst transmits. The preset byte is a label
// on the firmware side and never goes over the wire, so it is not compared.
static bool mztcTransmittedFieldsDiffer(const mztcConfig_t *a, const mztcConfig_t *b)
{
    return mztcOwnedFieldsDiffer(a, b)
        || a->zoom_level != b->zoom_level
        || a->mirror_mode != b->mirror_mode;
}

// Hand tuning a preset-owned setting moves the selection to CUSTOM, so the
// reported preset never claims values that are no longer in effect.
static void mztcMarkCustom(void)
{
    if (mztcConfig()->preset != MZTC_PRESET_CUSTOM) {
        mztcConfigMutable()->preset = MZTC_PRESET_CUSTOM;
        mztcStatus.preset = MZTC_PRESET_CUSTOM;
        mztcLastAppliedPreset = MZTC_PRESET_CUSTOM;
    }
}

/*
 * Public API
 */

void mztcInit(void)
{
    if (mztcInitialized) {
        return;
    }

    memset(&mztcStatus, 0, sizeof(mztcStatus));
    mztcStatus.status = MZTC_STATUS_OFFLINE;
    mztcStatus.preset = mztcConfig()->preset;
    mztcLastAppliedPreset = mztcConfig()->preset;
    mztcStatus.connected = false;

    mztcInitialized = true;

    // The port is opened from the update loop so that a missing or busy UART
    // does not stall init.
    mztcStatus.status = MZTC_STATUS_INITIALIZING;
    mztcLastPortRetry = 0;
    mztcLastCalibrationTime = millis();
}

void mztcUpdate(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if (!mztcInitialized) {
        return;
    }

    const timeMs_t now = millis();

    // A preset written through the settings framework or "set mztc_preset"
    // only stores a byte. The task applies it here, which is what makes every
    // path agree. The check runs before the connectivity handling. A preset
    // stored while the link was down therefore survives to the reconnect.
    if (mztcConfig()->preset != mztcLastAppliedPreset && mztcIsEnabled()) {
        mztcSetPreset((mztcPreset_e)mztcConfig()->preset);
    }

    if (mztcSerialPort == NULL) {
        mztcTryOpenPort(now);
        return;
    }

    // The Ports tab can reassign or remove the camera's UART at any time. A
    // stale handle would keep probing the old port forever. The open port is
    // therefore checked against the current assignment.
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MZTC_CAMERA);
    if (!portConfig || portConfig->identifier != mztcPortIdentifier
        || portConfig->peripheral_baudrateIndex != mztcPortBaudIndex) {
        closeSerialPort(mztcSerialPort);
        mztcSerialPort = NULL;
        mztcResetLinkState();
        mztcStatus.error_flags = 0;
        mztcStatus.status = portConfig ? MZTC_STATUS_INITIALIZING : MZTC_STATUS_OFFLINE;
        mztcLastPortRetry = 0;
        return;
    }

    if (!mztcStatus.connected) {
        // Still probing. Reopen the port after a bounded number of unanswered
        // probes, in case the UART came up before the camera did. The receive
        // timeout below does not apply here, because nothing has answered yet.
        if ((now - mztcLastProbeTime) >= MZTC_PROBE_INTERVAL_MS) {
            if (mztcProbeAttempts >= MZTC_PROBE_ATTEMPTS) {
                mztcClosePort(MZTC_ERROR_COMMUNICATION);
                return;
            }
            mztcSendProbe();
        }
        mztcUpdateConnectionQuality();
        return;
    }

    // An established link is only healthy while the camera keeps answering.
    if ((now - mztcLastValidResponse) > MZTC_RX_TIMEOUT_MS) {
        mztcClosePort(MZTC_ERROR_TIMEOUT);
        return;
    }

    if (mztcConfigurationPending) {
        mztcConfigurationPending = false;
        mztcSendConfiguration();
    }

    // A periodic probe both feeds the receive timeout and measures link quality.
    if ((now - mztcLastProbeTime) >= MZTC_PROBE_INTERVAL_MS) {
        mztcSendProbe();
    }
    mztcUpdateConnectionQuality();

    // A rising edge on the switch triggers one flat field correction. The
    // camera closes its own shutter as the reference, so this is safe to fire
    // at any time and in any attitude.
    const bool calibrateBoxActive = IS_RC_MODE_ACTIVE(BOXMZTCCALIBRATE);
    if (calibrateBoxActive && !mztcCalibrateBoxWasActive) {
        mztcTriggerCalibration();
    }
    mztcCalibrateBoxWasActive = calibrateBoxActive;

    // A generic "set mztc_*" stores a byte and nothing else. The task
    // compares the stored settings against the last transmitted ones. That
    // comparison is what turns those writes into camera commands. A change to
    // a preset-owned field moves the selection to CUSTOM, as documented. The
    // pending flag is checked first. Values a preset or MSP apply just queued
    // must not be mistaken for hand tuning.
    if (!mztcConfigurationPending && mztcLastSentValid
        && mztcTransmittedFieldsDiffer(mztcConfig(), &mztcLastSentConfig)) {
        if (mztcOwnedFieldsDiffer(mztcConfig(), &mztcLastSentConfig)) {
            mztcMarkCustom();
        }
        mztcConfigurationPending = true;
    }

    mztcCheckCalibration();

    // The camera answers a successful shutter cycle and an explicitly
    // rejected one. If neither reply ever arrives, the status would show
    // calibrating forever. A generous deadline returns it to ready.
    if (mztcStatus.status == MZTC_STATUS_CALIBRATING
        && (now - mztcLastCalibrationTime) > MZTC_CALIBRATION_TIMEOUT_MS) {
        mztcStatus.status = MZTC_STATUS_READY;
        mztcStatus.error_flags |= MZTC_ERROR_CALIBRATION;
    }

    mztcStatus.preset = mztcConfig()->preset;
}

bool mztcIsConnected(void)
{
    return mztcStatus.connected && (mztcSerialPort != NULL);
}

// The camera is enabled by assigning its function to a UART in the Ports tab.
bool mztcIsEnabled(void)
{
    return mztcInitialized && findSerialPortConfig(FUNCTION_MZTC_CAMERA) != NULL;
}

mztcStatus_t* mztcGetStatus(void)
{
    return &mztcStatus;
}

// Identity bytes from the camera model or version reply.
const uint8_t *mztcGetDeviceId(uint8_t *len)
{
    if (len) {
        *len = mztcDeviceModelLen;
    }
    return mztcDeviceModelLen > 0 ? mztcDeviceModel : NULL;
}

bool mztcTriggerCalibration(void)
{
    if (!mztcIsEnabled() || !mztcIsConnected()) {
        return false;
    }

    if (mztcSendPacket(MZTC_CLASS_SHUTTER, MZTC_SUB_MANUAL_SHUTTER, MZTC_FLAG_WRITE, NULL, 0)) {
        mztcStatus.status = MZTC_STATUS_CALIBRATING;
        mztcLastCalibrationTime = millis();
        return true;
    }

    return false;
}

// The settings a preset owns. Zoom and mirror are deliberately absent: zoom
// belongs to the pilot and mirror describes how the camera is mounted, so a
// preset that overwrote either would fight the operator.
typedef struct mztcPresetValues_s {
    uint8_t palette_mode;
    uint8_t brightness;
    uint8_t contrast;
    uint8_t digital_enhancement;
    uint8_t spatial_denoise;
    uint8_t temporal_denoise;
    uint8_t auto_shutter;
    uint8_t ffc_interval;
} mztcPresetValues_t;

// Two constraints shape every row. Temporal denoising averages across frames,
// so on a moving airframe it smears targets and leaves trails. Spatial
// denoising trades noise for sharpness, and a person at search range is only a
// few pixels wide. Both stay low wherever small distant targets matter.
//
// Indexed by mztcPreset_e. MZTC_PRESET_CUSTOM has no row because it writes
// nothing.
static const mztcPresetValues_t mztcPresets[] = {
    [MZTC_PRESET_GENERAL] = {
        MZTC_PALETTE_WHITE_HOT, 50, 50, 50, 40, 20, MZTC_SHUTTER_TIME_AND_TEMP, 5
    },
    [MZTC_PRESET_FIRE] = {
        // Low enhancement is the important value here. Enhancement lifts
        // mid-tones, and flat mid-tones are what let an extreme spike dominate.
        MZTC_PALETTE_IRON_RED_1, 45, 75, 25, 30, 15, MZTC_SHUTTER_TIME_AND_TEMP, 10
    },
    [MZTC_PRESET_SEARCH] = {
        // A clothed body sits a few degrees over ambient, so enhancement goes
        // high. Both denoise values drop to keep a few-pixel target alive. The
        // short correction interval matters more than it looks, because a
        // drifting sensor grows fixed-pattern blobs that read as false targets.
        MZTC_PALETTE_WHITE_HOT, 55, 60, 80, 20, 10, MZTC_SHUTTER_TIME_AND_TEMP, 3
    },
    [MZTC_PRESET_SURVEILLANCE] = {
        // The one case where temporal denoising earns its keep. Loiter and
        // hover mean little frame to frame motion, so averaging cleans rather
        // than smears.
        MZTC_PALETTE_GREEN_HOT, 50, 55, 60, 45, 45, MZTC_SHUTTER_TIME_AND_TEMP, 15
    },
    [MZTC_PRESET_INSPECTION] = {
        // Comparing one panel cell against its neighbour is a uniformity
        // problem, hence the shortest correction interval in the set.
        MZTC_PALETTE_RAINBOW, 50, 45, 70, 55, 35, MZTC_SHUTTER_TIME_AND_TEMP, 2
    },
    [MZTC_PRESET_MARITIME] = {
        // A uniform cold background takes high contrast well. Enhancement
        // stays moderate because raising it amplifies wave texture into
        // clutter.
        MZTC_PALETTE_BLACK_HOT, 50, 70, 45, 25, 15, MZTC_SHUTTER_TIME_AND_TEMP, 5
    },
};

bool mztcSetPreset(mztcPreset_e preset)
{
    if (!mztcIsEnabled() || preset > MZTC_PRESET_MARITIME) {
        return false;
    }

    mztcConfig_t *cfg = mztcConfigMutable();
    cfg->preset = preset;
    mztcStatus.preset = preset;
    mztcLastAppliedPreset = preset;

    // Custom keeps whatever the user configured.
    if (preset == MZTC_PRESET_CUSTOM) {
        return true;
    }

    const mztcPresetValues_t *v = &mztcPresets[preset];
    cfg->palette_mode = v->palette_mode;
    cfg->brightness = v->brightness;
    cfg->contrast = v->contrast;
    cfg->digital_enhancement = v->digital_enhancement;
    cfg->spatial_denoise = v->spatial_denoise;
    cfg->temporal_denoise = v->temporal_denoise;
    cfg->auto_shutter = v->auto_shutter;
    cfg->ffc_interval = v->ffc_interval;

    // Push the whole set on the next task run rather than writing here, so the
    // burst stays out of whatever context called this.
    mztcConfigurationPending = true;
    return true;
}

bool mztcSetPalette(mztcPaletteMode_e palette)
{
    if (!mztcIsEnabled() || palette > MZTC_PALETTE_RED_HOT) {
        return false;
    }

    const uint8_t value = (uint8_t)palette;
    if (mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_PSEUDO_COLOR, MZTC_FLAG_WRITE, &value, 1)) {
        mztcConfigMutable()->palette_mode = value;
        mztcLastSentConfig.palette_mode = value;
        mztcMarkCustom();
        return true;
    }

    return false;
}

bool mztcSetZoom(mztcZoomLevel_e zoom)
{
    if (!mztcIsEnabled() || zoom > MZTC_ZOOM_8X) {
        return false;
    }

    const uint8_t value = (uint8_t)zoom;
    if (mztcSendPacket(MZTC_CLASS_DISPLAY, MZTC_SUB_ZOOM, MZTC_FLAG_WRITE, &value, 1)) {
        // Zoom belongs to the pilot. A zoom change does not move the
        // selection to CUSTOM.
        mztcConfigMutable()->zoom_level = value;
        mztcLastSentConfig.zoom_level = value;
        return true;
    }

    return false;
}

bool mztcSetImageParams(uint8_t brightness, uint8_t contrast, uint8_t enhancement)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    if (brightness > MZTC_MAX_PERCENT || contrast > MZTC_MAX_PERCENT || enhancement > MZTC_MAX_PERCENT) {
        return false;
    }

    bool success = mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_BRIGHTNESS, MZTC_FLAG_WRITE, &brightness, 1);
    success = mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_CONTRAST, MZTC_FLAG_WRITE, &contrast, 1) && success;
    success = mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_DIGITAL_ENHANCE, MZTC_FLAG_WRITE, &enhancement, 1) && success;

    if (success) {
        mztcConfigMutable()->brightness = brightness;
        mztcConfigMutable()->contrast = contrast;
        mztcConfigMutable()->digital_enhancement = enhancement;
        mztcLastSentConfig.brightness = brightness;
        mztcLastSentConfig.contrast = contrast;
        mztcLastSentConfig.digital_enhancement = enhancement;
        mztcMarkCustom();
    }

    return success;
}

bool mztcSetDenoising(uint8_t spatial, uint8_t temporal)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    if (spatial > MZTC_MAX_PERCENT || temporal > MZTC_MAX_PERCENT) {
        return false;
    }

    bool success = mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_SPATIAL_DENOISE, MZTC_FLAG_WRITE, &spatial, 1);
    success = mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_TEMPORAL_DENOISE, MZTC_FLAG_WRITE, &temporal, 1) && success;

    if (success) {
        mztcConfigMutable()->spatial_denoise = spatial;
        mztcConfigMutable()->temporal_denoise = temporal;
        mztcLastSentConfig.spatial_denoise = spatial;
        mztcLastSentConfig.temporal_denoise = temporal;
        mztcMarkCustom();
    }

    return success;
}

// Validate a candidate configuration in full. The MSP set handler uses this so
// that a rejected request changes nothing at all.
bool mztcConfigIsValid(const mztcConfig_t *cfg)
{
    if (!cfg) {
        return false;
    }

    if (cfg->preset > MZTC_PRESET_MARITIME ||
        cfg->palette_mode > MZTC_PALETTE_RED_HOT ||
        cfg->auto_shutter > MZTC_SHUTTER_TIME_AND_TEMP ||
        cfg->zoom_level > MZTC_ZOOM_8X ||
        cfg->mirror_mode > MZTC_MIRROR_CENTRAL) {
        return false;
    }

    if (cfg->ffc_interval < MZTC_MIN_FFC_INTERVAL || cfg->ffc_interval > MZTC_MAX_FFC_INTERVAL) {
        return false;
    }

    if (cfg->digital_enhancement > MZTC_MAX_PERCENT || cfg->spatial_denoise > MZTC_MAX_PERCENT ||
        cfg->temporal_denoise > MZTC_MAX_PERCENT || cfg->brightness > MZTC_MAX_PERCENT ||
        cfg->contrast > MZTC_MAX_PERCENT) {
        return false;
    }

    return true;
}

// Apply a complete validated configuration, as MSP2_SET_MZTC_CONFIG does. The
// host supplied every value. The preset byte is stored as a label. Expanding
// it would overwrite the explicit values that came in the same payload.
bool mztcApplyConfig(const mztcConfig_t *cfg)
{
    if (!mztcConfigIsValid(cfg)) {
        return false;
    }

    *mztcConfigMutable() = *cfg;
    mztcStatus.preset = cfg->preset;
    mztcLastAppliedPreset = cfg->preset;
    mztcConfigurationPending = true;
    return true;
}

// The camera runs its own shutter schedule from the interval we push to it
// with MZTC_SUB_SHUTTER_INTERVAL, so this only tracks elapsed time for the
// status and OSD surfaces. A host-side timer here would fight the camera.
static void mztcCheckCalibration(void)
{
    const uint32_t minutes = (millis() - mztcLastCalibrationTime) / (60u * 1000u);

    mztcStatus.last_calibration = (uint16_t)MIN(minutes, (uint32_t)UINT16_MAX);
}

STATIC_UNIT_TESTED void mztcSendConfiguration(void)
{
    if (!mztcSerialPort) {
        return;
    }

    const mztcConfig_t *cfg = mztcConfig();

    // The camera accepts 0x01 to 0x03 for the shutter mode and rejects 0x00 as
    // out of range, so the zero-based setting is shifted before it goes out.
    const uint8_t shutterMode = (uint8_t)(cfg->auto_shutter + MZTC_SHUTTER_WIRE_OFFSET);
    mztcSendPacket(MZTC_CLASS_SHUTTER, MZTC_SUB_AUTO_SHUTTER, MZTC_FLAG_WRITE, &shutterMode, 1);

    // Two byte interval in minutes, high byte first. Everything in the allowed
    // range fits in the low byte, so the high byte is always zero here.
    const uint8_t interval[2] = { 0, cfg->ffc_interval };
    mztcSendPacket(MZTC_CLASS_SHUTTER, MZTC_SUB_SHUTTER_INTERVAL, MZTC_FLAG_WRITE, interval, 2);

    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_DIGITAL_ENHANCE, MZTC_FLAG_WRITE, &cfg->digital_enhancement, 1);
    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_SPATIAL_DENOISE, MZTC_FLAG_WRITE, &cfg->spatial_denoise, 1);
    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_TEMPORAL_DENOISE, MZTC_FLAG_WRITE, &cfg->temporal_denoise, 1);
    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_BRIGHTNESS, MZTC_FLAG_WRITE, &cfg->brightness, 1);
    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_CONTRAST, MZTC_FLAG_WRITE, &cfg->contrast, 1);
    mztcSendPacket(MZTC_CLASS_IMAGE, MZTC_SUB_PSEUDO_COLOR, MZTC_FLAG_WRITE, &cfg->palette_mode, 1);
    mztcSendPacket(MZTC_CLASS_DISPLAY, MZTC_SUB_ZOOM, MZTC_FLAG_WRITE, &cfg->zoom_level, 1);
    mztcSendPacket(MZTC_CLASS_DISPLAY, MZTC_SUB_IMAGE_MIRROR, MZTC_FLAG_WRITE, &cfg->mirror_mode, 1);

    // Remember what went out, so the task can detect later generic changes.
    mztcLastSentConfig = *cfg;
    mztcLastSentValid = true;
}

void mztcRequestReconnect(void)
{
    if (mztcSerialPort != NULL) {
        closeSerialPort(mztcSerialPort);
        mztcSerialPort = NULL;
    }
    mztcResetLinkState();
    mztcStatus.status = MZTC_STATUS_INITIALIZING;
    mztcStatus.error_flags = 0;
    mztcLastPortRetry = 0; // force an immediate retry in the update loop
}

bool mztcSaveConfiguration(void)
{
    if (!mztcIsEnabled() || !mztcIsConnected()) {
        return false;
    }

    return mztcSendPacket(MZTC_CLASS_DEVICE, MZTC_SUB_SAVE_CONFIG, MZTC_FLAG_WRITE, NULL, 0);
}

// Vignetting correction is a one-shot action, not a stored setting. The camera
// manual requires the lens to be pointed at a uniform surface first, so this is
// only ever run on request.
bool mztcTriggerVignettingCorrection(void)
{
    if (!mztcIsEnabled() || !mztcIsConnected()) {
        return false;
    }

    const uint8_t trigger = MZTC_VIGNETTING_TRIGGER;
    return mztcSendPacket(MZTC_CLASS_SHUTTER, MZTC_SUB_VIGNETTING, MZTC_FLAG_WRITE, &trigger, 1);
}

bool mztcRestoreDefaults(void)
{
    if (!mztcIsEnabled() || !mztcIsConnected()) {
        return false;
    }

    if (!mztcSendPacket(MZTC_CLASS_DEVICE, MZTC_SUB_RESTORE_DEFAULTS, MZTC_FLAG_WRITE, NULL, 0)) {
        return false;
    }

    // Mirror the camera-side reset in our own copy of the settings.
    mztcConfig_t *cfg = mztcConfigMutable();
    cfg->brightness = SETTING_MZTC_BRIGHTNESS_DEFAULT;
    cfg->contrast = SETTING_MZTC_CONTRAST_DEFAULT;
    cfg->digital_enhancement = SETTING_MZTC_DIGITAL_ENHANCEMENT_DEFAULT;
    cfg->spatial_denoise = SETTING_MZTC_SPATIAL_DENOISE_DEFAULT;
    cfg->temporal_denoise = SETTING_MZTC_TEMPORAL_DENOISE_DEFAULT;
    cfg->palette_mode = SETTING_MZTC_PALETTE_MODE_DEFAULT;
    cfg->zoom_level = SETTING_MZTC_ZOOM_LEVEL_DEFAULT;
    cfg->mirror_mode = SETTING_MZTC_MIRROR_MODE_DEFAULT;
    cfg->auto_shutter = SETTING_MZTC_AUTO_SHUTTER_DEFAULT;
    cfg->ffc_interval = SETTING_MZTC_FFC_INTERVAL_DEFAULT;
    cfg->preset = SETTING_MZTC_PRESET_DEFAULT;
    mztcStatus.preset = cfg->preset;
    mztcLastAppliedPreset = cfg->preset;

    // Push the restored values. The stored settings and the camera then
    // agree about what the defaults are.
    mztcConfigurationPending = true;

    return true;
}


#ifdef UNIT_TEST
/*
 * Test hooks.
 *
 * The receive-path tests drive mztcSerialReceiveCallback() directly rather than
 * opening a port, so they need a way to put the driver into the state a live
 * link would have reached and to read back state the driver otherwise keeps to
 * itself.
 */
void mztcTestReset(void)
{
    memset(&mztcStatus, 0, sizeof(mztcStatus));
    mztcRxLen = 0;
    mztcRxExpected = 0;
    mztcProbeAttempts = 0;
    mztcProbeHistory = 0;
    mztcProbeWindowFill = 0;
    mztcConfigurationPending = false;
    mztcCalibrateBoxWasActive = false;
    mztcLastSentValid = false;
    mztcLastAppliedPreset = mztcConfig()->preset;
    mztcDeviceModelLen = 0;
    mztcInitialized = true;
    mztcLastCalibrationTime = 0;
    mztcLastValidResponse = 0;
    mztcLastProbeTime = 0;

    // Match the identity the stubbed Ports tab reports. The reassignment
    // check then sees a consistent assignment.
    const serialPortConfig_t *pc = findSerialPortConfig(FUNCTION_MZTC_CAMERA);
    if (pc) {
        mztcPortIdentifier = pc->identifier;
        mztcPortBaudIndex = pc->peripheral_baudrateIndex;
    }

    // Any non-null value. Nothing dereferences it, because serialWriteBufShim
    // is stubbed in the test.
    mztcSerialPort = (serialPort_t *)&mztcStatus;
}

// Clear the initialised flag so a test can run the real mztcInit() boot path.
// Without this mztcInit() returns immediately and a boot test proves nothing.
void mztcTestForceReinit(void)
{
    mztcInitialized = false;
}

void mztcTestSetStatus(uint8_t status)
{
    mztcStatus.status = status;
}

void mztcTestSetLastCalibration(uint16_t minutes)
{
    mztcStatus.last_calibration = minutes;
}
#endif

#endif // USE_MZTC
