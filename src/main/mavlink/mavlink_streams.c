#include "mavlink/mavlink_internal.h"

#include "common/time.h"

#include "mavlink/mavlink_modes.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

/* Secondary profile for ports 2..N: heartbeat only. */
const uint8_t mavSecondaryRates[MAVLINK_STREAM_COUNT] = {
    [MAV_DATA_STREAM_EXTENDED_STATUS] = 0,
    [MAV_DATA_STREAM_RC_CHANNELS] = 0,
    [MAV_DATA_STREAM_POSITION] = 0,
    [MAV_DATA_STREAM_EXTRA1] = 0,
    [MAV_DATA_STREAM_EXTRA2] = 0,
    [MAV_DATA_STREAM_EXTRA3] = 0,
    [MAV_DATA_STREAM_EXTENDED_SYS_STATE] = 0,
    [MAV_DATA_STREAM_HEARTBEAT] = 1
};

uint8_t mavlinkClampStreamRate(uint8_t rate)
{
    if (rate > TELEMETRY_MAVLINK_MAXRATE) {
        return TELEMETRY_MAVLINK_MAXRATE;
    }

    return rate;
}

int32_t mavlinkRateToIntervalUs(uint8_t rate)
{
    rate = mavlinkClampStreamRate(rate);
    if (rate == 0) {
        return -1;
    }

    return 1000000 / rate;
}

bool mavlinkPeriodicMessageFromMessageId(uint16_t messageId, mavlinkPeriodicMessage_e *periodicMessage)
{
    switch (messageId) {
        case MAVLINK_MSG_ID_HEARTBEAT:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_HEARTBEAT;
            return true;
        case MAVLINK_MSG_ID_SYS_STATUS:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_SYS_STATUS;
            return true;
        case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_EXTENDED_SYS_STATE;
            return true;
        case MAVLINK_MSG_ID_RC_CHANNELS:
        case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
        case MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_RC_CHANNELS;
            return true;
        case MAVLINK_MSG_ID_GPS_RAW_INT:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_GPS_RAW_INT;
            return true;
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_GLOBAL_POSITION_INT;
            return true;
        case MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_GPS_GLOBAL_ORIGIN;
            return true;
        case MAVLINK_MSG_ID_ATTITUDE:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_ATTITUDE;
            return true;
        case MAVLINK_MSG_ID_VFR_HUD:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_VFR_HUD;
            return true;
        case MAVLINK_MSG_ID_BATTERY_STATUS:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_BATTERY_STATUS;
            return true;
        case MAVLINK_MSG_ID_SCALED_PRESSURE:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_SCALED_PRESSURE;
            return true;
        case MAVLINK_MSG_ID_SYSTEM_TIME:
            *periodicMessage = MAVLINK_PERIODIC_MESSAGE_SYSTEM_TIME;
            return true;
        default:
            return false;
    }
}

uint8_t mavlinkPeriodicMessageBaseStream(mavlinkPeriodicMessage_e periodicMessage)
{
    switch (periodicMessage) {
        case MAVLINK_PERIODIC_MESSAGE_HEARTBEAT:
            return MAV_DATA_STREAM_HEARTBEAT;
        case MAVLINK_PERIODIC_MESSAGE_SYS_STATUS:
            return MAV_DATA_STREAM_EXTENDED_STATUS;
        case MAVLINK_PERIODIC_MESSAGE_EXTENDED_SYS_STATE:
            return MAV_DATA_STREAM_EXTENDED_SYS_STATE;
        case MAVLINK_PERIODIC_MESSAGE_RC_CHANNELS:
            return MAV_DATA_STREAM_RC_CHANNELS;
        case MAVLINK_PERIODIC_MESSAGE_GPS_RAW_INT:
        case MAVLINK_PERIODIC_MESSAGE_GLOBAL_POSITION_INT:
        case MAVLINK_PERIODIC_MESSAGE_GPS_GLOBAL_ORIGIN:
            return MAV_DATA_STREAM_POSITION;
        case MAVLINK_PERIODIC_MESSAGE_ATTITUDE:
            return MAV_DATA_STREAM_EXTRA1;
        case MAVLINK_PERIODIC_MESSAGE_VFR_HUD:
            return MAV_DATA_STREAM_EXTRA2;
        case MAVLINK_PERIODIC_MESSAGE_BATTERY_STATUS:
        case MAVLINK_PERIODIC_MESSAGE_SCALED_PRESSURE:
        case MAVLINK_PERIODIC_MESSAGE_SYSTEM_TIME:
            return MAV_DATA_STREAM_EXTRA3;
        default:
            return MAV_DATA_STREAM_ALL;
    }
}

