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

/*
 * telemetry_mavlink.c
 *
 * Author: Konstantin Sharlaimov
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/string_light.h"

#include "config/feature.h"

#include "drivers/serial.h"
#include "drivers/time.h"
#include "drivers/display.h"
#include "drivers/osd_symbols.h"
#include "drivers/gimbal_common.h"
#include "drivers/headtracker_common.h"

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer_profile.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "io/adsb.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/serial.h"
#include "io/osd.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"
#include "rx/mavlink.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"
#include "sensors/diagnostics.h"
#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/esc_sensor.h"

#include "telemetry/mavlink.h"
#include "telemetry/telemetry.h"

#include "blackbox/blackbox_io.h"

#include "scheduler/scheduler.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#define MAVLINK_COMM_NUM_BUFFERS 1
#include "common/mavlink.h"
#pragma GCC diagnostic pop

#define TELEMETRY_MAVLINK_PORT_MODE     MODE_RXTX
#define TELEMETRY_MAVLINK_MAXRATE       50
#define TELEMETRY_MAVLINK_DELAY         ((1000 * 1000) / TELEMETRY_MAVLINK_MAXRATE)

/**
 * MAVLink requires angles to be in the range -Pi..Pi.
 * This converts angles from a range of 0..Pi to -Pi..Pi
 */
#define RADIANS_TO_MAVLINK_RANGE(angle) (angle > M_PIf) ? angle - (2 * M_PIf) : angle

/** @brief A mapping of plane flight modes for custom_mode field of heartbeat. */
typedef enum APM_PLANE_MODE
{
   PLANE_MODE_MANUAL=0,
   PLANE_MODE_CIRCLE=1,
   PLANE_MODE_STABILIZE=2,
   PLANE_MODE_TRAINING=3,
   PLANE_MODE_ACRO=4,
   PLANE_MODE_FLY_BY_WIRE_A=5,
   PLANE_MODE_FLY_BY_WIRE_B=6,
   PLANE_MODE_CRUISE=7,
   PLANE_MODE_AUTOTUNE=8,
   PLANE_MODE_AUTO=10,
   PLANE_MODE_RTL=11,
   PLANE_MODE_LOITER=12,
   PLANE_MODE_TAKEOFF=13,
   PLANE_MODE_AVOID_ADSB=14,
   PLANE_MODE_GUIDED=15,
   PLANE_MODE_INITIALIZING=16,
   PLANE_MODE_QSTABILIZE=17,
   PLANE_MODE_QHOVER=18,
   PLANE_MODE_QLOITER=19,
   PLANE_MODE_QLAND=20,
   PLANE_MODE_QRTL=21,
   PLANE_MODE_QAUTOTUNE=22,
   PLANE_MODE_ENUM_END=23,
} APM_PLANE_MODE;

/** @brief A mapping of copter flight modes for custom_mode field of heartbeat. */
typedef enum APM_COPTER_MODE
{
   COPTER_MODE_STABILIZE=0,
   COPTER_MODE_ACRO=1,
   COPTER_MODE_ALT_HOLD=2,
   COPTER_MODE_AUTO=3,
   COPTER_MODE_GUIDED=4,
   COPTER_MODE_LOITER=5,
   COPTER_MODE_RTL=6,
   COPTER_MODE_CIRCLE=7,
   COPTER_MODE_LAND=9,
   COPTER_MODE_DRIFT=11,
   COPTER_MODE_SPORT=13,
   COPTER_MODE_FLIP=14,
   COPTER_MODE_AUTOTUNE=15,
   COPTER_MODE_POSHOLD=16,
   COPTER_MODE_BRAKE=17,
   COPTER_MODE_THROW=18,
   COPTER_MODE_AVOID_ADSB=19,
   COPTER_MODE_GUIDED_NOGPS=20,
   COPTER_MODE_SMART_RTL=21,
   COPTER_MODE_ENUM_END=22,
} APM_COPTER_MODE;

static serialPort_t *mavlinkPort = NULL;
static serialPortConfig_t *portConfig;

static bool mavlinkTelemetryEnabled =  false;
static portSharing_e mavlinkPortSharing;
static uint8_t txbuff_free = 100;
static bool txbuff_valid = false;

/* MAVLink datastream rates in Hz */
static uint8_t mavRates[] = {
    [MAV_DATA_STREAM_EXTENDED_STATUS] = 2,      // 2Hz
    [MAV_DATA_STREAM_RC_CHANNELS] = 1,          // 1Hz
    [MAV_DATA_STREAM_POSITION] = 2,             // 2Hz
    [MAV_DATA_STREAM_EXTRA1] = 3,               // 3Hz
    [MAV_DATA_STREAM_EXTRA2] = 2,               // 2Hz, HEARTBEATs are important
    [MAV_DATA_STREAM_EXTRA3] = 1                // 1Hz
};

#define MAXSTREAMS (sizeof(mavRates) / sizeof(mavRates[0]))

static timeUs_t lastMavlinkMessage = 0;
static uint8_t mavTicks[MAXSTREAMS];
static mavlink_message_t mavSendMsg;
static mavlink_message_t mavRecvMsg;
static mavlink_status_t mavRecvStatus;

static uint8_t mavSystemId = 1;
static uint8_t mavComponentId = MAV_COMP_ID_AUTOPILOT1;

static APM_COPTER_MODE inavToArduCopterMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_ACRO:          return COPTER_MODE_ACRO;
        case FLM_ACRO_AIR:      return COPTER_MODE_ACRO;
        case FLM_ANGLE:         return COPTER_MODE_STABILIZE;
        case FLM_HORIZON:       return COPTER_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return COPTER_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return COPTER_MODE_ALT_HOLD;
        case FLM_POSITION_HOLD: return COPTER_MODE_POSHOLD;
        case FLM_RTH:           return COPTER_MODE_RTL;
        case FLM_MISSION:       return COPTER_MODE_AUTO;
        case FLM_LAUNCH:        return COPTER_MODE_THROW;
        case FLM_FAILSAFE:
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return COPTER_MODE_RTL;
                } else if (failsafePhase() == FAILSAFE_LANDING) {
                    return COPTER_MODE_LAND;
                } else {
                    // There is no valid mapping to ArduCopter
                    return COPTER_MODE_ENUM_END;
                }
            }
        default:                return COPTER_MODE_ENUM_END;
    }
}

