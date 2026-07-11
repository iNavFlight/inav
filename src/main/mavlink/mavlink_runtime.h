#pragma once

#include "common/time.h"

#include "mavlink/mavlink_types.h"
#include "telemetry/telemetry.h"

void mavlinkRuntimeInit(void);
void mavlinkRuntimeHandle(timeUs_t currentTimeUs);
void mavlinkRuntimeCheckState(void);
void mavlinkRuntimeFreePorts(void);

const mavlinkTelemetryPortConfig_t *mavlinkGetPortConfig(uint8_t portIndex);
const mavlinkTelemetryCommonConfig_t *mavlinkGetCommonConfig(void);
uint8_t mavlinkGetProtocolVersion(void);
const mavlinkMlrsPortRuntime_t *mavlinkGetPortMlrsRuntime(uint8_t portIndex);
const mavlinkMlrsPortRuntime_t *mavlinkGetActiveMlrsRuntime(void);
bool mavlinkPortTxBufferIsValid(uint8_t portIndex);
uint8_t mavlinkPortTxBufferFree(uint8_t portIndex);
void mavlinkSetActivePortContext(uint8_t portIndex);
void mavlinkSendMessage(void);