static void mavlinkResetMessageSchedule(mavlinkPeriodicMessage_e periodicMessage)
{
    if (!mavActivePort) {
        return;
    }

    mavActivePort->mavMessageNextDue[periodicMessage] = 0;
}

static void mavlinkResetMessagesForStream(uint8_t streamNum)
{
    if (!mavActivePort) {
        return;
    }

    for (uint8_t messageIndex = 0; messageIndex < MAVLINK_PERIODIC_MESSAGE_COUNT; messageIndex++) {
        if (mavlinkPeriodicMessageBaseStream((mavlinkPeriodicMessage_e)messageIndex) == streamNum) {
            mavActivePort->mavMessageNextDue[messageIndex] = 0;
        }
    }
}

int32_t mavlinkMessageBaseIntervalUs(mavlinkPeriodicMessage_e periodicMessage)
{
    if (!mavActivePort) {
        return -1;
    }

    return mavlinkRateToIntervalUs(mavActivePort->mavRates[mavlinkPeriodicMessageBaseStream(periodicMessage)]);
}

int32_t mavlinkMessageIntervalUs(mavlinkPeriodicMessage_e periodicMessage)
{
    if (!mavActivePort) {
        return -1;
    }

    const int32_t overrideIntervalUs = mavActivePort->mavMessageOverrideIntervalsUs[periodicMessage];
    if (overrideIntervalUs != 0) {
        return overrideIntervalUs;
    }

    return mavlinkMessageBaseIntervalUs(periodicMessage);
}

void mavlinkSetMessageOverrideIntervalUs(mavlinkPeriodicMessage_e periodicMessage, int32_t intervalUs)
{
    if (!mavActivePort) {
        return;
    }

    mavActivePort->mavMessageOverrideIntervalsUs[periodicMessage] = intervalUs;
    mavlinkResetMessageSchedule(periodicMessage);
}

int mavlinkStreamTrigger(enum MAV_DATA_STREAM streamNum, timeUs_t currentTimeUs)
{
    if (!mavActivePort || streamNum >= MAXSTREAMS) {
        return 0;
    }

    const uint8_t rate = mavlinkClampStreamRate(mavActivePort->mavRates[streamNum]);
    if (rate == 0) {
        return 0;
    }

    const timeUs_t intervalUs = 1000000UL / rate;
    if ((mavActivePort->mavStreamNextDue[streamNum] == 0) || (cmpTimeUs(currentTimeUs, mavActivePort->mavStreamNextDue[streamNum]) >= 0)) {
        mavActivePort->mavStreamNextDue[streamNum] = currentTimeUs + intervalUs;
        return 1;
    }

    return 0;
}

void mavlinkSetStreamRate(uint8_t streamNum, uint8_t rate)
{
    if (!mavActivePort || streamNum >= MAXSTREAMS) {
        return;
    }
    mavActivePort->mavRates[streamNum] = mavlinkClampStreamRate(rate);
    mavActivePort->mavStreamNextDue[streamNum] = 0;
    mavlinkResetMessagesForStream(streamNum);
}

int mavlinkMessageTrigger(mavlinkPeriodicMessage_e periodicMessage, timeUs_t currentTimeUs)
{
    if (!mavActivePort) {
        return 0;
    }

    const int32_t intervalUs = mavlinkMessageIntervalUs(periodicMessage);
    if (intervalUs <= 0) {
        return 0;
    }

    if ((mavActivePort->mavMessageNextDue[periodicMessage] == 0) || (cmpTimeUs(currentTimeUs, mavActivePort->mavMessageNextDue[periodicMessage]) >= 0)) {
        mavActivePort->mavMessageNextDue[periodicMessage] = currentTimeUs + intervalUs;
        return 1;
    }

    return 0;
}