static APM_PLANE_MODE inavToArduPlaneMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_MANUAL:        return PLANE_MODE_MANUAL;
        case FLM_ACRO:          return PLANE_MODE_ACRO;
        case FLM_ACRO_AIR:      return PLANE_MODE_ACRO;
        case FLM_ANGLE:         return PLANE_MODE_FLY_BY_WIRE_A;
        case FLM_HORIZON:       return PLANE_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return PLANE_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return PLANE_MODE_FLY_BY_WIRE_B;
        case FLM_POSITION_HOLD: return PLANE_MODE_LOITER;
        case FLM_RTH:           return PLANE_MODE_RTL;
        case FLM_MISSION:       return PLANE_MODE_AUTO;
        case FLM_CRUISE:        return PLANE_MODE_CRUISE;
        case FLM_LAUNCH:        return PLANE_MODE_TAKEOFF;
        case FLM_FAILSAFE:
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return PLANE_MODE_RTL;
                }
                else if (failsafePhase() == FAILSAFE_LANDING) {
                    return PLANE_MODE_AUTO;
                }
                else {
                    // There is no valid mapping to ArduPlane
                    return PLANE_MODE_ENUM_END;
                }
            }
        default:                return PLANE_MODE_ENUM_END;
    }
}

static int mavlinkStreamTrigger(enum MAV_DATA_STREAM streamNum)
{
    uint8_t rate = (uint8_t) mavRates[streamNum];
    if (rate == 0) {
        return 0;
    }

    if (mavTicks[streamNum] == 0) {
        // we're triggering now, setup the next trigger point
        if (rate > TELEMETRY_MAVLINK_MAXRATE) {
            rate = TELEMETRY_MAVLINK_MAXRATE;
        }

        mavTicks[streamNum] = (TELEMETRY_MAVLINK_MAXRATE / rate);
        return 1;
    }

    // count down at TASK_RATE_HZ
    mavTicks[streamNum]--;
    return 0;
}

void freeMAVLinkTelemetryPort(void)
{
    closeSerialPort(mavlinkPort);
    mavlinkPort = NULL;
    mavlinkTelemetryEnabled = false;
}

void initMAVLinkTelemetry(void)
{
    portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_MAVLINK);
    mavlinkPortSharing = determinePortSharing(portConfig, FUNCTION_TELEMETRY_MAVLINK);
}


bool isMAVLinkTelemetryEnabled(void)
{
    return portConfig && mavlinkPort;
}

void configureMAVLinkTelemetryPort(void)
{
    if (!portConfig) {
        return;
    }

    baudRate_e baudRateIndex = portConfig->telemetry_baudrateIndex;
    if (baudRateIndex == BAUD_AUTO) {
        // default rate for minimOSD
        baudRateIndex = BAUD_57600;
    }

    mavlinkPort = openSerialPort(portConfig->identifier, FUNCTION_TELEMETRY_MAVLINK, NULL, NULL, baudRates[baudRateIndex], TELEMETRY_MAVLINK_PORT_MODE, SERIAL_NOT_INVERTED);

    if (!mavlinkPort) {
        return;
    }

    mavlinkTelemetryEnabled = true;
}

static void configureMAVLinkStreamRates(void)
{
    mavRates[MAV_DATA_STREAM_EXTENDED_STATUS] = telemetryConfig()->mavlink.extended_status_rate;
    mavRates[MAV_DATA_STREAM_RC_CHANNELS] = telemetryConfig()->mavlink.rc_channels_rate;
    mavRates[MAV_DATA_STREAM_POSITION] = telemetryConfig()->mavlink.position_rate;
    mavRates[MAV_DATA_STREAM_EXTRA1] = telemetryConfig()->mavlink.extra1_rate;
    mavRates[MAV_DATA_STREAM_EXTRA2] = telemetryConfig()->mavlink.extra2_rate;
    mavRates[MAV_DATA_STREAM_EXTRA3] = telemetryConfig()->mavlink.extra3_rate;
}

void checkMAVLinkTelemetryState(void)
{
    bool newTelemetryEnabledValue = telemetryDetermineEnabledState(mavlinkPortSharing);

    if (newTelemetryEnabledValue == mavlinkTelemetryEnabled) {
        return;
    }

    if (newTelemetryEnabledValue) {
        configureMAVLinkTelemetryPort();
        configureMAVLinkStreamRates();
    } else
        freeMAVLinkTelemetryPort();
}

static void mavlinkSendMessage(void)
{
    uint8_t mavBuffer[MAVLINK_MAX_PACKET_LEN];

    mavlink_status_t* chan_state = mavlink_get_channel_status(MAVLINK_COMM_0);
    if (telemetryConfig()->mavlink.version == 1) {
        chan_state->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
    } else {
        chan_state->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
    }

    int msgLength = mavlink_msg_to_send_buffer(mavBuffer, &mavSendMsg);

    for (int i = 0; i < msgLength; i++) {
        serialWrite(mavlinkPort, mavBuffer[i]);
    }
}

