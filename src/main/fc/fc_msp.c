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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common/log.h" //for MSP_SIMULATOR
#include "platform.h"

#include "blackbox/blackbox.h"

#include "build/debug.h"
#include "build/version.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"
#include "common/streambuf.h"
#include "common/string_light.h"
#include "common/bitarray.h"
#include "common/time.h"
#include "common/utils.h"
#include "programming/global_variables.h"
#include "programming/pid.h"

#include "config/parameter_group_ids.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/compass/compass.h"
#include "drivers/compass/compass_msp.h"
#include "drivers/barometer/barometer_msp.h"
#include "drivers/pitotmeter/pitotmeter_msp.h"
#include "sensors/battery_sensor_fake.h"
#include "drivers/bus_i2c.h"
#include "drivers/display.h"
#include "drivers/flash.h"
#include "drivers/osd.h"
#include "drivers/osd_symbols.h"
#include "drivers/pwm_mapping.h"
#ifdef USE_PINIO
#include "drivers/pinio.h"
#endif
#include "drivers/sdcard/sdcard.h"
#include "drivers/serial.h"
#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/vtx_common.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/control_profile.h"
#include "fc/fc_msp.h"
#include "fc/fc_msp_box.h"
#include "fc/fc_msp_dronecan.h"
#include "fc/firmware_update.h"
#include "fc/rc_adjustments.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer_profile.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"
#include "flight/ez_tune.h"

#include "config/config_eeprom.h"
#include "config/feature.h"

#include "io/adsb.h"
#include "io/asyncfatfs/asyncfatfs.h"
#include "io/flashfs.h"
#include "io/gps.h"
#include "io/gps_ublox.h"
#include "io/opflow.h"
#include "io/rangefinder.h"
#include "io/ledstrip.h"
#include "io/osd.h"
#include "io/serial.h"
#include "io/serial_4way.h"
#include "io/vtx.h"
#include "io/vtx_string.h"
#include "io/gps_private.h"  //for MSP_SIMULATOR
#include "io/headtracker_msp.h"

#include "io/osd/custom_elements.h"

#include "msp/msp.h"
#include "msp_protocol.h"
#include "msp/msp_serial.h"
#include "io/rangefinder.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h" //for MSP_SIMULATOR
#include "navigation/navigation_pos_estimator_private.h" //for MSP_SIMULATOR

#include "rx/rx.h"
#include "rx/msp.h"
#include "rx/msp_override.h"
#include "rx/srxl2.h"
#include "rx/crsf.h"
#include "rx/sim.h"

#include "scheduler/scheduler.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"
#include "sensors/diagnostics.h"
#include "sensors/battery.h"
#include "sensors/rangefinder.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/pitotmeter.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/opflow.h"
#include "sensors/temperature.h"
#include "sensors/esc_sensor.h"
#ifdef USE_WIND_ESTIMATOR
#include "flight/wind_estimator.h"
#endif

#include "telemetry/telemetry.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

/* Generated from msp/schema/msp_v2.yaml, plus the read/write helpers that own
 * the length gate and the sbuf advance. These must follow the INAV headers
 * above: the payload structs reference escSensorData_t, ledConfig_t and
 * boxBitmask_t, which INAV declares itself. */
#include "msp_msgs.h"
#include "msp/msp_handler.h"

extern timeDelta_t cycleTime; // FIXME dependency on mw.c

static const char * const flightControllerIdentifier = INAV_IDENTIFIER; // 4 UPPER CASE alpha numeric characters that identify the flight controller.
static const char * const boardIdentifier = TARGET_BOARD_IDENTIFIER;

// from mixer.c
extern int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];

static const char pidnames[] =
    "ROLL;"
    "PITCH;"
    "YAW;"
    "ALT;"
    "Pos;"
    "PosR;"
    "NavR;"
    "LEVEL;"
    "MAG;"
    "VEL;"
    "HEADING;"
    "SPEED;";

typedef enum {
    MSP_SDCARD_STATE_NOT_PRESENT = 0,
    MSP_SDCARD_STATE_FATAL       = 1,
    MSP_SDCARD_STATE_CARD_INIT   = 2,
    MSP_SDCARD_STATE_FS_INIT     = 3,
    MSP_SDCARD_STATE_READY       = 4
} mspSDCardState_e;

typedef enum {
    MSP_SDCARD_FLAG_SUPPORTTED   = 1
} mspSDCardFlags_e;

typedef enum {
    MSP_FLASHFS_BIT_READY        = 1,
    MSP_FLASHFS_BIT_SUPPORTED    = 2
} mspFlashfsFlags_e;

typedef enum {
    MSP_PASSTHROUGH_SERIAL_ID          = 0xFD,
    MSP_PASSTHROUGH_SERIAL_FUNCTION_ID = 0xFE,
    MSP_PASSTHROUGH_ESC_4WAY           = 0xFF,
 } mspPassthroughType_e;

static uint8_t mspPassthroughMode;
static uint8_t mspPassthroughArgument;

static serialPort_t *mspFindPassthroughSerialPort(void)
 {
    serialPortUsage_t *portUsage = NULL;

    switch (mspPassthroughMode) {
    case MSP_PASSTHROUGH_SERIAL_ID:
    {
        portUsage = findSerialPortUsageByIdentifier(mspPassthroughArgument);
        break;
    }
    case MSP_PASSTHROUGH_SERIAL_FUNCTION_ID:
    {
        const serialPortConfig_t *portConfig = findSerialPortConfig(1 << mspPassthroughArgument);
        if (portConfig) {
            portUsage = findSerialPortUsageByIdentifier(portConfig->identifier);
        }
        break;
    }
    }
    return portUsage ? portUsage->serialPort : NULL;
}

static void mspSerialPassthroughFn(serialPort_t *serialPort)
{
    serialPort_t *passthroughPort = mspFindPassthroughSerialPort();
    if (passthroughPort && serialPort) {
        serialPassthrough(passthroughPort, serialPort, NULL, NULL);
    }
}

static void mspFcSetPassthroughCommand(sbuf_t *dst, sbuf_t *src, mspPostProcessFnPtr *mspPostProcessFn)
{
    const unsigned int dataSize = sbufBytesRemaining(src);  /* Payload size in Bytes */

    if (dataSize == 0) {
        // Legacy format
        mspPassthroughMode = MSP_PASSTHROUGH_ESC_4WAY;
    } else {
        mspPassthroughMode = sbufReadU8(src);
        if (!sbufReadU8Safe(&mspPassthroughArgument, src)) {
            mspPassthroughArgument = 0;
        }
    }

    mspSetPassthroughReply_t reply = { .status = 0 };

    switch (mspPassthroughMode) {
    case MSP_PASSTHROUGH_SERIAL_ID:
    case MSP_PASSTHROUGH_SERIAL_FUNCTION_ID:
         if (mspFindPassthroughSerialPort()) {
             if (mspPostProcessFn) {
                 *mspPostProcessFn = mspSerialPassthroughFn;
             }
             reply.status = 1;
         }
         break;
#ifdef USE_SERIAL_4WAY_BLHELI_INTERFACE
    case MSP_PASSTHROUGH_ESC_4WAY:
        // get channel number
        // switch all motor lines HI
        // reply with the count of ESC found
        reply.status = esc4wayInit();

        if (mspPostProcessFn) {
            *mspPostProcessFn = esc4wayProcess;
        }
        break;
#endif
    default:
        break;
    }

    mspWriteReply(dst, &reply);
}

static void mspRebootNormalFn(serialPort_t *serialPort)
{
    UNUSED(serialPort);
    fcReboot(false);
}

static void mspRebootDfuFn(serialPort_t *serialPort)
{
    UNUSED(serialPort);
    fcReboot(true);
}

static mspResult_e mspFcRebootCommand(sbuf_t *src, mspPostProcessFnPtr *mspPostProcessFn)
{
    const unsigned int dataSize = sbufBytesRemaining(src);

    // Validate payload size: 0 or 1 byte only
    if (dataSize > 1) {
        return MSP_RESULT_ERROR;
    }

    // Determine reboot type and set appropriate post-process function
    if (mspPostProcessFn) {
        if (dataSize == 1) {
            // Read bootloader flag: 0 = normal, non-zero = DFU
            const bool bootloaderMode = (sbufReadU8(src) != 0);
            *mspPostProcessFn = bootloaderMode ? mspRebootDfuFn : mspRebootNormalFn;
        } else {
            // Legacy behavior: no parameter means normal reboot
            *mspPostProcessFn = mspRebootNormalFn;
        }
    }

    return MSP_RESULT_ACK;
}

static void serializeSDCardSummaryReply(sbuf_t *dst)
{
    mspSdcardSummaryReply_t reply = { .sdCardSupported = 0 };
#ifdef USE_SDCARD
    reply.sdCardSupported = MSP_SDCARD_FLAG_SUPPORTTED;

    // Merge the card and filesystem states together
    if (!sdcard_isInserted()) {
        reply.sdCardState = MSP_SDCARD_STATE_NOT_PRESENT;
    } else if (!sdcard_isFunctional()) {
        reply.sdCardState = MSP_SDCARD_STATE_FATAL;
    } else {
        switch (afatfs_getFilesystemState()) {
            case AFATFS_FILESYSTEM_STATE_READY:
                reply.sdCardState = MSP_SDCARD_STATE_READY;
                break;
            case AFATFS_FILESYSTEM_STATE_INITIALIZATION:
                reply.sdCardState = sdcard_isInitialized() ? MSP_SDCARD_STATE_FS_INIT : MSP_SDCARD_STATE_CARD_INIT;
                break;
            case AFATFS_FILESYSTEM_STATE_FATAL:
            case AFATFS_FILESYSTEM_STATE_UNKNOWN:
            default:
                reply.sdCardState = MSP_SDCARD_STATE_FATAL;
                break;
        }
    }

    reply.fsError = afatfs_getLastError();
    // Free space and total space in kilobytes
    reply.freeSpaceKB = afatfs_getContiguousFreeSpace() / 1024;
    reply.totalSpaceKB = sdcard_getMetadata()->numBlocks / 2; // Block size is half a kilobyte
#endif
    mspWriteReply(dst, &reply);
}

static void serializeDataflashSummaryReply(sbuf_t *dst)
{
    mspDataflashSummaryReply_t reply = { .flashReady = 0 };
#ifdef USE_FLASHFS
    const flashGeometry_t *geometry = flashGetGeometry();
    reply.flashReady = flashIsReady() ? 1 : 0;
    reply.sectorCount = geometry->sectors;
    reply.totalSize = geometry->totalSize;
    reply.usedSize = flashfsGetOffset(); // Effectively the current number of bytes stored on the volume
#endif
    mspWriteReply(dst, &reply);
}

#ifdef USE_FLASHFS
static void serializeDataflashReadReply(sbuf_t *dst, uint32_t address, uint16_t size)
{
    // Check how much bytes we can read - leave room for the address written below
    const int bytesRemainingInBuf = sbufBytesRemaining(dst) - (int)sizeof(address);
    uint16_t readLen = (size > bytesRemainingInBuf) ? bytesRemainingInBuf : size;

    // size will be lower than that requested if we reach end of volume
    const uint32_t flashfsSize = flashfsGetSize();
    if (readLen > flashfsSize - address) {
        // truncate the request
        readLen = flashfsSize - address;
    }

    // Write address
    sbufWriteU32(dst, address);

    // Read into streambuf directly
    const int bytesRead = flashfsReadAbs(address, sbufPtr(dst), readLen);
    sbufAdvance(dst, bytesRead);
}
#endif

static void mspApplyServoParams(uint8_t servoIndex, int16_t min, int16_t max, int16_t middle, int8_t rate)
{
    servoParamsMutable(servoIndex)->min    = min;
    servoParamsMutable(servoIndex)->max    = max;
    servoParamsMutable(servoIndex)->middle = middle;
    servoParamsMutable(servoIndex)->rate   = rate;
    servoComputeScalingFactors(servoIndex);
}

/* Shared tail of MSP2_INAV_SET_MISC and MSP2_INAV_SET_BATTERY_CONFIG: validate the
 * voltage source and capacity unit that were just written, and keep the OSD energy
 * unit in step with the capacity unit. Returns false when a value was out of range,
 * in which case it has been forced back to its default. */
static bool mspApplyBatteryUnits(uint8_t previousCapacityUnit)
{
    if ((batteryMetersConfig()->voltageSource != BAT_VOLTAGE_RAW) && (batteryMetersConfig()->voltageSource != BAT_VOLTAGE_SAG_COMP)) {
        batteryMetersConfigMutable()->voltageSource = BAT_VOLTAGE_RAW;
        return false;
    }
    if ((batteryMetersConfig()->capacity_unit != BAT_CAPACITY_UNIT_MAH) && (batteryMetersConfig()->capacity_unit != BAT_CAPACITY_UNIT_MWH)) {
        batteryMetersConfigMutable()->capacity_unit = BAT_CAPACITY_UNIT_MAH;
        return false;
    }
    if (previousCapacityUnit != batteryMetersConfig()->capacity_unit) {
        osdConfigMutable()->stats_energy_unit = (batteryMetersConfig()->capacity_unit == BAT_CAPACITY_UNIT_MAH)
            ? OSD_STATS_ENERGY_UNIT_MAH
            : OSD_STATS_ENERGY_UNIT_WH;
    }
    return true;
}

/* One record of MSP2_INAV_SERVO_CONFIG / MSP_SERVO_CONFIGURATIONS. Emitting a
 * record at a time keeps the whole array off the stack. */
static void mspSerializeServoParams(sbuf_t *dst, const servoParam_t *sp)
{
    __typeof__(((msp2InavServoConfigReply_t *)0)->items[0]) record = {
        .min    = sp->min,
        .max    = sp->max,
        .middle = sp->middle,
        .rate   = sp->rate,
    };
    sbufWriteData(dst, &record, sizeof(record));
}

/* One record of MSP2_COMMON_MOTOR_MIXER: [throttle, roll, pitch, yaw]. */
static void mspSerializeMotorMixer(sbuf_t *dst, const motorMixer_t *m)
{
    __typeof__(((msp2CommonMotorMixerReply_t *)0)->items[0]) record = {
        .motorMix = {
            constrainf(m->throttle + 2.0f, 0.0f, 4.0f) * 1000,
            constrainf(m->roll     + 2.0f, 0.0f, 4.0f) * 1000,
            constrainf(m->pitch    + 2.0f, 0.0f, 4.0f) * 1000,
            constrainf(m->yaw      + 2.0f, 0.0f, 4.0f) * 1000,
        },
    };
    sbufWriteData(dst, &record, sizeof(record));
}

/* One rule of MSP2_INAV_SERVO_MIXER. The reply is a block of MAX_SERVO_RULES of
 * these per mixer profile. */
static void mspSerializeServoMixer(sbuf_t *dst, const servoMixer_t *m)
{
    __typeof__(((msp2InavServoMixerReply_t *)0)->items[0]) record = {
        .targetChannel = m->targetChannel,
        .inputSource = m->inputSource,
        .rate = m->rate,
        .speed = m->speed,
#ifdef USE_PROGRAMMING_FRAMEWORK
        .conditionId = m->conditionId,
#else
        .conditionId = -1,
#endif
    };
    sbufWriteData(dst, &record, sizeof(record));
}

/*
 * Returns true if the command was processd, false otherwise.
 * May set mspPostProcessFunc to a function to be called once the command has been processed
 */