void configureMAVLinkStreamRates(uint8_t portIndex)
{
    const mavlinkTelemetryPortConfig_t *primaryConfig = &telemetryConfig()->mavlink[0];
    const uint8_t mavPrimaryRates[MAVLINK_STREAM_COUNT] = {
        [MAV_DATA_STREAM_EXTENDED_STATUS] = primaryConfig->extended_status_rate,
        [MAV_DATA_STREAM_RC_CHANNELS] = primaryConfig->rc_channels_rate,
        [MAV_DATA_STREAM_POSITION] = primaryConfig->position_rate,
        [MAV_DATA_STREAM_EXTRA1] = primaryConfig->extra1_rate,
        [MAV_DATA_STREAM_EXTRA2] = primaryConfig->extra2_rate,
        [MAV_DATA_STREAM_EXTRA3] = primaryConfig->extra3_rate,
        [MAV_DATA_STREAM_EXTENDED_SYS_STATE] = primaryConfig->extra3_rate,
        [MAV_DATA_STREAM_HEARTBEAT] = 1
    };

    const uint8_t *selectedRates = (portIndex == 0) ? mavPrimaryRates : mavSecondaryRates;
    mavlinkPortRuntime_t *state = &mavPortStates[portIndex];

    for (uint8_t stream = 0; stream < MAVLINK_STREAM_COUNT; stream++) {
        state->mavRates[stream] = selectedRates[stream];
        state->mavRatesConfigured[stream] = selectedRates[stream];
        state->mavStreamNextDue[stream] = 0;
    }
}

void mavlinkSendAutopilotVersion(void)
{
    if (mavlinkGetProtocolVersion() == 1) {
        return;
    }

    // Capabilities aligned with what we actually support.
    uint64_t capabilities = 0;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MAVLINK2;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MISSION_FLOAT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MISSION_INT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_COMMAND_INT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED;
    capabilities |= MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_GLOBAL_INT;

    const uint32_t flightSwVersion =
        ((uint32_t)ARDUPILOT_VERSION_MAJOR << 24) |
        ((uint32_t)ARDUPILOT_VERSION_MINOR << 16) |
        ((uint32_t)ARDUPILOT_VERSION_PATCH << 8);

    // Bare minimum: caps + IDs. Everything else 0 is fine.
    mavlink_msg_autopilot_version_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        capabilities,
        flightSwVersion,
        0,
        0,
        0,
        0ULL,
        0ULL,
        0ULL,
        0ULL,
        0ULL,
        (uint64_t)mavSystemId,
        0ULL
    );
    mavlinkSendMessage();
}

void mavlinkSendProtocolVersion(void)
{
    if (mavlinkGetProtocolVersion() == 1) {
        return;
    }

    uint8_t specHash[8] = {0};
    uint8_t libHash[8] = {0};
    const uint16_t protocolVersion = (uint16_t)mavlinkGetProtocolVersion() * 100;

    mavlink_msg_protocol_version_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        protocolVersion,
        protocolVersion,
        protocolVersion,
        specHash,
        libHash);

    mavlinkSendMessage();
}

void mavlinkSendExtendedSysState(void)
{
    uint8_t vtolState = MAV_VTOL_STATE_UNDEFINED;
    uint8_t landedState;

    switch (NAV_Status.state) {
        case MW_NAV_STATE_LAND_START:
        case MW_NAV_STATE_LAND_IN_PROGRESS:
        case MW_NAV_STATE_LAND_SETTLE:
        case MW_NAV_STATE_LAND_START_DESCENT:
        case MW_NAV_STATE_EMERGENCY_LANDING:
            landedState = MAV_LANDED_STATE_LANDING;
            break;
        case MW_NAV_STATE_LANDED:
            landedState = MAV_LANDED_STATE_ON_GROUND;
            break;
        default:
            if (!ARMING_FLAG(ARMED) || STATE(LANDING_DETECTED)) {
                landedState = MAV_LANDED_STATE_ON_GROUND;
            } else {
                landedState = MAV_LANDED_STATE_IN_AIR;
            }
            break;
    }

    mavlink_msg_extended_sys_state_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        vtolState,
        landedState);

    mavlinkSendMessage();
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
        onboard_control_sensors_present,
        onboard_control_sensors_enabled,
        onboard_control_sensors_health,
        constrain(averageSystemLoadPercent * 10, 0, 1000),
        feature(FEATURE_VBAT) ? getBatteryVoltage() * 10 : 0,
        isAmperageConfigured() ? getAmperage() : -1,
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : 100,
        0,
        0,
        0,
        0,
        0,
        0, 0, 0, 0);

    mavlinkSendMessage();
}