void mavlinkSendSystemStatus(void)
{
    // Receiver is assumed to be always present
    uint32_t onboard_control_sensors_present    = (MAV_SYS_STATUS_SENSOR_RC_RECEIVER);
    // GYRO and RC are assumed as minimum requirements
    uint32_t onboard_control_sensors_enabled    = (MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_RC_RECEIVER);
    uint32_t onboard_control_sensors_health     = 0;

    if (getHwGyroStatus() == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
        // Missing presence will report as sensor unhealthy
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
    }

    hardwareSensorStatus_e accStatus = getHwAccelerometerStatus();
    if (accStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    } else if (accStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    } else if (accStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    }

    hardwareSensorStatus_e compassStatus = getHwCompassStatus();
    if (compassStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    } else if (compassStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    } else if (compassStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    }

    hardwareSensorStatus_e baroStatus = getHwBarometerStatus();
    if (baroStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    } else if (baroStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    } else if (baroStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    }

    hardwareSensorStatus_e pitotStatus = getHwPitotmeterStatus();
    if (pitotStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    } else if (pitotStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    } else if (pitotStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    }

    hardwareSensorStatus_e gpsStatus = getHwGPSStatus();
    if (gpsStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_GPS;
    } else if (gpsStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
    } else if (gpsStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
    }

    hardwareSensorStatus_e opFlowStatus = getHwOpticalFlowStatus();
    if (opFlowStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    } else if (opFlowStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    } else if (opFlowStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    }

    hardwareSensorStatus_e rangefinderStatus = getHwRangefinderStatus();
    if (rangefinderStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    } else if (rangefinderStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    } else if (rangefinderStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    }

    if (rxIsReceivingSignal() && rxAreFlightChannelsValid()) {
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_RC_RECEIVER;
    }

#ifdef USE_BLACKBOX
    // BLACKBOX is assumed enabled and present for boards with capability
    onboard_control_sensors_present |= MAV_SYS_STATUS_LOGGING;
    onboard_control_sensors_enabled |= MAV_SYS_STATUS_LOGGING;
    // Unhealthy only for cases with not enough space to record
    if (!isBlackboxDeviceFull()) {
        onboard_control_sensors_health |= MAV_SYS_STATUS_LOGGING;
    }
#endif

    mavlink_msg_sys_status_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // onboard_control_sensors_present Bitmask showing which onboard controllers and sensors are present.
        //Value of 0: not present. Value of 1: present. Indices according MAV_SYS_STATUS_SENSOR
        onboard_control_sensors_present,
        // onboard_control_sensors_enabled Bitmask showing which onboard controllers and sensors are enabled
        onboard_control_sensors_enabled,
        // onboard_control_sensors_health Bitmask showing which onboard controllers and sensors are operational or have an error.
        onboard_control_sensors_health,
        // load Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
        constrain(averageSystemLoadPercent*10, 0, 1000),
        // voltage_battery Battery voltage, in millivolts (1 = 1 millivolt)
        feature(FEATURE_VBAT) ? getBatteryVoltage() * 10 : 0,
        // current_battery Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current
        isAmperageConfigured() ? getAmperage() : -1,
        // battery_remaining Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot estimate the remaining battery
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : 100,
        // drop_rate_comm Communication drops in percent, (0%: 0, 100%: 10'000), (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        0,
        // errors_comm Communication errors (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        0,
        // errors_count1 Autopilot-specific errors
        0,
        // errors_count2 Autopilot-specific errors
        0,
        // errors_count3 Autopilot-specific errors
        0,
        // errors_count4 Autopilot-specific errors
        0);

    mavlinkSendMessage();
}

static inline int16_t getChannelValue(uint8_t x) 
{
    int16_t channel_value = (rxRuntimeConfig.channelCount > x) ? rxGetChannelValue(x) : 0;
#if defined(USE_GIMBAL) && defined(USE_GIMBAL_MAVLINK)
    // override channel values if gimbal is enabled
    if(gimbalCommonIsEnabled() && gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK && IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        headTrackerDevice_t *dev = headTrackerCommonDevice();
        if (dev != NULL) {
            x = x + 1;  // channels are 1-indexed
            if (x == gimbalConfig()->panChannel) {
                channel_value = headTrackerCommonGetPanPWM(dev);
            } else if (x == gimbalConfig()->tiltChannel) {
                channel_value = headTrackerCommonGetTiltPWM(dev);
            } else if (x == gimbalConfig()->rollChannel) {
                channel_value = headTrackerCommonGetRollPWM(dev);
            }
        }
    }
#endif

    return channel_value;
}

void mavlinkSendRCChannelsAndRSSI(void)
{
    if (telemetryConfig()->mavlink.version == 1) {
        mavlink_msg_rc_channels_raw_pack(mavSystemId, mavComponentId, &mavSendMsg,
            // time_boot_ms Timestamp (milliseconds since system boot)
            millis(),
            // port Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows to encode more than 8 servos.
            0,
            // chan1_raw RC channel 1 value, in microseconds
            getChannelValue(0),
            // chan2_raw RC channel 2 value, in microseconds
            getChannelValue(1),
            // chan3_raw RC channel 3 value, in microseconds
            getChannelValue(2),
            // chan4_raw RC channel 4 value, in microseconds
            getChannelValue(3),
            // chan5_raw RC channel 5 value, in microseconds
            getChannelValue(4),
            // chan6_raw RC channel 6 value, in microseconds
            getChannelValue(5),
            // chan7_raw RC channel 7 value, in microseconds
            getChannelValue(6),
            // chan8_raw RC channel 8 value, in microseconds
            getChannelValue(7),
            // rssi Receive signal strength indicator, 0: 0%, 254: 100%
    		//https://github.com/mavlink/mavlink/issues/1027
            scaleRange(getRSSI(), 0, 1023, 0, 254));
	} 
    else {
        mavlink_msg_rc_channels_pack(mavSystemId, mavComponentId, &mavSendMsg,
            // time_boot_ms Timestamp (milliseconds since system boot)
            millis(),
            // Total number of RC channels being received. 
            rxRuntimeConfig.channelCount,
            // chan1_raw RC channel 1 value, in microseconds
            getChannelValue(0),
            // chan2_raw RC channel 2 value, in microseconds
            getChannelValue(1),
            // chan3_raw RC channel 3 value, in microseconds
            getChannelValue(2),
            // chan4_raw RC channel 4 value, in microseconds
            getChannelValue(3),
            // chan5_raw RC channel 5 value, in microseconds
            getChannelValue(4),
            // chan6_raw RC channel 6 value, in microseconds
            getChannelValue(5),
            // chan7_raw RC channel 7 value, in microseconds
            getChannelValue(6),
            // chan8_raw RC channel 8 value, in microseconds
            getChannelValue(7),
            // chan9_raw RC channel 9 value, in microseconds
            getChannelValue(8),
            // chan10_raw RC channel 10 value, in microseconds
            getChannelValue(9),
            // chan11_raw RC channel 11 value, in microseconds
            getChannelValue(10),
            // chan12_raw RC channel 12 value, in microseconds
            getChannelValue(11),
            // chan13_raw RC channel 13 value, in microseconds
            getChannelValue(12),
            // chan14_raw RC channel 14 value, in microseconds
            getChannelValue(13),
            // chan15_raw RC channel 15 value, in microseconds
            getChannelValue(14),
            // chan16_raw RC channel 16 value, in microseconds
            getChannelValue(15),
            // chan17_raw RC channel 17 value, in microseconds
            getChannelValue(16),
            // chan18_raw RC channel 18 value, in microseconds
            getChannelValue(17),
            // rssi Receive signal strength indicator, 0: 0%, 254: 100%
    		//https://github.com/mavlink/mavlink/issues/1027
            scaleRange(getRSSI(), 0, 1023, 0, 254));
    }
#undef GET_CHANNEL_VALUE

    mavlinkSendMessage();
}