static bool mspFcProcessOutCommand(uint16_t cmdMSP, sbuf_t *dst, mspPostProcessFnPtr *mspPostProcessFn)
{
    UNUSED(mspPostProcessFn);

    switch (cmdMSP) {
    case MSP_API_VERSION:
        {
            mspApiVersionReply_t reply = {
                .mspProtocolVersion = MSP_PROTOCOL_VERSION,
                .apiVersionMajor    = API_VERSION_MAJOR,
                .apiVersionMinor    = API_VERSION_MINOR,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FC_VARIANT:
        {
            mspFcVariantReply_t reply;
            memcpy(reply.fcVariantIdentifier, flightControllerIdentifier,
                   sizeof(reply.fcVariantIdentifier));
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FC_VERSION:
        {
            mspFcVersionReply_t reply = {
                .fcVersionMajor = FC_VERSION_MAJOR,
                .fcVersionMinor = FC_VERSION_MINOR,
                .fcVersionPatch = FC_VERSION_PATCH_LEVEL,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_BOARD_INFO:
    {
        /* Variable-length tail: the struct's flexible targetName[] means
         * sizeof() is the fixed head, and the name follows it. */
        mspBoardInfoReply_t reply = {0};
        memcpy(reply.boardIdentifier, boardIdentifier, sizeof(reply.boardIdentifier));
#ifdef USE_HARDWARE_REVISION_DETECTION
        reply.hardwareRevision = hardwareRevision;
#else
        reply.hardwareRevision = 0; // No other build targets currently have hardware revision detection.
#endif
        // OSD support (for BF compatibility):
        // 0 = no OSD
        // 1 = OSD slave (not supported in INAV)
        // 2 = OSD chip on board
#if defined(USE_OSD)
        reply.osdSupport = 2;
#else
        reply.osdSupport = 0;
#endif
        // Board communication capabilities (uint8)
        // Bit 0: 1 iff the board has VCP
        // Bit 1: 1 iff the board supports software serial
#ifdef USE_VCP
        reply.commCapabilities |= 1 << 0;
#endif
#if defined(USE_SOFTSERIAL1) || defined(USE_SOFTSERIAL2)
        reply.commCapabilities |= 1 << 1;
#endif
        reply.targetNameLength = strlen(targetName);

        mspWriteReplyBytes(dst, &reply, sizeof(reply));
        sbufWriteData(dst, targetName, reply.targetNameLength);
        break;
    }

    case MSP_BUILD_INFO:
        {
            mspBuildInfoReply_t reply;
            memcpy(reply.buildDate,   buildDate,        sizeof(reply.buildDate));
            memcpy(reply.buildTime,   buildTime,        sizeof(reply.buildTime));
            memcpy(reply.gitRevision, shortGitRevision, sizeof(reply.gitRevision));
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_SENSOR_STATUS:
        {
            mspSensorStatusReply_t reply = {
                .overallHealth     = isHardwareHealthy() ? 1 : 0,
                .gyroStatus        = getHwGyroStatus(),
                .accStatus         = getHwAccelerometerStatus(),
                .magStatus         = getHwCompassStatus(),
                .baroStatus        = getHwBarometerStatus(),
                .gpsStatus         = getHwGPSStatus(),
                .rangefinderStatus = getHwRangefinderStatus(),
                .pitotStatus       = getHwPitotmeterStatus(),
                .opflowStatus      = getHwOpticalFlowStatus(),
            };
            mspWriteReply(dst, &reply);
        }

        isMspConfigActive(true);  // used to indicate configurator connection active
        break;

    case MSP_ACTIVEBOXES:
        {
            mspActiveboxesReply_t reply;
            packBoxModeFlags(&reply.activeModes);
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_STATUS_EX:
    case MSP_STATUS:
        {
            boxBitmask_t mspBoxModeFlags;
            packBoxModeFlags(&mspBoxModeFlags);
#ifdef USE_I2C
            const uint16_t i2cErrors = i2cGetErrorCounter();
#else
            const uint16_t i2cErrors = 0;
#endif
            /* Two schema messages, so two structs: MSP_STATUS_EX carries the
             * same head plus load/arming/calibration. activeModesLow is the
             * first 4 bytes of the box bitmask. */
            if (cmdMSP == MSP_STATUS_EX) {
                mspStatusExReply_t reply = {
                    .cycleTime         = (uint16_t)cycleTime,
                    .i2cErrors         = i2cErrors,
                    .sensorStatus      = packSensorStatus(),
                    .profile           = getConfigProfile(),
                    .cpuLoad           = averageSystemLoadPercent,
                    .armingFlags       = armingFlags,
                    .accCalibAxisFlags = accGetCalibrationAxisFlags(),
                };
                memcpy(&reply.activeModesLow, &mspBoxModeFlags, sizeof(reply.activeModesLow));
                mspWriteReply(dst, &reply);
            } else {
                mspStatusReply_t reply = {
                    .cycleTime    = (uint16_t)cycleTime,
                    .i2cErrors    = i2cErrors,
                    .sensorStatus = packSensorStatus(),
                    .profile      = getConfigProfile(),
                };
                memcpy(&reply.activeModesLow, &mspBoxModeFlags, sizeof(reply.activeModesLow));
                mspWriteReply(dst, &reply);
            }
        }
        break;

        case MSP2_INAV_STATUS:
        {
            // Preserves full arming flags and box modes
            msp2InavStatusReply_t reply = {
                .cycleTime             = (uint16_t)cycleTime,
#ifdef USE_I2C
                .i2cErrors             = i2cGetErrorCounter(),
#else
                .i2cErrors             = 0,
#endif
                .sensorStatus          = packSensorStatus(),
                .cpuLoad               = averageSystemLoadPercent,
                .profileAndBattProfile = (getConfigBatteryProfile() << 4) | getConfigProfile(),
                .armingFlags           = armingFlags,
                .mixerProfile          = getConfigMixerProfile(),
            };
            packBoxModeFlags(&reply.activeModes);
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RAW_IMU:
        {
            /* mag stays zero when USE_MAG is out: the field keeps its byte. */
            mspRawImuReply_t reply = {
                .accX  = (int16_t)lrintf(acc.accADCf[X] * 512),
                .accY  = (int16_t)lrintf(acc.accADCf[Y] * 512),
                .accZ  = (int16_t)lrintf(acc.accADCf[Z] * 512),
                .gyroX = gyroRateDps(X),
                .gyroY = gyroRateDps(Y),
                .gyroZ = gyroRateDps(Z),
#ifdef USE_MAG
                .magX  = lrintf(mag.magADC[X]),
                .magY  = lrintf(mag.magADC[Y]),
                .magZ  = lrintf(mag.magADC[Z]),
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_SERVO:
        {
            mspServoReply_t reply;
            memcpy(reply.servoOutputs, servo, sizeof(reply.servoOutputs));
            mspWriteReply(dst, &reply);
        }
        break;
    case MSP_SERVO_CONFIGURATIONS:
        /* Same endpoints as MSP2_INAV_SERVO_CONFIG plus a legacy tail. */
        for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
            const servoParam_t *sp = servoParams(i);
            __typeof__(((mspServoConfigurationsReply_t *)0)->items[0]) record = {
                .min = sp->min, .max = sp->max, .middle = sp->middle, .rate = sp->rate,
                .reserved1 = 0,
                .reserved2 = 0,
                .legacyForwardChan = 255,      // was forwardFromChannel, unused
                .legacyReversedSources = 0,    // input reversing is done at mixer level
            };
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;
    case MSP2_INAV_SERVO_CONFIG:
        for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
            mspSerializeServoParams(dst, servoParams(i));
        }
        break;
    case MSP_SERVO_MIX_RULES:
        for (int i = 0; i < MAX_SERVO_RULES; i++) {
            const servoMixer_t *m = customServoMixers(i);
            __typeof__(((mspServoMixRulesReply_t *)0)->items[0]) record = {
                .targetChannel = m->targetChannel,
                .inputSource = m->inputSource,
                .rate = m->rate,
                .speed = m->speed,
                .reserved1 = 0,
                .legacyMax = 100,
                .legacyBox = 0,
            };
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;
    case MSP2_INAV_SERVO_MIXER:
        for (int i = 0; i < MAX_SERVO_RULES; i++) {
            mspSerializeServoMixer(dst, customServoMixers(i));
        }
        if(MAX_MIXER_PROFILE_COUNT==1) break;
        for (int i = 0; i < MAX_SERVO_RULES; i++) {
            mspSerializeServoMixer(dst, &mixerServoMixersByIndex(nextMixerProfileIndex)[i]);
        }
        break;
#ifdef USE_PROGRAMMING_FRAMEWORK
    case MSP2_INAV_LOGIC_CONDITIONS:
        return false; // Deprecated, causes buffer overflow for 14*64 bytes.
    case MSP2_INAV_LOGIC_CONDITIONS_STATUS:
        {
            msp2InavLogicConditionsStatusReply_t reply;
            for (int i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
                reply.conditionValues[i] = logicConditionGetValue(i);
            }
            mspWriteReply(dst, &reply);
        }
        break;
    case MSP2_INAV_LOGIC_CONDITIONS_CONFIGURED:
        {
            // Returns 8-byte bitmask where bit N = 1 if logic condition N is configured (non-default)
            uint64_t mask = 0;
            for (int i = 0; i < MIN(MAX_LOGIC_CONDITIONS, 64); i++) {
                const logicCondition_t *lc = logicConditions(i);
                // Check if any field differs from default reset values
                bool isConfigured = (lc->enabled != 0) ||
                                    (lc->activatorId != -1) ||
                                    (lc->operation != 0) ||
                                    (lc->operandA.type != LOGIC_CONDITION_OPERAND_TYPE_VALUE) ||
                                    (lc->operandA.value != 0) ||
                                    (lc->operandB.type != LOGIC_CONDITION_OPERAND_TYPE_VALUE) ||
                                    (lc->operandB.value != 0) ||
                                    (lc->flags != 0);
                if (isConfigured) {
                    mask |= ((uint64_t)1 << i);
                }
            }
            msp2InavLogicConditionsConfiguredReply_t reply = {
                .configuredMaskLow  = (uint32_t)(mask & 0xFFFFFFFF),
                .configuredMaskHigh = (uint32_t)((mask >> 32) & 0xFFFFFFFF),
            };
            mspWriteReply(dst, &reply);
        }
        break;
    case MSP2_INAV_GVAR_STATUS:
        {
            msp2InavGvarStatusReply_t reply;
            for (int i = 0; i < MAX_GLOBAL_VARIABLES; i++) {
                reply.gvarValues[i] = gvGet(i);
            }
            mspWriteReply(dst, &reply);
        }
        break;
    case MSP2_INAV_PROGRAMMING_PID:
        {
            msp2InavProgrammingPidReply_t reply;
            for (int i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
                const programmingPid_t *pid = programmingPids(i);
                reply.items[i] = (__typeof__(reply.items[0])){
                    .enabled = pid->enabled,
                    .setpointType = pid->setpoint.type,
                    .setpointValue = pid->setpoint.value,
                    .measurementType = pid->measurement.type,
                    .measurementValue = pid->measurement.value,
                    .gainP = pid->gains.P,
                    .gainI = pid->gains.I,
                    .gainD = pid->gains.D,
                    .gainFF = pid->gains.FF,
                };
            }
            mspWriteReply(dst, &reply);
        }
        break;
    case MSP2_INAV_PROGRAMMING_PID_STATUS:
        {
            msp2InavProgrammingPidStatusReply_t reply;
            for (int i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
                reply.pidOutputs[i] = programmingPidGetOutput(i);
            }
            mspWriteReply(dst, &reply);
        }
        break;
#endif
    case MSP2_COMMON_MOTOR_MIXER:
        for (uint8_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
            mspSerializeMotorMixer(dst, primaryMotorMixer(i));
        }
        if (MAX_MIXER_PROFILE_COUNT==1) break;
        for (uint8_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
            mspSerializeMotorMixer(dst, &mixerMotorMixersByIndex(nextMixerProfileIndex)[i]);
        }
        break;

    case MSP_MOTOR:
        {
            mspMotorReply_t reply;
            for (unsigned i = 0; i < ARRAYLEN(reply.motorOutputs); i++) {
                reply.motorOutputs[i] = i < MAX_SUPPORTED_MOTORS ? motor[i] : 0;
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RC:
        for (int i = 0; i < rxRuntimeConfig.channelCount; i++) {
            sbufWriteU16(dst, rxGetChannelValue(i));
        }
        break;

    case MSP_ATTITUDE:
        {
            mspAttitudeReply_t reply = {
                .roll = attitude.values.roll,
                .pitch = attitude.values.pitch,
                .yaw = DECIDEGREES_TO_DEGREES(attitude.values.yaw),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_ALTITUDE:
        {
            mspAltitudeReply_t reply = {
                .estimatedAltitude = lrintf(getEstimatedActualPosition(Z)),
                .variometer = lrintf(getEstimatedActualVelocity(Z)),
#if defined(USE_BARO)
                .baroAltitude = baroGetLatestAltitude(),
#else
                .baroAltitude = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_FULL_LOCAL_POSE:
        {
            const navEstimatedPosVel_t *absoluteActualState = &posControl.actualState.abs;
            msp2InavFullLocalPoseReply_t reply = {
                .roll = attitude.values.roll,
                .pitch = attitude.values.pitch,
                .yaw = attitude.values.yaw,
                .localPositionNorth = lrintf(absoluteActualState->pos.v[X]),
                .localVelocityNorth = lrintf(absoluteActualState->vel.v[X]),
                .localPositionEast = lrintf(absoluteActualState->pos.v[Y]),
                .localVelocityEast = lrintf(absoluteActualState->vel.v[Y]),
                .localPositionUp = lrintf(absoluteActualState->pos.v[Z]),
                .localVelocityUp = lrintf(absoluteActualState->vel.v[Z]),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_TIMESYNC:
        {
            msp2InavTimesyncReply_t reply = { .timeNs = (uint64_t)micros() * 1000ULL };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_SONAR_ALTITUDE:
        {
            mspSonarAltitudeReply_t reply = {
#ifdef USE_RANGEFINDER
                .rangefinderAltitude = rangefinderGetLatestAltitude(),
#else
                .rangefinderAltitude = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_OPTICAL_FLOW:
        {
            msp2InavOpticalFlowReply_t reply = {
#ifdef USE_OPFLOW
                .quality = opflow.rawQuality,
                .flowRateX = RADIANS_TO_DEGREES(opflow.flowRate[X]),
                .flowRateY = RADIANS_TO_DEGREES(opflow.flowRate[Y]),
                .bodyRateX = RADIANS_TO_DEGREES(opflow.bodyRate[X]),
                .bodyRateY = RADIANS_TO_DEGREES(opflow.bodyRate[Y]),
#else
                .quality = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_ANALOG:
        {
            mspAnalogReply_t reply = {
                .vbat = constrain(getBatteryVoltage() / 10, 0, 255),
                .mAhDrawn = constrain(getMAhDrawn(), 0, 0xFFFF),
                .rssi = getRSSI(),
                // 0.01 A steps, range is -320A to 320A
                .amperage = constrain(getAmperage(), -0x8000, 0x7FFF),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_ANALOG:
        {
            msp2InavAnalogReply_t reply = {
                // Bit 1: battery full, Bit 2: use capacity threshold, Bit 3-4: battery state, Bit 5-8: battery cell count
                .batteryFlags = batteryWasFullWhenPluggedIn() | (batteryUsesCapacityThresholds() << 1)
                                | (getBatteryState() << 2) | (getBatteryCellCount() << 4),
                .vbat = getBatteryVoltage(),
                .amperage = getAmperage(),
                .powerDraw = getPower(),
                .mAhDrawn = getMAhDrawn(),
                .mWhDrawn = getMWhDrawn(),
                .remainingCapacity = getBatteryRemainingCapacity(),
                .percentageRemaining = calculateBatteryPercentage(),
                .rssi = getRSSI(),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_GET_LINK_STATS:
        {
            msp2InavGetLinkStatsReply_t reply = {
                .uplinkRSSI_dBm = -rxLinkStatistics.uplinkRSSI,
                .uplinkLQ = rxLinkStatistics.uplinkLQ,
                .uplinkSNR = rxLinkStatistics.uplinkSNR,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_LOOP_TIME:
        {
            mspLoopTimeReply_t reply = { .looptime = gyroConfig()->looptime };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RC_TUNING:
        {
            mspRcTuningReply_t reply = {
                .legacyRcRate = 100, // kept for compatibility, this setting is no longer used
                .rcExpo = currentControlProfile->stabilized.rcExpo8,
                .rollRate = currentControlProfile->stabilized.rates[FD_ROLL],
                .pitchRate = currentControlProfile->stabilized.rates[FD_PITCH],
                .yawRate = currentControlProfile->stabilized.rates[FD_YAW],
                .dynamicThrottlePID = currentControlProfile->throttle.dynPID,
                .throttleMid = currentControlProfile->throttle.rcMid8,
                .throttleExpo = currentControlProfile->throttle.rcExpo8,
                .tpaBreakpoint = currentControlProfile->throttle.pa_breakpoint,
                .rcYawExpo = currentControlProfile->stabilized.rcYawExpo8,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_RATE_PROFILE:
        {
            msp2InavRateProfileReply_t reply = {
                .throttleMid = currentControlProfile->throttle.rcMid8,
                .throttleExpo = currentControlProfile->throttle.rcExpo8,
                .dynamicThrottlePID = currentControlProfile->throttle.dynPID,
                .tpaBreakpoint = currentControlProfile->throttle.pa_breakpoint,

                .stabRcExpo = currentControlProfile->stabilized.rcExpo8,
                .stabRcYawExpo = currentControlProfile->stabilized.rcYawExpo8,
                .stabRollRate = currentControlProfile->stabilized.rates[FD_ROLL],
                .stabPitchRate = currentControlProfile->stabilized.rates[FD_PITCH],
                .stabYawRate = currentControlProfile->stabilized.rates[FD_YAW],

                .manualRcExpo = currentControlProfile->manual.rcExpo8,
                .manualRcYawExpo = currentControlProfile->manual.rcYawExpo8,
                .manualRollRate = currentControlProfile->manual.rates[FD_ROLL],
                .manualPitchRate = currentControlProfile->manual.rates[FD_PITCH],
                .manualYawRate = currentControlProfile->manual.rates[FD_YAW],
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_PID:
        {
            msp2PidReply_t reply;
            for (int i = 0; i < PID_ITEM_COUNT; i++) {
                reply.items[i].P = constrain(pidBank()->pid[i].P, 0, 255);
                reply.items[i].I = constrain(pidBank()->pid[i].I, 0, 255);
                reply.items[i].D = constrain(pidBank()->pid[i].D, 0, 255);
                reply.items[i].FF = constrain(pidBank()->pid[i].FF, 0, 255);
            }
            mspWriteReply(dst, &reply);
        }
        #ifdef USE_EZ_TUNE
            ezTuneUpdate();
        #endif
        break;

    case MSP_PIDNAMES:
        for (const char *c = pidnames; *c; c++) {
            sbufWriteU8(dst, *c);
        }
        break;

    case MSP_MODE_RANGES:
        {
            mspModeRangesReply_t reply;
            for (int i = 0; i < MAX_MODE_ACTIVATION_CONDITION_COUNT; i++) {
                const modeActivationCondition_t *mac = modeActivationConditions(i);
                const box_t *box = findBoxByActiveBoxId(mac->modeId);
                reply.items[i].modePermanentId = box ? box->permanentId : 0;
                reply.items[i].auxChannelIndex = mac->auxChannelIndex;
                reply.items[i].rangeStartStep = mac->range.startStep;
                reply.items[i].rangeEndStep = mac->range.endStep;
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_ADJUSTMENT_RANGES:
        {
            mspAdjustmentRangesReply_t reply;
            for (int i = 0; i < MAX_ADJUSTMENT_RANGE_COUNT; i++) {
                const adjustmentRange_t *adjRange = adjustmentRanges(i);
                reply.items[i].adjustmentIndex = adjRange->adjustmentIndex;
                reply.items[i].auxChannelIndex = adjRange->auxChannelIndex;
                reply.items[i].rangeStartStep = adjRange->range.startStep;
                reply.items[i].rangeEndStep = adjRange->range.endStep;
                reply.items[i].adjustmentFunction = adjRange->adjustmentFunction;
                reply.items[i].auxSwitchChannelIndex = adjRange->auxSwitchChannelIndex;
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_BOXNAMES:
        if (!serializeBoxNamesReply(dst)) {
            return false;
        }
        break;

    case MSP_BOXIDS:
        serializeBoxReply(dst);
        break;

    case MSP_MISC:
        {
            /* Fields left out of the initializer are zero: that is exactly what the
             * #else arms used to write when a feature is compiled out. */
            mspMiscReply_t reply = {
                .midRc = PWM_RANGE_MIDDLE,
                .legacyMinThrottle = 0, // was min_throttle
                .maxThrottle = getMaxThrottle(),
                .minCommand = motorConfig()->mincommand,
                .failsafeThrottle = currentBatteryProfile->failsafe_throttle,
#ifdef USE_GPS
                .gpsType = gpsConfig()->provider,
                .gpsSbasMode = gpsConfig()->sbasMode,
#endif
                .legacyGpsBaud = 0, // TODO gps_baudrate (an index, cleanflight uses a uint32_t)
                .legacyMwCurrentOut = 0,
                .rssiChannel = rxConfig()->rssi_channel,
#ifdef USE_MAG
                .magDeclination = compassConfig()->mag_declination / 10,
#endif
#ifdef USE_ADC
                .vbatScale = batteryMetersConfig()->voltage.scale / 10,
                .vbatMinCell = currentBatteryProfile->voltage.cellMin / 10,
                .vbatMaxCell = currentBatteryProfile->voltage.cellMax / 10,
                .vbatWarningCell = currentBatteryProfile->voltage.cellWarning / 10,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_MISC:
        {
            msp2InavMiscReply_t reply = {
                .midRc = PWM_RANGE_MIDDLE,
                .legacyMinThrottle = 0, // was min_throttle
                .maxThrottle = getMaxThrottle(),
                .minCommand = motorConfig()->mincommand,
                .failsafeThrottle = currentBatteryProfile->failsafe_throttle,
#ifdef USE_GPS
                .gpsType = gpsConfig()->provider,
                .gpsSbasMode = gpsConfig()->sbasMode,
#endif
                .legacyGpsBaud = 0, // TODO gps_baudrate (an index, cleanflight uses a uint32_t)
                .rssiChannel = rxConfig()->rssi_channel,
#ifdef USE_MAG
                .magDeclination = compassConfig()->mag_declination / 10,
#endif
#ifdef USE_ADC
                .vbatScale = batteryMetersConfig()->voltage.scale,
                .vbatSource = batteryMetersConfig()->voltageSource,
                .cellCount = currentBatteryProfile->cells,
                .vbatCellDetect = currentBatteryProfile->voltage.cellDetect,
                .vbatMinCell = currentBatteryProfile->voltage.cellMin,
                .vbatMaxCell = currentBatteryProfile->voltage.cellMax,
                .vbatWarningCell = currentBatteryProfile->voltage.cellWarning,
#endif
                .capacityValue = currentBatteryProfile->capacity.value,
                .capacityWarning = currentBatteryProfile->capacity.warning,
                .capacityCritical = currentBatteryProfile->capacity.critical,
                .capacityUnit = batteryMetersConfig()->capacity_unit,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_MISC2:
        {
            msp2InavMisc2Reply_t reply = {
                .uptimeSeconds = micros() / 1000000,
                .flightTimeSeconds = getFlightTime(),
                .throttlePercent = getThrottlePercent(true),
                .autoThrottleFlag = navigationIsControllingThrottle() ? 1 : 0,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_BATTERY_CONFIG:
        {
            msp2InavBatteryConfigReply_t reply = {
#ifdef USE_ADC
                .vbatScale = batteryMetersConfig()->voltage.scale,
                .vbatSource = batteryMetersConfig()->voltageSource,
                .cellCount = currentBatteryProfile->cells,
                .vbatCellDetect = currentBatteryProfile->voltage.cellDetect,
                .vbatMinCell = currentBatteryProfile->voltage.cellMin,
                .vbatMaxCell = currentBatteryProfile->voltage.cellMax,
                .vbatWarningCell = currentBatteryProfile->voltage.cellWarning,
#endif
                .currentOffset = batteryMetersConfig()->current.offset,
                .currentScale = batteryMetersConfig()->current.scale,
                .capacityValue = currentBatteryProfile->capacity.value,
                .capacityWarning = currentBatteryProfile->capacity.warning,
                .capacityCritical = currentBatteryProfile->capacity.critical,
                .capacityUnit = batteryMetersConfig()->capacity_unit,
            };
            mspWriteReply(dst, &reply);
        }
        break;

#ifdef USE_GPS
    case MSP_RAW_GPS:
        {
            mspRawGpsReply_t reply = {
                .fixType = gpsSol.fixType,
                .numSat = gpsSol.numSat,
                .latitude = gpsSol.llh.lat,
                .longitude = gpsSol.llh.lon,
                .altitude = gpsSol.llh.alt / 100, // meters
                .speed = gpsSol.groundSpeed,
                .groundCourse = gpsSol.groundCourse,
                .hdop = gpsSol.hdop,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_COMP_GPS:
        {
            mspCompGpsReply_t reply = {
                .distanceToHome = GPS_distanceToHome,
                .directionToHome = GPS_directionToHome,
                .gpsHeartbeat = gpsSol.flags.gpsHeartbeat ? 1 : 0,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_NAV_STATUS:
        {
            mspNavStatusReply_t reply = {
                .navMode = NAV_Status.mode,
                .navState = NAV_Status.state,
                .activeWpAction = NAV_Status.activeWpAction,
                .activeWpNumber = NAV_Status.activeWpNumber,
                .navError = NAV_Status.error,
                .targetHeading = getHeadingHoldTarget(),
                .desiredHeading = navDesiredHeading, // guidance course/track (centideg)
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_GPSSVINFO:
        {
            /* Compatibility stub - return zero SVs */
            mspGpssvinfoReply_t reply = {
                .protocolVersion = 1,
                .numChannels = 0,
                .hdopHundredsDigit = 0,
                .hdopTensDigit = gpsSol.hdop / 100,
                .hdopUnitsDigit = gpsSol.hdop / 100,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_GPSSTATISTICS:
        {
            mspGpsstatisticsReply_t reply = {
                .lastMessageDt = gpsStats.lastMessageDt,
                .errors = gpsStats.errors,
                .timeouts = gpsStats.timeouts,
                .packetCount = gpsStats.packetCount,
                .hdop = gpsSol.hdop,
                .eph = gpsSol.eph,
                .epv = gpsSol.epv,
                .hwVersion = gpsState.hwVersion,
            };
            mspWriteReply(dst, &reply);
        }
        break;
#endif
    case MSP2_ADSB_VEHICLE_LIST:
        {
            /* Fixed header, then one record per vehicle slot. With USE_ADSB off the
             * header is all zeroes and no records follow. */
            msp2AdsbVehicleListReply_t reply = {
#ifdef USE_ADSB
                .maxVehicles = MAX_ADSB_VEHICLES,
                .callsignLength = ADSB_CALL_SIGN_MAX_LENGTH,
                .totalVehicleMsgs = getAdsbStatus()->vehiclesMessagesTotal,
                .totalHeartbeatMsgs = getAdsbStatus()->heartbeatMessagesTotal,
#else
                .maxVehicles = 0,
#endif
            };
            mspWriteReplyBytes(dst, &reply, sizeof(reply));
#ifdef USE_ADSB
            for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
                const adsbVehicle_t *adsbVehicle = findVehicle(i);
                __typeof__(reply.items[0]) record = {
                    .icao = adsbVehicle->vehicleValues.icao,
                    .lat = adsbVehicle->vehicleValues.gps.lat,
                    .lon = adsbVehicle->vehicleValues.gps.lon,
                    .alt = adsbVehicle->vehicleValues.alt,
                    .headingDeg = CENTIDEGREES_TO_DEGREES(adsbVehicle->vehicleValues.heading),
                    .tslc = adsbVehicle->vehicleValues.tslc,
                    .emitterType = adsbVehicle->vehicleValues.emitterType,
                    .ttl = adsbVehicle->ttl,
                };
                memcpy(record.callsign, adsbVehicle->vehicleValues.callsign, sizeof(record.callsign));
                sbufWriteData(dst, &record, sizeof(record));
            }
#endif
        }
        break;

    case MSP2_ADSB_LIMITS:
        {
            msp2AdsbLimitsReply_t reply = {
#ifdef USE_ADSB
                .distanceWarning = osdConfig()->adsb_distance_warning,
                .distanceAlert = osdConfig()->adsb_distance_alert,
                .ignorePlaneAboveMeLimit = osdConfig()->adsb_ignore_plane_above_me_limit,
#else
                .distanceWarning = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_ADSB_WARNING_VEHICLE_ICAO:
        {
            msp2AdsbWarningVehicleIcaoReply_t reply = { .icao = 0, .isAlert = 0 };
#ifdef USE_ADSB
            if (isEnvironmentOkForCalculatingADSBDistanceBearing()) {
                bool isAlert = true;
                adsbVehicle_t *vehicle = findVehicleForAlert(
                        METERS_TO_CENTIMETERS(osdConfig()->adsb_distance_alert),
                        METERS_TO_CENTIMETERS(osdConfig()->adsb_distance_warning),
                        METERS_TO_CENTIMETERS(osdConfig()->adsb_ignore_plane_above_me_limit)
                );

                if (vehicle == NULL) {
                    vehicle = findVehicleForWarning(METERS_TO_CENTIMETERS(osdConfig()->adsb_distance_warning), METERS_TO_CENTIMETERS(osdConfig()->adsb_ignore_plane_above_me_limit));
                    isAlert = false;
                }

                if (vehicle != NULL) {
                    reply.icao = vehicle->vehicleValues.icao;
                    reply.isAlert = isAlert ? 1 : 0;
                }
            }
#endif
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_ADSB_VEHICLE_COUNT:
        {
            msp2AdsbVehicleCountReply_t reply = {
#ifdef USE_ADSB
                .count = MAX_ADSB_VEHICLES,   // iteration bound for the client
#else
                .count = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_DEBUG:
        // output some useful QA statistics
        // debug[x] = ((hse_value / 1000000) * 1000) + (SystemCoreClock / 1000000);         // XX0YY [crystal clock : core clock]
        {
            mspDebugReply_t reply;
            for (unsigned i = 0; i < ARRAYLEN(reply.debugValues); i++) {
                reply.debugValues[i] = debug[i];  // 4 variables are here for general monitoring purpose
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_DEBUG:
        {
            msp2InavDebugReply_t reply;
            for (int i = 0; i < DEBUG32_VALUE_COUNT; i++) {
                reply.debugValues[i] = debug[i];  // 8 variables are here for general monitoring purpose
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_UID:
        {
            mspUidReply_t reply = { .uid0 = U_ID_0, .uid1 = U_ID_1, .uid2 = U_ID_2 };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FEATURE:
        {
            mspFeatureReply_t reply = { .featureMask = featureMask() };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_BOARD_ALIGNMENT:
        {
            mspBoardAlignmentReply_t reply = {
                .rollAlign = boardAlignment()->rollDeciDegrees,
                .pitchAlign = boardAlignment()->pitchDeciDegrees,
                .yawAlign = boardAlignment()->yawDeciDegrees,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_VOLTAGE_METER_CONFIG:
        {
            mspVoltageMeterConfigReply_t reply = {
#ifdef USE_ADC
                .vbatScale = batteryMetersConfig()->voltage.scale / 10,
                .vbatMinCell = currentBatteryProfile->voltage.cellMin / 10,
                .vbatMaxCell = currentBatteryProfile->voltage.cellMax / 10,
                .vbatWarningCell = currentBatteryProfile->voltage.cellWarning / 10,
#else
                .vbatScale = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_CURRENT_METER_CONFIG:
        {
            mspCurrentMeterConfigReply_t reply = {
                .scale = batteryMetersConfig()->current.scale,
                .offset = batteryMetersConfig()->current.offset,
                .type = batteryMetersConfig()->current.type,
                .capacity = constrain(currentBatteryProfile->capacity.value, 0, 0xFFFF),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_MIXER:
        {
            // mixerMode no longer supported, send 3 (QuadX) as fallback
            mspMixerReply_t reply = { .mixerMode = 3 };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RX_CONFIG:
        {
            /* The reserved and bf-compatibility fields are all zero; they come from
             * the struct, not from a run of sbufWriteU8(dst, 0). */
            mspRxConfigReply_t reply = {
                .serialRxProvider = rxConfig()->serialrx_provider,
                .maxCheck = rxConfig()->maxcheck,
                .midRc = PWM_RANGE_MIDDLE,
                .minCheck = rxConfig()->mincheck,
#ifdef USE_SPEKTRUM_BIND
                .spektrumSatBind = rxConfig()->spektrum_sat_bind,
#endif
                .rxMinUsec = rxConfig()->rx_min_usec,
                .rxMaxUsec = rxConfig()->rx_max_usec,
                .receiverType = rxConfig()->receiverType,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FAILSAFE_CONFIG:
        {
            mspFailsafeConfigReply_t reply = {
                .failsafeDelay = failsafeConfig()->failsafe_delay,
                .failsafeOffDelay = failsafeConfig()->failsafe_off_delay,
                .failsafeThrottle = currentBatteryProfile->failsafe_throttle,
                .legacyKillSwitch = 0,    // was failsafe_kill_switch
                .failsafeThrottleLowDelay = failsafeConfig()->failsafe_throttle_low_delay,
                .failsafeProcedure = failsafeConfig()->failsafe_procedure,
                .failsafeRecoveryDelay = failsafeConfig()->failsafe_recovery_delay,
                .failsafeFWRollAngle = failsafeConfig()->failsafe_fw_roll_angle,
                .failsafeFWPitchAngle = failsafeConfig()->failsafe_fw_pitch_angle,
                .failsafeFWYawRate = failsafeConfig()->failsafe_fw_yaw_rate,
                .failsafeStickThreshold = failsafeConfig()->failsafe_stick_motion_threshold,
                .failsafeMinDistance = failsafeConfig()->failsafe_min_distance,
                .failsafeMinDistanceProc = failsafeConfig()->failsafe_min_distance_procedure,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RSSI_CONFIG:
        {
            mspRssiConfigReply_t reply = { .rssiChannel = rxConfig()->rssi_channel };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RX_MAP:
        {
            mspRxMapReply_t reply;
            memcpy(reply.rcMap, rxConfig()->rcmap, sizeof(reply.rcMap));
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_COMMON_SERIAL_CONFIG:
        /* One record per available port; the client reads to the end of the payload. */
        for (int i = 0; i < SERIAL_PORT_COUNT; i++) {
            const serialPortConfig_t *port = &serialConfig()->portConfigs[i];
            if (!serialIsPortAvailable(port->identifier)) {
                continue;
            }
            msp2CommonSerialConfigReplyElem_t record = {
                .identifier = port->identifier,
                .functionMask = port->functionMask,
                .mspBaudIndex = port->msp_baudrateIndex,
                .gpsBaudIndex = port->gps_baudrateIndex,
                .telemetryBaudIndex = port->telemetry_baudrateIndex,
                .peripheralBaudIndex = port->peripheral_baudrateIndex,
            };
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;

#ifdef USE_LED_STRIP
    case MSP_LED_COLORS:
        {
            mspLedColorsReply_t reply;
            for (int i = 0; i < LED_CONFIGURABLE_COLOR_COUNT; i++) {
                const hsvColor_t *color = &ledStripConfig()->colors[i];
                reply.items[i].hue = color->h;
                reply.items[i].saturation = color->s;
                reply.items[i].value = color->v;
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_LED_STRIP_CONFIG:
        /* 512 bytes: too big for the stack, so one record at a time. */
        for (int i = 0; i < LED_MAX_STRIP_LENGTH; i++) {
            const ledConfig_t *ledConfig = &ledStripConfig()->ledConfigs[i];

            uint32_t legacyLedConfig = ledConfig->led_position;
            int shiftCount = 8;
            legacyLedConfig |= ledConfig->led_function << shiftCount;
            shiftCount += 4;
            legacyLedConfig |= (ledConfig->led_overlay & 0x3F) << (shiftCount);
            shiftCount += 6;
            legacyLedConfig |= (ledConfig->led_color) << (shiftCount);
            shiftCount += 4;
            legacyLedConfig |= (ledConfig->led_direction) << (shiftCount);
            shiftCount += 6;
            legacyLedConfig |= (ledConfig->led_params) << (shiftCount);

            __typeof__(((mspLedStripConfigReply_t *)0)->items[0]) record = {
                .legacyLedConfig = legacyLedConfig,
            };
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;

    case MSP2_INAV_LED_STRIP_CONFIG_EX:
        for (int i = 0; i < LED_MAX_STRIP_LENGTH; i++) {
            __typeof__(((msp2InavLedStripConfigExReply_t *)0)->items[0]) record = {
                .ledConfig = ledStripConfig()->ledConfigs[i],
            };
            sbufWriteDataSafe(dst, &record, sizeof(record));
        }
        break;

    case MSP_LED_STRIP_MODECOLOR:
        {
            mspLedStripModecolorReply_t reply;
            unsigned n = 0;
            for (int i = 0; i < LED_MODE_COUNT; i++) {
                for (int j = 0; j < LED_DIRECTION_COUNT; j++, n++) {
                    reply.items[n].modeIndex = i;
                    reply.items[n].directionOrSpecialIndex = j;
                    reply.items[n].colorIndex = ledStripConfig()->modeColors[i].color[j];
                }
            }
            for (int j = 0; j < LED_SPECIAL_COLOR_COUNT; j++, n++) {
                reply.items[n].modeIndex = LED_MODE_COUNT;
                reply.items[n].directionOrSpecialIndex = j;
                reply.items[n].colorIndex = ledStripConfig()->specialColors.color[j];
            }
            mspWriteReply(dst, &reply);
        }
        break;
#endif

    case MSP_DATAFLASH_SUMMARY:
        serializeDataflashSummaryReply(dst);
        break;

    case MSP_BLACKBOX_CONFIG:
        {
            // API no longer supported: every field is zero
            mspBlackboxConfigReply_t reply = { .blackboxDevice = 0 };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_BLACKBOX_CONFIG:
        {
            msp2BlackboxConfigReply_t reply = {
#ifdef USE_BLACKBOX
                .blackboxSupported = 1,
                .blackboxDevice = blackboxConfig()->device,
                .blackboxRateNum = blackboxConfig()->rate_num,
                .blackboxRateDenom = blackboxConfig()->rate_denom,
                .blackboxIncludeFlags = blackboxConfig()->includeFlags,
#else
                .blackboxSupported = 0,
#endif
            };
#ifdef USE_BLACKBOX
            mspWriteReply(dst, &reply);
#else
            /* Without blackbox the reply has always stopped short of includeFlags;
             * the schema marks that field optional to match. */
            mspWriteReplyBytes(dst, &reply, offsetof(msp2BlackboxConfigReply_t, blackboxIncludeFlags));
#endif
        }
        break;

    case MSP_SDCARD_SUMMARY:
        serializeSDCardSummaryReply(dst);
        break;

#if defined (USE_DJI_HD_OSD) || defined (USE_MSP_DISPLAYPORT)
    case MSP_BATTERY_STATE:
        {
            mspBatteryStateReply_t reply = {
                // Battery characteristics
                .cellCount = constrain(getBatteryCellCount(), 0, 255),
                .capacity = currentBatteryProfile->capacity.value,

                // Battery state
                .vbatScaled = constrain(getBatteryVoltage() / 10, 0, 255), // in 0.1V steps
                .mAhDrawn = constrain(getMAhDrawn(), 0, 0xFFFF),
                .amperage = constrain(getAmperage(), -0x8000, 0x7FFF),

                // Battery alerts - used values match Betaflight's/DJI's
                .batteryState = getBatteryState(),

                // Additional battery voltage field (in 0.01V steps)
                .vbatActual = getBatteryVoltage(),
            };
            mspWriteReply(dst, &reply);
        }
        break;
#endif

    case MSP_OSD_CONFIG:
        {
            /* Without USE_OSD the reply is the driver byte alone; everything after
             * it is optional in the schema for that reason. */
            mspOsdConfigReply_t reply = { .osdDriverType = OSD_DRIVER_NONE };
#ifdef USE_OSD
            reply.osdDriverType = OSD_DRIVER_MAX7456;
            reply.videoSystem = osdConfig()->video_system; // AUTO/PAL/NTSC
            reply.units = osdConfig()->units;
            reply.rssiAlarm = osdConfig()->rssi_alarm;
            reply.capAlarm = currentBatteryProfile->capacity.warning;
            reply.timerAlarm = osdConfig()->time_alarm;
            reply.altAlarm = osdConfig()->alt_alarm;
            reply.distAlarm = osdConfig()->dist_alarm;
            reply.negAltAlarm = osdConfig()->neg_alt_alarm;
            for (int i = 0; i < OSD_ITEM_COUNT; i++) {
                reply.itemPositions[i] = osdLayoutsConfig()->item_pos[0][i];
            }
            mspWriteReply(dst, &reply);
#else
            mspWriteReplyBytes(dst, &reply, offsetof(mspOsdConfigReply_t, videoSystem));
#endif
        }
        break;

    case MSP_3D:
        {
            msp3dReply_t reply = {
                .deadbandLow = reversibleMotorsConfig()->deadband_low,
                .deadbandHigh = reversibleMotorsConfig()->deadband_high,
                .neutral = reversibleMotorsConfig()->neutral,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RC_DEADBAND:
        {
            mspRcDeadbandReply_t reply = {
                .deadband = rcControlsConfig()->deadband,
                .yawDeadband = rcControlsConfig()->yaw_deadband,
                .altHoldDeadband = rcControlsConfig()->alt_hold_deadband,
                .throttleDeadband = rcControlsConfig()->mid_throttle_deadband,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_SENSOR_ALIGNMENT:
        {
            mspSensorAlignmentReply_t reply = {
                .gyroAlign = 0, // was gyroConfig()->gyro_align
                .accAlign = 0,  // was accelerometerConfig()->acc_align
#ifdef USE_MAG
                .magAlign = compassConfig()->mag_align,
#endif
#ifdef USE_OPFLOW
                .opflowAlign = opticalFlowConfig()->opflow_align,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_ADVANCED_CONFIG:
        {
            mspAdvancedConfigReply_t reply = {
                .gyroSyncDenom = 1,   // gyroConfig()->gyroSyncDenominator
                .pidProcessDenom = 1, // BF: masterConfig.pid_process_denom
                .useUnsyncedPwm = 1,  // BF: motorConfig()->useUnsyncedPwm
                .motorPwmProtocol = motorConfig()->motorPwmProtocol,
                .motorPwmRate = motorConfig()->motorPwmRate,
                .servoPwmRate = servoConfig()->servoPwmRate,
                .legacyGyroSync = 0,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FILTER_CONFIG:
        {
            mspFilterConfigReply_t reply = {
                .gyroMainLpfHz = gyroConfig()->gyro_main_lpf_hz,
                .dtermLpfHz = pidProfile()->dterm_lpf_hz,
                .yawLpfHz = pidProfile()->yaw_lpf_hz,
                .legacyGyroNotchHz = 0,        // was gyroConfig()->gyro_notch_hz
                .legacyGyroNotchCutoff = 1,    // was gyroConfig()->gyro_notch_cutoff
                .bfCompatDtermNotchHz = 0,     // BF: pidProfile()->dterm_notch_hz
                .bfCompatDtermNotchCutoff = 1, // BF: pidProfile()->dterm_notch_cutoff
                .bfCompatGyroNotch2Hz = 0,     // BF: masterConfig.gyro_soft_notch_hz_2
                .bfCompatGyroNotch2Cutoff = 1, // BF: masterConfig.gyro_soft_notch_cutoff_2
                .accNotchHz = accelerometerConfig()->acc_notch_hz,
                .accNotchCutoff = accelerometerConfig()->acc_notch_cutoff,
                .legacyGyroStage2LpfHz = 0,    // was gyroConfig()->gyro_stage2_lowpass_hz
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_PID_ADVANCED:
        {
            /*
             * To keep compatibility on MSP frame length level with Betaflight, axis axisAccelerationLimitYaw
             * limit will be sent and received in [dps / 10]
             */
            mspPidAdvancedReply_t reply = {
                .accelLimitRollPitch = constrain(pidProfile()->axisAccelerationLimitRollPitch / 10, 0, 65535),
                .accelLimitYaw = constrain(pidProfile()->axisAccelerationLimitYaw / 10, 0, 65535),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_INAV_PID:
        {
            mspInavPidReply_t reply = {
                .headingHoldRateLimit = pidProfile()->heading_hold_rate_limit,
                .headingHoldLpfFreq = HEADING_HOLD_ERROR_LPF_FREQ,
                .legacyGyroLpf = GYRO_LPF_256HZ,
                .accLpfHz = accelerometerConfig()->acc_lpf_hz,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_SENSOR_CONFIG:
        {
            mspSensorConfigReply_t reply = {
                .accHardware = accelerometerConfig()->acc_hardware,
#ifdef USE_BARO
                .baroHardware = barometerConfig()->baro_hardware,
#endif
#ifdef USE_MAG
                .magHardware = compassConfig()->mag_hardware,
#endif
#ifdef USE_PITOT
                .pitotHardware = pitotmeterConfig()->pitot_hardware,
#endif
#ifdef USE_RANGEFINDER
                .rangefinderHardware = rangefinderConfig()->rangefinder_hardware,
#endif
#ifdef USE_OPFLOW
                .opflowHardware = opticalFlowConfig()->opflow_hardware,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_NAV_POSHOLD:
        {
            const bool airplane = mixerConfig()->platformType == PLATFORM_AIRPLANE;
            mspNavPosholdReply_t reply = {
                .userControlMode = navConfig()->general.flags.user_control_mode,
                .maxAutoSpeed = navConfig()->general.max_auto_speed,
                .maxAutoClimbRate = airplane ? navConfig()->fw.max_auto_climb_rate : navConfig()->mc.max_auto_climb_rate,
                .maxManualSpeed = navConfig()->general.max_manual_speed,
                .maxManualClimbRate = airplane ? navConfig()->fw.max_manual_climb_rate : navConfig()->mc.max_manual_climb_rate,
                .mcMaxBankAngle = navConfig()->mc.max_bank_angle,
                .mcAltHoldThrottleType = navConfig()->mc.althold_throttle_type,
                .mcHoverThrottle = currentBatteryProfile->nav.mc.hover_throttle,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RTH_AND_LAND_CONFIG:
        {
            mspRthAndLandConfigReply_t reply = {
                .minRthDistance = navConfig()->general.min_rth_distance,
                .rthClimbFirst = navConfig()->general.flags.rth_climb_first,
                .rthClimbIgnoreEmerg = navConfig()->general.flags.rth_climb_ignore_emerg,
                .rthTailFirst = navConfig()->general.flags.rth_tail_first,
                .rthAllowLanding = navConfig()->general.flags.rth_allow_landing,
                .rthAltControlMode = navConfig()->general.flags.rth_alt_control_mode,
                .rthAbortThreshold = navConfig()->general.rth_abort_threshold,
                .rthAltitude = navConfig()->general.rth_altitude,
                .landMinAltVspd = navConfig()->general.land_minalt_vspd,
                .landMaxAltVspd = navConfig()->general.land_maxalt_vspd,
                .landSlowdownMinAlt = navConfig()->general.land_slowdown_minalt,
                .landSlowdownMaxAlt = navConfig()->general.land_slowdown_maxalt,
                .emergDescentRate = navConfig()->general.emerg_descent_rate,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_FW_CONFIG:
        {
            mspFwConfigReply_t reply = {
                .cruiseThrottle = currentBatteryProfile->nav.fw.cruise_throttle,
                .minThrottle = currentBatteryProfile->nav.fw.min_throttle,
                .maxThrottle = currentBatteryProfile->nav.fw.max_throttle,
                .maxBankAngle = navConfig()->fw.max_bank_angle,
                .maxClimbAngle = navConfig()->fw.max_climb_angle,
                .maxDiveAngle = navConfig()->fw.max_dive_angle,
                .pitchToThrottle = currentBatteryProfile->nav.fw.pitch_to_throttle,
                .loiterRadius = navConfig()->fw.loiter_radius,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_CALIBRATION_DATA:
        {
            mspCalibrationDataReply_t reply = {
                .accCalibAxisFlags = accGetCalibrationAxisFlags(),
                .accZeroX = accelerometerConfig()->accZero.raw[X],
                .accZeroY = accelerometerConfig()->accZero.raw[Y],
                .accZeroZ = accelerometerConfig()->accZero.raw[Z],
                .accGainX = accelerometerConfig()->accGain.raw[X],
                .accGainY = accelerometerConfig()->accGain.raw[Y],
                .accGainZ = accelerometerConfig()->accGain.raw[Z],
#ifdef USE_MAG
                .magZeroX = compassConfig()->magZero.raw[X],
                .magZeroY = compassConfig()->magZero.raw[Y],
                .magZeroZ = compassConfig()->magZero.raw[Z],
                .magGainX = compassConfig()->magGain[X],
                .magGainY = compassConfig()->magGain[Y],
                .magGainZ = compassConfig()->magGain[Z],
#endif
#ifdef USE_OPFLOW
                .opflowScale = opticalFlowConfig()->opflow_scale * 256,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_POSITION_ESTIMATION_CONFIG:
        {
            mspPositionEstimationConfigReply_t reply = {
                .weightZBaroP = positionEstimationConfig()->w_z_baro_p * 100,
                .weightZGPSP = positionEstimationConfig()->w_z_gps_p * 100,
                .weightZGPSV = positionEstimationConfig()->w_z_gps_v * 100,
                .weightXYGPSP = positionEstimationConfig()->w_xy_gps_p * 100,
                .weightXYGPSV = positionEstimationConfig()->w_xy_gps_v * 100,
                .minSats = gpsConfigMutable()->gpsMinSats,
                .useGPSVelNED = 1, // inav_use_gps_velned ON/OFF
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_WP_GETINFO:
        {
            mspWpGetinfoReply_t reply = {
                .wpCapabilities = 0,                     // reserved for waypoint capabilities
                .maxWaypoints = NAV_MAX_WAYPOINTS,
                .missionValid = isWaypointListValid(),
                .waypointCount = getWaypointCount(),
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_TX_INFO:
        {
            dateTime_t dt;
            mspTxInfoReply_t reply = {
                .rssiSource = getRSSISource(),
                .rtcDateTimeIsSet = rtcGetDateTime(&dt) ? 1 : 0,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_RTC:
        {
            mspRtcReply_t reply = { .seconds = 0, .millis = 0 };
            rtcTime_t t;
            if (rtcGet(&t)) {
                reply.seconds = rtcTimeGetSeconds(&t);
                reply.millis = rtcTimeGetMillis(&t);
            }
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP_VTX_CONFIG:
        {
            /* Only the device-type byte is sent when there is no VTX; the rest of
             * the payload is optional in the schema for that reason. */
            mspVtxConfigReply_t reply = { .vtxDeviceType = VTXDEV_UNKNOWN };
#ifdef USE_VTX_CONTROL
            vtxDevice_t *vtxDevice = vtxCommonDevice();
            if (vtxDevice) {
                const uint8_t deviceType = vtxCommonGetDeviceType(vtxDevice);

                // Return band, channel and power from vtxSettingsConfig_t
                // since the VTX might be configured but temporarily offline.
                uint8_t pitmode = 0;
                vtxCommonGetPitMode(vtxDevice, &pitmode);

                reply.vtxDeviceType = deviceType;
                reply.band = vtxSettingsConfig()->band;
                reply.channel = vtxSettingsConfig()->channel;
                reply.power = vtxSettingsConfig()->power;
                reply.pitMode = pitmode;
                // technically there is bug here, we are missing the 16bit
                // freqency bf is transmitting (vtxSettingsConfig()->freq)

                // Betaflight < 4 doesn't send these fields
                reply.vtxReady = vtxCommonDeviceIsReady(vtxDevice) ? 1 : 0;
                reply.lowPowerDisarm = vtxSettingsConfig()->lowPowerDisarm;

                reply.vtxTableAvailable = 1;
                reply.bandCount = vtxDevice->capability.bandCount;
                reply.channelCount = vtxDevice->capability.channelCount;
                reply.powerCount = vtxDevice->capability.powerCount;
                reply.minPowerIndex = (deviceType == VTXDEV_MSP) ? 0 : 1;

                mspWriteReply(dst, &reply);
                break;
            }
#endif
            mspWriteReplyBytes(dst, &reply, offsetof(mspVtxConfigReply_t, band));
        }
        break;

    case MSP_NAME:
        {
            const char *name = systemConfig()->craftName;
            while (*name) {
                sbufWriteU8(dst, *name++);
            }
        }
        break;

    case MSP2_COMMON_TZ:
        {
            msp2CommonTzReply_t reply = {
                .tzOffsetMinutes = timeConfig()->tz_offset,
                .tzAutoDst = timeConfig()->tz_automatic_dst,
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_AIR_SPEED:
        {
            msp2InavAirSpeedReply_t reply = {
#ifdef USE_PITOT
                .airspeed = getAirspeedEstimate(),
#else
                .airspeed = 0,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_WIND:
        {
            msp2InavWindReply_t reply = { .windSpeed = 0, .windAngle = 0, .flags = 0 };
#ifdef USE_WIND_ESTIMATOR
            if (isEstimatedWindSpeedValid()) {
                uint16_t windAngle = 0;
                reply.windSpeed = getEstimatedHorizontalWindSpeed(&windAngle);
                reply.windAngle = windAngle / 100;
                reply.flags = 1;
            }
#endif
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_MIXER:
        {
            msp2InavMixerReply_t reply = {
                .motorDirectionInverted = mixerConfig()->motorDirectionInverted,
                .reserved1 = 0,
                .motorStopOnLow = mixerConfig()->motorstopOnLow,
                .platformType = mixerConfig()->platformType,
                .hasFlaps = mixerConfig()->hasFlaps,
                .appliedMixerPreset = mixerConfig()->appliedMixerPreset,
                .maxMotors = MAX_SUPPORTED_MOTORS,
                .maxServos = MAX_SUPPORTED_SERVOS,
            };
            mspWriteReply(dst, &reply);
        }
        break;

#if defined(USE_OSD)
    case MSP2_INAV_OSD_ALARMS:
        {
            msp2InavOsdAlarmsReply_t reply = {
                .rssiAlarm = osdConfig()->rssi_alarm,
                .timerAlarm = osdConfig()->time_alarm,
                .altAlarm = osdConfig()->alt_alarm,
                .distAlarm = osdConfig()->dist_alarm,
                .negAltAlarm = osdConfig()->neg_alt_alarm,
                .gForceAlarm = osdConfig()->gforce_alarm * 1000,
                .gForceAxisMinAlarm = osdConfig()->gforce_axis_alarm_min * 1000,
                .gForceAxisMaxAlarm = osdConfig()->gforce_axis_alarm_max * 1000,
                .currentAlarm = osdConfig()->current_alarm,
                .imuTempMinAlarm = osdConfig()->imu_temp_alarm_min,
                .imuTempMaxAlarm = osdConfig()->imu_temp_alarm_max,
#ifdef USE_BARO
                .baroTempMinAlarm = osdConfig()->baro_temp_alarm_min,
                .baroTempMaxAlarm = osdConfig()->baro_temp_alarm_max,
#endif
#ifdef USE_ADSB
                .adsbWarnDistance = osdConfig()->adsb_distance_warning,
                .adsbAlertDistance = osdConfig()->adsb_distance_alert,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

    case MSP2_INAV_OSD_PREFERENCES:
        {
            msp2InavOsdPreferencesReply_t reply = {
                .videoSystem = osdConfig()->video_system,
                .mainVoltageDecimals = osdConfig()->main_voltage_decimals,
                .ahiReverseRoll = osdConfig()->ahi_reverse_roll,
                .crosshairsStyle = osdConfig()->crosshairs_style,
                .leftSidebarScroll = osdConfig()->left_sidebar_scroll,
                .rightSidebarScroll = osdConfig()->right_sidebar_scroll,
                .sidebarScrollArrows = osdConfig()->sidebar_scroll_arrows,
                .units = osdConfig()->units,
                .statsEnergyUnit = osdConfig()->stats_energy_unit,
#ifdef USE_ADSB
                .adsbWarningStyle = osdConfig()->adsb_warning_style,
#endif
            };
            mspWriteReply(dst, &reply);
        }
        break;

#endif

    case MSP2_INAV_OUTPUT_MAPPING:
        for (uint8_t i = 0; i < timerHardwareCount; ++i)
            if (!(timerHardware[i].usageFlags & (TIM_USE_PPM | TIM_USE_PWM))) {
                msp2InavOutputMappingReplyElem_t record = {
                    .usageFlags = timerHardware[i].usageFlags,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
        break;

    // Obsolete, replaced by MSP2_INAV_OUTPUT_MAPPING_EXT2
    case MSP2_INAV_OUTPUT_MAPPING_EXT:
        for (uint8_t i = 0; i < timerHardwareCount; ++i)
            if (!(timerHardware[i].usageFlags & (TIM_USE_PPM | TIM_USE_PWM))) {
                msp2InavOutputMappingExtReplyElem_t record = {
                    #if defined(SITL_BUILD) || defined(WASM_BUILD)
                    .timerId = i,
                    #else
                    .timerId = timer2id(timerHardware[i].tim),
                    #endif
                    // usageFlags is u32, cuts out the higher 24bits
                    .usageFlags = timerHardware[i].usageFlags,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
        break;

    case MSP2_INAV_OUTPUT_MAPPING_EXT2:
        {
            #if !(defined(SITL_BUILD) || defined(WASM_BUILD)) && defined(WS2811_PIN)
            ioTag_t led_tag = IO_TAG(WS2811_PIN);
            #endif
            #ifdef USE_PINIO
            int nextPinioIndex = pinioHardwareCount;
            #endif
            for (uint8_t i = 0; i < timerHardwareCount; ++i) {
                if (timerHardware[i].usageFlags & (TIM_USE_PPM | TIM_USE_PWM)) {
                    continue;
                }

                uint8_t specialLabel = PIN_LABEL_NONE;
                #if !(defined(SITL_BUILD) || defined(WASM_BUILD))
                #if defined(WS2811_PIN)
                if (timerHardware[i].tag == led_tag) {
                    specialLabel = PIN_LABEL_LED;
                }
                #endif
                #ifdef USE_PINIO
                if (specialLabel == PIN_LABEL_NONE) {
                    for (int j = 0; j < pinioHardwareCount; j++) {
                        if (timerHardware[i].tag == pinioHardware[j].ioTag) {
                            specialLabel = PIN_LABEL_PINIO_BASE + j;
                            break;
                        }
                    }
                }
                // Timer-override PINIO pins: assign next USER index (up to PINIO_COUNT)
                if (specialLabel == PIN_LABEL_NONE && (timerHardware[i].usageFlags & TIM_USE_PINIO) && nextPinioIndex < PINIO_COUNT) {
                    specialLabel = PIN_LABEL_PINIO_BASE + nextPinioIndex;
                    nextPinioIndex++;
                }
                #endif
                #endif

                msp2InavOutputMappingExt2ReplyElem_t record = {
                    #if defined(SITL_BUILD) || defined(WASM_BUILD)
                    .timerId = i,
                    #else
                    .timerId = timer2id(timerHardware[i].tim),
                    #endif
                    .usageFlags = timerHardware[i].usageFlags,
                    .pinLabel = specialLabel,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
        }
        break;


#ifndef SITL_BUILD
    case MSP2_INAV_OUTPUT_ASSIGNMENT:
        {
            const timMotorServoHardware_t *hw = pwmGetOutputAssignment();
            msp2InavOutputAssignmentReplyElem_t record;
            for (int m = 0; m < hw->maxTimMotorCount; m++) {
                record = (msp2InavOutputAssignmentReplyElem_t){
                    .outputIndex = hw->timMotors[m] - timerHardware,
                    .usageType = __builtin_ctz(TIM_USE_MOTOR),
                    .functionIndex = m + 1,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
            for (int s = 0; s < hw->maxTimServoCount; s++) {
                record = (msp2InavOutputAssignmentReplyElem_t){
                    .outputIndex = hw->timServos[s] - timerHardware,
                    .usageType = __builtin_ctz(TIM_USE_SERVO),
                    .functionIndex = s + 1,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
            for (int idx = 0; idx < timerHardwareCount; idx++) {
                if (timerOverrides(timer2id(timerHardware[idx].tim))->outputMode == OUTPUT_MODE_BEEPER) {
                    record = (msp2InavOutputAssignmentReplyElem_t){
                        .outputIndex = idx,
                        .usageType = __builtin_ctz(TIM_USE_BEEPER),
                        .functionIndex = 1,
                    };
                    sbufWriteData(dst, &record, sizeof(record));
                    break;
                }
            }
        }
        break;
#endif

    case MSP2_INAV_MC_BRAKING:
#ifdef USE_MR_BRAKING_MODE
        {
            msp2InavMcBrakingReply_t reply = {
                .brakingSpeedThreshold = navConfig()->mc.braking_speed_threshold,
                .brakingDisengageSpeed = navConfig()->mc.braking_disengage_speed,
                .brakingTimeout = navConfig()->mc.braking_timeout,
                .brakingBoostFactor = navConfig()->mc.braking_boost_factor,
                .brakingBoostTimeout = navConfig()->mc.braking_boost_timeout,
                .brakingBoostSpeedThreshold = navConfig()->mc.braking_boost_speed_threshold,
                .brakingBoostDisengageSpeed = navConfig()->mc.braking_boost_disengage_speed,
                .brakingBankAngle = navConfig()->mc.braking_bank_angle,
            };
            mspWriteReply(dst, &reply);
        }
#endif
        break;

#ifdef USE_TEMPERATURE_SENSOR
    case MSP2_INAV_TEMP_SENSOR_CONFIG:
        for (uint8_t index = 0; index < MAX_TEMP_SENSORS; ++index) {
            const tempSensorConfig_t *sensorConfig = tempSensorConfig(index);
            __typeof__(((msp2InavTempSensorConfigReply_t *)0)->items[0]) record = {
                .type = sensorConfig->type,
                .address = sensorConfig->address,
                .alarmMin = sensorConfig->alarm_min,
                .alarmMax = sensorConfig->alarm_max,
                .osdSymbol = sensorConfig->osdSymbol,
            };
            memcpy(record.label, sensorConfig->label, sizeof(record.label));
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;

    case MSP2_INAV_TEMPERATURES:
        {
            msp2InavTemperaturesReply_t reply;
            for (uint8_t index = 0; index < MAX_TEMP_SENSORS; ++index) {
                int16_t temperature;
                reply.items[index].temperature = getSensorTemperature(index, &temperature) ? temperature : -1000;
            }
            mspWriteReply(dst, &reply);
        }
        break;
#endif

#ifdef USE_ESC_SENSOR
    case MSP2_INAV_ESC_RPM:
        for (uint8_t i = 0; i < getMotorCount(); i++) {
            msp2InavEscRpmReplyElem_t record = { .escRpm = getEscTelemetry(i)->rpm };
            sbufWriteData(dst, &record, sizeof(record));
        }
        break;

    case MSP2_INAV_ESC_TELEM:
        {
            msp2InavEscTelemReply_t reply = { .motorCount = getMotorCount() };
            mspWriteReplyBytes(dst, &reply, sizeof(reply));

            for (uint8_t i = 0; i < reply.motorCount; i++) {
                __typeof__(reply.escData[0]) record = { .esc = *getEscTelemetry(i) };
                sbufWriteDataSafe(dst, &record, sizeof(record));
            }
        }
        break;
#endif

#ifdef USE_DRONECAN
    case MSP2_INAV_DRONECAN_NODES:
        mspSerializeDronecanNodes(dst);
        break;
#endif

#ifdef USE_EZ_TUNE

    case MSP2_INAV_EZ_TUNE:
        {
            msp2InavEzTuneReply_t reply = {
                .enabled = ezTune()->enabled,
                .filterHz = ezTune()->filterHz,
                .axisRatio = ezTune()->axisRatio,
                .response = ezTune()->response,
                .damping = ezTune()->damping,
                .stability = ezTune()->stability,
                .aggressiveness = ezTune()->aggressiveness,
                .rate = ezTune()->rate,
                .expo = ezTune()->expo,
                .snappiness = ezTune()->snappiness,
            };
            mspWriteReply(dst, &reply);
        }
        break;
#endif

#ifdef USE_RATE_DYNAMICS

    case MSP2_INAV_RATE_DYNAMICS:
        {
            msp2InavRateDynamicsReply_t reply = {
                .sensitivityCenter = currentControlProfile->rateDynamics.sensitivityCenter,
                .sensitivityEnd = currentControlProfile->rateDynamics.sensitivityEnd,
                .correctionCenter = currentControlProfile->rateDynamics.correctionCenter,
                .correctionEnd = currentControlProfile->rateDynamics.correctionEnd,
                .weightCenter = currentControlProfile->rateDynamics.weightCenter,
                .weightEnd = currentControlProfile->rateDynamics.weightEnd,
            };
            mspWriteReply(dst, &reply);
        }
        break;

#endif
#ifdef USE_PROGRAMMING_FRAMEWORK
    case MSP2_INAV_CUSTOM_OSD_ELEMENTS:
        {
            msp2InavCustomOsdElementsReply_t reply = {
                .maxElements = MAX_CUSTOM_ELEMENTS,
                .maxTextLength = OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1,
                .maxParts = CUSTOM_ELEMENTS_PARTS,
            };
            mspWriteReply(dst, &reply);
        }
        break;
#endif

    case MSP2_COMMON_GET_RADAR_GPS:
        for (uint8_t i = 0; i < RADAR_MAX_POIS; i++) {
            __typeof__(((msp2CommonGetRadarGpsReply_t *)0)->items[0]) record = {
                .poiLatitude = radar_pois[i].gps.lat,
                .poiLongitude = radar_pois[i].gps.lon,
                .poiAltitude = radar_pois[i].gps.alt,
            };
            sbufWriteDataSafe(dst, &record, sizeof(record));
        }
        break;

    default:
        return false;
    }
    return true;
}


#ifdef USE_SAFE_HOME
static mspResult_e mspFcSafeHomeOutCommand(sbuf_t *dst, sbuf_t *src)
{
    const uint8_t safe_home_no = sbufReadU8(src);    // get the home number
    if (safe_home_no >= MAX_SAFE_HOMES) {
        return MSP_RESULT_ERROR;
    }
    msp2InavSafehomeReply_t reply = {
        .safehomeIndex = safe_home_no,
        .enabled = safeHomeConfig(safe_home_no)->enabled,
        .latitude = safeHomeConfig(safe_home_no)->lat,
        .longitude = safeHomeConfig(safe_home_no)->lon,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}
#endif

#ifdef USE_FW_AUTOLAND
static mspResult_e mspFwApproachOutCommand(sbuf_t *dst, sbuf_t *src)
{
    const uint8_t idx = sbufReadU8(src);
    if (idx >= MAX_FW_LAND_APPOACH_SETTINGS) {
        return MSP_RESULT_ERROR;
    }
    const navFwAutolandApproach_t *approach = fwAutolandApproachConfig(idx);
    msp2InavFwApproachReply_t reply = {
        .approachIndex = idx,
        .approachAlt = approach->approachAlt,
        .landAlt = approach->landAlt,
        .approachDirection = approach->approachDirection,
        .landHeading1 = approach->landApproachHeading1,
        .landHeading2 = approach->landApproachHeading2,
        .isSeaLevelRef = approach->isSeaLevelRef,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}
#endif

#ifdef USE_GEOZONE
static mspResult_e mspFcGeozoneOutCommand(sbuf_t *dst, sbuf_t *src)
{
    const uint8_t idx = sbufReadU8(src);
    if (idx >= MAX_GEOZONES_IN_CONFIG) {
        return MSP_RESULT_ERROR;
    }
    const geoZoneConfig_t *zone = geoZonesConfig(idx);
    msp2InavGeozoneReply_t reply = {
        .geozoneIndex = idx,
        .type = zone->type,
        .shape = zone->shape,
        .minAltitude = zone->minAltitude,
        .maxAltitude = zone->maxAltitude,
        .isSeaLevelRef = zone->isSealevelRef,
        .fenceAction = zone->fenceAction,
        .vertexCount = zone->vertexCount,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

static mspResult_e mspFcGeozoneVerteciesOutCommand(sbuf_t *dst, sbuf_t *src)
{
    const uint8_t zoneId = sbufReadU8(src);
    const uint8_t vertexId = sbufReadU8(src);
    if (zoneId >= MAX_GEOZONES_IN_CONFIG) {
        return MSP_RESULT_ERROR;
    }
    const int8_t vertexIdx = geozoneGetVertexIdx(zoneId, vertexId);
    if (vertexIdx < 0) {
        return MSP_RESULT_ERROR;
    }

    msp2InavGeozoneVertexReply_t reply = {
        .geozoneIndex = geoZoneVertices(vertexIdx)->zoneId,
        .vertexId = geoZoneVertices(vertexIdx)->idx,
        .latitude = geoZoneVertices(vertexIdx)->lat,
        .longitude = geoZoneVertices(vertexIdx)->lon,
    };
    /* A circular zone keeps its radius in the 'latitude' of the next vertex, and
     * only then is the radius field present. */
    if (geoZonesConfig(zoneId)->shape == GEOZONE_SHAPE_CIRCULAR) {
        const int8_t vertexRadiusIdx = geozoneGetVertexIdx(zoneId, vertexId + 1);
        if (vertexRadiusIdx < 0) {
            return MSP_RESULT_ERROR;
        }
        reply.radius = geoZoneVertices(vertexRadiusIdx)->lat;
        mspWriteReply(dst, &reply);
    } else {
        mspWriteReplyBytes(dst, &reply, offsetof(msp2InavGeozoneVertexReply_t, radius));
    }
    return MSP_RESULT_ACK;
}
#endif

static mspResult_e mspFcLogicConditionCommand(sbuf_t *dst, sbuf_t *src) {
    const uint8_t idx = sbufReadU8(src);
    if (idx >= MAX_LOGIC_CONDITIONS) {
        return MSP_RESULT_ERROR;
    }
    const logicCondition_t *lc = logicConditions(idx);
    msp2InavLogicConditionsSingleReply_t reply = {
        .enabled = lc->enabled,
        .activatorId = lc->activatorId,
        .operation = lc->operation,
        .operandAType = lc->operandA.type,
        .operandAValue = lc->operandA.value,
        .operandBType = lc->operandB.type,
        .operandBValue = lc->operandB.value,
        .flags = lc->flags,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

static void mspFcWaypointOutCommand(sbuf_t *dst, sbuf_t *src)
{
    const uint8_t msp_wp_no = sbufReadU8(src);    // get the wp number
    navWaypoint_t msp_wp;
    getWaypoint(msp_wp_no, &msp_wp);
    mspWpReply_t reply = {
        .waypointIndex = msp_wp_no,
        .action = msp_wp.action,
        .latitude = msp_wp.lat,
        .longitude = msp_wp.lon,
        .altitude = msp_wp.alt,
        .param1 = msp_wp.p1,
        .param2 = msp_wp.p2,
        .param3 = msp_wp.p3,
        .flag = msp_wp.flag,
    };
    mspWriteReply(dst, &reply);
}

#ifdef USE_FLASHFS
static void mspFcDataFlashReadCommand(sbuf_t *dst, sbuf_t *src)
{
    const unsigned int dataSize = sbufBytesRemaining(src); /* Payload size in Bytes */
    uint16_t readLength;

    const uint32_t readAddress = sbufReadU32(src);

    // Request payload:
    //  uint32_t    - address to read from
    //  uint16_t    - size of block to read (optional)
    if (dataSize == sizeof(uint32_t) + sizeof(uint16_t)) {
        readLength = sbufReadU16(src);
    }
    else {
        readLength = 128;
    }

    serializeDataflashReadReply(dst, readAddress, readLength);
}
#endif

static mspResult_e mspFcProcessInCommand(uint16_t cmdMSP, sbuf_t *src)
{
    const unsigned int dataSize = sbufBytesRemaining(src);  /* Payload size in Bytes */

    // SET handlers use lenient >= / < gates per forward-compat policy: MSP payloads
    // only gain fields at the end, so a newer configurator's longer message is
    // accepted by applying the known prefix and ignoring the trailing bytes.

    switch (cmdMSP) {
    case MSP_SELECT_SETTING:
        {
            mspSelectSettingRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || ARMING_FLAG(ARMED)) {
                return MSP_RESULT_ERROR;
            }
            setConfigProfileAndWriteEEPROM(pkt.profileIndex);
        }
        break;

    case MSP_SET_HEAD:
        {
            mspSetHeadRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            const int32_t headingCentidegrees = wrap_36000(DEGREES_TO_CENTIDEGREES(pkt.heading));
            updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(headingCentidegrees));

            if (navGetCurrentStateFlags() & NAV_CTL_YAW) {
                posControl.desiredState.yaw = headingCentidegrees;
                posControl.cruise.course = headingCentidegrees;
                posControl.cruise.previousCourse = headingCentidegrees;
            }
        }
        break;

#ifdef USE_RX_MSP
    case MSP_SET_RAW_RC:
        {
            uint8_t channelCount = dataSize / sizeof(uint16_t);
            if ((channelCount > MAX_SUPPORTED_RC_CHANNEL_COUNT) || (dataSize > channelCount * sizeof(uint16_t))) {
                return MSP_RESULT_ERROR;
            } else {
                uint16_t frame[MAX_SUPPORTED_RC_CHANNEL_COUNT];
                for (int i = 0; i < channelCount; i++) {
                    frame[i] = sbufReadU16(src);
                }
                rxMspFrameReceive(frame, channelCount);
            }
        }
        break;
#endif

    case MSP_SET_LOOP_TIME:
        {
            mspSetLoopTimeRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            gyroConfigMutable()->looptime = pkt.looptime;
        }
        break;

    case MSP2_SET_PID:
        {
            msp2SetPidRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            for (int i = 0; i < PID_ITEM_COUNT; i++) {
                pidBankMutable()->pid[i].P = pkt.items[i].P;
                pidBankMutable()->pid[i].I = pkt.items[i].I;
                pidBankMutable()->pid[i].D = pkt.items[i].D;
                pidBankMutable()->pid[i].FF = pkt.items[i].FF;
            }
            schedulePidGainsUpdate();
            navigationUsePIDs();
        }
        break;

    case MSP_SET_MODE_RANGE:
        {
            mspSetModeRangeRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.rangeIndex >= MAX_MODE_ACTIVATION_CONDITION_COUNT) {
                return MSP_RESULT_ERROR;
            }
            const box_t *box = findBoxByPermanentId(pkt.modePermanentId);
            if (!box) {
                return MSP_RESULT_ERROR;
            }
            modeActivationCondition_t *mac = modeActivationConditionsMutable(pkt.rangeIndex);
            mac->modeId = box->boxId;
            mac->auxChannelIndex = pkt.auxChannelIndex;
            mac->range.startStep = pkt.rangeStartStep;
            mac->range.endStep = pkt.rangeEndStep;

            updateUsedModeActivationConditionFlags();
        }
        break;

    case MSP_SET_ADJUSTMENT_RANGE:
        {
            mspSetAdjustmentRangeRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.rangeIndex >= MAX_ADJUSTMENT_RANGE_COUNT
                || pkt.adjustmentIndex >= MAX_SIMULTANEOUS_ADJUSTMENT_COUNT) {
                return MSP_RESULT_ERROR;
            }
            adjustmentRange_t *adjRange = adjustmentRangesMutable(pkt.rangeIndex);
            adjRange->adjustmentIndex = pkt.adjustmentIndex;
            adjRange->auxChannelIndex = pkt.auxChannelIndex;
            adjRange->range.startStep = pkt.rangeStartStep;
            adjRange->range.endStep = pkt.rangeEndStep;
            adjRange->adjustmentFunction = pkt.adjustmentFunction;
            adjRange->auxSwitchChannelIndex = pkt.auxSwitchChannelIndex;
        }
        break;

    case MSP_SET_RC_TUNING:
        {
            /* rcYawExpo is an appended optional byte: read the required prefix, then
             * ask for the tail separately. */
            mspSetRcTuningRequest_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(mspSetRcTuningRequest_t, rcYawExpo), dataSize)) {
                return MSP_RESULT_ERROR;
            }

            // need to cast away const to set controlProfile
            controlConfig_t *currentControlProfile_p = (controlConfig_t*)currentControlProfile;
            currentControlProfile_p->stabilized.rcExpo8 = pkt.rcExpo;
            currentControlProfile_p->stabilized.rates[FD_ROLL] = constrain(pkt.rollRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->stabilized.rates[FD_PITCH] = constrain(pkt.pitchRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->stabilized.rates[FD_YAW] = constrain(pkt.yawRate, SETTING_YAW_RATE_MIN, SETTING_YAW_RATE_MAX);
            currentControlProfile_p->throttle.dynPID = MIN(pkt.dynamicThrottlePID, SETTING_TPA_RATE_MAX);
            currentControlProfile_p->throttle.rcMid8 = pkt.throttleMid;
            currentControlProfile_p->throttle.rcExpo8 = pkt.throttleExpo;
            currentControlProfile_p->throttle.pa_breakpoint = pkt.tpaBreakpoint;

            if (mspReadOptional(src, &pkt.rcYawExpo)) {
                currentControlProfile_p->stabilized.rcYawExpo8 = pkt.rcYawExpo;
            }

            schedulePidGainsUpdate();
        }
        break;

    case MSP2_INAV_SET_RATE_PROFILE:
        {
            msp2InavSetRateProfileRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }

            controlConfig_t *currentControlProfile_p = (controlConfig_t*)currentControlProfile; // need to cast away const to set controlProfile

            // throttle
            currentControlProfile_p->throttle.rcMid8 = pkt.throttleMid;
            currentControlProfile_p->throttle.rcExpo8 = pkt.throttleExpo;
            currentControlProfile_p->throttle.dynPID = pkt.dynamicThrottlePID;
            currentControlProfile_p->throttle.pa_breakpoint = pkt.tpaBreakpoint;

            // stabilized
            currentControlProfile_p->stabilized.rcExpo8 = pkt.stabRcExpo;
            currentControlProfile_p->stabilized.rcYawExpo8 = pkt.stabRcYawExpo;
            currentControlProfile_p->stabilized.rates[FD_ROLL] = constrain(pkt.stabRollRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->stabilized.rates[FD_PITCH] = constrain(pkt.stabPitchRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->stabilized.rates[FD_YAW] = constrain(pkt.stabYawRate, SETTING_YAW_RATE_MIN, SETTING_YAW_RATE_MAX);

            // manual
            currentControlProfile_p->manual.rcExpo8 = pkt.manualRcExpo;
            currentControlProfile_p->manual.rcYawExpo8 = pkt.manualRcYawExpo;
            currentControlProfile_p->manual.rates[FD_ROLL] = constrain(pkt.manualRollRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->manual.rates[FD_PITCH] = constrain(pkt.manualPitchRate, SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
            currentControlProfile_p->manual.rates[FD_YAW] = constrain(pkt.manualYawRate, SETTING_YAW_RATE_MIN, SETTING_YAW_RATE_MAX);
        }
        break;

    case MSP_SET_MISC:
        {
            mspSetMiscRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            motorConfigMutable()->mincommand = constrain(pkt.minCommand, 0, PWM_RANGE_MAX);
            currentBatteryProfileMutable->failsafe_throttle = constrain(pkt.failsafeThrottle, PWM_RANGE_MIN, PWM_RANGE_MAX);

#ifdef USE_GPS
            gpsConfigMutable()->provider = pkt.gpsType;
            gpsConfigMutable()->sbasMode = pkt.gpsSbasMode;
#endif
            if (pkt.rssiChannel <= MAX_SUPPORTED_RC_CHANNEL_COUNT) {
                rxConfigMutable()->rssi_channel = pkt.rssiChannel;
                rxUpdateRSSISource(); // Changing rssi_channel might change the RSSI source
            }
#ifdef USE_MAG
            compassConfigMutable()->mag_declination = pkt.magDeclination * 10;
#endif
#ifdef USE_ADC
            batteryMetersConfigMutable()->voltage.scale = pkt.vbatScale * 10;
            currentBatteryProfileMutable->voltage.cellMin = pkt.vbatMinCell * 10;         // vbatlevel_warn1 in MWC2.3 GUI
            currentBatteryProfileMutable->voltage.cellMax = pkt.vbatMaxCell * 10;         // vbatlevel_warn2 in MWC2.3 GUI
            currentBatteryProfileMutable->voltage.cellWarning = pkt.vbatWarningCell * 10; // vbatlevel when buzzer starts to alert
#endif
        }
        break;

    case MSP2_INAV_SET_MISC:
        {
            msp2InavSetMiscRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            motorConfigMutable()->mincommand = constrain(pkt.minCommand, 0, PWM_RANGE_MAX);
            currentBatteryProfileMutable->failsafe_throttle = constrain(pkt.failsafeThrottle, PWM_RANGE_MIN, PWM_RANGE_MAX);

#ifdef USE_GPS
            gpsConfigMutable()->provider = pkt.gpsType;
            gpsConfigMutable()->sbasMode = pkt.gpsSbasMode;
#endif
            if (pkt.rssiChannel <= MAX_SUPPORTED_RC_CHANNEL_COUNT) {
                rxConfigMutable()->rssi_channel = pkt.rssiChannel;
            }
#ifdef USE_MAG
            compassConfigMutable()->mag_declination = pkt.magDeclination * 10;
#endif
#ifdef USE_ADC
            batteryMetersConfigMutable()->voltage.scale = pkt.vbatScale;
            batteryMetersConfigMutable()->voltageSource = pkt.vbatSource;
            currentBatteryProfileMutable->cells = pkt.cellCount;
            currentBatteryProfileMutable->voltage.cellDetect = pkt.vbatCellDetect;
            currentBatteryProfileMutable->voltage.cellMin = pkt.vbatMinCell;
            currentBatteryProfileMutable->voltage.cellMax = pkt.vbatMaxCell;
            currentBatteryProfileMutable->voltage.cellWarning = pkt.vbatWarningCell;
#endif
            currentBatteryProfileMutable->capacity.value = pkt.capacityValue;
            currentBatteryProfileMutable->capacity.warning = pkt.capacityWarning;
            currentBatteryProfileMutable->capacity.critical = pkt.capacityCritical;

            const uint8_t previousCapacityUnit = batteryMetersConfig()->capacity_unit;
            batteryMetersConfigMutable()->capacity_unit = pkt.capacityUnit;
            if (!mspApplyBatteryUnits(previousCapacityUnit)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

    case MSP2_INAV_SET_BATTERY_CONFIG:
        {
            msp2InavSetBatteryConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
#ifdef USE_ADC
            batteryMetersConfigMutable()->voltage.scale = pkt.vbatScale;
            batteryMetersConfigMutable()->voltageSource = pkt.vbatSource;
            currentBatteryProfileMutable->cells = pkt.cellCount;
            currentBatteryProfileMutable->voltage.cellDetect = pkt.vbatCellDetect;
            currentBatteryProfileMutable->voltage.cellMin = pkt.vbatMinCell;
            currentBatteryProfileMutable->voltage.cellMax = pkt.vbatMaxCell;
            currentBatteryProfileMutable->voltage.cellWarning = pkt.vbatWarningCell;
#endif
            batteryMetersConfigMutable()->current.offset = pkt.currentOffset;
            batteryMetersConfigMutable()->current.scale = pkt.currentScale;

            currentBatteryProfileMutable->capacity.value = pkt.capacityValue;
            currentBatteryProfileMutable->capacity.warning = pkt.capacityWarning;
            currentBatteryProfileMutable->capacity.critical = pkt.capacityCritical;

            const uint8_t previousCapacityUnit = batteryMetersConfig()->capacity_unit;
            batteryMetersConfigMutable()->capacity_unit = pkt.capacityUnit;
            if (!mspApplyBatteryUnits(previousCapacityUnit)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

    case MSP_SET_MOTOR:
        {
            mspSetMotorRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            for (unsigned i = 0; i < MIN(ARRAYLEN(pkt.motorValues), (unsigned)MAX_SUPPORTED_MOTORS); i++) {
                motor_disarmed[i] = pkt.motorValues[i];
            }
        }
        break;

    case MSP_SET_SERVO_CONFIGURATION:
        {
            mspSetServoConfigurationRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.servoIndex >= MAX_SUPPORTED_SERVOS) {
                return MSP_RESULT_ERROR;
            }
            mspApplyServoParams(pkt.servoIndex, pkt.min, pkt.max, pkt.middle, pkt.rate);
        }
        break;

    case MSP2_INAV_SET_SERVO_CONFIG:
        {
            msp2InavSetServoConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.servoIndex >= MAX_SUPPORTED_SERVOS) {
                return MSP_RESULT_ERROR;
            }
            mspApplyServoParams(pkt.servoIndex, pkt.min, pkt.max, pkt.middle, pkt.rate);
        }
        break;

    case MSP_SET_SERVO_MIX_RULE:
        {
            mspSetServoMixRuleRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.ruleIndex >= MAX_SERVO_RULES) {
                return MSP_RESULT_ERROR;
            }
            servoMixer_t *rule = customServoMixersMutable(pkt.ruleIndex);
            rule->targetChannel = pkt.targetChannel;
            rule->inputSource = pkt.inputSource;
            rule->rate = pkt.rate;
            rule->speed = pkt.speed;
            loadCustomServoMixer();
        }
        break;

    case MSP2_INAV_SET_SERVO_MIXER:
        {
            msp2InavSetServoMixerRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.ruleIndex >= MAX_SERVO_RULES) {
                return MSP_RESULT_ERROR;
            }
            servoMixer_t *rule = customServoMixersMutable(pkt.ruleIndex);
            rule->targetChannel = pkt.targetChannel;
            rule->inputSource = pkt.inputSource;
            rule->rate = pkt.rate;
            rule->speed = pkt.speed;
        #ifdef USE_PROGRAMMING_FRAMEWORK
            rule->conditionId = pkt.conditionId;
        #endif
            loadCustomServoMixer();
        }
        break;
#ifdef USE_PROGRAMMING_FRAMEWORK
    case MSP2_INAV_SET_LOGIC_CONDITIONS:
        {
            msp2InavSetLogicConditionsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.conditionIndex >= MAX_LOGIC_CONDITIONS) {
                return MSP_RESULT_ERROR;
            }
            logicCondition_t *lc = logicConditionsMutable(pkt.conditionIndex);
            lc->enabled = pkt.enabled;
            lc->activatorId = pkt.activatorId;
            lc->operation = pkt.operation;
            lc->operandA.type = pkt.operandAType;
            lc->operandA.value = pkt.operandAValue;
            lc->operandB.type = pkt.operandBType;
            lc->operandB.value = pkt.operandBValue;
            lc->flags = pkt.flags;
        }
        break;

    case MSP2_INAV_SET_PROGRAMMING_PID:
        {
            msp2InavSetProgrammingPidRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.pidIndex >= MAX_PROGRAMMING_PID_COUNT) {
                return MSP_RESULT_ERROR;
            }
            programmingPid_t *pid = programmingPidsMutable(pkt.pidIndex);
            pid->enabled = pkt.enabled;
            pid->setpoint.type = pkt.setpointType;
            pid->setpoint.value = pkt.setpointValue;
            pid->measurement.type = pkt.measurementType;
            pid->measurement.value = pkt.measurementValue;
            pid->gains.P = pkt.gainP;
            pid->gains.I = pkt.gainI;
            pid->gains.D = pkt.gainD;
            pid->gains.FF = pkt.gainFF;

            programmingPidInit();
        }
        break;

    case MSP2_INAV_SET_GVAR:
        {
            msp2InavSetGvarRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.gvarIndex >= MAX_GLOBAL_VARIABLES) {
                return MSP_RESULT_ERROR;
            }
            gvSet(pkt.gvarIndex, pkt.value);
        }
        break;
#endif

    case MSP2_INAV_SET_AUX_RC:
        {
            // Max valid payload: 1 def byte + 24 channels × 2 bytes (16-bit) = 49 bytes
            if (dataSize < 2 || dataSize > 49) {
                return MSP_RESULT_ERROR;
            }

            const uint8_t defByte = sbufReadU8(src);
            const uint8_t startChannel = defByte >> 3;          // Bits 7-3: start channel index (0-31)
            const uint8_t resolutionMode = defByte & 0x07;      // Bits 2-0: resolution

            // Safety: CH1-CH12 (index 0-11) are protected
            if (startChannel < 12) {
                return MSP_RESULT_ERROR;
            }

            const uint8_t dataBytes = dataSize - 1;
            uint8_t channelCount;
            uint8_t bitsPerChannel;

            switch (resolutionMode) {
                case 0: // 2-bit
                    bitsPerChannel = 2;
                    channelCount = dataBytes * 4;
                    break;
                case 1: // 4-bit
                    bitsPerChannel = 4;
                    channelCount = dataBytes * 2;
                    break;
                case 2: // 8-bit
                    bitsPerChannel = 8;
                    channelCount = dataBytes;
                    break;
                case 3: // 16-bit
                    bitsPerChannel = 16;
                    if (dataBytes % 2 != 0) {
                        return MSP_RESULT_ERROR;
                    }
                    channelCount = dataBytes / 2;
                    break;
                default:
                    return MSP_RESULT_ERROR;
            }

            if (channelCount == 0 || startChannel + channelCount > 32) {
                return MSP_RESULT_ERROR;
            }

            // Decode and apply channel values
            if (bitsPerChannel >= 8) {
                // Byte-aligned modes: 8-bit and 16-bit
                for (int i = 0; i < channelCount; i++) {
                    uint16_t rawValue;
                    if (bitsPerChannel == 16) {
                        rawValue = sbufReadU16(src);
                    } else {
                        rawValue = sbufReadU8(src);
                    }

                    if (rawValue == 0) {
                        continue; // skip: no update
                    }

                    uint16_t pwmValue;
                    if (bitsPerChannel == 16) {
                        pwmValue = constrain(rawValue, 750, 2250);
                    } else {
                        // 8-bit: 1-255 → 1000-2000
                        pwmValue = 1000 + ((uint32_t)(rawValue - 1) * 1000) / 254;
                    }

                    rxMspAuxOverlaySet(startChannel + i, pwmValue);
                }
            } else {
                // Sub-byte modes: 2-bit and 4-bit
                const uint8_t mask = (1 << bitsPerChannel) - 1;
                const uint8_t channelsPerByte = 8 / bitsPerChannel;
                int ch = 0;

                for (int byteIdx = 0; byteIdx < (int)dataBytes && ch < channelCount; byteIdx++) {
                    const uint8_t dataByte = sbufReadU8(src);
                    for (int sub = channelsPerByte - 1; sub >= 0 && ch < channelCount; sub--, ch++) {
                        const uint8_t rawValue = (dataByte >> (sub * bitsPerChannel)) & mask;

                        if (rawValue == 0) {
                            continue; // skip: no update
                        }

                        uint16_t pwmValue;
                        if (bitsPerChannel == 2) {
                            // 2-bit: 1→1000, 2→1500, 3→2000
                            pwmValue = 1000 + (rawValue - 1) * 500;
                        } else {
                            // 4-bit: 1-15 → 1000-2000
                            pwmValue = 1000 + ((uint32_t)(rawValue - 1) * 1000) / 14;
                        }

                        rxMspAuxOverlaySet(startChannel + ch, pwmValue);
                    }
                }
            }

            if (src->overrun) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    case MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE:
        {
            msp2InavFlightAxisAngleOverrideRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            mspOverrideSetFlightAxisAngleOverride(pkt.overrideMask, pkt.angleTargetRoll,
                                                  pkt.angleTargetPitch, pkt.angleTargetYaw);
        }
        break;

    case MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE:
        {
            msp2InavFlightAxisRateOverrideRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            mspOverrideSetFlightAxisRateOverride(pkt.overrideMask, pkt.rateTargetRoll,
                                                 pkt.rateTargetPitch, pkt.rateTargetYaw);
        }
        break;
#endif
    case MSP2_COMMON_SET_MOTOR_MIXER:
        {
            msp2CommonSetMotorMixerRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.motorIndex >= MAX_SUPPORTED_MOTORS) {
                return MSP_RESULT_ERROR;
            }
            motorMixer_t *mix = primaryMotorMixerMutable(pkt.motorIndex);
            mix->throttle = constrainf(pkt.throttleWeight / 1000.0f, 0.0f, 4.0f) - 2.0f;
            mix->roll     = constrainf(pkt.rollWeight     / 1000.0f, 0.0f, 4.0f) - 2.0f;
            mix->pitch    = constrainf(pkt.pitchWeight    / 1000.0f, 0.0f, 4.0f) - 2.0f;
            mix->yaw      = constrainf(pkt.yawWeight      / 1000.0f, 0.0f, 4.0f) - 2.0f;
        }
        break;

    case MSP_SET_3D:
        {
            mspSet3dRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            reversibleMotorsConfigMutable()->deadband_low = pkt.deadbandLow;
            reversibleMotorsConfigMutable()->deadband_high = pkt.deadbandHigh;
            reversibleMotorsConfigMutable()->neutral = pkt.neutral;
        }
        break;

    case MSP_SET_RC_DEADBAND:
        {
            mspSetRcDeadbandRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            rcControlsConfigMutable()->deadband = pkt.deadband;
            rcControlsConfigMutable()->yaw_deadband = pkt.yawDeadband;
            rcControlsConfigMutable()->alt_hold_deadband = pkt.altHoldDeadband;
            rcControlsConfigMutable()->mid_throttle_deadband = pkt.throttleDeadband;
        }
        break;

    case MSP_SET_RESET_CURR_PID:
        PG_RESET_CURRENT(pidProfile);
        break;

    case MSP_SET_SENSOR_ALIGNMENT:
        {
            mspSetSensorAlignmentRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
#ifdef USE_MAG
            compassConfigMutable()->mag_align = pkt.magAlign;
#endif
#ifdef USE_OPFLOW
            opticalFlowConfigMutable()->opflow_align = pkt.opflowAlign;
#endif
        }
        break;

    case MSP_SET_ADVANCED_CONFIG:
        {
            mspSetAdvancedConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            motorConfigMutable()->motorPwmProtocol = pkt.motorPwmProtocol;
            motorConfigMutable()->motorPwmRate = pkt.motorPwmRate;
            servoConfigMutable()->servoPwmRate = pkt.servoPwmRate;
        }
        break;

    case MSP_SET_FILTER_CONFIG:
        {
            /* The old staircase of length checks stopped one byte short of the last
             * field and read past the end of a 22-byte payload; the struct gate is
             * the full 23 bytes, which is what every sender actually sends. */
            mspSetFilterConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            gyroConfigMutable()->gyro_main_lpf_hz = pkt.gyroMainLpfHz;
            pidProfileMutable()->dterm_lpf_hz = constrain(pkt.dtermLpfHz, 0, 500);
            pidProfileMutable()->yaw_lpf_hz = constrain(pkt.yawLpfHz, 0, 255);
            accelerometerConfigMutable()->acc_notch_hz = constrain(pkt.accNotchHz, 0, 255);
            accelerometerConfigMutable()->acc_notch_cutoff = constrain(pkt.accNotchCutoff, 1, 255);
            pidInitFilters();
        }
        break;

    case MSP_SET_PID_ADVANCED:
        {
            mspSetPidAdvancedRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            /*
             * To keep compatibility on MSP frame length level with Betaflight, axis axisAccelerationLimitYaw
             * limit will be sent and received in [dps / 10]
             */
            pidProfileMutable()->axisAccelerationLimitRollPitch = pkt.accelLimitRollPitch * 10;
            pidProfileMutable()->axisAccelerationLimitYaw = pkt.accelLimitYaw * 10;
        }
        break;

    case MSP_SET_INAV_PID:
        {
            mspSetInavPidRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            pidProfileMutable()->heading_hold_rate_limit = pkt.headingHoldRateLimit;
            accelerometerConfigMutable()->acc_lpf_hz = pkt.accLpfHz;
        }
        break;

    case MSP_SET_SENSOR_CONFIG:
        {
            mspSetSensorConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            accelerometerConfigMutable()->acc_hardware = pkt.accHardware;
#ifdef USE_BARO
            barometerConfigMutable()->baro_hardware = pkt.baroHardware;
#endif
#ifdef USE_MAG
            compassConfigMutable()->mag_hardware = pkt.magHardware;
#endif
#ifdef USE_PITOT
            pitotmeterConfigMutable()->pitot_hardware = pkt.pitotHardware;
#endif
#ifdef USE_RANGEFINDER
            rangefinderConfigMutable()->rangefinder_hardware = pkt.rangefinderHardware;
#endif
#ifdef USE_OPFLOW
            opticalFlowConfigMutable()->opflow_hardware = pkt.opflowHardware;
#endif
        }
        break;

    case MSP_SET_NAV_POSHOLD:
        {
            mspSetNavPosholdRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            navConfigMutable()->general.flags.user_control_mode = pkt.userControlMode;
            navConfigMutable()->general.max_auto_speed = pkt.maxAutoSpeed;
            navConfigMutable()->general.max_manual_speed = pkt.maxManualSpeed;
            if (mixerConfig()->platformType == PLATFORM_AIRPLANE) {
                navConfigMutable()->fw.max_auto_climb_rate = pkt.maxAutoClimbRate;
                navConfigMutable()->fw.max_manual_climb_rate = pkt.maxManualClimbRate;
            } else {
                navConfigMutable()->mc.max_auto_climb_rate = pkt.maxAutoClimbRate;
                navConfigMutable()->mc.max_manual_climb_rate = pkt.maxManualClimbRate;
            }
            navConfigMutable()->mc.max_bank_angle = pkt.mcMaxBankAngle;
            navConfigMutable()->mc.althold_throttle_type = pkt.mcAltHoldThrottleType;
            currentBatteryProfileMutable->nav.mc.hover_throttle = pkt.mcHoverThrottle;
        }
        break;

    case MSP_SET_RTH_AND_LAND_CONFIG:
        {
            mspSetRthAndLandConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            navConfigMutable()->general.min_rth_distance = pkt.minRthDistance;
            navConfigMutable()->general.flags.rth_climb_first = pkt.rthClimbFirst;
            navConfigMutable()->general.flags.rth_climb_ignore_emerg = pkt.rthClimbIgnoreEmerg;
            navConfigMutable()->general.flags.rth_tail_first = pkt.rthTailFirst;
            navConfigMutable()->general.flags.rth_allow_landing = pkt.rthAllowLanding;
            navConfigMutable()->general.flags.rth_alt_control_mode = pkt.rthAltControlMode;
            navConfigMutable()->general.rth_abort_threshold = pkt.rthAbortThreshold;
            navConfigMutable()->general.rth_altitude = pkt.rthAltitude;
            navConfigMutable()->general.land_minalt_vspd = pkt.landMinAltVspd;
            navConfigMutable()->general.land_maxalt_vspd = pkt.landMaxAltVspd;
            navConfigMutable()->general.land_slowdown_minalt = pkt.landSlowdownMinAlt;
            navConfigMutable()->general.land_slowdown_maxalt = pkt.landSlowdownMaxAlt;
            navConfigMutable()->general.emerg_descent_rate = pkt.emergDescentRate;
        }
        break;

    case MSP_SET_FW_CONFIG:
        {
            mspSetFwConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            currentBatteryProfileMutable->nav.fw.cruise_throttle = pkt.cruiseThrottle;
            currentBatteryProfileMutable->nav.fw.min_throttle = pkt.minThrottle;
            currentBatteryProfileMutable->nav.fw.max_throttle = pkt.maxThrottle;
            navConfigMutable()->fw.max_bank_angle = pkt.maxBankAngle;
            navConfigMutable()->fw.max_climb_angle = pkt.maxClimbAngle;
            navConfigMutable()->fw.max_dive_angle = pkt.maxDiveAngle;
            currentBatteryProfileMutable->nav.fw.pitch_to_throttle = pkt.pitchToThrottle;
            navConfigMutable()->fw.loiter_radius = pkt.loiterRadius;
        }
        break;

    case MSP_SET_CALIBRATION_DATA:
        {
            /* 18 required bytes, then two appended tails: the opflow scale and the
             * mag gains. The old code gated the mag gains on 22 bytes and then read
             * six, running past the end of the payload. */
            mspSetCalibrationDataRequest_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(mspSetCalibrationDataRequest_t, opflowScale), dataSize)) {
                return MSP_RESULT_ERROR;
            }
            accelerometerConfigMutable()->accZero.raw[X] = pkt.accZeroX;
            accelerometerConfigMutable()->accZero.raw[Y] = pkt.accZeroY;
            accelerometerConfigMutable()->accZero.raw[Z] = pkt.accZeroZ;
            accelerometerConfigMutable()->accGain.raw[X] = pkt.accGainX;
            accelerometerConfigMutable()->accGain.raw[Y] = pkt.accGainY;
            accelerometerConfigMutable()->accGain.raw[Z] = pkt.accGainZ;
#ifdef USE_MAG
            compassConfigMutable()->magZero.raw[X] = pkt.magZeroX;
            compassConfigMutable()->magZero.raw[Y] = pkt.magZeroY;
            compassConfigMutable()->magZero.raw[Z] = pkt.magZeroZ;
#endif
            if (mspReadOptional(src, &pkt.opflowScale)) {
#ifdef USE_OPFLOW
                opticalFlowConfigMutable()->opflow_scale = pkt.opflowScale / 256.0f;
#endif
            }
            if (mspReadOptionalBytes(src, &pkt.magGainX, 3 * sizeof(pkt.magGainX))) {
#ifdef USE_MAG
                compassConfigMutable()->magGain[X] = pkt.magGainX;
                compassConfigMutable()->magGain[Y] = pkt.magGainY;
                compassConfigMutable()->magGain[Z] = pkt.magGainZ;
#endif
            }
        }
        break;

    case MSP_SET_POSITION_ESTIMATION_CONFIG:
        {
            mspSetPositionEstimationConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            positionEstimationConfigMutable()->w_z_baro_p = constrainf(pkt.weightZBaroP / 100.0f, 0.0f, 10.0f);
            positionEstimationConfigMutable()->w_z_gps_p = constrainf(pkt.weightZGPSP / 100.0f, 0.0f, 10.0f);
            positionEstimationConfigMutable()->w_z_gps_v = constrainf(pkt.weightZGPSV / 100.0f, 0.0f, 10.0f);
            positionEstimationConfigMutable()->w_xy_gps_p = constrainf(pkt.weightXYGPSP / 100.0f, 0.0f, 10.0f);
            positionEstimationConfigMutable()->w_xy_gps_v = constrainf(pkt.weightXYGPSV / 100.0f, 0.0f, 10.0f);
            gpsConfigMutable()->gpsMinSats = constrain(pkt.minSats, 5, 10);
        }
        break;

    case MSP_RESET_CONF:
        if (!ARMING_FLAG(ARMED)) {
            suspendRxSignal();
            resetEEPROM();
            writeEEPROM();
            readEEPROM();
            resumeRxSignal();
        } else
            return MSP_RESULT_ERROR;
        break;

    case MSP_ACC_CALIBRATION:
        if (!ARMING_FLAG(ARMED))
            accStartCalibration();
        else
            return MSP_RESULT_ERROR;
        break;

    case MSP_MAG_CALIBRATION:
        if (!ARMING_FLAG(ARMED))
            ENABLE_STATE(CALIBRATE_MAG);
        else
            return MSP_RESULT_ERROR;
        break;

#ifdef USE_OPFLOW
    case MSP2_INAV_OPFLOW_CALIBRATION:
        if (!ARMING_FLAG(ARMED))
            opflowStartCalibration();
        else
            return MSP_RESULT_ERROR;
        break;
#endif

    case MSP_EEPROM_WRITE:
        if (!ARMING_FLAG(ARMED)) {
            suspendRxSignal();
            writeEEPROM();
            readEEPROM();
            resumeRxSignal();
        } else
            return MSP_RESULT_ERROR;
        break;

#ifdef USE_BLACKBOX
    case MSP2_SET_BLACKBOX_CONFIG:
        {
            // Don't allow config to be updated while Blackbox is logging
            msp2SetBlackboxConfigRequest_t pkt;
            if (!blackboxMayEditConfig() || !mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            blackboxConfigMutable()->device = pkt.blackboxDevice;
            blackboxConfigMutable()->rate_num = pkt.blackboxRateNum;
            blackboxConfigMutable()->rate_denom = pkt.blackboxRateDenom;
            blackboxConfigMutable()->includeFlags = pkt.blackboxIncludeFlags;
        }
        break;
#endif

#ifdef USE_OSD
    case MSP_SET_OSD_CONFIG:
        /* The first byte selects the shape: 0xFF means "settings", anything else is
         * an item index and the message carries just that item's position. */
        if (sbufBytesRemaining(src) >= 1 && *sbufPtr(src) == 0xFF) {
            mspSetOsdConfig_dataSize_ge_10Request_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(mspSetOsdConfig_dataSize_ge_10Request_t, distAlarm), dataSize)) {
                return MSP_RESULT_ERROR;
            }
            osdConfigMutable()->video_system = pkt.videoSystem;
            osdConfigMutable()->units = pkt.units;
            osdConfigMutable()->rssi_alarm = pkt.rssiAlarm;
            currentBatteryProfileMutable->capacity.warning = pkt.capAlarm;
            osdConfigMutable()->time_alarm = pkt.timerAlarm;
            osdConfigMutable()->alt_alarm = pkt.altAlarm;
            // Won't be read if they weren't provided
            if (mspReadOptional(src, &pkt.distAlarm)) {
                osdConfigMutable()->dist_alarm = pkt.distAlarm;
            }
            if (mspReadOptional(src, &pkt.negAltAlarm)) {
                osdConfigMutable()->neg_alt_alarm = pkt.negAltAlarm;
            }
        } else {
            mspSetOsdConfig_dataSize_eq_3Request_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.itemIndex >= OSD_ITEM_COUNT) {
                return MSP_RESULT_ERROR;
            }
            osdLayoutsConfigMutable()->item_pos[0][pkt.itemIndex] = pkt.itemPosition;
        }
        // Either a element position change or a units change needs
        // a full redraw, since an element can change size significantly
        // and the old position or the now unused space due to the
        // size change need to be erased.
        osdStartFullRedraw();
        break;

    case MSP2_INAV_OSD_UPDATE_POSITION:
        {
            msp2InavOsdUpdatePositionRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.itemIndex >= OSD_ITEM_COUNT) {
                return MSP_RESULT_ERROR;
            }
            osdEraseCustomItem(pkt.itemIndex);
            osdLayoutsConfigMutable()->item_pos[getCurrentLayout()][pkt.itemIndex] = pkt.itemPosition | OSD_VISIBLE_FLAG;
            osdDrawCustomItem(pkt.itemIndex);
        }
        return MSP_RESULT_ACK;



    case MSP_OSD_CHAR_WRITE:
        if (dataSize >= 55) {
            osdCharacter_t chr;
            size_t osdCharacterBytes;
            uint16_t addr;
            if (dataSize >= OSD_CHAR_VISIBLE_BYTES + 2) {
                if (dataSize >= OSD_CHAR_BYTES + 2) {
                    // 16 bit address, full char with metadata
                    addr = sbufReadU16(src);
                    osdCharacterBytes = OSD_CHAR_BYTES;
                } else if (dataSize >= OSD_CHAR_BYTES + 1) {
                    // 8 bit address, full char with metadata
                    addr = sbufReadU8(src);
                    osdCharacterBytes = OSD_CHAR_BYTES;
                } else {
                    // 16 bit character address, only visible char bytes
                    addr = sbufReadU16(src);
                    osdCharacterBytes = OSD_CHAR_VISIBLE_BYTES;
                }
            } else {
                // 8 bit character address, only visible char bytes
                addr = sbufReadU8(src);
                osdCharacterBytes = OSD_CHAR_VISIBLE_BYTES;
            }
            for (unsigned ii = 0; ii < MIN(osdCharacterBytes, sizeof(chr.data)); ii++) {
                chr.data[ii] = sbufReadU8(src);
            }

            if (src->overrun) {
                return MSP_RESULT_ERROR;
            }

            displayPort_t *osdDisplayPort = osdGetDisplayPort();
            if (osdDisplayPort) {
                displayWriteFontCharacter(osdDisplayPort, addr, &chr);
            }
        } else {
            return MSP_RESULT_ERROR;
        }
        break;
#endif // USE_OSD

#ifdef USE_VTX_CONTROL
    case MSP_SET_VTX_CONFIG:
        if (dataSize >= 2) {
            vtxDevice_t *vtxDevice = vtxCommonDevice();
            if (vtxDevice) {
                if (vtxCommonGetDeviceType(vtxDevice) != VTXDEV_UNKNOWN) {
                    uint16_t newFrequency = sbufReadU16(src);
                    if (newFrequency <= VTXCOMMON_MSP_BANDCHAN_CHKVAL) {  //value is band and channel
                        const uint8_t newBand = (newFrequency / 8) + 1;
                        const uint8_t newChannel = (newFrequency % 8) + 1;
                        if (vtxSettingsConfig()->band != newBand || vtxSettingsConfig()->channel != newChannel) {
                            vtxSettingsConfigMutable()->band = newBand;
                            vtxSettingsConfigMutable()->channel = newChannel;
                        }
                    }

                    if (sbufBytesRemaining(src) > 1) {
                        uint8_t newPower = sbufReadU8(src);
                        if (vtxSettingsConfig()->power != newPower) {
                            vtxSettingsConfigMutable()->power = newPower;
                        }

                        // Delegate pitmode to vtx directly
                        const uint8_t newPitmode = sbufReadU8(src);
                        uint8_t currentPitmode = 0;
                        vtxCommonGetPitMode(vtxDevice, &currentPitmode);
                        if (currentPitmode != newPitmode) {
                            vtxCommonSetPitMode(vtxDevice, newPitmode);
                        }

                        if (sbufBytesRemaining(src) > 0) {
                            vtxSettingsConfigMutable()->lowPowerDisarm = sbufReadU8(src);
                        }

                        // API version 1.42 - extension for pitmode frequency
                        if (sbufBytesRemaining(src) >= 2) {
                            sbufReadU16(src); //skip pitModeFreq
                        }

                        // API version 1.42 - extensions for non-encoded versions of the band, channel or frequency
                        if (sbufBytesRemaining(src) >= 4) {
                            uint8_t newBand = sbufReadU8(src);
                            if (vtxSettingsConfig()->band != newBand) {
                                vtxSettingsConfigMutable()->band = newBand;
                            }

                            const uint8_t newChannel = sbufReadU8(src);
                            if (vtxSettingsConfig()->channel != newChannel) {
                                vtxSettingsConfigMutable()->channel = newChannel;
                            }
                        }

                        if (sbufBytesRemaining(src) >= 2) {
                            sbufReadU16(src); // freq
                        }

                        if (sbufBytesRemaining(src) >= 3) {
                            sbufReadU8(src); // band count
                            sbufReadU8(src); // channel count

                            uint8_t newPowerCount = sbufReadU8(src);
                            if (newPowerCount > 0 && newPowerCount < (vtxDevice->capability.powerCount)) {
                                vtxDevice->capability.powerCount = newPowerCount;
                            }
                        }
                    }
                }
            }
        } else {
            return MSP_RESULT_ERROR;
        }
        break;
#endif

#ifdef USE_FLASHFS
    case MSP_DATAFLASH_ERASE:
        if (blackboxMayEditConfig()) {
            flashfsEraseCompletely();
        } else {
            return MSP_RESULT_ERROR;
        }
        break;
#endif

#ifdef USE_GPS
    case MSP_SET_RAW_GPS:
        {
            mspSetRawGpsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            gpsSol.fixType = pkt.fixType;
            if (gpsSol.fixType) {
                ENABLE_STATE(GPS_FIX);
            } else {
                DISABLE_STATE(GPS_FIX);
            }
            gpsSol.flags.validVelNE = false;
            gpsSol.flags.validVelD = false;
            gpsSol.flags.validEPE = false;
            gpsSol.flags.validTime = false;
            gpsSol.numSat = pkt.numSat;
            gpsSol.llh.lat = pkt.latitude;
            gpsSol.llh.lon = pkt.longitude;
            gpsSol.llh.alt = 100 * pkt.altitude; // require cm
            gpsSol.groundSpeed = pkt.speed;
            gpsSol.velNED[X] = 0;
            gpsSol.velNED[Y] = 0;
            gpsSol.velNED[Z] = 0;
            gpsSol.eph = 100;
            gpsSol.epv = 100;
            // Feed data to navigation
            sensorsSet(SENSOR_GPS);
            onNewGPSData();
        }
        break;
#endif

    case MSP_SET_WP:
        {
            mspSetWpRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            navWaypoint_t msp_wp = {
                .action = pkt.action,
                .lat = pkt.latitude,
                .lon = pkt.longitude,
                .alt = pkt.altitude,
                .p1 = pkt.param1,
                .p2 = pkt.param2,
                .p3 = pkt.param3,
                .flag = pkt.flag,
            };
            setWaypoint(pkt.waypointIndex, &msp_wp);

#ifdef USE_FW_AUTOLAND
            static uint8_t mmIdx = 0, fwAppraochStartIdx = 8;
#ifdef USE_SAFE_HOME
            fwAppraochStartIdx = MAX_SAFE_HOMES;
#endif
            if (pkt.waypointIndex == 0) {
                mmIdx = 0;
            } else if (msp_wp.flag == NAV_WP_FLAG_LAST) {
                mmIdx++;
            }
            resetFwAutolandApproach(fwAppraochStartIdx + mmIdx);
#endif
        }
        break;

    case MSP2_COMMON_SET_RADAR_POS:
        {
            msp2CommonSetRadarPosRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            radar_pois_t *poi = &radar_pois[MIN(pkt.poiIndex, RADAR_MAX_POIS - 1)];
            poi->state = pkt.state;          // 0=undefined, 1=armed, 2=lost
            poi->gps.lat = pkt.latitude;     // lat 10E7
            poi->gps.lon = pkt.longitude;    // lon 10E7
            poi->gps.alt = pkt.altitude;     // altitude (cm)
            poi->heading = pkt.heading;      // °
            poi->speed = pkt.speed;          // cm/s
            poi->lq = pkt.linkQuality;       // Link quality, from 0 to 4
        }
        break;

    case MSP_SET_FEATURE:
        {
            mspSetFeatureRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            featureClearAll();
            featureSet(pkt.featureMask);
            rxUpdateRSSISource(); // For FEATURE_RSSI_ADC
        }
        break;

    case MSP_SET_BOARD_ALIGNMENT:
        {
            mspSetBoardAlignmentRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            boardAlignmentMutable()->rollDeciDegrees = pkt.rollAlign;
            boardAlignmentMutable()->pitchDeciDegrees = pkt.pitchAlign;
            boardAlignmentMutable()->yawDeciDegrees = pkt.yawAlign;
        }
        break;

    case MSP_SET_VOLTAGE_METER_CONFIG:
        {
            mspSetVoltageMeterConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
#ifdef USE_ADC
            batteryMetersConfigMutable()->voltage.scale = pkt.vbatScale * 10;
            currentBatteryProfileMutable->voltage.cellMin = pkt.vbatMinCell * 10;
            currentBatteryProfileMutable->voltage.cellMax = pkt.vbatMaxCell * 10;
            currentBatteryProfileMutable->voltage.cellWarning = pkt.vbatWarningCell * 10;
#endif
        }
        break;

    case MSP_SET_CURRENT_METER_CONFIG:
        {
            mspSetCurrentMeterConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            batteryMetersConfigMutable()->current.scale = pkt.scale;
            batteryMetersConfigMutable()->current.offset = pkt.offset;
            batteryMetersConfigMutable()->current.type = pkt.type;
            currentBatteryProfileMutable->capacity.value = pkt.capacity;
        }
        break;

    case MSP_SET_MIXER:
        {
            mspSetMixerRequest_t pkt; // mixerMode is ignored, INAV no longer supports it
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            mixerUpdateStateFlags();    // Required for correct preset functionality
        }
        break;

    case MSP_SET_RX_CONFIG:
        {
            mspSetRxConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            rxConfigMutable()->serialrx_provider = pkt.serialRxProvider;
            rxConfigMutable()->maxcheck = pkt.maxCheck;
            rxConfigMutable()->mincheck = pkt.minCheck;
#ifdef USE_SPEKTRUM_BIND
            rxConfigMutable()->spektrum_sat_bind = pkt.spektrumSatBind;
#endif
            rxConfigMutable()->rx_min_usec = pkt.rxMinUsec;
            rxConfigMutable()->rx_max_usec = pkt.rxMaxUsec;
            rxConfigMutable()->receiverType = pkt.receiverType;
        }
        break;

#ifdef USE_RX_MSP
    case MSP2_COMMON_SET_MSP_RC_LINK_STATS:
        {
            msp2CommonSetMspRcLinkStatsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            if (pkt.sublinkID == 0) {
                setRSSIFromMSP_RC(pkt.rssiPercent);
                rxLinkStatistics.uplinkRSSI = -pkt.uplinkRSSI_dBm;
                rxLinkStatistics.downlinkLQ = pkt.downlinkLQ;
                rxLinkStatistics.uplinkLQ = pkt.uplinkLQ;
                rxLinkStatistics.uplinkSNR = pkt.uplinkSNR;
            }
        }
        return MSP_RESULT_NO_REPLY;

    case MSP2_COMMON_SET_MSP_RC_INFO:
        {
            msp2CommonSetMspRcInfoRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            if (pkt.sublinkID == 0) {
                rxLinkStatistics.uplinkTXPower = pkt.uplinkTxPower;
                rxLinkStatistics.downlinkTXPower = pkt.downlinkTxPower;

                memcpy(rxLinkStatistics.band, pkt.band, sizeof(rxLinkStatistics.band));
                sl_toupperptr(rxLinkStatistics.band);

                memcpy(rxLinkStatistics.mode, pkt.mode, sizeof(rxLinkStatistics.mode));
                sl_toupperptr(rxLinkStatistics.mode);
            }
        }
        return MSP_RESULT_NO_REPLY;
#endif

    case MSP_SET_FAILSAFE_CONFIG:
        {
            mspSetFailsafeConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            failsafeConfigMutable()->failsafe_delay = pkt.failsafeDelay;
            failsafeConfigMutable()->failsafe_off_delay = pkt.failsafeOffDelay;
            currentBatteryProfileMutable->failsafe_throttle = pkt.failsafeThrottle;
            failsafeConfigMutable()->failsafe_throttle_low_delay = pkt.failsafeThrottleLowDelay;
            failsafeConfigMutable()->failsafe_procedure = pkt.failsafeProcedure;
            failsafeConfigMutable()->failsafe_recovery_delay = pkt.failsafeRecoveryDelay;
            failsafeConfigMutable()->failsafe_fw_roll_angle = pkt.failsafeFWRollAngle;
            failsafeConfigMutable()->failsafe_fw_pitch_angle = pkt.failsafeFWPitchAngle;
            failsafeConfigMutable()->failsafe_fw_yaw_rate = pkt.failsafeFWYawRate;
            failsafeConfigMutable()->failsafe_stick_motion_threshold = pkt.failsafeStickThreshold;
            failsafeConfigMutable()->failsafe_min_distance = pkt.failsafeMinDistance;
            failsafeConfigMutable()->failsafe_min_distance_procedure = pkt.failsafeMinDistanceProc;
        }
        break;

    case MSP_SET_RSSI_CONFIG:
        {
            mspSetRssiConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.rssiChannel > MAX_SUPPORTED_RC_CHANNEL_COUNT) {
                return MSP_RESULT_ERROR;
            }
            rxConfigMutable()->rssi_channel = pkt.rssiChannel;
            rxUpdateRSSISource();
        }
        break;

    case MSP_SET_RX_MAP:
        {
            mspSetRxMapRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            memcpy(rxConfigMutable()->rcmap, pkt.rcMap, sizeof(pkt.rcMap));
        }
        break;

    case MSP2_COMMON_SET_SERIAL_CONFIG:
        {
            /* A whole number of port records, no header. */
            if (dataSize % sizeof(msp2CommonSetSerialConfigRequestElem_t) != 0) {
                return MSP_RESULT_ERROR;
            }

            unsigned remainingPortsInPacket = dataSize / sizeof(msp2CommonSetSerialConfigRequestElem_t);
            while (remainingPortsInPacket--) {
                msp2CommonSetSerialConfigRequestElem_t record;
                if (!mspReadRequest(src, &record, sbufBytesRemaining(src))) {
                    return MSP_RESULT_ERROR;
                }

                serialPortConfig_t *portConfig = serialFindPortConfiguration(record.identifier);
                if (!portConfig) {
                    return MSP_RESULT_ERROR;
                }

                portConfig->identifier = record.identifier;
                portConfig->functionMask = record.functionMask;
                portConfig->msp_baudrateIndex = constrain(record.mspBaudIndex, BAUD_MIN, BAUD_MAX);
                portConfig->gps_baudrateIndex = constrain(record.gpsBaudIndex, BAUD_MIN, BAUD_MAX);
                portConfig->telemetry_baudrateIndex = constrain(record.telemetryBaudIndex, BAUD_MIN, BAUD_MAX);
                portConfig->peripheral_baudrateIndex = constrain(record.peripheralBaudIndex, BAUD_MIN, BAUD_MAX);
            }
        }
        break;

#ifdef USE_LED_STRIP
    case MSP_SET_LED_COLORS:
        {
            mspSetLedColorsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            for (int i = 0; i < LED_CONFIGURABLE_COLOR_COUNT; i++) {
                hsvColor_t *color = &ledStripConfigMutable()->colors[i];
                color->h = pkt.items[i].hue;
                color->s = pkt.items[i].saturation;
                color->v = pkt.items[i].value;
            }
        }
        break;

    case MSP_SET_LED_STRIP_CONFIG:
        {
            mspSetLedStripConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.ledIndex >= LED_MAX_STRIP_LENGTH) {
                return MSP_RESULT_ERROR;
            }
            ledConfig_t *ledConfig = &ledStripConfigMutable()->ledConfigs[pkt.ledIndex];

            const uint32_t legacyConfig = pkt.legacyLedConfig;

            ledConfig->led_position = legacyConfig & 0xFF;
            ledConfig->led_function = (legacyConfig >> 8) & 0xF;
            ledConfig->led_overlay = (legacyConfig >> 12) & 0x3F;
            ledConfig->led_color = (legacyConfig >> 18) & 0xF;
            ledConfig->led_direction = (legacyConfig >> 22) & 0x3F;
            ledConfig->led_params = (legacyConfig >> 28) & 0xF;

            reevaluateLedConfig();
        }
        break;

    case MSP2_INAV_SET_LED_STRIP_CONFIG_EX:
        {
            msp2InavSetLedStripConfigExRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.ledIndex >= LED_MAX_STRIP_LENGTH) {
                return MSP_RESULT_ERROR;
            }
            ledStripConfigMutable()->ledConfigs[pkt.ledIndex] = pkt.ledConfig;
            reevaluateLedConfig();
        }
        break;

    case MSP_SET_LED_STRIP_MODECOLOR:
        {
            mspSetLedStripModecolorRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            if (!setModeColor(pkt.modeIndex, pkt.directionOrSpecialIndex, pkt.colorIndex)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;
#endif

#ifdef NAV_NON_VOLATILE_WAYPOINT_STORAGE
    case MSP_WP_MISSION_LOAD:
        sbufReadU8Safe(NULL, src);    // Mission ID (reserved)
        if ((dataSize != 1) || (!loadNonVolatileWaypointList(false)))
            return MSP_RESULT_ERROR;
        break;

    case MSP_WP_MISSION_SAVE:
        sbufReadU8Safe(NULL, src);    // Mission ID (reserved)
        if ((dataSize != 1) || (!saveNonVolatileWaypointList()))
            return MSP_RESULT_ERROR;
        break;
#endif

    case MSP_SET_RTC:
        {
            // Use seconds and milliseconds to make senders
            // easier to implement. Generating a 64 bit value
            // might not be trivial in some platforms.
            mspSetRtcRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            rtcTime_t t = rtcTimeMake(pkt.seconds, pkt.millis);
            rtcSet(&t);
        }
        break;

    case MSP_SET_TX_INFO:
        {
            // This message will be sent while the aircraft is
            // armed. Better to guard ourselves against potentially
            // malformed requests.
            mspSetTxInfoRequest_t pkt;
            if (mspReadRequest(src, &pkt, dataSize)) {
                setRSSIFromMSP(pkt.rssi);
            }
        }
        break;

    case MSP_SET_NAME:
        if (dataSize <= MAX_NAME_LENGTH) {
            char *name = systemConfigMutable()->craftName;
            int len = MIN(MAX_NAME_LENGTH, (int)dataSize);
            sbufReadData(src, name, len);
            memset(&name[len], '\0', (MAX_NAME_LENGTH + 1) - len);
        } else
            return MSP_RESULT_ERROR;
        break;

    case MSP2_COMMON_SET_TZ:
        {
            /* The DST flag is an optional third byte. */
            msp2CommonSetTz_dataSize_eq_3Request_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(msp2CommonSetTz_dataSize_eq_3Request_t, tz_automatic_dst), dataSize)) {
                return MSP_RESULT_ERROR;
            }
            timeConfigMutable()->tz_offset = pkt.tz_offset;
            if (mspReadOptional(src, &pkt.tz_automatic_dst)) {
                timeConfigMutable()->tz_automatic_dst = pkt.tz_automatic_dst;
            }
        }
        break;

    case MSP2_INAV_SET_MIXER:
        {
            msp2InavSetMixerRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            mixerConfigMutable()->motorDirectionInverted = pkt.motorDirectionInverted;
            mixerConfigMutable()->motorstopOnLow = pkt.motorStopOnLow;
            mixerConfigMutable()->platformType = pkt.platformType;
            mixerConfigMutable()->hasFlaps = pkt.hasFlaps;
            mixerConfigMutable()->appliedMixerPreset = pkt.appliedMixerPreset;
            mixerUpdateStateFlags();
        }
        break;

#if defined(USE_OSD)
    case MSP2_INAV_OSD_SET_LAYOUT_ITEM:
        {
            msp2InavOsdSetLayoutItemRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            osdLayoutsConfigMutable()->item_pos[pkt.layoutIndex][pkt.itemIndex] = pkt.itemPosition;
            // If the layout is not already overriden and it's different
            // than the layout for the item that was just configured,
            // override it for 10 seconds.
            bool overridden;
            int activeLayout = osdGetActiveLayout(&overridden);
            if (activeLayout != pkt.layoutIndex && !overridden) {
                osdOverrideLayout(pkt.layoutIndex, 10000);
            } else {
                osdStartFullRedraw();
            }
        }
        break;

    case MSP2_INAV_OSD_SET_ALARMS:
        {
            msp2InavOsdSetAlarmsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            osdConfigMutable()->rssi_alarm = pkt.rssiAlarm;
            osdConfigMutable()->time_alarm = pkt.timerAlarm;
            osdConfigMutable()->alt_alarm = pkt.altAlarm;
            osdConfigMutable()->dist_alarm = pkt.distAlarm;
            osdConfigMutable()->neg_alt_alarm = pkt.negAltAlarm;
            osdConfigMutable()->gforce_alarm = pkt.gForceAlarm / 1000.0f;
            osdConfigMutable()->gforce_axis_alarm_min = pkt.gForceAxisMinAlarm / 1000.0f;
            osdConfigMutable()->gforce_axis_alarm_max = pkt.gForceAxisMaxAlarm / 1000.0f;
            osdConfigMutable()->current_alarm = pkt.currentAlarm;
            osdConfigMutable()->imu_temp_alarm_min = pkt.imuTempMinAlarm;
            osdConfigMutable()->imu_temp_alarm_max = pkt.imuTempMaxAlarm;
#ifdef USE_BARO
            osdConfigMutable()->baro_temp_alarm_min = pkt.baroTempMinAlarm;
            osdConfigMutable()->baro_temp_alarm_max = pkt.baroTempMaxAlarm;
#endif
        }
        break;

    case MSP2_INAV_OSD_SET_PREFERENCES:
        {
            /* adsbWarningStyle is an appended optional byte. */
            msp2InavOsdSetPreferencesRequest_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(msp2InavOsdSetPreferencesRequest_t, adsbWarningStyle), dataSize)) {
                return MSP_RESULT_ERROR;
            }
            osdConfigMutable()->video_system = pkt.videoSystem;
            osdConfigMutable()->main_voltage_decimals = pkt.mainVoltageDecimals;
            osdConfigMutable()->ahi_reverse_roll = pkt.ahiReverseRoll;
            osdConfigMutable()->crosshairs_style = pkt.crosshairsStyle;
            osdConfigMutable()->left_sidebar_scroll = pkt.leftSidebarScroll;
            osdConfigMutable()->right_sidebar_scroll = pkt.rightSidebarScroll;
            osdConfigMutable()->sidebar_scroll_arrows = pkt.sidebarScrollArrows;
            osdConfigMutable()->units = pkt.units;
            osdConfigMutable()->stats_energy_unit = pkt.statsEnergyUnit;
            if (mspReadOptional(src, &pkt.adsbWarningStyle)) {
#ifdef USE_ADSB
                osdConfigMutable()->adsb_warning_style = pkt.adsbWarningStyle;
#endif
            }
            osdStartFullRedraw();
        }
        break;
#endif

    case MSP2_INAV_SET_MC_BRAKING:
#ifdef USE_MR_BRAKING_MODE
        {
            msp2InavSetMcBrakingRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            navConfigMutable()->mc.braking_speed_threshold = pkt.brakingSpeedThreshold;
            navConfigMutable()->mc.braking_disengage_speed = pkt.brakingDisengageSpeed;
            navConfigMutable()->mc.braking_timeout = pkt.brakingTimeout;
            navConfigMutable()->mc.braking_boost_factor = pkt.brakingBoostFactor;
            navConfigMutable()->mc.braking_boost_timeout = pkt.brakingBoostTimeout;
            navConfigMutable()->mc.braking_boost_speed_threshold = pkt.brakingBoostSpeedThreshold;
            navConfigMutable()->mc.braking_boost_disengage_speed = pkt.brakingBoostDisengageSpeed;
            navConfigMutable()->mc.braking_bank_angle = pkt.brakingBankAngle;
        }
#else
        return MSP_RESULT_ERROR;
#endif
        break;

    case MSP2_INAV_SELECT_BATTERY_PROFILE:
        {
            msp2InavSelectBatteryProfileRequest_t pkt;
            if (ARMING_FLAG(ARMED) || !mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            setConfigBatteryProfileAndWriteEEPROM(pkt.batteryProfileIndex);
        }
        break;

    case MSP2_INAV_SELECT_MIXER_PROFILE:
        {
            msp2InavSelectMixerProfileRequest_t pkt;
            if (ARMING_FLAG(ARMED) || !mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            setConfigMixerProfileAndWriteEEPROM(pkt.mixerProfileIndex);
        }
        break;

#ifdef USE_TEMPERATURE_SENSOR
    case MSP2_INAV_SET_TEMP_SENSOR_CONFIG:
        {
            msp2InavSetTempSensorConfigRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            for (uint8_t index = 0; index < MAX_TEMP_SENSORS; ++index) {
                tempSensorConfig_t *sensorConfig = tempSensorConfigMutable(index);
                sensorConfig->type = pkt.items[index].type;
                sensorConfig->address = pkt.items[index].address;
                sensorConfig->alarm_min = pkt.items[index].alarmMin;
                sensorConfig->alarm_max = pkt.items[index].alarmMax;
                sensorConfig->osdSymbol = pkt.items[index].osdSymbol > TEMP_SENSOR_SYM_COUNT ? 0 : pkt.items[index].osdSymbol;
                for (uint8_t labelIndex = 0; labelIndex < TEMPERATURE_LABEL_LEN; ++labelIndex) {
                    sensorConfig->label[labelIndex] = toupper(pkt.items[index].label[labelIndex]);
                }
            }
        }
        break;
#endif

#ifdef MSP_FIRMWARE_UPDATE
    case MSP2_INAV_FWUPDT_PREPARE:
        if (!firmwareUpdatePrepare(sbufReadU32(src))) {
            return MSP_RESULT_ERROR;
        }
        break;
    case MSP2_INAV_FWUPDT_STORE:
        if (!firmwareUpdateStore(sbufPtr(src), sbufBytesRemaining(src))) {
            return MSP_RESULT_ERROR;
        }
        break;
    case MSP2_INAV_FWUPDT_EXEC:
        firmwareUpdateExec(sbufReadU8(src));
        return MSP_RESULT_ERROR; // will only be reached if the update is not ready
        break;
    case MSP2_INAV_FWUPDT_ROLLBACK_PREPARE:
        if (!firmwareUpdateRollbackPrepare()) {
            return MSP_RESULT_ERROR;
        }
        break;
    case MSP2_INAV_FWUPDT_ROLLBACK_EXEC:
        firmwareUpdateRollbackExec();
        return MSP_RESULT_ERROR; // will only be reached if the rollback is not ready
        break;
#endif
#ifdef USE_SAFE_HOME
    case MSP2_INAV_SET_SAFEHOME:
        {
            msp2InavSetSafehomeRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.safehomeIndex >= MAX_SAFE_HOMES) {
                return MSP_RESULT_ERROR;
            }
            safeHomeConfigMutable(pkt.safehomeIndex)->enabled = pkt.enabled;
            safeHomeConfigMutable(pkt.safehomeIndex)->lat = pkt.latitude;
            safeHomeConfigMutable(pkt.safehomeIndex)->lon = pkt.longitude;
#ifdef USE_FW_AUTOLAND
            resetFwAutolandApproach(pkt.safehomeIndex);
#endif
        }
        break;
#endif

#ifdef USE_FW_AUTOLAND
    case MSP2_INAV_SET_FW_APPROACH:
        {
            msp2InavSetFwApproachRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.approachIndex >= MAX_FW_LAND_APPOACH_SETTINGS) {
                return MSP_RESULT_ERROR;
            }
            navFwAutolandApproach_t *approach = fwAutolandApproachConfigMutable(pkt.approachIndex);
            approach->approachAlt = pkt.approachAlt;
            approach->landAlt = pkt.landAlt;
            approach->approachDirection = pkt.approachDirection;
            approach->landApproachHeading1 = pkt.landHeading1;
            approach->landApproachHeading2 = pkt.landHeading2;
            approach->isSeaLevelRef = pkt.isSeaLevelRef;
        }
        break;
#endif
    case MSP2_INAV_GPS_UBLOX_COMMAND:
        if(dataSize < 8 || !isGpsUblox()) {
            return MSP_RESULT_ERROR;
        }

        gpsUbloxSendCommand(src->ptr, dataSize, 0);
        break;

#ifdef USE_GEOZONE
    case MSP2_INAV_SET_GEOZONE:
        {
            msp2InavSetGeozoneRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.geozoneIndex >= MAX_GEOZONES_IN_CONFIG) {
                return MSP_RESULT_ERROR;
            }
            geozoneResetVertices(pkt.geozoneIndex, -1);
            geoZoneConfig_t *zone = geoZonesConfigMutable(pkt.geozoneIndex);
            zone->type = pkt.type;
            zone->shape = pkt.shape;
            zone->minAltitude = pkt.minAltitude;
            zone->maxAltitude = pkt.maxAltitude;
            zone->isSealevelRef = pkt.isSeaLevelRef;
            zone->fenceAction = pkt.fenceAction;
            zone->vertexCount = pkt.vertexCount;
        }
        break;

    case MSP2_INAV_SET_GEOZONE_VERTEX:
        {
            /* A circular zone carries a fourth field, the radius, stored as the
             * 'latitude' of the following vertex. */
            msp2InavSetGeozoneVertex_circleRequest_t pkt;
            if (!mspReadRequestBytes(src, &pkt, sizeof(msp2InavSetGeozoneVertex_polygonRequest_t), dataSize)
                || pkt.geozoneIndex >= MAX_GEOZONES_IN_CONFIG) {
                return MSP_RESULT_ERROR;
            }
            if (!geozoneSetVertex(pkt.geozoneIndex, pkt.vertexId, pkt.latitude, pkt.longitude)) {
                return MSP_RESULT_ERROR;
            }

            if (geoZonesConfig(pkt.geozoneIndex)->shape == GEOZONE_SHAPE_CIRCULAR) {
                if (!mspReadOptional(src, &pkt.radius)
                    || !geozoneSetVertex(pkt.geozoneIndex, pkt.vertexId + 1, pkt.radius, 0)) {
                    return MSP_RESULT_ERROR;
                }
            }
        }
        break;
#endif

#ifdef USE_EZ_TUNE

    case MSP2_INAV_EZ_TUNE_SET:
        {
            /* snappiness is an appended optional byte. */
            msp2InavEzTuneSetRequest_t pkt;
            if (!mspReadRequestBytes(src, &pkt, offsetof(msp2InavEzTuneSetRequest_t, snappiness), dataSize)) {
                return MSP_RESULT_ERROR;
            }
            ezTuneMutable()->enabled = pkt.enabled;
            ezTuneMutable()->filterHz = pkt.filterHz;
            ezTuneMutable()->axisRatio = pkt.axisRatio;
            ezTuneMutable()->response = pkt.response;
            ezTuneMutable()->damping = pkt.damping;
            ezTuneMutable()->stability = pkt.stability;
            ezTuneMutable()->aggressiveness = pkt.aggressiveness;
            ezTuneMutable()->rate = pkt.rate;
            ezTuneMutable()->expo = pkt.expo;

            if (mspReadOptional(src, &pkt.snappiness)) {
                ezTuneMutable()->snappiness = pkt.snappiness;
            }
            ezTuneUpdate();
        }
        break;

#endif

#ifdef USE_RATE_DYNAMICS

    case MSP2_INAV_SET_RATE_DYNAMICS:
        {
            msp2InavSetRateDynamicsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)) {
                return MSP_RESULT_ERROR;
            }
            controlConfig_t *profile = (controlConfig_t *)currentControlProfile;
            profile->rateDynamics.sensitivityCenter = pkt.sensitivityCenter;
            profile->rateDynamics.sensitivityEnd = pkt.sensitivityEnd;
            profile->rateDynamics.correctionCenter = pkt.correctionCenter;
            profile->rateDynamics.correctionEnd = pkt.correctionEnd;
            profile->rateDynamics.weightCenter = pkt.weightCenter;
            profile->rateDynamics.weightEnd = pkt.weightEnd;
        }
        break;

#endif
#ifdef USE_PROGRAMMING_FRAMEWORK
    case MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS:
        {
            msp2InavSetCustomOsdElementsRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.elementIndex >= MAX_CUSTOM_ELEMENTS) {
                return MSP_RESULT_ERROR;
            }
            for (int i = 0; i < CUSTOM_ELEMENTS_PARTS; i++) {
                if (pkt.items[i].partType >= CUSTOM_ELEMENT_TYPE_END) {
                    return MSP_RESULT_ERROR;
                }
            }
            osdCustomElement_t *element = osdCustomElementsMutable(pkt.elementIndex);
            for (int i = 0; i < CUSTOM_ELEMENTS_PARTS; i++) {
                element->part[i].type = pkt.items[i].partType;
                element->part[i].value = pkt.items[i].partValue;
            }
            element->visibility.type = pkt.visibilityType;
            element->visibility.value = pkt.visibilityValue;
            memcpy(element->osdCustomElementText, pkt.elementText, sizeof(pkt.elementText));
            element->osdCustomElementText[OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1] = '\0';
        }
        break;
#endif
    case MSP2_BETAFLIGHT_BIND:
        if (rxConfig()->receiverType == RX_TYPE_SERIAL) {
            switch (rxConfig()->serialrx_provider) {
            default:
                return MSP_RESULT_ERROR;
    #if defined(USE_SERIALRX_SRXL2)
            case SERIALRX_SRXL2:
                srxl2Bind();
                break;
    #endif
    #if defined(USE_SERIALRX_CRSF)
            case SERIALRX_CRSF:
                crsfBind();
                break;
    #endif
            }
        } else {
            return MSP_RESULT_ERROR;
        }
        break;

    case MSP2_INAV_SET_CRUISE_HEADING:
        // Set heading while Cruise / Course Hold is active.
        {
            msp2InavSetCruiseHeadingRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || !navSetCruiseHeading(pkt.heading_centidegrees)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

    case MSP2_INAV_SET_WP_INDEX:
        // Jump to waypoint N during an active WP mission.
        {
            msp2InavSetWpIndexRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || !navSetActiveWaypointIndex(pkt.wp_index)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

    case MSP2_INAV_ACTIVATE_LANDING:
        if (dataSize == 0 && activateForcedLanding()) {
            break;
        }
        return MSP_RESULT_ERROR;

    case MSP2_INAV_ACTIVATE_RTH:
        if (dataSize == 0 && activateRTHMode()) {
            break;
        }
        return MSP_RESULT_ERROR;

    case MSP2_INAV_ARM_DISARM:
        {
            msp2InavArmDisarmRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.arm > 1 || !fcSetArmState(pkt.arm)) {
                return MSP_RESULT_ERROR;
            }
        }
        break;

    default:
        return MSP_RESULT_ERROR;
    }
    return MSP_RESULT_ACK;
}

static const setting_t *mspReadSetting(sbuf_t *src)
{
    char name[SETTING_MAX_NAME_LENGTH];
    uint16_t id;
    uint8_t c;
    size_t s = 0;
    while (1) {
        if (!sbufReadU8Safe(&c, src)) {
            return NULL;
        }
        name[s++] = c;
        if (c == '\0') {
            if (s == 1) {
                // Payload starts with a zero, setting index
                // as uint16_t follows
                if (sbufReadU16Safe(&id, src)) {
                    return settingGet(id);
                }
                return NULL;
            }
            break;
        }
        if (s == SETTING_MAX_NAME_LENGTH) {
            // Name is too long
            return NULL;
        }
    }
    return settingFind(name);
}

static bool mspSettingCommand(sbuf_t *dst, sbuf_t *src)
{
    const setting_t *setting = mspReadSetting(src);
    if (!setting) {
        return false;
    }

    const void *ptr = settingGetValuePointer(setting);
    size_t size = settingGetValueSize(setting);
    sbufWriteDataSafe(dst, ptr, size);
    return true;
}

static bool mspSetSettingCommand(sbuf_t *dst, sbuf_t *src)
{
    UNUSED(dst);

    const setting_t *setting = mspReadSetting(src);
    if (!setting) {
        return false;
    }

    setting_min_t min = settingGetMin(setting);
    setting_max_t max = settingGetMax(setting);

    void *ptr = settingGetValuePointer(setting);
    switch (SETTING_TYPE(setting)) {
        case VAR_UINT8:
            {
                uint8_t val;
                if (!sbufReadU8Safe(&val, src)) {
                    return false;
                }
                if (val > max) {
                    return false;
                }
                *((uint8_t*)ptr) = val;
            }
            break;
        case VAR_INT8:
            {
                int8_t val;
                if (!sbufReadI8Safe(&val, src)) {
                    return false;
                }
                if (val < min || val > (int8_t)max) {
                    return false;
                }
                *((int8_t*)ptr) = val;
            }
            break;
        case VAR_UINT16:
            {
                uint16_t val;
                if (!sbufReadU16Safe(&val, src)) {
                    return false;
                }
                if (val > max) {
                    return false;
                }
                *((uint16_t*)ptr) = val;
            }
            break;
        case VAR_INT16:
            {
                int16_t val;
                if (!sbufReadI16Safe(&val, src)) {
                    return false;
                }
                if (val < min || val > (int16_t)max) {
                    return false;
                }
                *((int16_t*)ptr) = val;
            }
            break;
        case VAR_UINT32:
            {
                uint32_t val;
                if (!sbufReadU32Safe(&val, src)) {
                    return false;
                }
                if (val > max) {
                    return false;
                }
                *((uint32_t*)ptr) = val;
            }
            break;
        case VAR_FLOAT:
            {
                float val;
                if (!sbufReadDataSafe(src, &val, sizeof(float))) {
                    return false;
                }
                if (val < (float)min || val > (float)max) {
                    return false;
                }
                *((float*)ptr) = val;
            }
            break;
        case VAR_STRING:
            {
                settingSetString(setting, (const char*)sbufPtr(src), sbufBytesRemaining(src));
            }
            break;
    }

    return true;
}

static bool mspSettingInfoCommand(sbuf_t *dst, sbuf_t *src)
{
    const setting_t *setting = mspReadSetting(src);
    if (!setting) {
        return false;
    }

    char name_buf[SETTING_MAX_WORD_LENGTH+1];
    settingGetName(setting, name_buf);
    sbufWriteDataSafe(dst, name_buf, strlen(name_buf) + 1);

    // Parameter Group ID
    sbufWriteU16(dst, settingGetPgn(setting));

    // Type, section and mode
    sbufWriteU8(dst, SETTING_TYPE(setting));
    sbufWriteU8(dst, SETTING_SECTION(setting));
    sbufWriteU8(dst, SETTING_MODE(setting));

    // Min as int32_t
    int32_t min = settingGetMin(setting);
    sbufWriteU32(dst, (uint32_t)min);
    // Max as uint32_t
    uint32_t max = settingGetMax(setting);
    sbufWriteU32(dst, max);

    // Absolute setting index
    sbufWriteU16(dst, settingGetIndex(setting));

    // If the setting is profile based, send the current one
    // and the count, both as uint8_t. For MASTER_VALUE, we
    // send two zeroes, so the MSP client can assume there
    // will always be two bytes.
    switch (SETTING_SECTION(setting)) {
    case MASTER_VALUE:
        sbufWriteU8(dst, 0);
        sbufWriteU8(dst, 0);
        break;
    case EZ_TUNE_VALUE:
        FALLTHROUGH;
    case PROFILE_VALUE:
        FALLTHROUGH;
    case CONTROL_VALUE:
        sbufWriteU8(dst, getConfigProfile());
        sbufWriteU8(dst, MAX_PROFILE_COUNT);
        break;
    case BATTERY_CONFIG_VALUE:
        sbufWriteU8(dst, getConfigBatteryProfile());
        sbufWriteU8(dst, MAX_BATTERY_PROFILE_COUNT);
        break;
    case MIXER_CONFIG_VALUE:
        sbufWriteU8(dst, getConfigMixerProfile());
        sbufWriteU8(dst, MAX_MIXER_PROFILE_COUNT);
        break;
    }

    // If the setting uses a table, send each possible string (null terminated)
    if (SETTING_MODE(setting) == MODE_LOOKUP) {
        for (int ii = (int)min; ii <= (int)max; ii++) {
            const char *name = settingLookupValueName(setting, ii);
            sbufWriteDataSafe(dst, name, strlen(name) + 1);
        }
    }

    // Finally, include the setting value. This way resource constrained callers
    // (e.g. a script in the radio) don't need to perform another call to retrieve
    // the value.
    const void *ptr = settingGetValuePointer(setting);
    size_t size = settingGetValueSize(setting);
    sbufWriteDataSafe(dst, ptr, size);

    return true;
}

static bool mspParameterGroupsCommand(sbuf_t *dst, sbuf_t *src)
{
    uint16_t first;
    uint16_t last;
    uint16_t start;
    uint16_t end;

    if (sbufReadU16Safe(&first, src)) {
        last = first;
    } else {
        first = PG_ID_FIRST;
        last = PG_ID_LAST;
    }

    for (int ii = first; ii <= last; ii++) {
        if (settingsGetParameterGroupIndexes(ii, &start, &end)) {
            msp2CommonPgListReplyElem_t record = { .pgn = ii, .startIndex = start, .endIndex = end };
            sbufWriteData(dst, &record, sizeof(record));
        }
    }
    return true;
}

#ifdef USE_SIMULATOR

bool isOSDTypeSupportedBySimulator(void)
{
#ifdef USE_OSD
    displayPort_t *osdDisplayPort = osdGetDisplayPort();
	return (!!osdDisplayPort && !!osdDisplayPort->vTable->readChar);
#else
    return false;
#endif
}

void mspWriteSimulatorOSD(sbuf_t *dst)
{
	//RLE encoding
	//scan displayBuffer iteratively
	//no more than 80+3+2 bytes output in single run
	//0 and 255 are special symbols
	//255 [char] - font bank switch
	//0 [flags,count] [char] - font bank switch, blink switch and character repeat
    //original 0 is sent as 32
    //original 0xff, 0x100 and 0x1ff are forcibly sent inside command 0

	static uint8_t osdPos_y = 0;
	static uint8_t osdPos_x = 0;

    //indicate new format hitl 1.4.0
	sbufWriteU8(dst, 255);

	if (isOSDTypeSupportedBySimulator())
	{
		displayPort_t *osdDisplayPort = osdGetDisplayPort();

		sbufWriteU8(dst, osdDisplayPort->rows);
		sbufWriteU8(dst, osdDisplayPort->cols);

		sbufWriteU8(dst, osdPos_y);
		sbufWriteU8(dst, osdPos_x);

		int bytesCount = 0;

		uint16_t c = 0;
		textAttributes_t attr = 0;
		bool highBank = false;
		bool blink = false;
		int count = 0;

		int processedRows = osdDisplayPort->rows;

		while (bytesCount < 80) //whole response should be less 155 bytes at worst.
		{
			bool blink1;
			uint16_t lastChar = 0;

			count = 0;
			while ( true )
			{
				displayReadCharWithAttr(osdDisplayPort, osdPos_x, osdPos_y, &c, &attr);
				if (c == 0) c = 32;

				//REVIEW: displayReadCharWithAttr() should return mode with _TEXT_ATTRIBUTES_BLINK_BIT !
				//for max7456 it returns mode with MAX7456_MODE_BLINK instead (wrong)
				//because max7456ReadChar() does not decode from MAX7456_MODE_BLINK to _TEXT_ATTRIBUTES_BLINK_BIT
				//it should!

				//bool blink2 = TEXT_ATTRIBUTES_HAVE_BLINK(attr);
				bool blink2 = attr & (1<<4); //MAX7456_MODE_BLINK

				if (count == 0)
				{
					lastChar = c;
					blink1 = blink2;
				}
				else if ((lastChar != c) || (blink2 != blink1) || (count == 63))
				{
					break;
				}

				count++;

				osdPos_x++;
				if (osdPos_x == osdDisplayPort->cols)
				{
					osdPos_x = 0;
					osdPos_y++;
					processedRows--;
					if (osdPos_y == osdDisplayPort->rows)
					{
						osdPos_y = 0;
					}
				}
			}

			uint8_t cmd = 0;
            uint8_t lastCharLow = (uint8_t)(lastChar & 0xff);
			if (blink1 != blink)
			{
				cmd |= 128;//switch blink attr
				blink = blink1;
			}

			bool highBank1 = lastChar > 255;
			if (highBank1 != highBank)
			{
				cmd |= 64;//switch bank attr
				highBank = highBank1;
			}

			if (count == 1 && cmd == 64)
			{
				sbufWriteU8(dst, 255);  //short command for bank switch with char following
				sbufWriteU8(dst, lastChar & 0xff);
				bytesCount += 2;
			}
			else if ((count > 2) || (cmd !=0) || (lastChar == 255) || (lastChar == 0x100) || (lastChar == 0x1ff))
			{
				cmd |= count;  //long command for blink/bank switch and symbol repeat
				sbufWriteU8(dst, 0);
				sbufWriteU8(dst, cmd);
				sbufWriteU8(dst, lastCharLow);
				bytesCount += 3;
			}
			else if (count == 2)  //cmd == 0 here
			{
				sbufWriteU8(dst, lastCharLow);
				sbufWriteU8(dst, lastCharLow);
				bytesCount+=2;
			}
			else
			{
				sbufWriteU8(dst, lastCharLow);
				bytesCount++;
			}

			if ( processedRows <= 0 )
			{
				break;
			}
		}
		sbufWriteU8(dst, 0);  //command 0 with length=0 -> stop
		sbufWriteU8(dst, 0);
	}
	else
	{
		sbufWriteU8(dst, 0);
	}
}

static void readMspSimulatorValues(sbuf_t *src, const int dataSize, const uint8_t simMspVersion)
{
    if (!ARMING_FLAG(SIMULATOR_MODE_HITL)) { // Just once
#ifdef USE_BARO
        if ( requestedSensors[SENSOR_INDEX_BARO] != BARO_NONE ) {
            sensorsSet(SENSOR_BARO);
            setTaskEnabled(TASK_BARO, true);
            DISABLE_ARMING_FLAG(ARMING_DISABLED_HARDWARE_FAILURE);
            baroStartCalibration();
        }
#endif

#ifdef USE_MAG
        if (compassConfig()->mag_hardware != MAG_NONE) {
            sensorsSet(SENSOR_MAG);
            ENABLE_STATE(COMPASS_CALIBRATED);
            DISABLE_ARMING_FLAG(ARMING_DISABLED_HARDWARE_FAILURE);
            mag.magADC[X] = 800;
            mag.magADC[Y] = 0;
            mag.magADC[Z] = 0;
        }
#endif
        ENABLE_ARMING_FLAG(SIMULATOR_MODE_HITL);
        ENABLE_STATE(ACCELEROMETER_CALIBRATED);
        LOG_DEBUG(SYSTEM, "Simulator enabled");
    }

    const int minSensorBytes = (simMspVersion == SIMULATOR_MSP_VERSION_3) ? 11 : 12;
    if (dataSize < minSensorBytes) {
        DISABLE_STATE(GPS_FIX);
        return;
    }

    if (feature(FEATURE_GPS) && SIMULATOR_HAS_OPTION(HITL_HAS_NEW_GPS_DATA)) {
        gpsSolDRV.fixType = sbufReadU8(src);
        gpsSolDRV.hdop = gpsSolDRV.fixType == GPS_NO_FIX ? 9999 : 100;
        gpsSolDRV.numSat = sbufReadU8(src);

        if (gpsSolDRV.fixType != GPS_NO_FIX) {
            gpsSolDRV.flags.validVelNE = true;
            gpsSolDRV.flags.validVelD = true;
            gpsSolDRV.flags.validEPE = true;
            gpsSolDRV.flags.validTime = false;

            gpsSolDRV.llh.lat = sbufReadU32(src);
            gpsSolDRV.llh.lon = sbufReadU32(src);
            gpsSolDRV.llh.alt = sbufReadU32(src);
            gpsSolDRV.groundSpeed = (int16_t)sbufReadU16(src);
            gpsSolDRV.groundCourse = (int16_t)sbufReadU16(src);

            gpsSolDRV.velNED[X] = (int16_t)sbufReadU16(src);
            gpsSolDRV.velNED[Y] = (int16_t)sbufReadU16(src);
            gpsSolDRV.velNED[Z] = (int16_t)sbufReadU16(src);

            gpsSolDRV.eph = 100;
            gpsSolDRV.epv = 100;
        } else {
            sbufAdvance(src, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) * 3);
        }
        // Feed data to navigation
        gpsProcessNewDriverData();
        gpsProcessNewSolutionData(false);
    } else {
        sbufAdvance(src, sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) * 3);
    }

    if (!SIMULATOR_HAS_OPTION(HITL_USE_IMU)) {
        attitude.values.roll = (int16_t)sbufReadU16(src);
        attitude.values.pitch = (int16_t)sbufReadU16(src);
        attitude.values.yaw = (int16_t)sbufReadU16(src);
    } else {
        sbufAdvance(src, sizeof(uint16_t) * XYZ_AXIS_COUNT);
    }

    // Get the acceleration in 1G units
    acc.accADCf[X] = ((int16_t)sbufReadU16(src)) / 1000.0f;
    acc.accADCf[Y] = ((int16_t)sbufReadU16(src)) / 1000.0f;
    acc.accADCf[Z] = ((int16_t)sbufReadU16(src)) / 1000.0f;
    acc.accVibeSq[X] = 0.0f;
    acc.accVibeSq[Y] = 0.0f;
    acc.accVibeSq[Z] = 0.0f;

    // Get the angular velocity in DPS
    gyro.gyroADCf[X] = ((int16_t)sbufReadU16(src)) / 16.0f;
    gyro.gyroADCf[Y] = ((int16_t)sbufReadU16(src)) / 16.0f;
    gyro.gyroADCf[Z] = ((int16_t)sbufReadU16(src)) / 16.0f;

    if (sensors(SENSOR_BARO)) {
        baro.baroPressure = (int32_t)sbufReadU32(src);
        baro.baroTemperature = DEGREES_TO_CENTIDEGREES(SIMULATOR_BARO_TEMP);
    } else {
        sbufAdvance(src, sizeof(uint32_t));
    }

    if (sensors(SENSOR_MAG)) {
        mag.magADC[X] = ((int16_t)sbufReadU16(src)) / 20;  // 16000 / 20 = 800uT
        mag.magADC[Y] = ((int16_t)sbufReadU16(src)) / 20;   //note that mag failure is simulated by setting all readings to zero
        mag.magADC[Z] = ((int16_t)sbufReadU16(src)) / 20;
    } else {
        sbufAdvance(src, sizeof(uint16_t) * XYZ_AXIS_COUNT);
    }

    if (SIMULATOR_HAS_OPTION(HITL_EXT_BATTERY_VOLTAGE)) {
        simulatorData.vbat = sbufReadU8(src);
    } else {
        simulatorData.vbat = SIMULATOR_FULL_BATTERY;
    }

    if (SIMULATOR_HAS_OPTION(HITL_AIRSPEED)) {
        simulatorData.airSpeed = sbufReadU16(src);
    } else if (SIMULATOR_HAS_OPTION(HITL_EXTENDED_FLAGS)) {
        sbufReadU16(src);
    }

    if (simMspVersion == SIMULATOR_MSP_VERSION_3) {

        if (SIMULATOR_HAS_OPTION(HITL_RANGEFINDER)) {
            simulatorData.rangefinder = sbufReadU16(src);
            if (simulatorData.rangefinder == 0xFFFF) {
                fakeRangefindersSetData(-1);
            } else {
                fakeRangefindersSetData(simulatorData.rangefinder);
            }

        } else {
            sbufReadU16(src);
        }

        if (SIMULATOR_HAS_OPTION(HITL_CURRENT_SENSOR)) {
            simulatorData.current = sbufReadU16(src);
        } else {
            sbufReadU16(src);
        }

        if (SIMULATOR_HAS_OPTION(HITL_SIM_RC_INPUT)) {
            for (int i = 0; i < HITL_SIM_MAX_RC_INPUTS; i++) {
                simulatorData.rcInput[i] = sbufReadU16(src);
            }
            rxSimSetChannelValue(simulatorData.rcInput, HITL_SIM_MAX_RC_INPUTS);
            simulatorData.rssi = sbufReadU16(src);
            rxSimSetRssi(simulatorData.rssi);
        } else {
            sbufAdvance(src, sizeof(uint16_t) * HITL_SIM_MAX_RC_INPUTS + sizeof(uint16_t)); // + RSSI
        }

        rxSimSetFailsafe(SIMULATOR_HAS_OPTION(HITL_FAILSAFE_TRIGGERED));
    }

    // Backward compatibility for HITL Plugin 1.X
    if (simMspVersion == SIMULATOR_MSP_VERSION_2 && SIMULATOR_HAS_OPTION(HITL_EXTENDED_FLAGS)) {
        simulatorData.flags |= ((uint16_t)sbufReadU8(src)) << 8;
    }
}

static mspResult_e mspProcessSimulatorCommand(sbuf_t *dst, sbuf_t *src, const int dataSize)
{
    if (dataSize < 2) {
        return MSP_RESULT_ERROR;
    }

    const uint8_t simMspVersion = sbufReadU8(src); // Get the Simulator MSP version
    if (simMspVersion != SIMULATOR_MSP_VERSION_2 && simMspVersion != SIMULATOR_MSP_VERSION_3) {
        return MSP_RESULT_ERROR;
    }

    if (simMspVersion == SIMULATOR_MSP_VERSION_3) {
        if (dataSize < 3) {
            return MSP_RESULT_ERROR;
        }
        simulatorData.flags = sbufReadU16(src);
    } else {
        simulatorData.flags = sbufReadU8(src);
    }

    const int remainingPayload = (int)sbufBytesRemaining(src);

    if (!SIMULATOR_HAS_OPTION(HITL_SITL_MODE)) {
        if (!SIMULATOR_HAS_OPTION(HITL_ENABLE)) {
            if (simulatorData.flags) {
                // Non-zero flags but HITL_ENABLE cleared — reboot to clean state
                fcReboot(false);
                return MSP_RESULT_NO_REPLY;
            }
            // flags == 0: clean stop signal — disable HITL and disarm
            if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
                DISABLE_ARMING_FLAG(SIMULATOR_MODE_HITL);
                simulatorData.flags = HITL_RESET_FLAGS;
                disarm(DISARM_SWITCH);
            }
        } else {
            readMspSimulatorValues(src, remainingPayload, simMspVersion);
        }
    }

    sbufWriteU16(dst, (uint16_t)simulatorData.input[INPUT_STABILIZED_ROLL]);
    sbufWriteU16(dst, (uint16_t)simulatorData.input[INPUT_STABILIZED_PITCH]);
    sbufWriteU16(dst, (uint16_t)simulatorData.input[INPUT_STABILIZED_YAW]);
    sbufWriteU16(dst, (uint16_t)(ARMING_FLAG(ARMED) ? simulatorData.input[INPUT_STABILIZED_THROTTLE] : -500));

    simulatorData.debugIndex++;
    if (simulatorData.debugIndex == 8) {
        simulatorData.debugIndex = 0;
    }

    const uint8_t debugIndex = simulatorData.debugIndex |
        ((mixerConfig()->platformType == PLATFORM_AIRPLANE) ? 128 : 0) |
        (ARMING_FLAG(ARMED) ? 64 : 0) |
        (!feature(FEATURE_OSD) ? 32: 0) |
        (!isOSDTypeSupportedBySimulator() ? 16 : 0);

    sbufWriteU8(dst, debugIndex);
    sbufWriteU32(dst, debug[simulatorData.debugIndex]);

    sbufWriteU16(dst, attitude.values.roll);
    sbufWriteU16(dst, attitude.values.pitch);
    sbufWriteU16(dst, attitude.values.yaw);

    mspWriteSimulatorOSD(dst);

    return MSP_RESULT_ACK;
}

#endif


bool mspFCProcessInOutCommand(uint16_t cmdMSP, sbuf_t *dst, sbuf_t *src, mspResult_e *ret)
{
    const unsigned int dataSize = sbufBytesRemaining(src);

    switch (cmdMSP) {

    case MSP_WP:
        mspFcWaypointOutCommand(dst, src);
        *ret = MSP_RESULT_ACK;
        break;

#ifdef USE_DRONECAN
    case MSP2_INAV_DRONECAN_ASYNC_REQUEST:
        mspHandleDronecanAsyncRequest(src, dst, ret);
        break;

    case MSP2_INAV_DRONECAN_ASYNC_RESULT:
        mspSerializeDronecanAsyncResult(dst);
        *ret = MSP_RESULT_ACK;
        break;
#endif

#ifdef USE_ADSB
    case MSP2_ADSB_VEHICLE:
        if (sbufBytesRemaining(src) >= 1) {
            adsbVehicle_t *vehicle = findVehicle(sbufReadU8(src));
            if (vehicle == NULL) {                 // index past MAX_ADSB_VEHICLES
                *ret = MSP_RESULT_ERROR;
                break;
            }
            msp2AdsbVehicleReply_t reply = {
                .icao = vehicle->vehicleValues.icao,
                .lat = vehicle->vehicleValues.gps.lat,
                .lon = vehicle->vehicleValues.gps.lon,
                .alt = vehicle->vehicleValues.alt,
                .heading = vehicle->vehicleValues.heading,          // centideg, full-res
                .horVelocity = vehicle->vehicleValues.horVelocity,  // cm/s - omitted by the bulk list
                .tslc = vehicle->vehicleValues.tslc,
                .emitterType = vehicle->vehicleValues.emitterType,
                .ttl = vehicle->ttl,
            };
            memcpy(reply.callsign, vehicle->vehicleValues.callsign, sizeof(reply.callsign));
            mspWriteReply(dst, &reply);
        } else {
            *ret = MSP_RESULT_ERROR;               // no index supplied
            break;
        }
        *ret = MSP_RESULT_ACK;
        break;
#endif

#if defined(USE_FLASHFS)
    case MSP_DATAFLASH_READ:
        mspFcDataFlashReadCommand(dst, src);
        *ret = MSP_RESULT_ACK;
        break;
#endif

    case MSP2_COMMON_SETTING:
        *ret = mspSettingCommand(dst, src) ? MSP_RESULT_ACK : MSP_RESULT_ERROR;
        break;

    case MSP2_COMMON_SET_SETTING:
        *ret = mspSetSettingCommand(dst, src) ? MSP_RESULT_ACK : MSP_RESULT_ERROR;
        break;

    case MSP2_COMMON_SETTING_INFO:
        *ret = mspSettingInfoCommand(dst, src) ? MSP_RESULT_ACK : MSP_RESULT_ERROR;
        break;

    case MSP2_COMMON_PG_LIST:
        *ret = mspParameterGroupsCommand(dst, src) ? MSP_RESULT_ACK : MSP_RESULT_ERROR;
        break;

#if defined(USE_OSD)
    case MSP2_INAV_OSD_LAYOUTS:
        if (sbufBytesRemaining(src) >= 1) {
            uint8_t layout = sbufReadU8(src);
            if (layout >= OSD_LAYOUT_COUNT) {
                *ret = MSP_RESULT_ERROR;
                break;
            }
            if (sbufBytesRemaining(src) >= 2) {
                // Asking for an specific item in a layout
                uint16_t item = sbufReadU16(src);
                if (item >= OSD_ITEM_COUNT) {
                    *ret = MSP_RESULT_ERROR;
                    break;
                }
                sbufWriteU16(dst, osdLayoutsConfig()->item_pos[layout][item]);
            } else {
                // Asking for an specific layout
                for (unsigned ii = 0; ii < OSD_ITEM_COUNT; ii++) {
                    sbufWriteU16(dst, osdLayoutsConfig()->item_pos[layout][ii]);
                }
            }
        } else {
            // Return the number of layouts and items
            sbufWriteU8(dst, OSD_LAYOUT_COUNT);
            sbufWriteU8(dst, OSD_ITEM_COUNT);
        }
        *ret = MSP_RESULT_ACK;
        break;
#endif

#ifdef USE_PROGRAMMING_FRAMEWORK
    case MSP2_INAV_LOGIC_CONDITIONS_SINGLE:
        *ret = mspFcLogicConditionCommand(dst, src);
        break;
    case MSP2_INAV_CUSTOM_OSD_ELEMENT:
        {
            const uint8_t idx = sbufReadU8(src);

            if (idx < MAX_CUSTOM_ELEMENTS) {
                const osdCustomElement_t *customElement = osdCustomElements(idx);
                msp2InavCustomOsdElementReply_t reply = {
                    .visibilityType = customElement->visibility.type,
                    .visibilityValue = customElement->visibility.value,
                };
                for (int ii = 0; ii < CUSTOM_ELEMENTS_PARTS; ii++) {
                    reply.items[ii].partType = customElement->part[ii].type;
                    reply.items[ii].partValue = customElement->part[ii].value;
                }
                memcpy(reply.elementText, customElement->osdCustomElementText, sizeof(reply.elementText));
                mspWriteReply(dst, &reply);
            }
        }
        break;
#endif
#ifdef USE_SAFE_HOME
    case MSP2_INAV_SAFEHOME:
        *ret = mspFcSafeHomeOutCommand(dst, src);
        break;
#endif
#ifdef USE_FW_AUTOLAND
    case MSP2_INAV_FW_APPROACH:
        *ret = mspFwApproachOutCommand(dst, src);
        break;
#endif
#ifdef USE_GEOZONE
    case MSP2_INAV_GEOZONE:
        *ret = mspFcGeozoneOutCommand(dst, src);
        break;
    case MSP2_INAV_GEOZONE_VERTEX:
        *ret = mspFcGeozoneVerteciesOutCommand(dst, src);
        break;
#endif

#if defined(USE_BARO) || defined(USE_GPS)
    case MSP2_INAV_SET_ALT_TARGET:
        {
            msp2InavSetAltTargetRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize)
                || !navigationSetAltitudeTargetWithDatum(pkt.altitudeDatum, pkt.altitudeTarget)) {
                *ret = MSP_RESULT_ERROR;
                break;
            }
            *ret = MSP_RESULT_ACK;
        }
        break;
#endif

    case MSP2_INAV_SET_LOCAL_TARGET:
        {
            msp2InavSetLocalTargetRequest_t pkt;
            if (!isGCSValid() || !mspReadRequest(src, &pkt, dataSize)) {
                *ret = MSP_RESULT_ERROR;
                break;
            }

            fpVector3_t targetPos = posControl.desiredState.pos;

            const navEstimatedPosVel_t *actual = navGetCurrentActualPositionAndVelocity();
            const float offsetBodyX = pkt.offsetForward;
            const float offsetBodyY = pkt.offsetRight;
            const float offsetBodyZ = pkt.offsetUp;

            const float cosYaw = posControl.actualState.cosYaw;
            const float sinYaw = posControl.actualState.sinYaw;

            // Rotate body-frame offsets into NEU and apply relative to current position
            const float offsetN = offsetBodyX * cosYaw - offsetBodyY * sinYaw;
            const float offsetE = offsetBodyX * sinYaw + offsetBodyY * cosYaw;

            targetPos.x = actual->pos.x + offsetN;
            targetPos.y = actual->pos.y + offsetE;

            navSetWaypointFlags_t updateMask = NAV_POS_UPDATE_XY;
            if (offsetBodyZ != 0.0f) {
                targetPos.z = actual->pos.z + offsetBodyZ;
            }
            updateMask |= NAV_POS_UPDATE_Z;

            setDesiredPosition(&targetPos, posControl.desiredState.yaw, updateMask);
            *ret = MSP_RESULT_ACK;
        }
        break;

    case MSP2_INAV_LOCAL_TARGET:
        {
            msp2InavLocalTargetReply_t reply = {
                .posX = lrintf(posControl.desiredState.pos.x),
                .posY = lrintf(posControl.desiredState.pos.y),
                .posZ = lrintf(posControl.desiredState.pos.z),
                .velX = lrintf(posControl.desiredState.vel.x),
                .velY = lrintf(posControl.desiredState.vel.y),
                .velZ = lrintf(posControl.desiredState.vel.z),
                .yaw = posControl.desiredState.yaw,
                .climbRate = posControl.desiredState.climbRateDemand,
            };
            mspWriteReply(dst, &reply);
            *ret = MSP_RESULT_ACK;
        }
        break;

    case MSP2_INAV_SET_GLOBAL_TARGET:
        {
            /* loiterRadius is an appended optional field. */
            msp2InavSetGlobalTargetRequest_t pkt;
            if (!isGCSValid()
                || !mspReadRequestBytes(src, &pkt, offsetof(msp2InavSetGlobalTargetRequest_t, loiterRadius), dataSize)) {
                *ret = MSP_RESULT_ERROR;
                break;
            }

            gpsLocation_t targetLlh = {
                .lat = pkt.latitude,
                .lon = pkt.longitude,
                .alt = pkt.altitudeTarget,
            };

            const geoAltitudeDatumFlag_e datumFlag = (geoAltitudeDatumFlag_e)pkt.altitudeDatum;
            const bool hasLoiterRadius = mspReadOptional(src, &pkt.loiterRadius);
            const int32_t loiterRadius = hasLoiterRadius ? pkt.loiterRadius : 0;

            if (datumFlag == NAV_WP_TERRAIN_DATUM || loiterRadius < 0) {
                *ret = MSP_RESULT_ERROR;
                break;
            }

            fpVector3_t targetPos;
            if (!geoConvertGeodeticToLocal(&targetPos, &posControl.gpsOrigin, &targetLlh, waypointMissionAltConvMode(datumFlag))) {
                *ret = MSP_RESULT_ERROR;
                break;
            }

            navSetWaypointFlags_t updateMask = NAV_POS_UPDATE_XY;

            if (targetLlh.alt != 0) {
                updateMask |= NAV_POS_UPDATE_Z;
            }

            setDesiredPosition(&targetPos, posControl.desiredState.yaw, updateMask);
            if (hasLoiterRadius) {
                navigationSetLoiterRadiusOverride((uint32_t)loiterRadius);
            }
            *ret = MSP_RESULT_ACK;
        }
        break;

    case MSP2_INAV_NAV_TARGET:
        if (!posControl.gpsOrigin.valid) {
            *ret = MSP_RESULT_ERROR;
            break;
        }

        {
            gpsLocation_t targetLlh;
            geoConvertLocalToGeodetic(&targetLlh, &posControl.gpsOrigin, &posControl.desiredState.pos);

            msp2InavNavTargetReply_t reply = {
                .latTarget = targetLlh.lat,
                .lonTarget = targetLlh.lon,
                .altitudeTarget = lrintf(posControl.desiredState.pos.z),
                .headingTarget = CENTIDEGREES_TO_DEGREES(wrap_36000(DEGREES_TO_CENTIDEGREES(getHeadingHoldTarget()))),
                .climbRate = posControl.desiredState.climbRateDemand,
                .loiterRadius = navigationGetLoiterRadiusOverride(),
            };
            mspWriteReply(dst, &reply);
            *ret = MSP_RESULT_ACK;
        }
        break;

#ifdef USE_SIMULATOR
    case MSP_SIMULATOR:
        *ret = mspProcessSimulatorCommand(dst, src, dataSize);
        break;
#endif
#ifndef SITL_BUILD
    case MSP2_INAV_TIMER_OUTPUT_MODE:
        /* No request byte means "all timers"; one byte selects a single timer. */
        if (dataSize == 0) {
            msp2InavTimerOutputMode_dataSize_eq_0Reply_t reply;
            for (int i = 0; i < HARDWARE_TIMER_DEFINITION_COUNT; ++i) {
                reply.items[i].timerIndex = i;
                reply.items[i].outputMode = timerOverrides(i)->outputMode;
            }
            mspWriteReply(dst, &reply);
            *ret = MSP_RESULT_ACK;
        } else if (dataSize == 1) {
            msp2InavTimerOutputMode_dataSize_eq_1Request_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
                *ret = MSP_RESULT_ERROR;
                break;
            }
            msp2InavTimerOutputMode_dataSize_eq_1Reply_t reply = {
                .timerIndex = pkt.timerIndex,
                .outputMode = timerOverrides(pkt.timerIndex)->outputMode,
            };
            mspWriteReply(dst, &reply);
            *ret = MSP_RESULT_ACK;
        } else {
            *ret = MSP_RESULT_ERROR;
        }
        break;

    case MSP2_INAV_SET_TIMER_OUTPUT_MODE:
        {
            msp2InavSetTimerOutputModeRequest_t pkt;
            if (!mspReadRequest(src, &pkt, dataSize) || pkt.timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
                *ret = MSP_RESULT_ERROR;
                break;
            }
            timerOverridesMutable(pkt.timerIndex)->outputMode = pkt.outputMode;
            *ret = MSP_RESULT_ACK;
        }
        break;

    case MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT:
        {
            // Build proposed overrides array (defaults to current stored overrides)
            uint8_t proposedModes[HARDWARE_TIMER_DEFINITION_COUNT];
            for (int i = 0; i < HARDWARE_TIMER_DEFINITION_COUNT; i++) {
                proposedModes[i] = timerOverrides(i)->outputMode;
            }

            if (dataSize >= 1) {
                msp2InavQueryOutputAssignment_dataSize_ge_1Request_t header;
                if (!mspReadRequest(src, &header, dataSize)) {
                    *ret = MSP_RESULT_ERROR;
                    break;
                }
                // Reject malformed payloads: must be exactly timerCount pairs.
                if (header.timerCount > HARDWARE_TIMER_DEFINITION_COUNT ||
                    sbufBytesRemaining(src) != (int)(header.timerCount * sizeof(header.items[0]))) {
                    *ret = MSP_RESULT_ERROR;
                    break;
                }
                for (int i = 0; i < header.timerCount; i++) {
                    __typeof__(header.items[0]) record;
                    if (!mspReadRequest(src, &record, sbufBytesRemaining(src))) {
                        *ret = MSP_RESULT_ERROR;
                        break;
                    }
                    if (record.timerId < HARDWARE_TIMER_DEFINITION_COUNT) {
                        proposedModes[record.timerId] = record.outputMode;
                    }
                }
            }

            timMotorServoHardware_t tempOut = {0};
            pwmCalculateAssignment(&tempOut, proposedModes);

            msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t record;
            for (int m = 0; m < tempOut.maxTimMotorCount; m++) {
                record = (msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t){
                    .outputIndex = tempOut.timMotors[m] - timerHardware,
                    .usageType = __builtin_ctz(TIM_USE_MOTOR),
                    .functionIndex = m + 1,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
            for (int s = 0; s < tempOut.maxTimServoCount; s++) {
                record = (msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t){
                    .outputIndex = tempOut.timServos[s] - timerHardware,
                    .usageType = __builtin_ctz(TIM_USE_SERVO),
                    .functionIndex = s + 1,
                };
                sbufWriteData(dst, &record, sizeof(record));
            }
            for (int idx = 0; idx < timerHardwareCount; idx++) {
                if (proposedModes[timer2id(timerHardware[idx].tim)] == OUTPUT_MODE_BEEPER) {
                    record = (msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t){
                        .outputIndex = idx,
                        .usageType = __builtin_ctz(TIM_USE_BEEPER),
                        .functionIndex = 1,
                    };
                    sbufWriteData(dst, &record, sizeof(record));
                    break;
                }
            }
            *ret = MSP_RESULT_ACK;
        }
        break;
#endif

    case MSP_VTXTABLE_POWERLEVEL: {
        vtxDevice_t *vtxDevice = vtxCommonDevice();
        if (!vtxDevice) {
            return MSP_RESULT_ERROR;
        }

        const uint8_t powerLevel = sbufBytesRemaining(src) ? sbufReadU8(src) : 0;
        if (powerLevel == 0 || powerLevel > vtxDevice->capability.powerCount) {
            return MSP_RESULT_ERROR;
        }

        sbufWriteU8(dst, powerLevel);
        sbufWriteU16(dst, 0);

        const char *str = vtxDevice->capability.powerNames[powerLevel - 1];
        const uint32_t str_len = strnlen(str, 5);  // these _should_ all be null-terminated
        sbufWriteU8(dst, str_len);
        for (uint32_t i = 0; i < str_len; i++)
            sbufWriteU8(dst, str[i]);

    } break;

    default:
        // Not handled
        return false;
    }
    return true;
}

static mspResult_e mspProcessSensorCommand(uint16_t cmdMSP, sbuf_t *src)
{
    int dataSize = sbufBytesRemaining(src);
    UNUSED(dataSize);

    switch (cmdMSP) {
#if defined(USE_RANGEFINDER_MSP)
        case MSP2_SENSOR_RANGEFINDER:
            mspRangefinderReceiveNewData(sbufPtr(src));
            break;
#endif

#if defined(USE_OPFLOW_MSP)
        case MSP2_SENSOR_OPTIC_FLOW:
            mspOpflowReceiveNewData(sbufPtr(src));
            break;
#endif

#if defined(USE_GPS_PROTO_MSP)
        case MSP2_SENSOR_GPS:
            mspGPSReceiveNewData(sbufPtr(src));
            break;
#endif

#if defined(USE_MAG_MSP)
        case MSP2_SENSOR_COMPASS:
            mspMagReceiveNewData(sbufPtr(src));
            break;
#endif

#if defined(USE_BARO_MSP)
        case MSP2_SENSOR_BAROMETER:
            mspBaroReceiveNewData(sbufPtr(src));
            break;
#endif

#if defined(USE_PITOT_MSP)
        case MSP2_SENSOR_AIRSPEED:
            mspPitotmeterReceiveNewData(sbufPtr(src));
            break;
#endif

#if (defined(USE_HEADTRACKER) && defined(USE_HEADTRACKER_MSP))
        case MSP2_SENSOR_HEADTRACKER:
            mspHeadTrackerReceiverNewData(sbufPtr(src), dataSize);
            break;
#endif
    }

    return MSP_RESULT_NO_REPLY;
}

/*
 * Returns MSP_RESULT_ACK, MSP_RESULT_ERROR or MSP_RESULT_NO_REPLY
 */
mspResult_e mspFcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    mspResult_e ret = MSP_RESULT_ACK;
    sbuf_t *dst = &reply->buf;
    sbuf_t *src = &cmd->buf;
    const uint16_t cmdMSP = cmd->cmd;

    // initialize reply by default
    reply->cmd = cmd->cmd;

    if (MSP2_IS_SENSOR_MESSAGE(cmdMSP)) {
        ret = mspProcessSensorCommand(cmdMSP, src);
    } else if (mspFcProcessOutCommand(cmdMSP, dst, mspPostProcessFn)) {
        ret = MSP_RESULT_ACK;
    } else if (cmdMSP == MSP_SET_PASSTHROUGH) {
        mspFcSetPassthroughCommand(dst, src, mspPostProcessFn);
        ret = MSP_RESULT_ACK;
    } else if (cmdMSP == MSP_REBOOT) {
        if (!ARMING_FLAG(ARMED)) {
            ret = mspFcRebootCommand(src, mspPostProcessFn);
        } else {
            ret = MSP_RESULT_ERROR;
        }
    } else {
        if (!mspFCProcessInOutCommand(cmdMSP, dst, src, &ret)) {
            ret = mspFcProcessInCommand(cmdMSP, src);
        }
    }

    // Process DONT_REPLY flag
    if (cmd->flags & MSP_FLAG_DONT_REPLY) {
        ret = MSP_RESULT_NO_REPLY;
    }
    reply->flags = cmd->flags;
    reply->result = ret;
    return ret;
}

/*
 * Return a pointer to the process command function
 */
void mspFcInit(void)
{
    initActiveBoxIds();
}