void mavlinkSendRCChannelsAndRSSI(void)
{
#define GET_CHANNEL_VALUE(x) ((rxRuntimeConfig.channelCount >= (x + 1)) ? rxGetChannelValue(x) : 0)
    if (mavlinkGetProtocolVersion() == 1) {
        mavlink_msg_rc_channels_raw_pack(mavSystemId, mavComponentId, &mavSendMsg,
            millis(),
            0,
            GET_CHANNEL_VALUE(0),
            GET_CHANNEL_VALUE(1),
            GET_CHANNEL_VALUE(2),
            GET_CHANNEL_VALUE(3),
            GET_CHANNEL_VALUE(4),
            GET_CHANNEL_VALUE(5),
            GET_CHANNEL_VALUE(6),
            GET_CHANNEL_VALUE(7),
            scaleRange(getRSSI(), 0, 1023, 0, 254));
    }
    else {
        mavlink_msg_rc_channels_pack(mavSystemId, mavComponentId, &mavSendMsg,
            millis(),
            rxRuntimeConfig.channelCount,
            GET_CHANNEL_VALUE(0),
            GET_CHANNEL_VALUE(1),
            GET_CHANNEL_VALUE(2),
            GET_CHANNEL_VALUE(3),
            GET_CHANNEL_VALUE(4),
            GET_CHANNEL_VALUE(5),
            GET_CHANNEL_VALUE(6),
            GET_CHANNEL_VALUE(7),
            GET_CHANNEL_VALUE(8),
            GET_CHANNEL_VALUE(9),
            GET_CHANNEL_VALUE(10),
            GET_CHANNEL_VALUE(11),
            GET_CHANNEL_VALUE(12),
            GET_CHANNEL_VALUE(13),
            GET_CHANNEL_VALUE(14),
            GET_CHANNEL_VALUE(15),
            GET_CHANNEL_VALUE(16),
            GET_CHANNEL_VALUE(17),
            scaleRange(getRSSI(), 0, 1023, 0, 254));
    }
#undef GET_CHANNEL_VALUE

    mavlinkSendMessage();
}

#if defined(USE_GPS)
static void mavlinkSendHomePosition(void)
{
    float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

    mavlink_msg_home_position_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        GPS_home.lat,
        GPS_home.lon,
        GPS_home.alt * 10,
        0.0f,
        0.0f,
        0.0f,
        q,
        0.0f,
        0.0f,
        0.0f,
        ((uint64_t) millis()) * 1000);

    mavlinkSendMessage();
}

void mavlinkSendGpsRawInt(timeUs_t currentTimeUs)
{
    uint8_t gpsFixType = 0;
    rtcTime_t rtcTime;
    uint64_t timeUnixUsec = currentTimeUs;

    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        )) {
        return;
    }

    if (gpsSol.fixType == GPS_NO_FIX) {
        gpsFixType = 1;
    } else if (gpsSol.fixType == GPS_FIX_2D) {
        gpsFixType = 2;
    } else if (gpsSol.fixType == GPS_FIX_3D) {
        gpsFixType = 3;
    }

    if (rtcGet(&rtcTime)) {
        timeUnixUsec = (uint64_t)rtcTime * 1000ULL;
    }

    mavlink_msg_gps_raw_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        timeUnixUsec,
        gpsFixType,
        gpsSol.llh.lat,
        gpsSol.llh.lon,
        gpsSol.llh.alt * 10,
        gpsSol.eph,
        gpsSol.epv,
        gpsSol.groundSpeed,
        gpsSol.groundCourse * 10,
        gpsSol.numSat,
        0,
        gpsSol.eph * 10,
        gpsSol.epv * 10,
        0,
        0,
        0);

    mavlinkSendMessage();
}

void mavlinkSendGlobalPositionInt(timeUs_t currentTimeUs)
{
    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        )) {
        return;
    }

    mavlink_msg_global_position_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        currentTimeUs / 1000,
        gpsSol.llh.lat,
        gpsSol.llh.lon,
        gpsSol.llh.alt * 10,
        getEstimatedActualPosition(Z) * 10,
        getEstimatedActualVelocity(X),
        getEstimatedActualVelocity(Y),
        -getEstimatedActualVelocity(Z),
        DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw)
    );

    mavlinkSendMessage();
}

void mavlinkSendGpsGlobalOrigin(void)
{
    mavlink_msg_gps_global_origin_pack(mavSystemId, mavComponentId, &mavSendMsg,
        GPS_home.lat,
        GPS_home.lon,
        GPS_home.alt * 10,
        ((uint64_t) millis()) * 1000);

    mavlinkSendMessage();
}