#if defined(USE_GPS)
void mavlinkSendPosition(timeUs_t currentTimeUs)
{
    uint8_t gpsFixType = 0;

    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ))
        return;

    if (gpsSol.fixType == GPS_NO_FIX)
        gpsFixType = 1;
    else if (gpsSol.fixType == GPS_FIX_2D)
            gpsFixType = 2;
    else if (gpsSol.fixType == GPS_FIX_3D)
            gpsFixType = 3;

    mavlink_msg_gps_raw_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_usec Timestamp (microseconds since UNIX epoch or microseconds since system boot)
        currentTimeUs,
        // fix_type 0-1: no fix, 2: 2D fix, 3: 3D fix. Some applications will not use the value of this field unless it is at least two, so always correctly fill in the fix.
        gpsFixType,
        // lat Latitude in 1E7 degrees
        gpsSol.llh.lat,
        // lon Longitude in 1E7 degrees
        gpsSol.llh.lon,
        // alt Altitude in 1E3 meters (millimeters) above MSL
        gpsSol.llh.alt * 10,
        // eph GPS HDOP horizontal dilution of position in cm (m*100). If unknown, set to: 65535
        gpsSol.eph,
        // epv GPS VDOP horizontal dilution of position in cm (m*100). If unknown, set to: 65535
        gpsSol.epv,
        // vel GPS ground speed (m/s * 100). If unknown, set to: 65535
        gpsSol.groundSpeed,
        // cog Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. If unknown, set to: 65535
        gpsSol.groundCourse * 10,
        // satellites_visible Number of satellites visible. If unknown, set to 255
        gpsSol.numSat,
        // alt_ellipsoid Altitude (above WGS84, EGM96 ellipsoid). Positive for up
        0,
        // h_acc Position uncertainty in mm,
        gpsSol.eph * 10,
        // v_acc Altitude uncertainty in mm,
        gpsSol.epv * 10,
        // vel_acc Speed uncertainty in mm (??)
        0,
        // hdg_acc Heading uncertainty in degE5
        0,
        // yaw Yaw in earth frame from north. Use 0 if this GPS does not provide yaw. Use 65535 if this GPS is configured to provide yaw and is currently unable to provide it. Use 36000 for north.
        0);

    mavlinkSendMessage();

    // Global position
    mavlink_msg_global_position_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_usec Timestamp (microseconds since UNIX epoch or microseconds since system boot)
        currentTimeUs,
        // lat Latitude in 1E7 degrees
        gpsSol.llh.lat,
        // lon Longitude in 1E7 degrees
        gpsSol.llh.lon,
        // alt Altitude in 1E3 meters (millimeters) above MSL
        gpsSol.llh.alt * 10,
        // relative_alt Altitude above ground in meters, expressed as * 1000 (millimeters)
        getEstimatedActualPosition(Z) * 10,
        // [cm/s] Ground X Speed (Latitude, positive north)
        getEstimatedActualVelocity(X),
        // [cm/s] Ground Y Speed (Longitude, positive east)
        getEstimatedActualVelocity(Y),
        // [cm/s] Ground Z Speed (Altitude, positive down)
        getEstimatedActualVelocity(Z),
        // [cdeg] Vehicle heading (yaw angle) (0.0..359.99 degrees, 0=north)
        DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw)
    );

    mavlinkSendMessage();

    mavlink_msg_gps_global_origin_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // latitude Latitude (WGS84), expressed as * 1E7
        GPS_home.lat,
        // longitude Longitude (WGS84), expressed as * 1E7
        GPS_home.lon,
        // altitude Altitude(WGS84), expressed as * 1000
        GPS_home.alt * 10, // FIXME
        // time_usec Timestamp (microseconds since system boot)
        // Use millis() * 1000 as micros() will overflow after 1.19 hours.
        ((uint64_t) millis()) * 1000);

    mavlinkSendMessage();
}
#endif

void mavlinkSendAttitude(void)
{
    mavlink_msg_attitude_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_boot_ms Timestamp (milliseconds since system boot)
        millis(),
        // roll Roll angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.roll)),
        // pitch Pitch angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(-attitude.values.pitch)),
        // yaw Yaw angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.yaw)),
        // rollspeed Roll angular speed (rad/s)
        gyro.gyroADCf[FD_ROLL],
        // pitchspeed Pitch angular speed (rad/s)
        gyro.gyroADCf[FD_PITCH],
        // yawspeed Yaw angular speed (rad/s)
        gyro.gyroADCf[FD_YAW]);

    mavlinkSendMessage();
}

