#pragma once

#include "common/time.h"

#include "telemetry/mavlink.h"

typedef enum {
    MAVLINK_FC_DISPATCH_NOT_HANDLED = 0,
    MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY,
    MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY,
} mavlinkFcDispatchResult_e;

void mavlinkSendSystemStatus(void);
void mavlinkSendRCChannelsAndRSSI(void);
void mavlinkSendPosition(timeUs_t currentTimeUs);
void mavlinkSendGpsRawInt(timeUs_t currentTimeUs);
void mavlinkSendGlobalPositionInt(timeUs_t currentTimeUs);
void mavlinkSendGpsGlobalOrigin(void);
void mavlinkSendAttitude(void);
void mavlinkSendVfrHud(void);
void mavlinkSendHeartbeat(void);
void mavlinkSendBatteryTemperatureStatusText(void);
void mavlinkSendExtendedSysState(void);
void mavlinkSendBatteryStatus(void);
void mavlinkSendScaledPressure(void);
void mavlinkSendSystemTime(void);
bool mavlinkSendStatusText(void);
void mavlinkSendHighLatency2(timeUs_t currentTimeUs);
mavlinkFcDispatchResult_e mavlinkFcDispatchIncomingMessage(uint8_t ingressPortIndex);