void mavlinkSendPosition(timeUs_t currentTimeUs)
{
    mavlinkSendGpsRawInt(currentTimeUs);
    mavlinkSendGlobalPositionInt(currentTimeUs);
    mavlinkSendGpsGlobalOrigin();
}
#endif

void mavlinkSendAttitude(void)
{
    mavlink_msg_attitude_pack(mavSystemId, mavComponentId, &mavSendMsg,
        millis(),
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.roll)),
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(-attitude.values.pitch)),
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.yaw)),
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_ROLL]),
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_PITCH]),
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_YAW]));

    mavlinkSendMessage();
}

void mavlinkSendVfrHud(void)
{
    float mavAltitude = 0;
    float mavGroundSpeed = 0;
    float mavAirSpeed = 0;
    float mavClimbRate = 0;

#if defined(USE_GPS)
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

    mavAltitude = getEstimatedActualPosition(Z) / 100.0f;
    mavClimbRate = getEstimatedActualVelocity(Z) / 100.0f;

    int16_t thr = getThrottlePercent(osdUsingScaledThrottle());
    mavlink_msg_vfr_hud_pack(mavSystemId, mavComponentId, &mavSendMsg,
        mavAirSpeed,
        mavGroundSpeed,
        DECIDEGREES_TO_DEGREES(attitude.values.yaw),
        thr,
        mavAltitude,
        mavClimbRate);

    mavlinkSendMessage();
}

void mavlinkSendHeartbeat(void)
{
    uint8_t mavModes = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

    const bool isPlane = mavlinkIsFixedWingVehicle();
    const mavlinkModeSelection_t modeSelection = mavlinkSelectMode();
    flightModeForTelemetry_e flm = modeSelection.flightMode;
    uint8_t mavCustomMode = modeSelection.customMode;
    uint8_t mavSystemType;
    if (isPlane) {
        mavSystemType = MAV_TYPE_FIXED_WING;
    }
    else {
        mavSystemType = mavlinkGetVehicleType();
    }

    const bool manualInputAllowed = !(flm == FLM_MISSION || flm == FLM_RTH || flm == FLM_FAILSAFE);
    if (manualInputAllowed) {
        mavModes |= MAV_MODE_FLAG_MANUAL_INPUT_ENABLED;
    }
    if (flm == FLM_POSITION_HOLD && isGCSValid()) {
        mavModes |= MAV_MODE_FLAG_GUIDED_ENABLED;
    }
    else if (flm == FLM_MISSION || flm == FLM_RTH ) {
        mavModes |= MAV_MODE_FLAG_AUTO_ENABLED;
    }
    else if (flm != FLM_MANUAL && flm != FLM_ACRO && flm != FLM_ACRO_AIR) {
        mavModes |= MAV_MODE_FLAG_STABILIZE_ENABLED;
    }

    if (ARMING_FLAG(ARMED)) {
        mavModes |= MAV_MODE_FLAG_SAFETY_ARMED;
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
        mavSystemType,
        mavlinkGetAutopilotEnum(),
        mavModes,
        mavCustomMode,
        mavSystemState);

    mavlinkSendMessage();
}