void mavlinkSendHUDAndHeartbeat(void)
{
    float mavAltitude = 0;
    float mavGroundSpeed = 0;
    float mavAirSpeed = 0;
    float mavClimbRate = 0;

#if defined(USE_GPS)
    // use ground speed if source available
    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        mavGroundSpeed = gpsSol.groundSpeed / 100.0f;
    }
#endif

#if defined(USE_PITOT)
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        mavAirSpeed = getAirspeedEstimate() / 100.0f;
    }
#endif

    // select best source for altitude
    mavAltitude = getEstimatedActualPosition(Z) / 100.0f;
    mavClimbRate = getEstimatedActualVelocity(Z) / 100.0f;

    int16_t thr = getThrottlePercent(osdUsingScaledThrottle());
    mavlink_msg_vfr_hud_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // airspeed Current airspeed in m/s
        mavAirSpeed,
        // groundspeed Current ground speed in m/s
        mavGroundSpeed,
        // heading Current heading in degrees, in compass units (0..360, 0=north)
        DECIDEGREES_TO_DEGREES(attitude.values.yaw),
        // throttle Current throttle setting in integer percent, 0 to 100
        thr,
        // alt Current altitude (MSL), in meters, if we have surface or baro use them, otherwise use GPS (less accurate)
        mavAltitude,
        // climb Current climb rate in meters/second
        mavClimbRate);

    mavlinkSendMessage();


    uint8_t mavModes = MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    if (ARMING_FLAG(ARMED))
        mavModes |= MAV_MODE_FLAG_SAFETY_ARMED;

    uint8_t mavSystemType;
    switch (mixerConfig()->platformType)
    {
        case PLATFORM_MULTIROTOR:
            mavSystemType = MAV_TYPE_QUADROTOR;
            break;
        case PLATFORM_TRICOPTER:
            mavSystemType = MAV_TYPE_TRICOPTER;
            break;
        case PLATFORM_AIRPLANE:
            mavSystemType = MAV_TYPE_FIXED_WING;
            break;
        case PLATFORM_ROVER:
            mavSystemType = MAV_TYPE_GROUND_ROVER;
            break;
        case PLATFORM_BOAT:
            mavSystemType = MAV_TYPE_SURFACE_BOAT;
            break;
        case PLATFORM_HELICOPTER:
            mavSystemType = MAV_TYPE_HELICOPTER;
            break;
        default:
            mavSystemType = MAV_TYPE_GENERIC;
            break;
    }

    flightModeForTelemetry_e flm = getFlightModeForTelemetry();
    uint8_t mavCustomMode;

    if (STATE(FIXED_WING_LEGACY)) {
        mavCustomMode = (uint8_t)inavToArduPlaneMap(flm);
    }
    else {
        mavCustomMode = (uint8_t)inavToArduCopterMap(flm);
    }

    if (flm != FLM_MANUAL) {
        mavModes |= MAV_MODE_FLAG_STABILIZE_ENABLED;
    }
    if (flm == FLM_POSITION_HOLD || flm == FLM_RTH || flm == FLM_MISSION) {
        mavModes |= MAV_MODE_FLAG_GUIDED_ENABLED;
    }

    uint8_t mavSystemState = 0;
    if (ARMING_FLAG(ARMED)) {
        if (failsafeIsActive()) {
            mavSystemState = MAV_STATE_CRITICAL;
        }
        else {
            mavSystemState = MAV_STATE_ACTIVE;
        }
    }
    else if (areSensorsCalibrating()) {
        mavSystemState = MAV_STATE_CALIBRATING;
    }
    else {
        mavSystemState = MAV_STATE_STANDBY;
    }

    mavlink_msg_heartbeat_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // type Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
        mavSystemType,
        // autopilot Autopilot type / class. defined in MAV_AUTOPILOT ENUM
        MAV_AUTOPILOT_GENERIC,
        // base_mode System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
        mavModes,
        // custom_mode A bitfield for use for autopilot-specific flags.
        mavCustomMode,
        // system_status System status flag, see MAV_STATE ENUM
        mavSystemState);

    mavlinkSendMessage();
}

