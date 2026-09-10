#pragma once

void configureMAVLinkTelemetryPort(uint8_t portIndex);
void freeMAVLinkTelemetryPortByIndex(uint8_t portIndex);
#ifdef USE_MAVLINK_MSP_TUNNEL
void mavlinkResetTunnelPortState(uint8_t portIndex);
#endif