void mavlinkSendBatteryStatus(void)
{
    uint16_t batteryVoltages[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN];
    uint16_t batteryVoltagesExt[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN];
    memset(batteryVoltages, UINT16_MAX, sizeof(batteryVoltages));
    memset(batteryVoltagesExt, 0, sizeof(batteryVoltagesExt));
    if (feature(FEATURE_VBAT)) {
        uint8_t batteryCellCount = getBatteryCellCount();
        if (batteryCellCount > 0) {
            for (int cell = 0; cell < batteryCellCount && cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN + MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN; cell++) {
                if (cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN) {
                    batteryVoltages[cell] = getBatteryAverageCellVoltage() * 10;
                } else {
                    batteryVoltagesExt[cell - MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN] = getBatteryAverageCellVoltage() * 10;
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
        0,
        MAV_BATTERY_FUNCTION_UNKNOWN,
        MAV_BATTERY_TYPE_UNKNOWN,
        INT16_MAX,
        batteryVoltages,
        isAmperageConfigured() ? getAmperage() : -1,
        isAmperageConfigured() ? getMAhDrawn() : -1,
        isAmperageConfigured() ? getMWhDrawn() * 36 : -1,
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : -1,
        0,
        0,
        batteryVoltagesExt,
        0,
        0);

    mavlinkSendMessage();
}

void mavlinkSendScaledPressure(void)
{
    int16_t temperature;
    sensors(SENSOR_BARO) ? getBaroTemperature(&temperature) : getIMUTemperature(&temperature);
    mavlink_msg_scaled_pressure_pack(mavSystemId, mavComponentId, &mavSendMsg,
        millis(),
        0,
        0,
        temperature * 10,
        0);

    mavlinkSendMessage();
}

void mavlinkSendSystemTime(void)
{
    uint64_t timeUnixUsec = 0;
    rtcTime_t rtcTime;

    if (rtcGet(&rtcTime)) {
        timeUnixUsec = (uint64_t)rtcTime * 1000ULL;
    }

    mavlink_msg_system_time_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        timeUnixUsec,
        millis());

    mavlinkSendMessage();
}

bool mavlinkSendStatusText(void)
{
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
        return true;
    }
#endif
    return false;
}

void mavlinkSendBatteryTemperatureStatusText(void)
{
    mavlinkSendBatteryStatus();
    mavlinkSendScaledPressure();
    mavlinkSendStatusText();
}

static uint8_t mavlinkHeadingDeg2FromCd(int32_t headingCd)
{
    return (uint8_t)(wrap_36000(headingCd) / 200);
}

static uint16_t mavlinkComputeHighLatencyFailures(void)
{
    uint16_t flags = 0;

#if defined(USE_GPS)
    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        ) || gpsSol.fixType == GPS_NO_FIX) {
        flags |= HL_FAILURE_FLAG_GPS;
    }
#endif

#ifdef USE_PITOT
    if (getHwPitotmeterStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_DIFFERENTIAL_PRESSURE;
    }
#endif

    if (getHwBarometerStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_ABSOLUTE_PRESSURE;
    }

    if (getHwAccelerometerStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_ACCEL;
    }

    if (getHwGyroStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_GYRO;
    }

    if (getHwCompassStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_MAG;
    }

    if (getHwRangefinderStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_TERRAIN;
    }

    const batteryState_e batteryState = getBatteryState();
    if (batteryState == BATTERY_WARNING || batteryState == BATTERY_CRITICAL) {
        flags |= HL_FAILURE_FLAG_BATTERY;
    }

    if (!rxIsReceivingSignal() || !rxAreFlightChannelsValid()) {
        flags |= HL_FAILURE_FLAG_RC_RECEIVER;
    }

    return flags;
}

void mavlinkSendHighLatency2(timeUs_t currentTimeUs)
{
    const mavlinkModeSelection_t modeSelection = mavlinkSelectMode();

    int32_t latitude = 0;
    int32_t longitude = 0;
    int16_t altitude = (int16_t)constrain((int32_t)(getEstimatedActualPosition(Z) / 100), INT16_MIN, INT16_MAX);
    int16_t targetAltitude = (int16_t)constrain((int32_t)(posControl.desiredState.pos.z / 100), INT16_MIN, INT16_MAX);
    uint16_t targetDistance = 0;
    uint16_t wpNum = 0;
    uint8_t heading = mavlinkHeadingDeg2FromCd(DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw));
    uint8_t targetHeading = mavlinkHeadingDeg2FromCd(posControl.desiredState.yaw);
    uint8_t groundspeed = 0;
    uint8_t airspeed = 0;
    uint8_t airspeedSp = 0;
    uint8_t windspeed = 0;
    uint8_t windHeading = 0;
    uint8_t eph = UINT8_MAX;
    uint8_t epv = UINT8_MAX;
    int8_t temperatureAir = 0;
    int8_t climbRate = (int8_t)constrain(lrintf(getEstimatedActualVelocity(Z) / 10.0f), INT8_MIN, INT8_MAX);
    int8_t battery = feature(FEATURE_VBAT) ? (int8_t)calculateBatteryPercentage() : -1;

#if defined(USE_GPS)
    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        latitude = gpsSol.llh.lat;
        longitude = gpsSol.llh.lon;
        altitude = (int16_t)constrain((int32_t)(gpsSol.llh.alt / 100), INT16_MIN, INT16_MAX);

        const int32_t desiredAltCm = lrintf(posControl.desiredState.pos.z);
        const int32_t targetAltCm = GPS_home.alt + desiredAltCm;
        targetAltitude = (int16_t)constrain(targetAltCm / 100, INT16_MIN, INT16_MAX);

        groundspeed = (uint8_t)constrain(lrintf(gpsSol.groundSpeed / 20.0f), 0, UINT8_MAX);

        if (gpsSol.flags.validEPE) {
            eph = (uint8_t)constrain((gpsSol.eph + 5) / 10, 0, UINT8_MAX);
            epv = (uint8_t)constrain((gpsSol.epv + 5) / 10, 0, UINT8_MAX);
        }

        if (posControl.activeWaypointIndex >= 0) {
            wpNum = (uint16_t)posControl.activeWaypointIndex;
            targetDistance = (uint16_t)constrain(lrintf(posControl.wpDistance / 1000.0f), 0, UINT16_MAX);
        }
    }