void mavlinkSendBatteryTemperatureStatusText(void)
{
    uint16_t batteryVoltages[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN];
    uint16_t batteryVoltagesExt[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN];
    memset(batteryVoltages, UINT16_MAX, sizeof(batteryVoltages));
    memset(batteryVoltagesExt, 0, sizeof(batteryVoltagesExt));
    if (feature(FEATURE_VBAT)) {
        uint8_t batteryCellCount = getBatteryCellCount();
        if (batteryCellCount > 0) {
            for (int cell=0; cell < batteryCellCount && cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN + MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN; cell++) {
                if (cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN) {
                    batteryVoltages[cell] = getBatteryAverageCellVoltage() * 10;
                } else {
                    batteryVoltagesExt[cell-MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN] = getBatteryAverageCellVoltage() * 10;
                }
            }
        }
        else {
            batteryVoltages[0] = getBatteryVoltage() * 10;
        }
    }
    else {
        batteryVoltages[0] = 0;
    }

    mavlink_msg_battery_status_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // id Battery ID
        0,
        // battery_function Function of the battery
        MAV_BATTERY_FUNCTION_UNKNOWN,
        // type Type (chemistry) of the battery
        MAV_BATTERY_TYPE_UNKNOWN,
        // temperature Temperature of the battery in centi-degrees celsius. INT16_MAX for unknown temperature
        INT16_MAX,
        // voltages Battery voltage of cells, in millivolts (1 = 1 millivolt). Cells above the valid cell count for this battery should have the UINT16_MAX value.
        batteryVoltages,
        // current_battery Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current
        isAmperageConfigured() ? getAmperage() : -1,
        // current_consumed Consumed charge, in milliampere hours (1 = 1 mAh), -1: autopilot does not provide mAh consumption estimate
        isAmperageConfigured() ? getMAhDrawn() : -1,
        // energy_consumed Consumed energy, in 100*Joules (intergrated U*I*dt)  (1 = 100 Joule), -1: autopilot does not provide energy consumption estimate
        isAmperageConfigured() ? getMWhDrawn()*36 : -1,
        // battery_remaining Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot does not estimate the remaining battery);
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : -1,
        // time_remaining Remaining battery time, 0: autopilot does not provide remaining battery time estimate
        0, // TODO this could easily be implemented
        // charge_state State for extent of discharge, provided by autopilot for warning or external reactions
        0,
        // voltages_ext Battery voltages for cells 11 to 14. Cells above the valid cell count for this battery should have a value of 0, where zero indicates not supported (note, this is different than for the voltages field and allows empty byte truncation). If the measured value is 0 then 1 should be sent instead.
        batteryVoltagesExt,
        // mode Battery mode. Default (0) is that battery mode reporting is not supported or battery is in normal-use mode.
        0,
        // fault_bitmask Fault/health indications. These should be set when charge_state is MAV_BATTERY_CHARGE_STATE_FAILED or MAV_BATTERY_CHARGE_STATE_UNHEALTHY (if not, fault reporting is not supported).
        0);

    mavlinkSendMessage();


    int16_t temperature;
    sensors(SENSOR_BARO) ? getBaroTemperature(&temperature) : getIMUTemperature(&temperature);
    mavlink_msg_scaled_pressure_pack(mavSystemId, mavComponentId, &mavSendMsg,
        millis(),
        0,
        0,
        temperature * 10,
        0);

    mavlinkSendMessage();


// FIXME - Status text is limited to boards with USE_OSD
#ifdef USE_OSD
    char buff[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN] = {" "};
    textAttributes_t elemAttr = osdGetSystemMessage(buff, sizeof(buff), false);
    if (buff[0] != SYM_BLANK) {
        MAV_SEVERITY severity = MAV_SEVERITY_NOTICE;
        if (TEXT_ATTRIBUTES_HAVE_BLINK(elemAttr)) {
            severity = MAV_SEVERITY_CRITICAL;
        } else if TEXT_ATTRIBUTES_HAVE_INVERTED(elemAttr) {
            severity = MAV_SEVERITY_WARNING;
        }

        mavlink_msg_statustext_pack(mavSystemId, mavComponentId, &mavSendMsg,
            (uint8_t)severity,
            buff,
            0,
            0);

        mavlinkSendMessage();
    }
#endif


}

#if defined(USE_GIMBAL) && defined(USE_GIMBAL_MAVLINK)
void mavlinkSendGimbalAttitude(void)
{
    // https://mavlink.io/en/messages/common.html#AUTOPILOT_STATE_FOR_GIMBAL_DEVICE
    uint8_t targetSystem = MAV_TYPE_GIMBAL;
    uint8_t targetComponent = MAV_COMP_ID_GIMBAL;
    uint8_t time_boot_us = micros();
    float q[4] = {NAN, NAN, NAN, NAN};
    uint16_t estimator_status = (ESTIMATOR_ATTITUDE | ESTIMATOR_VELOCITY_HORIZ | ESTIMATOR_VELOCITY_VERT | ESTIMATOR_POS_HORIZ_REL | ESTIMATOR_POS_HORIZ_ABS | ESTIMATOR_POS_VERT_ABS | ESTIMATOR_POS_VERT_AGL | ESTIMATOR_CONST_POS_MODE | ESTIMATOR_PRED_POS_HORIZ_REL | ESTIMATOR_POS_HORIZ_ABS);

    mavlink_euler_to_quaternion(attitude.values.roll, attitude.values.pitch, attitude.values.yaw, q);

    // https://mavlink.io/en/messages/common.html#AUTOPILOT_STATE_FOR_GIMBAL_DEVICE
    mavlink_msg_autopilot_state_for_gimbal_device_pack(mavSystemId, mavComponentId, &mavSendMsg,
        targetSystem, targetComponent, time_boot_us, q, 0, getEstimatedActualVelocity(X), getEstimatedActualVelocity(Y), getEstimatedActualVelocity(Z), 0,
        NAN, estimator_status, MAV_LANDED_STATE_UNDEFINED);

    mavlinkSendMessage();
}
#endif

void processMAVLinkTelemetry(timeUs_t currentTimeUs)
{
    // is executed @ TELEMETRY_MAVLINK_MAXRATE rate
    if (mavlinkStreamTrigger(MAV_DATA_STREAM_EXTENDED_STATUS)) {
        mavlinkSendSystemStatus();
    }

    if (mavlinkStreamTrigger(MAV_DATA_STREAM_RC_CHANNELS)) {
        mavlinkSendRCChannelsAndRSSI();
#if defined(USE_GIMBAL) && defined(USE_GIMBAL_MAVLINK)
        if(gimbalCommonIsEnabled() && gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK) {
            mavlinkSendGimbalAttitude();
        }
#endif
    }

#ifdef USE_GPS
    if (mavlinkStreamTrigger(MAV_DATA_STREAM_POSITION)) {
        mavlinkSendPosition(currentTimeUs);
    }
#endif

    if (mavlinkStreamTrigger(MAV_DATA_STREAM_EXTRA1)) {
        mavlinkSendAttitude();
    }

    if (mavlinkStreamTrigger(MAV_DATA_STREAM_EXTRA2)) {
        mavlinkSendHUDAndHeartbeat();
    }

    if (mavlinkStreamTrigger(MAV_DATA_STREAM_EXTRA3)) {
        mavlinkSendBatteryTemperatureStatusText();
    }
}

