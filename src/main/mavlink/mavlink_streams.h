#pragma once

#include "common/time.h"

#include "mavlink/mavlink_types.h"

extern const uint8_t mavSecondaryRates[MAVLINK_STREAM_COUNT];

void mavlinkSendAutopilotVersion(void);
void mavlinkSendProtocolVersion(void);
bool mavlinkSendRequestedMessage(uint16_t messageId);
uint8_t mavlinkClampStreamRate(uint8_t rate);
int32_t mavlinkRateToIntervalUs(uint8_t rate);
bool mavlinkPeriodicMessageFromMessageId(uint16_t messageId, mavlinkPeriodicMessage_e *periodicMessage);
uint8_t mavlinkPeriodicMessageBaseStream(mavlinkPeriodicMessage_e periodicMessage);
int32_t mavlinkMessageBaseIntervalUs(mavlinkPeriodicMessage_e periodicMessage);
int32_t mavlinkMessageIntervalUs(mavlinkPeriodicMessage_e periodicMessage);
void mavlinkSetMessageOverrideIntervalUs(mavlinkPeriodicMessage_e periodicMessage, int32_t intervalUs);
int mavlinkStreamTrigger(enum MAV_DATA_STREAM streamNum, timeUs_t currentTimeUs);
void mavlinkSetStreamRate(uint8_t streamNum, uint8_t rate);
int mavlinkMessageTrigger(mavlinkPeriodicMessage_e periodicMessage, timeUs_t currentTimeUs);
void configureMAVLinkStreamRates(uint8_t portIndex);
void processMAVLinkTelemetry(timeUs_t currentTimeUs);
bool mavlinkHandleIncomingHeartbeat(void);
bool mavlinkHandleIncomingRequestDataStream(void);