#endif

#if defined(USE_PITOT)
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        airspeed = (uint8_t)constrain(lrintf(getAirspeedEstimate() / 20.0f), 0, UINT8_MAX);
        airspeedSp = airspeed;
    }
#endif

    if (airspeedSp == 0) {
        const float desiredVelXY = calc_length_pythagorean_2D(posControl.desiredState.vel.x, posControl.desiredState.vel.y);
        airspeedSp = (uint8_t)constrain(lrintf(desiredVelXY / 20.0f), 0, UINT8_MAX);
    }

    if (airspeed == 0) {
        airspeed = groundspeed;
    }

#ifdef USE_WIND_ESTIMATOR
    if (isEstimatedWindSpeedValid()) {
        uint16_t windAngleCd = 0;
        const float windHoriz = getEstimatedHorizontalWindSpeed(&windAngleCd);
        windspeed = (uint8_t)constrain(lrintf(windHoriz / 20.0f), 0, UINT8_MAX);
        windHeading = mavlinkHeadingDeg2FromCd(windAngleCd);
    }
#endif

    int16_t temperature;
    sensors(SENSOR_BARO) ? getBaroTemperature(&temperature) : getIMUTemperature(&temperature);
    temperatureAir = (int8_t)constrain(temperature, INT8_MIN, INT8_MAX);

    const uint16_t failureFlags = mavlinkComputeHighLatencyFailures();

    mavlink_msg_high_latency2_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        (uint32_t)(currentTimeUs / 1000),
        mavlinkGetVehicleType(),
        mavlinkGetAutopilotEnum(),
        modeSelection.customMode,
        latitude,
        longitude,
        altitude,
        targetAltitude,
        heading,
        targetHeading,
        targetDistance,
        (uint8_t)constrain(getThrottlePercent(osdUsingScaledThrottle()), 0, 100),
        airspeed,
        airspeedSp,
        groundspeed,
        windspeed,
        windHeading,
        eph,
        epv,
        temperatureAir,
        climbRate,
        battery,
        wpNum,
        failureFlags,
        0,
        0,
        0);

    mavlinkSendMessage();
}

bool mavlinkSendRequestedMessage(uint16_t messageId)
{
    switch (messageId) {
        case MAVLINK_MSG_ID_HEARTBEAT:
            mavlinkSendHeartbeat();
            return true;
        case MAVLINK_MSG_ID_AUTOPILOT_VERSION:
            if (mavlinkGetProtocolVersion() != 1) {
                mavlinkSendAutopilotVersion();
                return true;
            }
            return false;
        case MAVLINK_MSG_ID_PROTOCOL_VERSION:
            if (mavlinkGetProtocolVersion() != 1) {
                mavlinkSendProtocolVersion();
                return true;
            }
            return false;
        case MAVLINK_MSG_ID_SYS_STATUS:
            mavlinkSendSystemStatus();
            return true;
        case MAVLINK_MSG_ID_ATTITUDE:
            mavlinkSendAttitude();
            return true;
        case MAVLINK_MSG_ID_VFR_HUD:
            mavlinkSendVfrHud();
            return true;
        case MAVLINK_MSG_ID_AVAILABLE_MODES:
            mavlinkSendAvailableModesForCurrentMode();
            return true;
        case MAVLINK_MSG_ID_CURRENT_MODE:
            {
                const mavlinkModeSelection_t modeSelection = mavlinkSelectMode();
                mavlinkSendCurrentMode(&modeSelection);
                return true;
            }
        case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
            mavlinkSendExtendedSysState();
            return true;
        case MAVLINK_MSG_ID_RC_CHANNELS:
        case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
        case MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
            mavlinkSendRCChannelsAndRSSI();
            return true;
#ifdef USE_GPS
        case MAVLINK_MSG_ID_GPS_RAW_INT:
            mavlinkSendGpsRawInt(micros());
            return true;
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            mavlinkSendGlobalPositionInt(micros());
            return true;
        case MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN:
            mavlinkSendGpsGlobalOrigin();
            return true;
        case MAVLINK_MSG_ID_HOME_POSITION:
            if (STATE(GPS_FIX_HOME)) {
                mavlinkSendHomePosition();
                return true;
            }
            return false;
#endif
        case MAVLINK_MSG_ID_BATTERY_STATUS:
            mavlinkSendBatteryStatus();
            return true;
        case MAVLINK_MSG_ID_SCALED_PRESSURE:
            mavlinkSendScaledPressure();
            return true;
        case MAVLINK_MSG_ID_SYSTEM_TIME:
            mavlinkSendSystemTime();
            return true;
        case MAVLINK_MSG_ID_STATUSTEXT:
            return mavlinkSendStatusText();
        default:
            return false;
    }
}