static bool handleIncoming_MISSION_CLEAR_ALL(void)
{
    mavlink_mission_clear_all_t msg;
    mavlink_msg_mission_clear_all_decode(&mavRecvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        resetWaypointList();
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION);
        mavlinkSendMessage();
        return true;
    }

    return false;
}

// Static state for MISSION UPLOAD transaction (starting with MISSION_COUNT)
static int incomingMissionWpCount = 0;
static int incomingMissionWpSequence = 0;

static bool handleIncoming_MISSION_COUNT(void)
{
    mavlink_mission_count_t msg;
    mavlink_msg_mission_count_decode(&mavRecvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        if (msg.count <= NAV_MAX_WAYPOINTS) {
            incomingMissionWpCount = msg.count; // We need to know how many items to request
            incomingMissionWpSequence = 0;
            mavlink_msg_mission_request_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }
        else if (ARMING_FLAG(ARMED)) {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }
        else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_NO_SPACE, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }
    }

    return false;
}

static bool handleIncoming_MISSION_ITEM(void)
{
    mavlink_mission_item_t msg;
    mavlink_msg_mission_item_decode(&mavRecvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        // Check supported values first
        if (ARMING_FLAG(ARMED)) {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }

        if ((msg.autocontinue == 0) || (msg.command != MAV_CMD_NAV_WAYPOINT && msg.command != MAV_CMD_NAV_RETURN_TO_LAUNCH)) {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }

        if ((msg.frame != MAV_FRAME_GLOBAL_RELATIVE_ALT) && !(msg.frame == MAV_FRAME_MISSION && msg.command == MAV_CMD_NAV_RETURN_TO_LAUNCH)) {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }

        if (msg.seq == incomingMissionWpSequence) {
            incomingMissionWpSequence++;

            navWaypoint_t wp;
            wp.action = (msg.command == MAV_CMD_NAV_RETURN_TO_LAUNCH) ? NAV_WP_ACTION_RTH : NAV_WP_ACTION_WAYPOINT;
            wp.lat = (int32_t)(msg.x * 1e7f);
            wp.lon = (int32_t)(msg.y * 1e7f);
            wp.alt = msg.z * 100.0f;
            wp.p1 = 0;
            wp.p2 = 0;
            wp.p3 = 0;
            wp.flag = (incomingMissionWpSequence >= incomingMissionWpCount) ? NAV_WP_FLAG_LAST : 0;

            setWaypoint(incomingMissionWpSequence, &wp);

            if (incomingMissionWpSequence >= incomingMissionWpCount) {
                if (isWaypointListValid()) {
                    mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION);
                    mavlinkSendMessage();
                }
                else {
                    mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_INVALID, MAV_MISSION_TYPE_MISSION);
                    mavlinkSendMessage();
                }
            }
            else {
                mavlink_msg_mission_request_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
                mavlinkSendMessage();
            }
        }
        else {
            // Wrong sequence number received
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
        }

        return true;
    }

    return false;
}

static bool handleIncoming_MISSION_REQUEST_LIST(void)
{
    mavlink_mission_request_list_t msg;
    mavlink_msg_mission_request_list_decode(&mavRecvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        mavlink_msg_mission_count_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, getWaypointCount(), MAV_MISSION_TYPE_MISSION);
        mavlinkSendMessage();
        return true;
    }

    return false;
}

static bool handleIncoming_MISSION_REQUEST(void)
{
    mavlink_mission_request_t msg;
    mavlink_msg_mission_request_decode(&mavRecvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        int wpCount = getWaypointCount();

        if (msg.seq < wpCount) {
            navWaypoint_t wp;
            getWaypoint(msg.seq + 1, &wp);

            mavlink_msg_mission_item_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid,
                        msg.seq,
                        wp.action == NAV_WP_ACTION_RTH ? MAV_FRAME_MISSION : MAV_FRAME_GLOBAL_RELATIVE_ALT,
                        wp.action == NAV_WP_ACTION_RTH ? MAV_CMD_NAV_RETURN_TO_LAUNCH : MAV_CMD_NAV_WAYPOINT,
                        0,
                        1,
                        0, 0, 0, 0,
                        wp.lat / 1e7f,
                        wp.lon / 1e7f,
                        wp.alt / 100.0f,
                        MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
        }
        else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavRecvMsg.sysid, mavRecvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
        }

        return true;
    }

    return false;
}

static bool handleIncoming_RC_CHANNELS_OVERRIDE(void) {
    mavlink_rc_channels_override_t msg;
    mavlink_msg_rc_channels_override_decode(&mavRecvMsg, &msg);
    // Don't check system ID because it's not configurable with systems like Crossfire
    mavlinkRxHandleMessage(&msg);
    return true;
}

static bool handleIncoming_PARAM_REQUEST_LIST(void) {
    mavlink_param_request_list_t msg;
    mavlink_msg_param_request_list_decode(&mavRecvMsg, &msg);

    // Respond that we don't have any parameters to force Mission Planner to give up quickly
    if (msg.target_system == mavSystemId) {
        // mavlink_msg_param_value_pack(system_id, component_id, msg, param_value->param_id, param_value->param_value, param_value->param_type, param_value->param_count, param_value->param_index);
        mavlink_msg_param_value_pack(mavSystemId, mavComponentId, &mavSendMsg, 0, 0, 0, 0, 0);
        mavlinkSendMessage();
    }
    return true;
}

static void mavlinkParseRxStats(const mavlink_radio_status_t *msg) {
    switch(telemetryConfig()->mavlink.radio_type) {
        case MAVLINK_RADIO_SIK:
            // rssi scaling info from: https://ardupilot.org/rover/docs/common-3dr-radio-advanced-configuration-and-technical-information.html
            rxLinkStatistics.uplinkRSSI = (msg->rssi / 1.9) - 127;
            rxLinkStatistics.uplinkSNR = msg->noise / 1.9;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
        case MAVLINK_RADIO_ELRS:
            rxLinkStatistics.uplinkRSSI = -msg->remrssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = scaleRange(msg->rssi, 0, 255, 0, 100);
            break;
        case MAVLINK_RADIO_GENERIC:
        default:
            rxLinkStatistics.uplinkRSSI = msg->rssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
    }
}

