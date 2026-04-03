#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

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