bool mavlinkHandleIncomingHeartbeat(void)
{
    mavlink_heartbeat_t msg;
    mavlink_msg_heartbeat_decode(&mavlinkContext.recvMsg, &msg);

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

static bool mavlinkIsLocalTarget(uint8_t targetSystem, uint8_t targetComponent)
{
    if (targetSystem != 0 && targetSystem != mavSystemId) {
        return false;
    }

    if (targetComponent != 0 && targetComponent != mavComponentId) {
        return false;
    }

    return true;
}

bool mavlinkHandleIncomingRequestDataStream(void)
{
    mavlink_request_data_stream_t msg;
    mavlink_msg_request_data_stream_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkIsLocalTarget(msg.target_system, msg.target_component)) {
        return false;
    }

    uint8_t rate = 0;
    if (msg.start_stop != 0) {
        rate = (uint8_t)msg.req_message_rate;
        if (rate > TELEMETRY_MAVLINK_MAXRATE) {
            rate = TELEMETRY_MAVLINK_MAXRATE;
        }
    }

    if (msg.req_stream_id == MAV_DATA_STREAM_ALL) {
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTENDED_STATUS, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_RC_CHANNELS, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_POSITION, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA1, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA2, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA3, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTENDED_SYS_STATE, rate);
        return true;
    }

    mavlinkSetStreamRate(msg.req_stream_id, rate);
    return true;
}

void processMAVLinkTelemetry(timeUs_t currentTimeUs)
{
    if (mavActivePort->highLatencyEnabled && mavlinkGetProtocolVersion() != 1) {
        if ((currentTimeUs - mavActivePort->lastHighLatencyMessageUs) >= TELEMETRY_MAVLINK_HIGH_LATENCY_INTERVAL_US) {
            mavlinkSendHighLatency2(currentTimeUs);
            mavActivePort->lastHighLatencyMessageUs = currentTimeUs;
        }
        return;
    }

    // is executed @ TELEMETRY_MAVLINK_MAXRATE rate
    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_SYS_STATUS, currentTimeUs)) {
        mavlinkSendSystemStatus();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_RC_CHANNELS, currentTimeUs)) {
        mavlinkSendRCChannelsAndRSSI();
    }

#ifdef USE_GPS
    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_GPS_RAW_INT, currentTimeUs)) {
        mavlinkSendGpsRawInt(currentTimeUs);
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_GLOBAL_POSITION_INT, currentTimeUs)) {
        mavlinkSendGlobalPositionInt(currentTimeUs);
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_GPS_GLOBAL_ORIGIN, currentTimeUs)) {
        mavlinkSendGpsGlobalOrigin();
    }
#endif

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_ATTITUDE, currentTimeUs)) {
        mavlinkSendAttitude();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_VFR_HUD, currentTimeUs)) {
        mavlinkSendVfrHud();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_HEARTBEAT, currentTimeUs)) {
        mavlinkSendHeartbeat();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_EXTENDED_SYS_STATE, currentTimeUs)) {
        mavlinkSendExtendedSysState();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_BATTERY_STATUS, currentTimeUs)) {
        mavlinkSendBatteryStatus();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_SCALED_PRESSURE, currentTimeUs)) {
        mavlinkSendScaledPressure();
    }

    if (mavlinkMessageTrigger(MAVLINK_PERIODIC_MESSAGE_SYSTEM_TIME, currentTimeUs)) {
        mavlinkSendSystemTime();
    }

    if (mavlinkStreamTrigger(MAV_DATA_STREAM_EXTRA3, currentTimeUs)) {
        mavlinkSendStatusText();
    }
}

#endif