static bool handleIncoming_RADIO_STATUS(void) {
    mavlink_radio_status_t msg;
    mavlink_msg_radio_status_decode(&mavRecvMsg, &msg);
    txbuff_valid = true;
    txbuff_free = msg.txbuf;
       
    if (rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK) {
        mavlinkParseRxStats(&msg);
    }

    return true;
}

static bool handleIncoming_HEARTBEAT(void) {
    mavlink_heartbeat_t msg;
    mavlink_msg_heartbeat_decode(&mavRecvMsg, &msg);

    switch (msg.type) {
#ifdef USE_ADSB
        case MAV_TYPE_ADSB:
            return adsbHeartbeat();
#endif
        default:
            break;
    }
    
    return false;
}

#ifdef USE_ADSB
static bool handleIncoming_ADSB_VEHICLE(void) {
    mavlink_adsb_vehicle_t msg;
    mavlink_msg_adsb_vehicle_decode(&mavRecvMsg, &msg);

    adsbVehicleValues_t* vehicle = getVehicleForFill();
    if(vehicle != NULL){
        vehicle->icao = msg.ICAO_address;
        vehicle->lat = msg.lat;
        vehicle->lon = msg.lon;
        vehicle->alt = (int32_t)(msg.altitude / 10);
        vehicle->heading = msg.heading;
        vehicle->flags = msg.flags;
        vehicle->altitudeType = msg.altitude_type;
        memcpy(&(vehicle->callsign), msg.callsign, sizeof(vehicle->callsign));
        vehicle->emitterType = msg.emitter_type;
        vehicle->tslc = msg.tslc;

        adsbNewVehicle(vehicle);
    }

    //debug vehicle
   /* if(vehicle != NULL){

        char name[9] = "DUMMY    ";

        vehicle->icao = 666;
        vehicle->lat = 492383514;
        vehicle->lon = 165148681;
        vehicle->alt = 100000;
        vehicle->heading = 180;
        vehicle->flags = ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_COORDS;
        vehicle->altitudeType = 0;
        memcpy(&(vehicle->callsign), name, sizeof(vehicle->callsign));
        vehicle->emitterType = 6;
        vehicle->tslc = 0;

        adsbNewVehicle(vehicle);
    }*/

    return true;
}
#endif

// Returns whether a message was processed
static bool processMAVLinkIncomingTelemetry(void)
{
    while (serialRxBytesWaiting(mavlinkPort) > 0) {
        // Limit handling to one message per cycle
        char c = serialRead(mavlinkPort);
        uint8_t result = mavlink_parse_char(0, c, &mavRecvMsg, &mavRecvStatus);
        if (result == MAVLINK_FRAMING_OK) {
            switch (mavRecvMsg.msgid) {
                case MAVLINK_MSG_ID_HEARTBEAT:
                   return handleIncoming_HEARTBEAT();
                case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
                    return handleIncoming_PARAM_REQUEST_LIST();
                case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
                    return handleIncoming_MISSION_CLEAR_ALL();
                case MAVLINK_MSG_ID_MISSION_COUNT:
                    return handleIncoming_MISSION_COUNT();
                case MAVLINK_MSG_ID_MISSION_ITEM:
                    return handleIncoming_MISSION_ITEM();
                case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
                    return handleIncoming_MISSION_REQUEST_LIST();
                case MAVLINK_MSG_ID_MISSION_REQUEST:
                    return handleIncoming_MISSION_REQUEST();
                case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
                    handleIncoming_RC_CHANNELS_OVERRIDE();
                    // Don't set that we handled a message, otherwise RC channel packets will block telemetry messages
                    return false;
#ifdef USE_ADSB
                case MAVLINK_MSG_ID_ADSB_VEHICLE:
                    return handleIncoming_ADSB_VEHICLE();
#endif
                case MAVLINK_MSG_ID_RADIO_STATUS:
                    handleIncoming_RADIO_STATUS();
                    // Don't set that we handled a message, otherwise radio status packets will block telemetry messages.
                    return false;
                default:
                    return false;
            }
        }
    }

    return false;
}

static bool isMAVLinkTelemetryHalfDuplex(void) {
    return telemetryConfig()->halfDuplex ||
            (rxConfig()->receiverType == RX_TYPE_SERIAL && rxConfig()->serialrx_provider == SERIALRX_MAVLINK && tristateWithDefaultOffIsActive(rxConfig()->halfDuplex));
}

void handleMAVLinkTelemetry(timeUs_t currentTimeUs)
{
    if (!mavlinkTelemetryEnabled) {
        return;
    }

    if (!mavlinkPort) {
        return;
    }

    // Process incoming MAVLink
    bool receivedMessage = processMAVLinkIncomingTelemetry();
    bool shouldSendTelemetry = false;

    // Determine whether to send telemetry back based on flow control / pacing
    if (txbuff_valid) {
        // Use flow control if available
        shouldSendTelemetry = txbuff_free >= telemetryConfig()->mavlink.min_txbuff;
    } else {
        // If not, use blind frame pacing - and back off for collision avoidance if half-duplex
        bool halfDuplexBackoff = (isMAVLinkTelemetryHalfDuplex() && receivedMessage);
        shouldSendTelemetry = ((currentTimeUs - lastMavlinkMessage) >= TELEMETRY_MAVLINK_DELAY) && !halfDuplexBackoff;
    }

    if (shouldSendTelemetry) {
        processMAVLinkTelemetry(currentTimeUs);
        lastMavlinkMessage = currentTimeUs;
    }
}

#endif
